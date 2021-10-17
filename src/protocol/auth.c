#include "./auth.h"
#include "protocol.h"
#include <string.h>

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

    size_t passphrase_length = 0;
    char *passphrase = s3_attrib_get (request->header_table, S3_ATTRIBNAME_SECRET, &passphrase_length, NULL);

    if (passphrase == NULL) {
        return S3_RESPONSE_SERVER_ERROR_NOAUTH;
    }

    uchar sha_hash[SHA256_LENGTH] = {0};
    uchar sha_hash_hex[SHA256_HEXLENGTH] = {0};

    SHA256 ((uchar *) passphrase, passphrase_length, sha_hash);
    string_hexify (sha_hash_hex, sha_hash, SHA256_LENGTH);

    int error = s3_auth_query_sqlite3 (username, username_length, (char *) sha_hash_hex, sizeof (sha_hash_hex));
    return error == SUCCESS ? SUCCESS : S3_RESPONSE_FAILED_AUTHENTICATION;
}

int
s3_auth_add_user (char *username, char *passphrase, int username_length, int passphrase_length)
{
    uchar sha_hash[SHA256_LENGTH] = {0};
    uchar sha_hash_hex[SHA256_HEXLENGTH] = {0};

    SHA256 ((uchar *) passphrase, passphrase_length, sha_hash);
    string_hexify (sha_hash_hex, sha_hash, SHA256_LENGTH);

    return s3_auth_insert_sqlite3 (username, username_length, (char *) sha_hash_hex, sizeof (sha_hash_hex));
}

int
s3_auth_query_sqlite3 (char *username, int username_length, char *password, int password_length)
{
    sqlite3_stmt *stmt = database_get_stmt (AUTH_QUERY, sizeof (AUTH_QUERY));
    if (stmt == NULL) {
        return FAILED;
    }

    sqlite3_bind_text (stmt, 1, username, username_length, SQLITE_TRANSIENT);
    sqlite3_bind_text (stmt, 2, password, password_length, SQLITE_TRANSIENT);
    return database_rows_affected (stmt, SQLITE_ROW, "Authentication failed");
}

int
s3_auth_insert_sqlite3 (char *username, int username_length, char *password, int password_length)
{
    sqlite3_stmt *stmt = database_get_stmt (AUTH_INSERT, sizeof (AUTH_INSERT));

    if (stmt == NULL) {
        return FAILED;
    }

    sqlite3_bind_text (stmt, 1, username, username_length, SQLITE_TRANSIENT);
    sqlite3_bind_text (stmt, 2, password, password_length, SQLITE_TRANSIENT);
    return database_rows_affected (stmt, SQLITE_ROW, "Adding new auth user failed");
}