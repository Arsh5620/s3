#include "./auth.h"
#include "../crypto/sha.h"

// The way auth_transaction works is that it assumes that for any operation that requires authentication
// we have already set the login username and secret as required in header key/value pairs. 
// Then this function checks to confirms that both username/password are present or it will S3_RESPONSE_MIX_AUTH_ERROR.
// If both are not present that is because that user wasn't required to provide them and thus 
// wasn't asked to authenticate for the action that they are performing. 
int
s3_auth_transaction (s3_request_s *request)
{
    boolean user = s3_attrib_contains (request->header_table, S3_ATTRIBNAME_USERNAME);
    boolean secret = s3_attrib_contains (request->header_table, S3_ATTRIBNAME_SECRET);

    if (user == TRUE && secret == TRUE) {
        return s3_auth_query (request);
    } else if (user == TRUE || secret == TRUE) {
        return S3_RESPONSE_MIX_AUTH_ERROR;
    } else {
        return SUCCESS; // no authentication was required for the action.
    }
}

int
s3_auth_query (s3_request_s *request)
{
    size_t username_length = 0;
    char *username = s3_attrib_get (request->header_table, S3_ATTRIBNAME_USERNAME, &username_length, NULL);

    if (username == NULL) {
        return S3_RESPONSE_SERVER_ERROR_NOAUTH;
    }

    size_t secret_length = 0;
    char *password = s3_attrib_get (request->header_table, S3_ATTRIBNAME_SECRET, &secret_length, NULL);

    if (password == NULL) {
        return S3_RESPONSE_SERVER_ERROR_NOAUTH;
    }

    uchar sha_hash[SHA256_LENGTH] = {0};
    uchar sha_hash_hex[SHA256_HEXLENGTH] = {0};

    sha_256 (sha_hash, (uchar *) password, secret_length);
    sha_256_hex (sha_hash_hex, sha_hash);

    int error = s3_auth_query_sqlite3 (username, username_length, (char *) sha_hash_hex, sizeof (sha_hash_hex));
    return error == SUCCESS ? SUCCESS : S3_RESPONSE_FAILED_AUTHENTICATION;
}

int
s3_auth_query_sqlite3 (char *username, int username_length, char *password, int password_length)
{
    int error;
    sqlite3_stmt *stmt = database_get_stmt (AUTH_QUERY, sizeof (AUTH_QUERY), &error);

    if (error != SUCCESS) {
        return error;
    }

    sqlite3_bind_text (stmt, 1, username, username_length, SQLITE_TRANSIENT);
    sqlite3_bind_text (stmt, 2, password, password_length, SQLITE_TRANSIENT);
    return database_rows_affected (stmt, SQLITE_ROW, "Authentication failed");
}