#include <string.h>
#include "paths.h"
#include "file.h"

my_list_s paths_parse(string_s string)
{
	my_list_s path_tree	= my_list_new(8, sizeof(file_path_table_s));

	char *index_p	= string.address
		, *length_p	= (string.address + string.length);

	while (index_p < length_p)
	{
		char *new_index_p	= strchr(index_p, '/');
		ulong diff	= new_index_p - index_p;
	
		file_path_table_s dir	= {
			.is_file	= FALSE
			, .buffer	= string.address
			, .index	= index_p - (char*)string.address
			, .length	= diff
		};
		
		if (new_index_p == NULL)
		{
			dir.is_file	= TRUE;
			dir.length	= length_p - index_p;
		}

		if (diff > 0)
		{
			my_list_push(&path_tree, (char*)&dir);
		}

		if (new_index_p	== NULL)
		{
			break;
		}

		index_p = new_index_p + 1;
	}
	return (path_tree);
}

int paths_mkdir_recursive(my_list_s path_list)
{
	char *buffer	= m_malloc(512, MEMORY_FILE_LINE);
	boolean success	= TRUE;
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
			success	= FALSE;
			return (FAILED);
		}

		memcpy(buffer, path.buffer, path.index + path.length);
		buffer[path.index + path.length]	= 0;

		if (file_dir_mkine(buffer) != FILE_DIR_EXISTS)
		{
			perror("mkdir");
			success	= FALSE;
			break;
		}
	}
	m_free(buffer, MEMORY_FILE_LINE);
	return (success ? SUCCESS : FAILED);
}