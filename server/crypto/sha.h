#ifndef SHA_INCLUDE_GAURD
#define SHA_INCLUDE_GAURD
#include "../general/string.h"
#include <openssl/sha.h>

#define SHA256_LENGTH 256 / 8
#define SHA256_HEXLENGTH 256 / 4

// Use these functions when all of the message is not in memory (such as when buffering)
void
sha_256_init (SHA256_CTX *context);
void
sha_256_update (SHA256_CTX *context, uchar *memory, int length);
void
sha_256_finalize (SHA256_CTX *context, uchar *hex_value);

uchar *
sha_256 (uchar *hash, uchar *string, size_t length);

void
sha_256_hex (uchar *dest, uchar *source);
uchar
sha_uchar_to_hex (uchar digit);
#endif // SHA_INCLUDE_GAURD