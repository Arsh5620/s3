// the purpose of this file is to handle the management of 
// checking, placing and retrieving the files given provided
// the relocation data. 
#include "filemgmt.h"
#include "../databases/database.h"


#define FILEMGMT_QUERY_FILEFOLDEREXISTS  \
"SELECT * FROM " DATABASE_TABLE_NAME " WHERE file_name = ?"
    

int filemgmt_file_exists(string_s *folder_name, string_s *file_name)
{
    database_table_bind_s bind_in = database_bind_setup(database_get_handle());
    database_table_bind_s bind_out = database_bind_setup(database_get_handle());
	
	string_s strings[] = {
		TABLE1_FI_COLUMN_FILE_NAME
	};
	MYSQL_BIND *bind_in_some	= database_bind_some(bind_in, strings, 1);

	memcpy(bind_in_some[0].buffer, "what", 5);
	*(bind_in_some[0].length)	= 4;

    MYSQL_STMT *stmt    = database_table_query(
		FILEMGMT_QUERY_FILEFOLDEREXISTS
		, bind_in_some
		, bind_out.bind_params);
    
    if(stmt)
        __database_query_print_dbg(stmt, bind_out);

    mysql_stmt_close(stmt);
    return(0);
}