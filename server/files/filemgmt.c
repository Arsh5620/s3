// the purpose of this file is to handle the management of 
// checking, placing and retrieving the files given provided
// the relocation data. 
#include <string.h>
#include "filemgmt.h"
#include "../errors/errorhandler.h"
#include "../databases/database.h"

int filemgmt_file_exists(string_s *folder_name, string_s *file_name)
{
	if(file_name == NULL || folder_name == NULL)
		return(FILE_INPUT_ERROR);

    database_table_bind_s bind_out = database_get_global_bind();
	
	string_s strings[] = {
		TABLE1_FI_COLUMN_FILE_NAME
		, TABLE1_FI_COLUMN_FOLDER_NAME
	};
	database_table_bind_s bind_in	= database_bind_select_copy(bind_out
		, strings, sizeof(strings) / sizeof(string_s));

	int file_index	= database_bind_column_index(bind_in
		, TABLE1_FI_COLUMN_FILE_NAME);

	int folder_index	= database_bind_column_index(bind_in
		, TABLE1_FI_COLUMN_FOLDER_NAME);
	
	if(file_index < 0 || folder_index < 0)
		return(FILE_INPUT_ERROR);

	database_bind_data_copy(bind_in.bind_params + file_index
		, *file_name);
	database_bind_data_copy(bind_in.bind_params + folder_index
		, *folder_name);

    MYSQL_STMT *stmt    = database_table_query(
		FILEMGMT_QUERY_FILEFOLDEREXISTS
		, bind_in.bind_params
		, bind_out.bind_params);
	int recordexists	= database_table_rowexists(stmt);

	error_handle(ERRORS_HANDLE_LOGS, LOGGER_DEBUG
			, FILEMGMT_RECORD_STATUS
			, folder_name->length, folder_name->address
			, file_name->length, file_name->address
			, recordexists);

	database_bind_free(bind_in);
    mysql_stmt_close(stmt);
    return(recordexists > 0);
}