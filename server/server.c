#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dbp.h"
#include "memory.h"
#include "parser/parser.h"
int main(int argc, char *argv[])
{
    dbp_s protocol  = dbp_init(APPLICATION_PORT);
    if(protocol.is_init)
        dbp_accept_connection_loop(&protocol);
    dbp_cleanup(protocol);
}