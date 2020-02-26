#include <stdio.h>
#include "../networking/network.h"
#include "../dbp.h"
#include "../logs.h"
#include "../file.h"

#define DBP_FILE_TEMP_DIR "./temp/"

#define DBP_ACTION_NOID  -1
#define DBP_ATTRIBS_ERR_NAMETOOLONG   0x0003
#define DBP_ATTRIBS_ERR_CRC32NOTFOUND       0x0004

#define DBP_TEMP_FILE_FORMAT "%s/%.*s"

typedef struct dbp_common_attribs {
    string_s filename;
    unsigned int crc32;
} dbp_common_attribs_s;

// one-to-one mapping to the actions_supported
enum actions_supported_enum {
    ACTION_CREATE = 0
    , ACTION_NOTIFICATION = 1
    , ACTION_REQUEST = 2
    , ACTION_UPDATE = 3
};

enum attribs_supported_enum {
    ATTRIBS_FILENAME = 0
    , ATTRIBS_CRC = 1
};

int dbp_protocol_notification(dbp_s *protocol);
int dbp_create(dbp_s *protocol);
dbp_common_attribs_s dbp_attribs_try_find(dbp_s *protocol);
file_write_s create_download_file(dbp_s *protocol, string_s *filename);
int create_setup_environment();
