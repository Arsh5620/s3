#include "protocol.h"
#include "../files/file.h"
#include "../general/defines.h"
#include "../files/filemgmt.h"
#include <time.h>

int dbp_create_prehook(packet_info_s *info)
{
    dbp_common_attribs_s attribs = info->attribs;
    if(attribs.filename.address == 0 || attribs.filename.length == 0)
        return(FAILED);
    filemgmt_file_exists(0, &attribs.filename);
    return(SUCCESS);
}