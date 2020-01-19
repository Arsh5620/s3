#include "./networking/network.h"
#include "memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./networking/defines.h"

int main(int argc, char *argv[])
{
    netconn_info_s conn = network_connect_init_sync(APPLICATION_PORT);
    printf("Connection Status: %d, errors: %x\n", conn.connection_status, conn.error_code);
    network_connect_accept_sync(&conn);
    while(1){
        netconn_data_basetypes_s data = network_data_read_long(&conn);
        if(! DATA_READ_SUCCESS(data.read_status))
        {
            break;
        }
        netconn_data_s d = network_data_readxbytes(&conn, data.data_u._long);

        printf("client wrote: %s\n", (char*)d.data_address);
        printf("your reply: ");
        char buffer[512];
        if(fgets(buffer + 8, 500, stdin) == NULL) {  
            printf("ERROR READING DATA FROM STDIN.\nEXITING\n");
            exit(1);
        }
        size_t length = strlen(buffer+8);
        memcpy(buffer, (void*)&length, 8);
        network_connection_write(&conn, buffer, length + 8);
    }

    void *address = m_malloc(20, "myfile.c:myfunction_name");
    void *new_address = m_realloc(address, 40, "myfile.c:new_function_name");

    m_free(new_address, "whateverthefuck");
    m_print_dbg();
    
    printf("Connection Status: %d, errors: %x\n", conn.connection_status, conn.error_code);
}