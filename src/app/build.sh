#! /usr/bin/bash

project_work_dir="$(dirname "$(readlink -f "$0")")"


#install_deps_args=()
#sh -x ${project_work_dir}/install-deps.sh ${install_deps_args}

#source ${project_work_dir}/RELEASE.rc

build_dir=$project_work_dir/build
mkdir -p $build_dir

cd $build_dir  
cmake3 -DCMAKE_EXPORT_COMPILE_COMMANDS=1 $project_work_dir \
    -DCMAKE_BUILD_TYPE=$build_type \
    && make

