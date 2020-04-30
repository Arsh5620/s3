#include "../general/define.h"
#include "../general/string.h"
#include "../memdbg/memory.h"

typedef struct {
	boolean is_file;
	char *buffer;
	ulong index;
	ulong length;
} file_path_node_s;

typedef struct {
	my_list_s path_list;
	boolean is_absolute;
} file_path_s;

void path_free(file_path_s path);
my_list_s path_lex(string_s path, boolean *is_absolute);
file_path_s path_parse(string_s path);
int path_mkdir_recursive(char *path);
string_s path_construct(my_list_s path_list);