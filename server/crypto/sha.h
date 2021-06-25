#ifndef SHA_INCLUDE_GAURD
#define SHA_INCLUDE_GAURD
#include "../general/string.h"
#include <openssl/sha.h>

#define SHA256LENGTH 256 / 4 // reason we are dividing by 4 is to use hex

uchar *
sha_256 (char *string, size_t length, uchar *buffer, long max_len);
void
sha_256_hex (uchar *dest, uchar *source);
uchar
sha_tohex (uchar digit);
#endif // SHA_INCLUDE_GAURD