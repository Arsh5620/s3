#include "../general/defines.h"
#include "../general/strings.h"
#include "../memdbg/memory.h"

typedef struct {
	boolean is_file;
	char *buffer;
	ulong index;
	ulong length;
} file_path_table_s;

my_list_s paths_parse(char *string, long size);
int paths_mkdir_recursive(my_list_s path_list);