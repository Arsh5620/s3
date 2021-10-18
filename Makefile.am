AUTOMAKE_OPTIONS = subdir-objects
bin_PROGRAMS = s3
s3_SOURCES = \
src/networking/network.h\
src/networking/network.c\
src/protocol/auth.h\
src/protocol/protocol.h\
src/protocol/response.c\
src/protocol/build.h\
src/protocol/actions/action-delete.c\
src/protocol/actions/action-dirlist.c\
src/protocol/actions/action-update.c\
src/protocol/actions/action-request.c\
src/protocol/actions/action-create.c\
src/protocol/actions/action-login.c\
src/protocol/actions/action-serverinfo.c\
src/protocol/constants.c\
src/protocol/download.c\
src/protocol/protocol.c\
src/protocol/notification.c\
src/protocol/request.c\
src/protocol/auth.c\
src/main.c\
src/data/linked_list.c\
src/data/hash_table.c\
src/data/linked_list.h\
src/data/hash_table.h\
src/logger/logs.c\
src/logger/logs.h\
src/memdbg/memory.h\
src/memdbg/memory.c\
src/general/string.h\
src/general/string.c\
src/general/common.c\
src/general/common.h\
src/general/messages.h\
src/database/sqlite_database.c\
src/database/database.h\
src/files/file.c\
src/files/filemgmt.c\
src/files/path.c\
src/files/file.h\
src/files/path.h\
src/files/filemgmt.h\
config.h

s3_CFLAGS=-Isrc -Wall -Werror -Os
s3_LDADD = -lssl -lcrypto -lsqlite3 src/ssbs/ssbs.a

## https://stackoverflow.com/a/58028204
systemdsysdir = `pkg-config systemd --variable=systemdsystemunitdir`
systemdsys_DATA = services/s3d.service