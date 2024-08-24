/*
 * Synchronous rbd image backing store routine
 *
 * modified from bs_rdrw.c:
 * Copyright (C) 2006-2007 FUJITA Tomonori <tomof@acm.org>
 * Copyright (C) 2006-2007 Mike Christie <michaelc@cs.wisc.edu>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 2 of the
 * License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */
#define _XOPEN_SOURCE 600

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <linux/fs.h>
#include <sys/epoll.h>

#include "list.h"
#include "util.h"
#include "tgtd.h"
#include "scsi.h"
#include "spc.h"
#include "bs_thread.h"

#include "rados/librados.h"
#include "rbd/librbd.h"

struct bs_list{
	struct list_head list;
	pthread_mutex_t lock;
};

struct active_cluster {
	struct list_head list;
	rados_t cluster;
	int refnum;
	char cluster_name[64];
};

struct active_rbd {
	char *poolname;
	char *imagename;
	char *snapname;
/* modify begin by hy, 2020-09-21, BugId:123 原因: 修改 tgt中集群信息将其句柄放在集群列表中 */
	//rados_t cluster;
	char *cluster;
/* modify end by hy, 2020-09-21 */
	rados_ioctx_t ioctx;
	rbd_image_t rbd_image;
};

/* active_rbd is allocated just after the bs_thread_info */
#define RBDP(lu)	((struct active_rbd *) \
				((char *)lu + \
				sizeof(struct scsi_lu) + \
				sizeof(struct bs_thread_info)) \
			)

/* modify begin by hy, 2020-09-21, BugId:123 原因: 将设备的线程移动到统一调度处理 */
struct bs_thread_info bs_info;
/* modify end by hy, 2020-09-21 */
/* modify begin by hy, 2020-09-21, BugId:123 原因: */
struct bs_list cluster_list;
/* modify end by hy, 2020-09-21 */

static void parse_imagepath(char *path, char **pool, char **image, char **snap)
{
	char *origp = strdup(path);
	char *p, *sep;

	p = origp;
	sep = strchr(p, '/');
	if (sep == NULL) {
		*pool = "rbd";
	} else {
		*sep = '\0';
		*pool = strdup(p);
		p = sep + 1;
	}
	/* p points to image[@snap] */
	sep = strchr(p, '@');
	if (sep == NULL) {
		*snap = "";
	} else {
		*snap = strdup(sep + 1);
		*sep = '\0';
	}
	/* p points to image\0 */
	*image = strdup(p);
	free(origp);
}

static void set_medium_error(int *result, uint8_t *key, uint16_t *asc)
{
	*result = SAM_STAT_CHECK_CONDITION;
	*key = MEDIUM_ERROR;
	*asc = ASC_READ_ERROR;
}

static int is_opt(const char *opt, char *p)
{
	int ret = 0;
	if ((strncmp(p, opt, strlen(opt)) == 0) &&
	    (p[strlen(opt)] == '=')) {
		ret = 1;
	}
	return ret;
}

// Slurp up and return a copy of everything to the next ';', and update p
static char *slurp_to_semi(char **p)
{
	char *end = index(*p, ';');
	char *ret = NULL;
	int len;

	if (end == NULL)
		end = *p + strlen(*p);
	len = end - *p;
	ret = malloc(len + 1);
	strncpy(ret, *p, len);
	ret[len] = '\0';
	*p = end;
	/* Jump past the semicolon, if we stopped at one */
	if (**p == ';')
		*p = end + 1;
	return ret;
}

static char *slurp_value(char **p)
{
	char *equal = index(*p, '=');
	if (equal) {
		*p = equal + 1;
		return slurp_to_semi(p);
	} else {
		// uh...no?
		return NULL;
	}
}

#if 0
static struct active_cluster * lookup_cluster(char * cluster_name)
{
	struct active_cluster *cluster = NULL;
	struct active_cluster *pos = NULL;

	list_for_each_entry(pos, &cluster_list.list, list) {
		if (strcmp(pos->cluster_name,cluster_name)==0) {
			cluster = pos;
			break;
		}
	}

	return cluster;
}
#endif

static struct active_cluster * get_cluster(char *bsopts)
{
	int rados_ret = 0;
	char *cluster_name = NULL;
	char *confname = NULL;
	char *clientid = NULL;
	char *opt_cluster = NULL;
	char *opt_conf = NULL;
	char *opt_id = NULL;
#if 0
	char *virsecretuuid = NULL;
	char *ignore = NULL;
	char *given_cephx_key = NULL;
	char disc_cephx_key[256];
#endif
	struct active_cluster *cluster = NULL;
	struct active_cluster *pos = NULL;
	struct active_cluster *next = NULL;

