#ifndef FILEMGMT_INCLUDE_GAURD
#define FILEMGMT_INCLUDE_GAURD

#include "file.h"
#include "strings.h"

#define FILE_EXISTS 1
#define FILE_NOTFOUND  0

int filemgmt_file_exists(string_s *folder_name, string_s *file_name);

#endif