// the purpose of this file is to handle the management of 
// checking, placing and retrieving the files given provided
// the relocation data. 
#include <string.h>
#include "./filemgmt.h"
#include "../output/output.h"
#include "../databases/database.h"

int filemgmt_bind_fileinfo(database_table_bind_s *bind
	, string_s file_name, boolean new_bind)
{
	string_s strings[] = {
		TABLE_FI_COLUMN_FILE_NAME
	};

	database_table_bind_s bind_in	= new_bind
		? database_bind_select_copy (database_get_global_bind(), strings, 1)
		: *bind;
	database_bind_clean(bind_in);

	if(database_bind_add_data(bind_in, TABLE_FI_COLUMN_FILE_NAME
		, file_name) == MYSQL_ERROR)
	{
		return(FILEMGMT_SQL_COULD_NOT_BIND);
	}

	*bind = bind_in;
	return(MYSQL_SUCCESS);
}

int filemgmt_file_exists(string_s file_name)
{
	database_table_bind_s bind_out	= database_get_global_bind();
	database_table_bind_s bind_in	= {0};

	int result	= filemgmt_bind_fileinfo(&bind_in, file_name, TRUE);
	if (result != SUCCESS)
	{
		return (result);
	}

	result = database_table_query(database_table_row_exists
		, STRING(FILEMGMT_QUERY_FILEFOLDEREXISTS)
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
	return (result == TRUE);
}

int filemgmt_file_add(string_s file_name)
{
	database_table_bind_s bind_in	= database_get_global_bind();

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
	
	if (deleted != -1)
	{
		return (SUCCESS);
	}
	else 
	{
		return (FAILED);
	}
}

// this function shall setup the environment, which means building 
// the directory tree, setting up working/temporary/sha1 hash file names
// for success it will return FILEMGMT_SUCCESS, and for failure 
// it will return FILEMGMT_**failure code**
int filemgmt_setup_environment(string_s folder_name, string_s file_name)
{
	return(0);
}