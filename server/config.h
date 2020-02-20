#ifndef CONFIG_INCLUDE_GAURD
#define CONFIG_INCLUDE_GAURD

// refer to CONFIGFORMAT for further information

#include "file.h"
#include "list.h"
#include "strings.h"
#include "parser/parser.h"
#include "./databases/database.h"
#include "binarysearch.h"

array_list_s config_read_file(FILE *file);
database_connection_s config_parse_dbc(char *filename);

extern b_search_string_s config_property[5];
#endif // CONFIG_INCLUDE_GAURD