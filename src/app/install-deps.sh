
cmd=$(rpm -qa |grep libacl-devel)
ret=$?
if [ ${ret} -ne 0 ]; then
    yum install libacl-devel -y
fi

if [ ${with_install_memcheck} -ne 0 ];then
    yum install libasan libasan-static -y
    yum install devtoolset-7-libasan-devel -y
fi

if [ ${with_install_cppcheck} -ne 0 ];then
    cmd=$(rpm -qa |grep cppcheck)
    ret=$?
    if [ ${ret} -ne 0 ]; then
        yum install -y cppcheck
    fi
fi

if [ ${with_install_ut} -ne 0 ];then
    cmd=$(rpm -qa |grep lcov)
    ret=$?
    if [ ${ret} -ne 0 ]; then
        yum install lcov -y
    fi
fi

