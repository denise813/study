#! /usr/bin/bash

#scl enable devtoolset-7 bash

project_work_dir="$(dirname "$(readlink -f "$0")")"

#install_deps_args=()
#sh -x ${project_work_dir}/install-deps.sh ${install_deps_args}

build_type=Debug

rm -rf ${build_dir}
rm -rf ${install_dir}
build_dir=$project_work_dir/build
install_dir=${project_work_dir}/install
mkdir -p ${build_dir}
mkdir -p ${install_dir}
cd $build_dir  
cmake3 -DCMAKE_INSTALL_PREFIX=${install_dir} \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=1 $project_work_dir \
    -DCMAKE_BUILD_TYPE=$build_type
make
make install
cd -
#export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${build_dir}/lib
#echo ${LD_LIBRARY_PATH}

