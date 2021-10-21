AUTOMAKE_OPTIONS = subdir-objects
bin_PROGRAMS = s3
s3_SOURCES = \
config.h\
../src/data/hash_table.c\
../src/data/hash_table.h\
../src/data/linked_list.c\
../src/data/linked_list.h\
../src/database/database.h\
../src/database/sqlite_database.c\
../src/files/file.c\
../src/files/file.h\
../src/files/filemgmt.c\
../src/files/filemgmt.h\
../src/files/path.c\
../src/files/path.h\
../src/general/common.c\
../src/general/common.h\
../src/general/messages.h\
../src/general/string.c\
../src/general/string.h\
../src/logging/logs.c\
../src/logging/logs.h\
../src/main.c\
../src/memdbg/memory.c\
../src/memdbg/memory.h\
../src/networking/network.c\
../src/networking/network.h\
../src/protocol/actions/action-create.c\
../src/protocol/actions/action-delete.c\
../src/protocol/actions/action-dirlist.c\
../src/protocol/actions/action-login.c\
../src/protocol/actions/action-request.c\
../src/protocol/actions/action-serverinfo.c\
../src/protocol/actions/action-update.c\
../src/protocol/auth.c\
../src/protocol/auth.h\
../src/protocol/build.h\
../src/protocol/constants.c\
../src/protocol/download.c\
../src/protocol/notification.c\
../src/protocol/protocol.c\
../src/protocol/protocol.h\
../src/protocol/request.c\
../src/protocol/response.c

s3_CFLAGS = -I../src -Wall -Werror -Os
s3_LDADD = -lssl -lcrypto -lsqlite3 ../src/ssbs/ssbs.a

## https://stackoverflow.com/a/58028204
systemdsysdir = `pkg-config systemd --variable=systemdsystemunitdir`
systemdsys_DATA = services/s3d.service