	// look for conf= or id= or cluster=
	while (bsopts && strlen(bsopts)) {
		if (is_opt("conf", bsopts))
			opt_conf = slurp_value(&bsopts);
		else if (is_opt("id", bsopts))
			opt_id = slurp_value(&bsopts);
		else if (is_opt("cluster", bsopts))
			opt_cluster = slurp_value(&bsopts);
#if 0
		else if (is_opt("virsecretuuid", bsopts))
			virsecretuuid = slurp_value(&bsopts);
		else if (is_opt("cephx_key", bsopts))
			given_cephx_key = slurp_value(&bsopts);
		else {
			ignore = slurp_to_semi(&bsopts);
			eprintf("bs_rbd: ignoring unknown option \"%s\"\n",
				ignore);
			free(ignore);
			break;
		}
#endif
	}

	eprintf("bs_rbd_init bsopts=%s\n", bsopts);

	if (!opt_cluster) {
		cluster_name = "ceph";
	} else {
		cluster_name = opt_cluster;
	}

	if (!confname) {
		confname = "/etc/ceph/ceph.conf";
	} else {
		confname = opt_conf;
	}

	if (!clientid) {
		clientid = "admin";
	} else {
		clientid = opt_id;
	}

	list_for_each_entry_safe(pos, next, &cluster_list.list, list) {
		if (strcmp(pos->cluster_name,cluster_name)==0) {
			cluster = pos;
			goto l_out;
		}
	}

	cluster = malloc(sizeof(struct active_cluster));
	if (!cluster) {
		rados_ret  = -ENOMEM;
		goto l_out;
	}

#if 0
	/* virsecretuuid && given_cephx_key are conflicting options. */
	if (virsecretuuid && given_cephx_key) {
		eprintf("Conflicting options virsecretuuid=[%s] cephx_key=[%s]",
			virsecretuuid, given_cephx_key);
		goto l_free_cluster;
	}

	/* Get stored key from secret uuid. */
	if (virsecretuuid) {
		char libvir_uuid_file_path_buf[256] = "/etc/libvirt/secrets/";
		strcat(libvir_uuid_file_path_buf, virsecretuuid);
		strcat(libvir_uuid_file_path_buf, ".base64");

		FILE *fp;
		fp = fopen(libvir_uuid_file_path_buf , "r");
		if (fp == NULL) {
			eprintf("bs_rbd_init: Unable to read %s\n",
				libvir_uuid_file_path_buf);
			goto l_free_cluster;
		}
		if (fgets(disc_cephx_key, 256, fp) == NULL) {
			eprintf("bs_rbd_init: Unable to read %s\n",
				libvir_uuid_file_path_buf);
			goto l_free_cluster;
		}
		fclose(fp);
		strtok(disc_cephx_key, "\n");
	}
#endif

	rados_ret = rados_create(&(cluster->cluster), clientid);
	if (rados_ret < 0) {
		eprintf("get_cluster: rados_create: %d\n", rados_ret);
		goto l_free_cluster;
	}

	rados_ret = rados_conf_parse_env(cluster->cluster, NULL);
	if (rados_ret < 0) {
		eprintf("get_cluster: rados_conf_parse_env: %d\n", rados_ret);
		goto l_free_cluster;
	}

	rados_ret = rados_conf_read_file(cluster->cluster, confname);
	if (rados_ret < 0) {
		eprintf("get_cluster: rados_conf_read_file: %d\n", rados_ret);
		goto l_free_cluster;
	}

#if 0
	/* Set given key */
	if (virsecretuuid) {
		if (rados_conf_set(cluster->cluster, "key", disc_cephx_key) < 0) {
			eprintf("bs_rbd_init: failed to set cephx_key: %s\n",
				disc_cephx_key);
			goto l_free_cluster;
		}
	}

	if (given_cephx_key) {
		if (rados_conf_set(cluster->cluster, "key", given_cephx_key) < 0) {
			eprintf("bs_rbd_init: failed to set cephx_key: %s\n",
				given_cephx_key);
			goto l_free_cluster;
		}
	}
#endif

