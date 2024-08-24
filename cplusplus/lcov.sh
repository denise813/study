module_name=$1
module_sub_dir=$2

cmd=$(rpm -qa |grep lcov)
ret=$?
if [ ${ret} -ne 0 ]; then
    yum install lcov -y
fi

#sh locv.sh ${projetc_name}ut_vmbw
#sh locv.sh ut_vmbw

current_dir=$(pwd)
bin_dir="${current_dir}/yuanshuo/bin"
export LD_LIBRARY_PATH="${bin_dir}:"${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}

vm_lib_dpath="${bin_dir}/vmware/${vmver}/lib64"
export LD_LIBRARY_PATH="${ldpath}:"${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}

build_dir="${current_dir}/build/debug"
module_dir="${build_dir}/src/unittest/${module_sub_dir}"
rm -rf ${module_dir}/core.*
rm -rf ${module_dir}/collect_files
rm -rf ${module_dir}/*.log

${current_dir}/build.sh debug
cd ${build_dir}
make lcov_${module_name}

