# yum install centos-release-scl -y
# yum install devtoolset-7 -y
# scl enable devtoolset-7 bash

# yum install python3-pip
# pip3 install virtualenv
env_name=study
virtualenv --python=python3 --system-site-packages ${env_name}
source ${env_name}/bin/activate

deactivate

pushd ./src/thridparty
rm -rf ./boost_1_80_0
#wget -O ./boost_1_80_0.tar.bz2 https://boostorg.jfrog.io/artifactory/main/release/1.80.0/source/boost_1_80_0.tar.bz2
tar -xf boost_1_80_0.tar.bz2
mkdir -p ./libboost
cd ./boost_1_80_0
./bootstrap.sh
./b2  install --prefix=./liboost stage link=static runtime-link=static  threading=multi variant=debug define=BOOST_LOG_USE_CHAR --with-log --with-thread --with-system --with-filesystem --with-boost_program_options
#./b2  install --prefix=./liboost link=static runtime-link=static define=BOOST_LOG_WITHOUT_CHAR define=BOOST_LOG_WITHOUT_WCHAR_T define=BOOST_LOG_WITHOUT_SYSLOG define=BOOST_LOG_USE_STD_REGEX --with-log --with-thread --with-system --with-filesystem
popd