	INIT_LIST_HEAD(&cluster->list);

/** comment by hy 2020-09-21
 * # 前面已经拷贝了
 */
	strncpy(cluster->cluster_name, cluster_name,
		sizeof(cluster->cluster_name));

#if 0
	if (virsecretuuid)
		free(virsecretuuid);
	if (given_cephx_key)
		free(given_cephx_key);
#endif
	if (opt_id)
		free(opt_id);
	if (opt_conf)
		free(opt_conf);
	if (opt_cluster)
		free(opt_cluster);

	rados_ret = rados_connect(cluster->cluster);
	if (rados_ret < 0) {
		eprintf("get_cluster: rados_connect: %d\n", rados_ret);
		goto l_free_cluster;
	}

	cluster->refnum++;
	list_add_tail(&cluster->list, &cluster_list.list);

l_out:
	return cluster;

l_free_cluster:
	if (opt_cluster)
		free(opt_cluster);
	if (opt_conf)
		free(opt_conf);
	if (opt_id)
		free(opt_id);
#if 0
	if (virsecretuuid)
		free(virsecretuuid);
	if (given_cephx_key)
		free(given_cephx_key);
#endif
	if (cluster) {
		free(cluster);
		cluster = NULL;
	}
	goto l_out;
}

static void put_cluster(char * cluster_name)
{
	int ret = 0;
	struct active_cluster *cluster = NULL;
	struct active_cluster *pos = NULL;
	struct active_cluster *next = NULL;

	list_for_each_entry_safe(pos, next, &cluster_list.list, list) {
		if (strcmp(pos->cluster_name,cluster_name)==0) {
			list_del(&cluster->list);
			cluster = pos;
			break;
		}
	}
	if (!cluster) {
		ret = -ENOENT;
		eprintf("put_cluster: lookup_cluster: %d\n", ret);
		goto l_out;
	}

	cluster->refnum--;

	if (cluster->refnum == 0) {
		rados_shutdown(cluster->cluster);
		if (!cluster->cluster_name)
			free(cluster->cluster_name);
	}

l_out:
	return;
}

static void bs_sync_sync_range(struct scsi_cmd *cmd, uint32_t length,
			       int *result, uint8_t *key, uint16_t *asc)
{
	int ret;

	ret = rbd_flush(RBDP(cmd->dev)->rbd_image);
	if (ret)
		set_medium_error(result, key, asc);
}

