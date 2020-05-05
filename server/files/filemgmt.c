// the purpose of this file is to handle the management of 
// checking, placing and retrieving the files given provided
// the relocation data. 
#include <string.h>
#include "./filemgmt.h"
#include "../output/output.h"
#include "../files/path.h"

database_table_bind_s filemgmt_binds = {0};

int filemgmt_binds_setup(MYSQL *mysql)
{
	filemgmt_binds	= database_bind_setup(mysql, FILEMGMT_BIND);
	if (filemgmt_binds.error)
	{
		return (FAILED);
	}
	return (SUCCESS);
}

int filemgmt_bind_fileinfo(database_table_bind_s *bind
	, string_s file_name, boolean new_bind)
{
	string_s strings[] = {
		FILEMGMT_COLUMN_FILE_NAME
	};

	database_table_bind_s bind_in	= new_bind
		? database_bind_select_copy(filemgmt_binds, strings, 1)
		: *bind;
		
	database_bind_clean(bind_in);

	if(database_bind_add_data(bind_in, FILEMGMT_COLUMN_FILE_NAME
		, file_name) == MYSQL_ERROR)
	{
		return(FILEMGMT_SQL_COULD_NOT_BIND);
	}

	*bind = bind_in;
	return(MYSQL_SUCCESS);
}

int filemgmt_file_exists(string_s file_name
	, string_s real_name,  struct stat *file_stats)
{
	database_table_bind_s bind_out	= filemgmt_binds;
	database_table_bind_s bind_in	= {0};

	int result	= filemgmt_bind_fileinfo(&bind_in, file_name, TRUE);
	if (result != SUCCESS)
	{
		return (result);
	}

	result = database_table_query(database_table_row_exists
		, STRING(FILEMGMT_QUERY_EXISTS)
		, bind_in.bind_params
		, 1
		, bind_out.bind_params);

	if (result != FALSE)
	{
		output_handle(OUTPUT_HANDLE_LOGS, LOGGER_LEVEL_DEBUG
			, FILEMGMT_RECORD_EXISTS
			, file_name.length, file_name.address);
	}

	database_bind_free(bind_in);

	if (result == FALSE)
	{
		return (FALSE);
	}

	FILE *file	= fopen(real_name.address, FILE_MODE_READBINARY);

	if (file == NULL)
	{
		return (FALSE);
	}
	
	if (file_stats != NULL)
	{
		struct stat f1	= file_stat(file);
		*file_stats	= f1;
	}
	fclose(file);

	return (TRUE);
}

int filemgmt_file_add(string_s file_name)
{
	database_table_bind_s bind_in	= filemgmt_binds;

	int result	= filemgmt_bind_fileinfo(&bind_in, file_name, FALSE);
	if (result != SUCCESS)
	{
		return (result);
	}

	if (database_table_insert(NULL, STRING(FILEMGMT_QUERY_INSERT)
		, bind_in.bind_params, bind_in.count) == -1)
	{
		return(FAILED);
	}
	return(SUCCESS);
}

int filemgmt_rename_file(string_s dest, string_s src)
{
	file_dir_mkine(FILEMGMT_FOLDER_NAME);
	if (file_rename(dest, src) == FAILED)
	{
		return (FAILED);
	}
	return (SUCCESS);
}

int filemgmt_remove_meta(string_s file_name)
{
	database_table_bind_s bind_in	= {0};

	int result = filemgmt_bind_fileinfo(&bind_in, file_name, TRUE);
	if (result != SUCCESS)
	{
		return (result);
	}

	int deleted	= database_table_stmt
		(STRING(FILEMGMT_QUERY_DELETE), bind_in.bind_params, 1);
	
	database_bind_free(bind_in);
	
	if (deleted != -1)
	{
		return (SUCCESS);
	}
	else 
	{
		return (FAILED);
	}
}

int filemgmt_mkdirs(filemgmt_file_name_s *file_info)
{
	int result	= path_mkdir_recursive(file_info->real_file_name.address);
	if (result != SUCCESS)
	{
		return (result);
	}

	result	= path_mkdir_recursive(file_info->real_hash_file_name.address);

	return (result);
}

int filemgmt_setup_environment(string_s client_filename
	, filemgmt_file_name_s *file_info)
{
	if (file_dir_mkine(FILEMGMT_FOLDER_NAME) != FILE_DIR_EXISTS)
	{
		return (FAILED);
	}

	if (file_dir_mkine(FILEMGMT_HASH_FOLDER) != FILE_DIR_EXISTS)
	{
		return (FAILED);
	}

	my_list_s path_list	= path_parse(client_filename).path_list;
	string_s file_name	= path_construct(path_list);
	my_list_free(path_list);
	
	if (file_name.address == NULL)
	{
		return (FAILED);
	}

	string_s real_file_name		= 
		file_path_concat(STRING(FILEMGMT_FOLDER_NAME), file_name);
	string_s real_hash_file_name		= 
		file_path_concat(STRING(FILEMGMT_HASH_FOLDER), file_name);
	
	file_info->file_name			= file_name;
	file_info->real_file_name		= real_file_name;
	file_info->real_hash_file_name	= real_hash_file_name;
	
	return (SUCCESS);
}

int filemgmt_setup_temp_files(filemgmt_file_name_s *file_info)
{
	if (file_dir_mkine(FILEMGMT_TEMP_DIR) != FILE_DIR_EXISTS)
	{
		return (FAILED);
	}
	static long counter = 0;

	long length;
	char *temp_file_p	= string_sprintf(FILEMGMT_TEMP_FORMAT, &length
		, FILEMGMT_TEMP_DIR
		, ++counter);
	
	string_s temp_file	= {0, .address = temp_file_p, .length = length};

	if (temp_file_p == NULL)
	{
		return (FAILED);
	}

	char *hash_file_p = string_sprintf(FILEMGMT_HASH_FORMAT, &length
		, FILEMGMT_TEMP_DIR, counter);
	string_s hash_file	= {0, .address = hash_file_p, .length = length};
	
	if (hash_file_p == NULL)
	{
		return (FAILED);
	}

	file_info->temp_hash_file_name	= hash_file;
	file_info->temp_file_name		= temp_file;

	return (SUCCESS);
}