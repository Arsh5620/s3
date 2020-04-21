#include <string.h>
#include "paths.h"
#include "file.h"

my_list_s paths_parse(char *string, long size)
{
	my_list_s path_tree	= my_list_new(8, sizeof(file_path_table_s));

	char *index_p	= string, *length_p	= (string + size);
	boolean is_absolute	= FALSE, has_started	= FALSE;

	while (index_p < length_p)
	{
		char *new_index_p	= strchr(index_p, '/');
		if (new_index_p == NULL)
		{
			// means that the path does not contain any more directories
			break;
		}
		ulong diff	= new_index_p - index_p;

		if (diff == 0 && has_started == FALSE)
		{
			is_absolute	= TRUE;
		}
		has_started	= TRUE;
	
		file_path_table_s dir	= {
			.is_file	= FALSE
			, .buffer	= string
			, .index	= index_p - string
			, .length	= diff
		};
		
		my_list_push(&path_tree, &dir);

		index_p = new_index_p + 1;
	}

	if (index_p < length_p)
	{
		file_path_table_s file	= {
			.is_file	= TRUE
			, .buffer	= string
			, .index	= index_p - string
			, .length	= length_p - index_p
		};
		
		my_list_push(&path_tree, &file);
	}
	return (path_tree);
}

int paths_mkdir_recursive(my_list_s path_list)
{
	char *buffer	= m_malloc(512, MEMORY_FILE_LINE);
	for (size_t i = 0; i < path_list.count; i++)
	{
		file_path_table_s path	= 
			*(file_path_table_s*)my_list_get(path_list, i);

		if (path.is_file)
		{
			break;
		}

		ulong path_length	= path.index + path.length;
		if (path_length > 512)
		{
			return (FAILED);
		}

		memcpy(buffer, path.buffer, path.index + path.length);
		buffer[path.index + path.length]	= 0;

		mkdir(buffer, S_IRWXU);
	}
	return (SUCCESS);
}