static void bs_rbd_request(struct scsi_cmd *cmd)
{
	int ret;
	uint32_t length;
	int result = SAM_STAT_GOOD;
	uint8_t key;
	uint16_t asc;
#if 0
	/*
	 * This should go in the sense data on error for COMPARE_AND_WRITE, but
	 * there doesn't seem to be any attempt to do so...
	 */

	uint32_t info = 0;
#endif
	char *tmpbuf;
	size_t blocksize;
	uint64_t offset = cmd->offset;
	uint32_t tl     = cmd->tl;
	int do_verify = 0;
	int i;
	char *ptr;
	const char *write_buf = NULL;
	ret = length = 0;
	key = asc = 0;
	struct active_rbd *rbd = RBDP(cmd->dev);

	switch (cmd->scb[0]) {
/** comment by hy 2020-09-21
 * # 16 8个byte 大容量的写
 */
	case ORWRITE_16:
		length = scsi_get_out_length(cmd);

		tmpbuf = malloc(length);
		if (!tmpbuf) {
			result = SAM_STAT_CHECK_CONDITION;
			key = HARDWARE_ERROR;
			asc = ASC_INTERNAL_TGT_FAILURE;
			break;
		}

		ret = rbd_read(rbd->rbd_image, offset, length, tmpbuf);

		if (ret != length) {
			set_medium_error(&result, &key, &asc);
			free(tmpbuf);
			break;
		}

		ptr = scsi_get_out_buffer(cmd);
		for (i = 0; i < length; i++)
			ptr[i] |= tmpbuf[i];

		free(tmpbuf);

		write_buf = scsi_get_out_buffer(cmd);
		goto write;
	case COMPARE_AND_WRITE:
		/* Blocks are transferred twice, first the set that
		 * we compare to the existing data, and second the set
		 * to write if the compare was successful.
		 */
		length = scsi_get_out_length(cmd) / 2;
		if (length != cmd->tl) {
			result = SAM_STAT_CHECK_CONDITION;
			key = ILLEGAL_REQUEST;
			asc = ASC_INVALID_FIELD_IN_CDB;
			break;
		}

		tmpbuf = malloc(length);
		if (!tmpbuf) {
			result = SAM_STAT_CHECK_CONDITION;
			key = HARDWARE_ERROR;
			asc = ASC_INTERNAL_TGT_FAILURE;
			break;
		}

		ret = rbd_read(rbd->rbd_image, offset, length, tmpbuf);

		if (ret != length) {
			set_medium_error(&result, &key, &asc);
			free(tmpbuf);
			break;
		}

		if (memcmp(scsi_get_out_buffer(cmd), tmpbuf, length)) {
			uint32_t pos = 0;
			char *spos = scsi_get_out_buffer(cmd);
			char *dpos = tmpbuf;

			/*
			 * Data differed, this is assumed to be 'rare'
			 * so use a much more expensive byte-by-byte
			 * comparasion to find out at which offset the
			 * data differs.
			 */
			for (pos = 0; pos < length && *spos++ == *dpos++;
			     pos++)
				;
#if 0
			/* See comment above at declaration */
			info = pos;
#endif
			result = SAM_STAT_CHECK_CONDITION;
			key = MISCOMPARE;
			asc = ASC_MISCOMPARE_DURING_VERIFY_OPERATION;
			free(tmpbuf);
			break;
		}

		/* no DPO bit (cache retention advice) support */
		free(tmpbuf);

		write_buf = scsi_get_out_buffer(cmd) + length;
		goto write;
	case SYNCHRONIZE_CACHE:
	case SYNCHRONIZE_CACHE_16:
		/* TODO */
		length = (cmd->scb[0] == SYNCHRONIZE_CACHE) ? 0 : 0;

		if (cmd->scb[1] & 0x2) {
			result = SAM_STAT_CHECK_CONDITION;
			key = ILLEGAL_REQUEST;
			asc = ASC_INVALID_FIELD_IN_CDB;
		} else
			bs_sync_sync_range(cmd, length, &result, &key, &asc);
		break;
	case WRITE_VERIFY:
	case WRITE_VERIFY_12:
	case WRITE_VERIFY_16:
		do_verify = 1;
	case WRITE_6:
	case WRITE_10:
	case WRITE_12:
	case WRITE_16:
		length = scsi_get_out_length(cmd);
		write_buf = scsi_get_out_buffer(cmd);
write:
		ret = rbd_write(rbd->rbd_image, offset, length, write_buf);
		if (ret == length) {
			struct mode_pg *pg;

			/*
			 * it would be better not to access to pg
			 * directy.
			 */
			pg = find_mode_page(cmd->dev, 0x08, 0);
			if (pg == NULL) {
				result = SAM_STAT_CHECK_CONDITION;
				key = ILLEGAL_REQUEST;
				asc = ASC_INVALID_FIELD_IN_CDB;
				break;
			}
			if (((cmd->scb[0] != WRITE_6) && (cmd->scb[1] & 0x8)) ||
			    !(pg->mode_data[0] & 0x04))
				bs_sync_sync_range(cmd, length, &result, &key,
						   &asc);
		} else
			set_medium_error(&result, &key, &asc);

		if (do_verify)
			goto verify;
		break;
	case WRITE_SAME:
	case WRITE_SAME_16:
		/* WRITE_SAME used to punch hole in file */
		if (cmd->scb[1] & 0x08) {
			ret = rbd_discard(rbd->rbd_image, offset, tl);
			if (ret != 0) {
				eprintf("Failed to punch hole for WRITE_SAME"
					" command\n");
				result = SAM_STAT_CHECK_CONDITION;
				key = HARDWARE_ERROR;
				asc = ASC_INTERNAL_TGT_FAILURE;
				break;
			}
			break;
		}
		while (tl > 0) {
			blocksize = 1 << cmd->dev->blk_shift;
			tmpbuf = scsi_get_out_buffer(cmd);

			switch (cmd->scb[1] & 0x06) {
			case 0x02: /* PBDATA==0 LBDATA==1 */
				put_unaligned_be32(offset, tmpbuf);
				break;
			case 0x04: /* PBDATA==1 LBDATA==0 */
				/* physical sector format */
				put_unaligned_be64(offset, tmpbuf);
				break;
			}

			ret = rbd_write(rbd->rbd_image, offset, blocksize,
					tmpbuf);
			if (ret != blocksize)
				set_medium_error(&result, &key, &asc);

			offset += blocksize;
			tl     -= blocksize;
		}
		break;
	case READ_6:
	case READ_10:
	case READ_12:
	case READ_16:
		length = scsi_get_in_length(cmd);
		ret = rbd_read(rbd->rbd_image, offset, length,
			       scsi_get_in_buffer(cmd));

		if (ret != length)
			set_medium_error(&result, &key, &asc);

		break;
	case PRE_FETCH_10:
	case PRE_FETCH_16:
		break;
	case VERIFY_10:
	case VERIFY_12:
	case VERIFY_16:
verify:
		length = scsi_get_out_length(cmd);

		tmpbuf = malloc(length);
		if (!tmpbuf) {
			result = SAM_STAT_CHECK_CONDITION;
			key = HARDWARE_ERROR;
			asc = ASC_INTERNAL_TGT_FAILURE;
			break;
		}

		ret = rbd_read(rbd->rbd_image, offset, length, tmpbuf);

		if (ret != length)
			set_medium_error(&result, &key, &asc);
		else if (memcmp(scsi_get_out_buffer(cmd), tmpbuf, length)) {
			result = SAM_STAT_CHECK_CONDITION;
			key = MISCOMPARE;
			asc = ASC_MISCOMPARE_DURING_VERIFY_OPERATION;
		}

		free(tmpbuf);
		break;
	case UNMAP:
		if (!cmd->dev->attrs.thinprovisioning) {
			result = SAM_STAT_CHECK_CONDITION;
			key = ILLEGAL_REQUEST;
			asc = ASC_INVALID_FIELD_IN_CDB;
			break;
		}

		length = scsi_get_out_length(cmd);
		tmpbuf = scsi_get_out_buffer(cmd);

		if (length < 8)
			break;

		length -= 8;
		tmpbuf += 8;

		while (length >= 16) {
			offset = get_unaligned_be64(&tmpbuf[0]);
			offset = offset << cmd->dev->blk_shift;

			tl = get_unaligned_be32(&tmpbuf[8]);
			tl = tl << cmd->dev->blk_shift;

			if (offset + tl > cmd->dev->size) {
				eprintf("UNMAP beyond EOF\n");
				result = SAM_STAT_CHECK_CONDITION;
				key = ILLEGAL_REQUEST;
				asc = ASC_LBA_OUT_OF_RANGE;
				break;
			}

			if (tl > 0) {
				if (rbd_discard(rbd->rbd_image, offset, tl)
				    != 0) {
					eprintf("Failed to punch hole for"
						" UNMAP at offset:%" PRIu64
						" length:%d\n",
						offset, tl);
					result = SAM_STAT_CHECK_CONDITION;
					key = HARDWARE_ERROR;
					asc = ASC_INTERNAL_TGT_FAILURE;
					break;
				}
			}

			length -= 16;
			tmpbuf += 16;
		}
		break;
	default:
		break;
	}

	dprintf("io done %p %x %d %u\n", cmd, cmd->scb[0], ret, length);

	scsi_set_result(cmd, result);

	if (result != SAM_STAT_GOOD) {
		eprintf("io error %p %x %d %d %" PRIu64 ", %m\n",
			cmd, cmd->scb[0], ret, length, offset);
		sense_data_build(cmd, key, asc);
	}
}

