#include <stdio.h>
#include "../networking/network.h"
#include "../dbp.h"
#include "../logs.h"
#include "../file.h"
#include "../binarysearch.h"
#include "../defines.h"
#include "../hash_table.h"

#define DBP_FILE_TEMP_DIR "./temp/"

#define DBP_ACTION_NOID  -1
#define DBP_ATTRIBS_ERR_NAMETOOLONG 0x0003
#define DBP_ATTRIBS_ERR_CRC32NOTFOUND   0x0004

#define DBP_TEMP_FILE_FORMAT "%s/%.*s"

typedef struct dbp_common_attribs {
    string_s filename;
    unsigned int crc32;
} dbp_common_attribs_s;

// one-to-one mapping to the actions_supported
enum actions_supported_enum {
    ACTION_CREATE = 0
    , ACTION_NOTIFICATION
    , ACTION_REQUEST
    , ACTION_UPDATE
};

enum attrib_supported_enum {
    ATTRIB_ACTION = 128
    , ATTRIB_FILENAME
    , ATTRIB_CRC
};

int dbp_protocol_notification(packet_info_s *info);
int dbp_create(packet_info_s *protocol);

int dbp_assert_list(array_list_s list, 
    b_search_string_s *codes, int code_length, 
    int *match, int match_length);

hash_table_s dbp_attribs_find(packet_info_s *info);
dbp_common_attribs_s dbp_attribs_try_find(packet_info_s *info);
file_write_s create_download_file(packet_info_s *info, string_s *filename);
int create_setup_environment();

int dbp_magic_check(long magic);
long dbp_data_length(unsigned long magic);