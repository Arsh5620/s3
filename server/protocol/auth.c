#include "./auth.h"
#include "../crypto/sha.h"

int
s3_auth_transaction (s3_request_s *request)
{
    boolean user = s3_attrib_contains (request->header_table, S3_ATTRIB_USERNAME);
    boolean secret = s3_attrib_contains (request->header_table, S3_ATTRIB_PASSWORD);

    if (user == TRUE && secret == TRUE)
    {
        return (s3_auth_query (request));
    }
    else if (user == TRUE || secret == TRUE)
    {
        return (S3_RESPONSE_MIX_AUTH_ERROR);
    }
    else
    {
        return (SUCCESS); // no authentication was required for the action.
    }
}

int
s3_auth_query (s3_request_s *request)
{
    int error;
    string_s username = data_get_string_s (
      request->header_list, request->header_table, S3_ATTRIB_USERNAME, &error);

    if (error != SUCCESS)
    {
        return (S3_RESPONSE_SERVER_ERROR_NOAUTH);
    }

    string_s password = data_get_string_s (
      request->header_list, request->header_table, S3_ATTRIB_PASSWORD, &error);

    if (error != SUCCESS)
    {
        return (S3_RESPONSE_SERVER_ERROR_NOAUTH);
    }

    char sha_hash[SHA256LENGTH] = {0};
    sha_256 (password, (uchar *) sha_hash, SHA256LENGTH);

    error = s3_auth_query_sqlite3 (username.address, username.length, sha_hash, sizeof (sha_hash));
    return (error == SUCCESS ? S3_RESPONSE_SUCCESS : S3_RESPONSE_FAILED_AUTHENTICATION);
}

int
s3_auth_query_sqlite3 (char *username, int username_length, char *password, int password_length)
{
    int error;
    sqlite3_stmt *stmt = database_get_stmt (AUTH_QUERY, sizeof (AUTH_QUERY), &error);

    if (error != SUCCESS)
    {
        return error;
    }

    sqlite3_bind_text (stmt, 1, username, username_length, SQLITE_TRANSIENT);
    sqlite3_bind_text (stmt, 2, password, password_length, SQLITE_TRANSIENT);
    return database_finish_stmt (stmt, SQLITE_ROW, "Authentication failed");
}