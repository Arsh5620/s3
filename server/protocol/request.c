#include "protocol.h"

int dbp_request_prehook(packet_info_s *info)
{
	dbp_common_attribs_s attribs = info->attribs;
	
    if(attribs.filename.address == 0 || attribs.filename.length <= 0
		|| attribs.folder_name.address	== 0 || attribs.folder_name.length	<= 0)
        return(FAILED);

    if(filemgmt_file_exists(&attribs.folder_name, &attribs.filename))
		info->response.dbp_response	= DBP_RESPONSE_ACK;
	
    return(SUCCESS);
}