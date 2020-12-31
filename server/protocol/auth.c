#include "./auth.h"
#include "../crypto/sha.h"

database_table_bind_s auth_table_binds;

int
auth_binds_setup (MYSQL *mysql)
{
    auth_table_binds = database_bind_setup (mysql, AUTH_BIND);
    if (auth_table_binds.error)
    {
        return (FAILED);
    }
    return (SUCCESS);
}

int
dbp_auth_transaction (dbp_request_s *request)
{
    boolean user = dbp_attrib_contains (request->header_table, DBP_ATTRIB_USERNAME);
    boolean secret = dbp_attrib_contains (request->header_table, DBP_ATTRIB_PASSWORD);

    if (user == TRUE && secret == TRUE)
    {
        return (dbp_auth_query (request));
    }
    else if (user == TRUE || secret == TRUE)
    {
        return (DBP_RESPONSE_MIX_AUTH_ERROR);
    }
    else
    {
        return (SUCCESS); // no authentication was required for the action.
    }
}

int
dbp_auth_query (dbp_request_s *request)
{
    int error;
    string_s username = data_get_string_s (
      request->header_list, request->header_table, DBP_ATTRIB_USERNAME, &error);

    if (error != SUCCESS)
    {
        return (DBP_RESPONSE_SERVER_ERROR_NOAUTH);
    }

    string_s password = data_get_string_s (
      request->header_list, request->header_table, DBP_ATTRIB_PASSWORD, &error);

    if (error != SUCCESS)
    {
        return (DBP_RESPONSE_SERVER_ERROR_NOAUTH);
    }

    database_bind_clean (auth_table_binds); // to erase any previous values

    string_s columns[] = {AUTH_COLUMN_USERNAME, AUTH_COLUMN_SECRET};
    database_table_bind_s bind_copy = database_bind_select_copy (auth_table_binds, columns, 2);

    if (database_bind_add_data (auth_table_binds, AUTH_COLUMN_USERNAME, username) == MYSQL_ERROR)
    {
        return (FILEMGMT_SQL_COULD_NOT_BIND);
    }

    char sha_hash[SHA256LENGTH + 1] = {0};
    sha_256_compute (password, (uchar *) sha_hash, SHA256LENGTH);

    // printf("Computed hash is \"%s\"\n", sha_hash);
    string_s sha_hash_s = STRING (sha_hash);

    if (database_bind_add_data (auth_table_binds, AUTH_COLUMN_SECRET, sha_hash_s) == MYSQL_ERROR)
    {
        return (FILEMGMT_SQL_COULD_NOT_BIND);
    }

    int result = database_table_query (
      database_table_row_exists,
      STRING (AUTH_QUERY_VERIFY),
      auth_table_binds.bind_params,
      2,
      bind_copy.bind_params);

    database_bind_free (bind_copy);

    return (result == TRUE ? DBP_RESPONSE_SUCCESS : DBP_RESPONSE_FAILED_AUTHENTICATION);
}
