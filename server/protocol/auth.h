#ifndef AUTH_INCLUDE_GAURD
#define AUTH_INCLUDE_GAURD
#include "./protocol.h"
#include "../databases/database.h"

#define AUTH_TABLE_NAME	"auth"

#define AUTH_BIND	\
	DATABASE_COMMON_BIND_STMT(AUTH_TABLE_NAME)

#define AUTH_TABLE_CHECK \
	DATABASE_COMMON_TABLE_EXISTS(AUTH_TABLE_NAME)

#define AUTH_TABLE_CREATE \
	DATABASE_CREATE_TABLE(AUTH_TABLE_NAME, \
	" username VARCHAR(256)" \
	", secret VARCHAR(256)")

#define AUTH_COLUMN_USERNAME	STRING("username")
#define AUTH_COLUMN_SECRET	STRING("secret")

#define AUTH_QUERY_VERIFY \
	"SELECT * FROM " AUTH_TABLE_NAME " WHERE " \
	" username = ? AND" \
	" secret = ? LIMIT 1"

#endif //AUTH_INCLUDE_GAURD