#include "protocol.h"
// the function will attempt to find the most common attributes
// that are mostly used by all the "actions", attributes that 
// are specific (one-to-one) to an "action" is not handled here. 

data_keys_s attribs[] = 
{
	DBP_KEY(DBP_ATTRIBNAME_ACTION, DBP_ATTRIB_ACTION)
	, DBP_KEY(DBP_ATTRIBNAME_CRC, DBP_ATTRIB_CRC)
	, DBP_KEY(DBP_ATTRIBNAME_FILENAME, DBP_ATTRIB_FILENAME)
	, DBP_KEY(DBP_ATTRIBNAME_SECRET, DBP_ATTRIB_SECRET)
	, DBP_KEY(DBP_ATTRIBNAME_UPDATETRIM, DBP_ATTRIB_UPDATETRIM)
	, DBP_KEY(DBP_ATTRIBNAME_UPDATEAT, DBP_ATTRIB_UPDATEAT)
	, DBP_KEY(DBP_ATTRIBNAME_USERNAME, DBP_ATTRIB_USERNAME)
};

data_keys_s actions[] = 
{
	DBP_KEY("create", DBP_ACTION_CREATE)
	, DBP_KEY("delete", DBP_ACTION_DELETE)
	, DBP_KEY("notification", DBP_ACTION_NOTIFICATION)
	, DBP_KEY("request", DBP_ACTION_REQUEST)
	, DBP_KEY("server", DBP_ACTION_SERVER)
	, DBP_KEY("update", DBP_ACTION_UPDATE)
};

enum dbp_attribs_enum dbp_call_asserts
	[DBP_ACTIONS_COUNT][DBP_ATTRIBS_COUNT] = {
	// ACTION_CREATE
	{ DBP_ATTRIB_ACTION
		, DBP_ATTRIB_CRC
		, DBP_ATTRIB_FILENAME
		, DBP_ATTRIB_USERNAME
		, DBP_ATTRIB_SECRET 
	}
	, { DBP_ATTRIB_ACTION
		, DBP_ATTRIB_FILENAME
		, DBP_ATTRIB_USERNAME
		, DBP_ATTRIB_SECRET
	} // ACTION_DELETE
	, { DBP_ATTRIB_ACTION } // ACTION_NOTIFICATION
	, { DBP_ATTRIB_ACTION
		, DBP_ATTRIB_USERNAME
		, DBP_ATTRIB_SECRET 
	} // ACTION_REQUEST
	, { DBP_ATTRIB_ACTION } // ACTION_SERVER
	, {
		DBP_ATTRIB_ACTION
		, DBP_ATTRIB_FILENAME
		, DBP_ATTRIB_UPDATEAT
		, DBP_ATTRIB_UPDATETRIM
		, DBP_ATTRIB_USERNAME
		, DBP_ATTRIB_SECRET
	} // ACTION_UPDATE
};

/*
 * when adding new attributes, you need to edit, the attrib name, 
 * its enum must be added, it must be added to the attribute struct
 * and also make sure to add a struct_config_parse declaration of it
 * 
 * please also make sure that attribs_parse is sorted lexically for binary search
 */