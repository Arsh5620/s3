AC_INIT([s3],[0.2],[https://github.com/Arsh5620/s3])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_PROG_CC
LT_INIT

# AC_CHECK_FUNCS([strstr])
# AC_CHECK_HEADERS([inttypes.h])
# AC_CHECK_HEADERS([stdint.h])
# AC_CHECK_HEADERS([stdlib.h])
# AC_CHECK_HEADERS([string.h]) 
# 
# AC_FUNC_MALLOC
# AC_TYPE_INT32_T
# AC_TYPE_INT64_T
# AC_TYPE_SIZE_T 
# AC_TYPE_UINT32_T
# AC_TYPE_UINT64_T

USERID=`id -un`

AC_CONFIG_COMMANDS_POST([cp services/s3d.service.in services/s3d.service])
AC_CONFIG_COMMANDS_POST([sed -i "s/@@USERID@@/${USERID}/g" services/s3d.service])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
	Makefile
])
AC_OUTPUT