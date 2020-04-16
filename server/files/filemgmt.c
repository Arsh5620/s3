// the purpose of this file is to handle the management of 
// checking, placing and retrieving the files given provided
// the relocation data. 
#include <string.h>
#include "filemgmt.h"
#include "../errors/errorhandler.h"
#include "../databases/database.h"

int filemgmt_bind_fileinfo(database_table_bind_s bind
	, string_s *folder_name, string_s *file_name)
{
	if(database_bind_add_data(bind, TABLE1_FI_COLUMN_FILE_NAME
		, *file_name) == MYSQL_ERROR)
	{
		return(FILE_SQL_COULD_NOT_BIND);
	}
	
	if(database_bind_add_data(bind, TABLE1_FI_COLUMN_FOLDER_NAME
		, *folder_name)  == MYSQL_ERROR)
	{
		return(FILE_SQL_COULD_NOT_BIND);
	}
	return(MYSQL_SUCCESS);
}

int filemgmt_file_exists(string_s *folder_name, string_s *file_name)
{
	database_table_bind_s bind_out = database_get_global_bind();
	
	string_s strings[] = {
		TABLE1_FI_COLUMN_FILE_NAME
		, TABLE1_FI_COLUMN_FOLDER_NAME
	};
	uint bind_in_count	= sizeof(strings) / sizeof(string_s);
	database_table_bind_s bind_in	= database_bind_select_copy(bind_out
		, strings, bind_in_count);

	int result = filemgmt_bind_fileinfo(bind_in, folder_name, file_name);
	if (result != SUCCESS)
	{
		return (result);
	}

	result = database_table_query(database_table_row_exists
		, STRING_S(FILEMGMT_QUERY_FILEFOLDEREXISTS)
		, bind_in.bind_params
		, bind_in_count
		, bind_out.bind_params);

	error_handle(ERRORS_HANDLE_LOGS, LOGGER_LEVEL_DEBUG
			, FILEMGMT_RECORD_STATUS
			, folder_name->length, folder_name->address
			, file_name->length, file_name->address
			, result);

	database_bind_free(bind_in);
	return (result == TRUE);
}

int filemgmt_file_add(string_s *folder_name, string_s *file_name)
{
	database_table_bind_s bind_in	= database_get_global_bind();

	// just making sure that all values are set correctly before use.
	database_bind_clean(bind_in); 

	int result = filemgmt_bind_fileinfo(bind_in, folder_name, file_name);
	if (result != SUCCESS)
	{
		return (result);
	}

	if (database_table_insert(NULL, STRING_S(DATABASE_TABLE_FI_INSERT)
		, bind_in.bind_params, bind_in.count) == FALSE)
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