AC_INIT([s3],[0.2],[https://github.com/Arsh5620/s3])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_PROG_CC
LT_INIT

USERID=`id -un`

AC_CONFIG_COMMANDS_POST([cp services/s3d.service.in services/s3d.service])
AC_CONFIG_COMMANDS_POST([sed -i "s/@@USERID@@/${USERID}/g" services/s3d.service])
AC_CONFIG_COMMANDS_POST(mv config.h ../src/config.h)

AH_BOTTOM([#define _COMPILED_ON_ "compiled on [" __DATE__ ", " __TIME__ "]"])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
	Makefile
])
AC_OUTPUT