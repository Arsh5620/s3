#include "protocol.h"
// the function will attempt to find the most common attributes
// that are mostly used by all the "actions", attributes that
// are specific (one-to-one) to an "action" is not handled here.

data_keys_s attribs[] = {
  S3_KEY (S3_ATTRIBNAME_ACTION, S3_ATTRIB_ACTION),
  S3_KEY (S3_ATTRIBNAME_CRC, S3_ATTRIB_CRC),
  S3_KEY (S3_ATTRIBNAME_DIRNAME, S3_ATTRIB_DIRNAME),
  S3_KEY (S3_ATTRIBNAME_FILENAME, S3_ATTRIB_FILENAME),
  S3_KEY (S3_ATTRIBNAME_SECRET, S3_ATTRIB_PASSWORD),
  S3_KEY (S3_ATTRIBNAME_UPDATETRIM, S3_ATTRIB_UPDATETRIM),
  S3_KEY (S3_ATTRIBNAME_UPDATEAT, S3_ATTRIB_UPDATEAT),
  S3_KEY (S3_ATTRIBNAME_USERNAME, S3_ATTRIB_USERNAME)};

data_keys_s actions[] = {
  S3_KEY ("create", S3_ACTION_CREATE),
  S3_KEY ("delete", S3_ACTION_DELETE),
  S3_KEY ("dir", S3_ACTION_DIR),
  S3_KEY ("notification", S3_ACTION_NOTIFICATION),
  S3_KEY ("request", S3_ACTION_REQUEST),
  S3_KEY ("server", S3_ACTION_SERVER),
  S3_KEY ("update", S3_ACTION_UPDATE)};

#define ATTRIB_COUNT sizeof (attribs) / sizeof (data_keys_s)
#define ACTION_COUNT sizeof (actions) / sizeof (data_keys_s)

const int attribs_count = ATTRIB_COUNT;
const int actions_count = ACTION_COUNT;

enum s3_attribs_enum s3_asserts_1[ACTION_COUNT][ATTRIB_COUNT] = {
  {S3_ATTRIB_ACTION, S3_ATTRIB_FILENAME, S3_ATTRIB_USERNAME, S3_ATTRIB_PASSWORD}, // ACTION_CREATE
  {S3_ATTRIB_ACTION, S3_ATTRIB_FILENAME, S3_ATTRIB_USERNAME, S3_ATTRIB_PASSWORD}, // ACTION_DELETE
  {0},                                                                            // ACTION_DIR
  {S3_ATTRIB_ACTION},                                         // ACTION_NOTIFICATION
  {S3_ATTRIB_ACTION, S3_ATTRIB_USERNAME, S3_ATTRIB_PASSWORD}, // ACTION_REQUEST
  {S3_ATTRIB_ACTION},                                         // ACTION_SERVER
  {S3_ATTRIB_ACTION,
   S3_ATTRIB_FILENAME,
   S3_ATTRIB_UPDATEAT,
   S3_ATTRIB_UPDATETRIM,
   S3_ATTRIB_USERNAME,
   S3_ATTRIB_PASSWORD} // ACTION_UPDATE
};

enum s3_attribs_enum *s3_asserts = (enum s3_attribs_enum *) s3_asserts_1;