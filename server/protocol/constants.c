#include "protocol.h"
// the function will attempt to find the most common attributes
// that are mostly used by all the "actions", attributes that 
// are specific (one-to-one) to an "action" is not handled here. 

#define DBP_ATTRIBNAME_ACTION	"action"
#define DBP_ATTRIBNAME_CRC		"crc"
#define DBP_ATTRIBNAME_FILENAME	"filename"
#define DBP_ATTRIBNAME_UPDATEAT	"updateat"
#define DBP_ATTRIBNAME_UPDATETRIM "trim"

dbp_header_keys_s attribs[] = 
{
	DBP_STRINGKEY(DBP_ATTRIBNAME_ACTION, DBP_ATTRIB_ACTION)
	, DBP_STRINGKEY(DBP_ATTRIBNAME_CRC, DBP_ATTRIB_CRC)
	, DBP_STRINGKEY(DBP_ATTRIBNAME_FILENAME, DBP_ATTRIB_FILENAME)
	, DBP_STRINGKEY(DBP_ATTRIBNAME_UPDATETRIM, DBP_ATTRIB_UPDATETRIM)
	, DBP_STRINGKEY(DBP_ATTRIBNAME_UPDATEAT, DBP_ATTRIB_UPDATEAT)
};

dbp_header_keys_s actions[] = 
{
	DBP_STRINGKEY("create", DBP_ACTION_CREATE)
	, DBP_STRINGKEY("delete", DBP_ACTION_DELETE)
	, DBP_STRINGKEY("notification", DBP_ACTION_NOTIFICATION)
	, DBP_STRINGKEY("request", DBP_ACTION_REQUEST)
	, DBP_STRINGKEY("update", DBP_ACTION_UPDATE)
};

enum dbp_attribs_enum dbp_call_asserts[DBP_ACTIONS_COUNT][DBP_ATTRIBS_COUNT] = {
	// ACTION_CREATE
	{ DBP_ATTRIB_ACTION, DBP_ATTRIB_CRC, DBP_ATTRIB_FILENAME }
	, { DBP_ATTRIB_ACTION, DBP_ATTRIB_FILENAME}
	, { DBP_ATTRIB_ACTION } // ACTION_NOTIFICATION
	, { DBP_ATTRIB_ACTION } // ACTION_REQUEST
	, {
		DBP_ATTRIB_ACTION
		, DBP_ATTRIB_FILENAME
		, DBP_ATTRIB_UPDATEAT
		, DBP_ATTRIB_UPDATETRIM
	} // ACTION_UPDATE
};

struct config_parse attribs_parse[] = {
	STRUCT_CONFIG_PARSE(DBP_ATTRIBNAME_CRC, DBP_ATTRIB_CRC
		, dbp_protocol_attribs_s, crc32, CONFIG_TYPE_INT)
	, STRUCT_CONFIG_PARSE(DBP_ATTRIBNAME_FILENAME, DBP_ATTRIB_FILENAME
		, dbp_protocol_attribs_s, file_name, CONFIG_TYPE_STRING_S)
	, STRUCT_CONFIG_PARSE(DBP_ATTRIBNAME_UPDATETRIM, DBP_ATTRIB_UPDATETRIM
		, dbp_protocol_attribs_s, trim, CONFIG_TYPE_BOOLEAN)
	, STRUCT_CONFIG_PARSE(DBP_ATTRIBNAME_UPDATEAT, DBP_ATTRIB_UPDATEAT
		, dbp_protocol_attribs_s, update_at, CONFIG_TYPE_LONG)
};

/*
 * when adding new attributes, you need to edit, the attrib name, 
 * its enum must be added, it must be added to the attribute struct
 * and also make sure to add a struct_config_parse declaration of it
 * 
 * please also make sure that attribs_parse is sorted lexically for binary search
 */