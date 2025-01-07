prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=@CMAKE_INSTALL_PREFIX@
libdir=@CMAKE_INSTALL_PREFIX@/lib@LIB_SUFFIX@
includedir=@CMAKE_INSTALL_PREFIX@/include/libqapt-qt6

Name: libqapt-qt6
Description: Qt wrapper around libapt-pkg
Version: @QAPT_VERSION_STRING@
Libs: -L${libdir} -lQApt-qt6
Cflags: -I${includedir}
