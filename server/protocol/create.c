#include "dbp_protocol.h"
#include "../file.h"
#include "../defines.h"

int create_setup_environment();

int dbp_create(dbp_s *protocol)
{
    // dbp setup the environment before accepting data from connection.
    if (create_setup_environment() == SUCCESS)
    {
        
    }
    
}

// create:: setup_environment
int create_setup_environment()
{
    // first make sure the temporary file directory exists. 
    if(file_dir_mkine(DBP_FILE_TEMP_DIR) != FILE_DIR_EXISTS)
    {
        logger_write_printf("could not open \"" DBP_FILE_TEMP_DIR 
                "\" dir, check if the program has appropriate "
                "permissions.");
        return(FAILED);
    }
    return(SUCCESS);
}