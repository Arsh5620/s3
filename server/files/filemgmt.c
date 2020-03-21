// the purpose of this file is to handle the management of 
// checking, placing and retrieving the files given provided
// the relocation data. 
#include "filemgmt.h"
#include "./databases/database.h"


#define FILEMGMT_QUERY_FILEFOLDEREXISTS  \
"SELECT * FROM " DATABASE_TABLE_NAME " WHERE file_name = ?"
    

int filemgmt_file_exists(string_s *folder_name, string_s *file_name)
{
    database_table1_s table = {0};
    table.file_name = *file_name;

    int select[1]   = { TABLE1_INDEX_FILE_NAME };

    db_table_stmt_s *bind   =
        database_table1_bind_getselective(&table, select, 1);

    database_table1_s *row  = database_table1_allocate();
    db_table_stmt_s *table1  = database_table1_bind_get(row);
    
    MYSQL_STMT *stmt    = database_table1_query(table1
        , FILEMGMT_QUERY_FILEFOLDEREXISTS, bind->bind_params);
    
    if(stmt)
        __database_query_print_dbg(stmt, table1);

    mysql_stmt_close(stmt);
    database_table1_bind_free(bind);
    database_table1_bind_free(table1);
    database_table1_free(row);
    return(0);
}