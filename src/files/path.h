#ifndef SRC_FILES_PATH_
#define SRC_FILES_PATH_

#include "general/common.h"
#include "general/string.h"
#include "ssbs/list.h"

int
path_mkdir_recursive (char *path);
char *
path_validate_and_expand (char *path);
char *
path_concat_and_validate (char *path_first, char *path_second, short path_first_length, short path_second_length);

#endif // SRC_FILES_PATH_