#include "protocol.h"
// the function will attempt to find the most common attributes
// that are mostly used by all the "actions", attributes that
// are specific (one-to-one) to an "action" is not handled here.

char *attribs[] = {
  [S3_ATTRIB_ACTION] = S3_ATTRIBNAME_ACTION,
  [S3_ATTRIB_CRC] = S3_ATTRIBNAME_CRC,
  [S3_ATTRIB_DIRNAME] = S3_ATTRIBNAME_DIRNAME,
  [S3_ATTRIB_FILENAME] = S3_ATTRIBNAME_FILENAME,
  [S3_ATTRIB_PASSWORD] = S3_ATTRIBNAME_SECRET,
  [S3_ATTRIB_UPDATETRIM] = S3_ATTRIBNAME_UPDATETRIM,
  [S3_ATTRIB_UPDATEAT] = S3_ATTRIBNAME_UPDATEAT,
  [S3_ATTRIB_USERNAME] = S3_ATTRIBNAME_USERNAME,
};

char *actions[] = {
  [S3_ACTION_CREATE] = "create",
  [S3_ACTION_DELETE] = "delete",
  [S3_ACTION_DIR] = "dir",
  [S3_ACTION_NOTIFICATION] = "notification",
  [S3_ACTION_REQUEST] = "request",
  [S3_ACTION_SERVER] = "server",
  [S3_ACTION_UPDATE] = "update",
};

#define ATTRIB_COUNT sizeof (attribs) / sizeof (char *)
#define ACTION_COUNT sizeof (actions) / sizeof (char *)

const int attribs_count = ATTRIB_COUNT;
const int actions_count = ACTION_COUNT;

char *asserts[ACTION_COUNT][ATTRIB_COUNT] = {
  // Note that S3_ATTRIB_CREATE is 0 and any default elements are zeroed out so that is implicitly included
  [S3_ACTION_CREATE] = {S3_ATTRIBNAME_ACTION, S3_ATTRIBNAME_FILENAME, S3_ATTRIBNAME_USERNAME, S3_ATTRIBNAME_SECRET},
  [S3_ACTION_DELETE] = {S3_ATTRIBNAME_ACTION, S3_ATTRIBNAME_FILENAME, S3_ATTRIBNAME_USERNAME, S3_ATTRIBNAME_SECRET},
  [S3_ACTION_DIR] = {S3_ATTRIBNAME_ACTION, S3_ATTRIBNAME_DIRNAME, S3_ATTRIBNAME_USERNAME, S3_ATTRIBNAME_SECRET},
  [S3_ACTION_NOTIFICATION] = {S3_ATTRIBNAME_ACTION},
  [S3_ACTION_REQUEST] = {S3_ATTRIBNAME_ACTION, S3_ATTRIBNAME_USERNAME, S3_ATTRIBNAME_SECRET},
  [S3_ACTION_SERVER] = {S3_ATTRIBNAME_ACTION},
  [S3_ACTION_UPDATE] =
    {
      S3_ATTRIBNAME_ACTION,
      S3_ATTRIBNAME_FILENAME,
      S3_ATTRIBNAME_UPDATEAT,
      S3_ATTRIBNAME_UPDATETRIM,
      S3_ATTRIBNAME_USERNAME,
      S3_ATTRIBNAME_SECRET,
    },
};

// NOTE: the reason we have two pointers, not three (one for (char*, as a string), then two pointers for each level of array)
// is because C stores multidimensional array as a one-dimensional array with padding to match block length
// To access any element in this array we can use the formula:
//      address + (first-dimension-length * first-dimension-index) + second-dimension-index
char **s3_asserts = (char **) asserts;