#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dbp.h"
#include "memory.h"
#include "parser/parser.h"
int main(int argc, char *argv[])
{
    FILE *file  = fopen("CONFIGFORMAT", "rw");
    parser_parse_file(file);
    // dbp_s protocol  = dbp_init(APPLICATION_PORT);
    // if(protocol.setup_complete)
    //     dbp_accept_connection_loop(&protocol);
    // dbp_cleanup(protocol);
}