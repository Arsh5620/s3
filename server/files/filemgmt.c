// the purpose of this file is to handle the management of 
// checking, placing and retrieving the files given provided
// the relocation data. 
#include <string.h>
#include "filemgmt.h"
#include "../databases/database.h"

#define FILEMGMT_QUERY_FILEFOLDEREXISTS  \
	"SELECT * FROM " DATABASE_TABLE_FI_NAME \
	" WHERE file_name = ? and folder_name = ?"

int filemgmt_file_exists(string_s *folder_name, string_s *file_name)
{
    database_table_bind_s bind_out = database_get_global_bind();
	
	string_s strings[] = {
		TABLE1_FI_COLUMN_FILE_NAME
		, TABLE1_FI_COLUMN_FOLDER_NAME
	};
	database_table_bind_s bind_in	= database_bind_some_copy(bind_out, strings, 2);

	memcpy(bind_in.bind_params[0].buffer, "what", 5);
	*(bind_in.bind_params[0].length)	= 4;

	memcpy(bind_in.bind_params[1].buffer, "hi", 3);
	*(bind_in.bind_params[1].length)	= 2;

    MYSQL_STMT *stmt    = database_table_query(
		FILEMGMT_QUERY_FILEFOLDEREXISTS
		, bind_in.bind_params
		, bind_out.bind_params);
    
    if(stmt)
        __database_query_print_dbg(stmt, bind_out);

    mysql_stmt_close(stmt);
    return(0);
}