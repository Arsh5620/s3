#include "protocol.h"
#include "../files/file.h"
#include "../general/defines.h"
#include "../files/filemgmt.h"
#include <time.h>

int dbp_prehook_create(dbp_request_s *request)
{
	dbp_protocol_attribs_s attribs = request->attribs;
	
	// if(attribs.filename.address == 0 || attribs.filename.length <= 0
	// 	|| attribs.folder_name.address	== 0 || attribs.folder_name.length	<= 0)
	// 	return(FAILED);

	// if(filemgmt_file_exists(&attribs.folder_name, &attribs.filename))
	// 	info->response.dbp_response	= DBP_RESPONSE_ACK;
	
	return(SUCCESS);
}