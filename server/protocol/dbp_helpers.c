#include <string.h>
#include "dbp_protocol.h"
#include "../binarysearch.h"
#include "../file.h"

// the function will attempt to find the most common attributes
// that are mostly used by all the "actions", attributes that 
// are specific (one-to-one) to an "action" is not handled here. 

static b_search_string_s attribs_supported[2] = 
{
    {"crc", 3, .code = ATTRIBS_CRC}
    , {"filename", 8, .code = ATTRIBS_FILENAME}
};

dbp_common_attribs_s dbp_attribs_try_find(dbp_s *protocol)
{
    dbp_common_attribs_s attributes = {0};

    array_list_s header_list    = protocol->headers;
    int list_length = header_list.index;

    for(int i=0; i<list_length; ++i) {
        key_value_pair_s pair   = 
                *(key_value_pair_s*)my_list_get(header_list, i);

        int attrib  =  b_search(attribs_supported
                        , sizeof(attribs_supported) 
                        / sizeof(b_search_string_s)
                        , pair.key, pair.key_length);
        
        if(attrib != -1) {
            switch (attribs_supported[attrib].code)
            {
            case ATTRIBS_FILENAME:
                if(pair.value_length > FILE_NAME_MAXLENGTH 
                    || pair.value_length == 0){
                    attributes.filename.error   =
                                DBP_ATTRIBS_ERR_NAMETOOLONG;
                } else {
                    attributes.filename.address = pair.value;
                    attributes.filename.length  = pair.value_length;
                }
                break;
            case ATTRIBS_CRC: 
                if(pair.value_length != 8) {
                    attributes.filename.error   =
                                DBP_ATTRIBS_ERR_CRC32NOTFOUND;
                }
                attributes.crc32    = *(unsigned int*) pair.value;
                break;
            }
        }
    }
    return(attributes);
}

// MAKE SURE THAT THIS LIST IS SORTED

static b_search_string_s actions_supported[4] = 
{
    {"create", 6, .code = ACTION_CREATE}
    , {"notification", 12, .code = ACTION_NOTIFICATION} 
    , {"request", 7, .code = ACTION_REQUEST}
    , {"update", 6, .code = ACTION_UPDATE}
};


int dbp_headers_action(dbp_s *_read, key_value_pair_s pair)
{
    int action = -1;
    if(memcmp(pair.key, actions_supported[0].string
        , actions_supported[0].strlen) == 0) {
        // now here to check the action that the client is requesting.
        action = b_search(actions_supported
            , sizeof(actions_supported)/sizeof(b_search_string_s)
            , pair.value
            , pair.value_length);
    }
    return(action);
}

