########################################################
#当定义一个子包时，必须至少包含Summary:,Group:,%description选项
Summary:   Test package for LinuxFly webblog

#必须的
Name:     suite

#必须的
Version:   1.0.0

#必须的
Release:   1

#列出这个包属于那个发行版
Distribution: Centos

#设置rpm包的分类，所有组列在文件/usr/share/doc/rpm-version/GROUP
#常用分类 system Environment/Shells
#	  System
#         也可以自己创建一个新的分组 Application/Server
Group:     System

# 许可证类型
License:    GPL

# 软件的源站
URL:        http://XXXX.org

# 制作rpm包的人员信息
Packager:   denise

# 源码包的名称，在%_topdir/SOURCE下，如果有多个源码包的话，可以通过
# Source0、Source1这样的字段来指定其他的源码包
Source:    demo.tar.gz

#指定了make install的测试安装目录
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root

#指定安装的路径
Prefix: %{_prefix} /home/test
#Prefix: %{sysconfdir}

# 该rpm包所依赖的软件包名称，
# 可以用>=或<=表示大于或小于某一特定版本，例如：libpng-devel >= 1.0.20 zlib >= 1.1.1
# 还有PreReq、Requires(pre)、Requires(post)、Requires(preun)、
# Requires(postun)、BuildRequires等 都是针对不同阶段的依赖指定
# Requires:	pcre,openssl

#指明本软件一些特定的功能，以便其他rpm识别
#Provides:

#自定义变量
%define    user_path /home/a

# 制作过程需要的工具或软件包
#BuildRequires:  gcc,make

# 软件的描述，这个可以尽情地写
%description
Just a test rpm suite.

########################################################
# SPEC脚本主体
#######################################################
#在安装包之前运行 定义准备编译的命令
%prep
echo "---------------------prep"

#-c 在解压之前创建子目录 	-q 在安静模式下且最少输出
#-T 禁用自动化解压包     	-i name 设置子目录名字为name
#-D 在解压之前禁止删除目录
#-a number 在改变目录后，仅解压给定数字的源码，如-a 0 for source0
#-b number 在改变目录前，仅解压给定数字的源码，如-b 0 for source0
#-n 编译多个源码包 源码的tar包的名称与解压出来的目录 %setup -n %{name}-%{version}
%setup -c

#-----------------------------------------------------
#编译阶段
%build
echo "---------------------build"

#-------------------------------------------------------
%install
echo "---------------------install"
rm -rf %{buildroot}/*
install -d %{buildroot}/%{user_path}
echo `pwd`
cp -a /home/code/debug/test/install/* %{buildroot}/%{user_path}
#ls -l %{buildroot}/%{user_path}/

cp -a install/* %{buildroot}/
ls -l %{buildroot}

#-----------------------------------------------------
# 安装前执行的脚本，语法和shell脚本的语法相同
%pre
echo "---------------------pre"

#-------------------------------------------------
# 安装后执行的脚本
%post
echo "---------------------post"
#-----------------------------------------------------

#在卸载包之前运行脚本
%preun
echo "---------------------preun"

#-------------------------------------------------------
#在卸载包之后运行脚本
echo "---------------------postun"
%postun

#-----------------------------------------------------
#rpm包中要安装的所有文件列表 文件中也可以包含通配符，如*
%files
#---
#设置默认权限，如果没有指定，则继承默认的权限
#-表示默认值
%defattr (-,root,root,0755)
%{user_path}
#%attr(0755,root,root) /etc/rc.d/init.d/zabbix_agentd

#------------------------------------------------
%changelog

#------------------------------------------------------
#清理
%clean
echo "---------------------clean"
#rm -rf %{buildroot}
#rm -rf %{buildroot}/%{name}-%{version}
