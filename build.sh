#./install-deps.sh
#scl enable devtoolset-8 bash
#source scl_source enable devtoolset-8
env_name=study
source ${env_name}/bin/activate
#deactivate

PROJECT_DIR=$(pwd)
BUILD_DIR=${PROJECT_DIR}/build
rm -rf ${BUILD_DIR}
mkdir $BUILD_DIR
cd ${BUILD_DIR}
CMAKE=cmake3
TOP_DIR=${BUILD_DIR}

${CMAKE}  -DCMAKE_INSTALL_PREFIX=${TOP_DIR}/usr \
        -DCMAKE_INSTALL_LIBDIR=${TOP_DIR}/usr/lib64 \
        -DCMAKE_INSTALL_LIBEXECDIR=${TOP_DIR}/usr/lib \
        -DCMAKE_INSTALL_LOCALSTATEDIR=${TOP_DIR}/var \
        -DCMAKE_INSTALL_SYSCONFDIR=${TOP_DIR}/etc \
        -DCMAKE_INSTALL_MANDIR=${TOP_DIR}/usr/share/man \
        -DCMAKE_INSTALL_DOCDIR=${TOP_DIR}/usr/share/doc \
        -DCMAKE_INSTALL_INCLUDEDIR=${TOP_DIR}/usr/include \
        -DCMAKE_INSTALL_INCLUDEDIR=${TOP_DIR}/usr/include \
        -DCMAKE_BUILD_TYPE=Debug -DWITH_TESTS=OFF \
        -DCMAKE_C_FLAGS="-W -Wall -Wfatal-errors -O0 -g3 -gdwarf-4" \
        $ARGS "$@" ${PROJECT_DIR} || exit 1

# 执行 build
pushd ${BUILD_DIR}
make -j2
make install
popd