static int bs_rbd_open(struct scsi_lu *lu, char *path, int *fd, uint64_t *size)
{
	uint32_t blksize = 0;
	int ret = 0;
	rbd_image_info_t inf;
	char *poolname;
	char *imagename;
	char *snapname;
	struct active_rbd *rbd = RBDP(lu);
	struct active_cluster *cluster = NULL;

	parse_imagepath(path, &poolname, &imagename, &snapname);

	rbd->poolname = poolname;
	rbd->imagename = imagename;
	rbd->snapname = snapname;
	eprintf("bs_rbd_open: pool: %s image: %s snap: %s\n",
		poolname, imagename, snapname);
	
	ret = rados_ioctx_create(cluster->cluster, poolname, &rbd->ioctx);
	if (ret < 0) {
		eprintf("bs_rbd_open: rados_ioctx_create: %d\n", ret);
		return -EIO;
	}

	ret = rbd_open(rbd->ioctx, imagename, &rbd->rbd_image, snapname);
	if (ret < 0) {
		eprintf("bs_rbd_open: rbd_open: %d\n", ret);
		return ret;
	}
	if (rbd_stat(rbd->rbd_image, &inf, sizeof(inf)) < 0) {
		eprintf("bs_rbd_open: rbd_stat: %d\n", ret);
		return ret;
	}
	*size = inf.size;
	blksize = inf.obj_size;

	if (!lu->attrs.no_auto_lbppbe)
		update_lbppbe(lu, blksize);

	return 0;
}

