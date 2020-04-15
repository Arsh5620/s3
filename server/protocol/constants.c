#include "protocol.h"
// the function will attempt to find the most common attributes
// that are mostly used by all the "actions", attributes that 
// are specific (one-to-one) to an "action" is not handled here. 

dbp_header_keys_s attribs[] = 
{
	DBP_STRINGKEY("action", DBP_ATTRIB_ACTION)
	, DBP_STRINGKEY("crc", DBP_ATTRIB_CRC)
	, DBP_STRINGKEY("filename", DBP_ATTRIB_FILENAME)
	, DBP_STRINGKEY("folder", DBP_ATTRIB_FOLDER)
};

dbp_header_keys_s actions[] = 
{
	DBP_STRINGKEY("create", DBP_ACTION_CREATE)
	, DBP_STRINGKEY("notification", DBP_ACTION_NOTIFICATION)
	, DBP_STRINGKEY("request", DBP_ACTION_REQUEST)
	, DBP_STRINGKEY("update", DBP_ACTION_UPDATE)
};

enum dbp_attribs_enum dbp_call_asserts[][DBP_ATTRIBS_COUNT] = {
	// ACTION_CREATE
	{DBP_ATTRIB_ACTION, DBP_ATTRIB_CRC, DBP_ATTRIB_FILENAME, DBP_ATTRIB_FOLDER}
	, {DBP_ATTRIB_ACTION, 0, 0} // ACTION_NOTIFICATION
	, {DBP_ATTRIB_ACTION, 0, 0} // ACTION_REQUEST
	, {DBP_ATTRIB_ACTION, 0, 0} // ACTION_UPDATE
};

struct config_parse attribs_parse[] = {
	STRUCT_CONFIG_PARSE("crc", DBP_ATTRIB_CRC
		, dbp_protocol_attribs_s, crc32, CONFIG_TYPE_INT)
	, STRUCT_CONFIG_PARSE("filename", DBP_ATTRIB_FILENAME
		, dbp_protocol_attribs_s, file_name, CONFIG_TYPE_STRING_S)
	, STRUCT_CONFIG_PARSE("folder", DBP_ATTRIB_FOLDER
		, dbp_protocol_attribs_s, folder_name, CONFIG_TYPE_STRING_S)
};