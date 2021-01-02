#ifndef AUTH_INCLUDE_GAURD
#define AUTH_INCLUDE_GAURD
#include "protocol.h"
#include "../databases/database.h"

#define AUTH_TABLE_NAME "authentication"

#define AUTH_COLUMN_SECRET "secret"
#define AUTH_COLUMN_USERNAME "username"

#define AUTH_TABLE_CREATE                                                                          \
    "CREATE TABLE IF NOT EXISTS " AUTH_TABLE_NAME " (" AUTH_COLUMN_USERNAME                        \
    " TEXT, " AUTH_COLUMN_SECRET " TEXT);"

#define AUTH_QUERY                                                                                 \
    "SELECT COUNT(*), 'Hello world!' FROM " AUTH_TABLE_NAME " WHERE"                               \
    " " AUTH_COLUMN_USERNAME " = ? AND"                                                            \
    " " AUTH_COLUMN_SECRET " = ? LIMIT 1;"

#endif // AUTH_INCLUDE_GAURD