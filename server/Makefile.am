AUTOMAKE_OPTIONS = subdir-objects
bin_PROGRAMS = s3
s3_SOURCES = \
./crypto/sha.c\
./data-structures/hash_table.c\
./data-structures/linked_list.c\
./data/data.c\
./databases/sqlite_database.c\
./files/file.c\
./files/filemgmt.c\
./files/path.c\
./general/binarysearch.c\
./general/string.c\
./logger/logs.c\
./main.c\
./memdbg/memory.c\
./networking/network.c\
./protocol/actions/action-create.c\
./protocol/actions/action-delete.c\
./protocol/actions/action-dirlist.c\
./protocol/actions/action-login.c\
./protocol/actions/action-request.c\
./protocol/actions/action-serverinfo.c\
./protocol/actions/action-update.c\
./protocol/auth.c\
./protocol/constants.c\
./protocol/download.c\
./protocol/notification.c\
./protocol/protocol.c\
./protocol/request.c\
./protocol/response.c

include_HEADERS = \
./crypto/sha.h\
./data-structures/hash_table.h\
./data-structures/linked_list.h\
./data/data.h\
./databases/database.h\
./files/file.h\
./files/filemgmt.h\
./files/path.h\
./general/binarysearch.h\
./general/define.h\
./general/string.h\
./logger/logs.h\
./logger/messages.h\
./memdbg/memory.h\
./networking/network.h\
./protocol/auth.h\
./protocol/build.h\
./protocol/protocol.h\
./ssbs/deserializer.h\
./ssbs/list.h\
./ssbs/serializer.h

s3_LDADD = -lssl -lcrypto -lsqlite3 ssbs/ssbs.a