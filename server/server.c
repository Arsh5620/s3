#include "dbp.h"
#include "memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defines.h"
#include "logs.h"
#include "binarysearch.h"
#include "./databases/database.h"
#include "./config.h"
#include "./parser/lexer.h"

int main(int argc, char *argv[])
{
    char *string    = "Hello=howareyou";
    lexer_s lex = lexer_init(string, strlen(string));
    lexer_token_s token = lexer_next_token(&lex);
    printf("Token type is %d\n", token.token_type);
//     database_connection_s conninfo  = config_parse_dbc("CONFIGFORMAT");
//     int result  = database_init(conninfo);
//     int integrity   = database_verify_integrity();

//     // database_table1_s row = {0};
//     // row.folder_name = (string_s) {
//     //     (void*)"./tmpfolder/magicmushroom" ,26 ,0 
//     // };
//     // row.file_name  = (string_s){ (void*)"filename", 8, 0};
//     // row.file_size   = 128;
//     // row.permissions = 777;

//     // memcpy(row.owner, "arshdeep", 8);
//     // row.file_ud.year    = 2020;
//     // row.file_ud.month   = 1;
//     // row.file_ud.day     = 21;
//     // row.file_times_set |= DATETIME_UPLOAD_DATE;
    
//     // database_table1_s *row  = database_table1_allocate();
//     // db_table_stmt_s *table1  = database_table1_bind_get(row);
    
//     // // result = database_table1_insert(row, table1);

//     // #define MY_QUERY  "SELECT * FROM " DATABASE_TABLE_NAME " WHERE owner = ?"

//     // MYSQL_BIND bind = {0};
//     // bind.buffer = "strong-motherfucker";
//     // long int length = 20;
//     // bind.length = &length;
//     // bind.buffer_type    = MYSQL_TYPE_STRING;
    
//     // MYSQL_STMT *stmt    =   database_table1_query(table1, MY_QUERY, &bind);
//     // __database_query_print_dbg(stmt, table1);
//     // database_table1_bind_free(table1);
//     // printf("%d is returned\n", result);

// //     while(1){
// // create_setup_environment();

// //     }
//     dbp_s protocol  = dbp_init(APPLICATION_PORT);
//     dbp_accept_connection_loop(&protocol);
//     dbp_cleanup(protocol);
//     m_print_dbg();

//     // logger_init();
//     // char *string = "Hello, world!%s, %s, %s, %s\n", * string2 = "how are you";
//     // char *string3 = "What the fuck", *string4="I like jaspinder", *string5="But I know she doesn't";
//     // printf("%p: %p: %p: %p: %p\n", string, string2, string3, string4, string5);
//     // logger_write_printf(string, string2, string3, string4, string5);
//     // logger_cleanup();
//     // return 0;
//     // netconn_info_s conn = network_connect_init_sync(APPLICATION_PORT);
//     // printf("Connection Status: %d, errors: %x\n", conn.connection_status, conn.error_code);
//     // network_connect_accept_sync(&conn);
//     // while(1){
//     //     netconn_data_basetypes_s data = network_data_read_long(&conn);
//     //     if(! DATA_READ_SUCCESS(data.read_status))
//     //     {
//     //         break;
//     //     }
//     //     netconn_data_s d = network_data_readxbytes(&conn, data.data_u._long);

//     //     printf("client wrote: %s\n", (char*)d.data_address);
//     //     printf("your reply: ");
//     //     char buffer[512];
//     //     if(fgets(buffer + 8, 500, stdin) == NULL) {  
//     //         printf("ERROR READING DATA FROM STDIN.\nEXITING\n");
//     //         exit(1);
//     //     }
//     //     size_t length = strlen(buffer+8);
//     //     memcpy(buffer, (void*)&length, 8);
//     //     network_connection_write(&conn, buffer, length + 8);
//     // }

//     // void *address = m_malloc(20, "myfile.c:myfunction_name");
//     // void *new_address = m_realloc(address, 40, "myfile.c:new_function_name");

//     // m_free(new_address, "whateverthefuck");
//     // m_print_dbg();
    
//     // printf("Connection Status: %d, errors: %x\n", conn.connection_status, conn.error_code);
}