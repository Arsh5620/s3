#ifndef CONFIG_INCLUDE_GAURD
#define CONFIG_INCLUDE_GAURD

// refer to CONFIGFORMAT for further information

#include "../files/file.h"
#include "../data-structures/list.h"
#include "../parser/parser.h"
#include "../databases/database.h"
#include "../general/strings.h"
#include "../general/binarysearch.h"

database_connection_s config_parse_dbc(char *filename);

extern key_code_pair_s config_property[5];
#endif // CONFIG_INCLUDE_GAURD