static void bs_rbd_close(struct scsi_lu *lu)
{
	struct active_rbd *rbd = RBDP(lu);

	if (rbd->rbd_image) {
		rbd_close(rbd->rbd_image);
		rados_ioctx_destroy(rbd->ioctx);
		rbd->rbd_image = rbd->ioctx = NULL;
	}
}

static tgtadm_err bs_rbd_init(struct scsi_lu *lu, char *bsopts)
{
/* modify begin by hy, 2020-09-21, BugId:123 原因: */
/* modify end by hy, 2020-09-21 */
	tgtadm_err ret = TGTADM_UNKNOWN_ERR;
	struct active_rbd *rbd = RBDP(lu);
	struct active_cluster *cluster = NULL;

	dprintf("bs_rbd_init bsopts: \"%s\"\n", bsopts);

	/*
	 * clientid may be set by -i/--id. If clustername is set, then
	 * we use rados_create2, else rados_create
	 */


/** comment by hy 2020-09-21
 * # 这里添加一个全局集群链表
 */	
	cluster = get_cluster(bsopts);
	if (!cluster) {
		ret = TGTADM_UNKNOWN_ERR;
		eprintf("bs_rbd_init: get_cluster: %d, bsopts(%s)\n",
			ret, bsopts);
		goto fail;
	}

	rbd->cluster = cluster->cluster_name;
	ret = TGTADM_SUCCESS;

/* modify begin by hy, 2020-09-21, BugId:123 原因: */
/** comment by hy 2020-09-21
 * # 设备创建对应的线程,这里进行修改,将其移动到加载阶段,
     即 register_bs_module 阶段
 */
	//ret = bs_thread_open(info, bs_rbd_request, nr_iothreads);
/* modify end by hy, 2020-09-21 */
l_out:
	return ret;

fail:
	if (cluster) {
		put_cluster(cluster->cluster_name);
		free(cluster);
		cluster = NULL;
	}
	goto l_out;
}

static void bs_rbd_exit(struct scsi_lu *lu)
{
/* modify begin by hy, 2020-09-21, BugId:123 原因: */
	//struct bs_thread_info *info = BS_THREAD_I(lu);
/* modify end by hy, 2020-09-21 */
	struct active_rbd *rbd = RBDP(lu);

	/* do this first to try to be sure there's no outstanding I/O */

/* modify begin by hy, 2020-09-21, BugId:123 原因: */
	//bs_thread_close(info);
/* modify end by hy, 2020-09-21 */

	put_cluster(rbd->cluster);
}

int bs_rbd_cmd_submit(struct scsi_cmd *cmd)
{
	pthread_mutex_lock(&bs_info.pending_lock);
	list_add_tail(&cmd->bs_list, &bs_info.pending_list);
	pthread_mutex_unlock(&bs_info.pending_lock);
	pthread_cond_signal(&bs_info.pending_cond);
	set_cmd_async(cmd);

	return 0;
}

static struct backingstore_template rbd_bst = {
	.bs_name		= "rbd",
	.bs_datasize		= sizeof(struct bs_thread_info) +
				  sizeof(struct active_rbd),
	.bs_open		= bs_rbd_open,
	.bs_close		= bs_rbd_close,
	.bs_init		= bs_rbd_init,
	.bs_exit		= bs_rbd_exit,
	.bs_cmd_submit		= bs_rbd_cmd_submit,
	.bs_oflags_supported    = O_SYNC | O_DIRECT,
};

/** comment by hy 2020-09-21
 * # 模块只注册,不卸载,在这添加一个卸载阶段,以便完成卸载对应的操作
     现在就不卸载了
 */
void register_bs_module(void)
{
/** comment by hy 2020-09-21
 * # 初始化集群链表结构
 */
	INIT_LIST_HEAD(&cluster_list.list);
	pthread_mutex_init(&cluster_list.lock, NULL);
/** comment by hy 2020-09-21
 * # 注册后端存储引擎模板
 */
	register_backingstore_template(&rbd_bst);


/** comment by hy 2020-10-09
 * #  打开后端 ceph 存储
 */
 
/** comment by hy 2020-09-21
 * # 创建 bs 的线程组
 */
	bs_thread_open(&bs_info, bs_rbd_request, nr_iothreads);
/* modify end by hy, 2020-09-21 */
}
