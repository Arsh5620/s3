#include <stdio.h>
#include "../networking/network.h"
#include "../dbp.h"
#include "../logs.h"

#define DBP_FILE_TEMP_DIR "./temp/"

#define DBP_ACTION_ERR  -1
#define DBP_ATTRIBS_ERR_FILENAME_TOO_LONG   0x0003
#define DBP_ATTRIBS_ERR_CRC32NOTFOUND       0x0004

typedef struct dbp_attrib_type {
    void *address;
    int length;
    int error;
} dbp_attrib_base_s;

typedef struct dbp_common_attribs {
    dbp_attrib_base_s filename;
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
    ATTRIBS_FILENAME
    , ATTRIBS_CRC
};

int dbp_protocol_notification(dbp_s *protocol);
int dbp_create(dbp_s *protocol);
dbp_common_attribs_s dbp_attribs_try_find(dbp_s *protocol);
