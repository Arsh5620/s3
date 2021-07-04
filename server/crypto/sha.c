#include "sha.h"

uchar *
sha_256 (uchar *hash, uchar *string, size_t length)
{
    SHA256 ((uchar *) string, length, hash);
    return hash;
}

void
sha_256_init (SHA256_CTX *context)
{
    SHA256_Init (context);
}

void
sha_256_update (SHA256_CTX *context, uchar *memory, int length)
{
    SHA256_Update (context, memory, length);
}

void
sha_256_finalize (SHA256_CTX *context, uchar *hex_value)
{
    uchar sha256[SHA256_LENGTH];
    SHA256_Final (sha256, context);

    sha_256_hex (hex_value, sha256);
}

void
sha_256_hex (uchar *dest, uchar *source)
{
    for (long i = 0; i < SHA256_HEXLENGTH / 2; ++i) {
        *(dest + i * 2) = (*(source + i) >> 4) & 0xF;
        *(dest + i * 2 + 1) = *(source + i) & 0xF;
    }

    for (long i = 0; i < SHA256_HEXLENGTH; ++i) {
        *(dest + i) = sha_uchar_to_hex (*(dest + i));
    }
}

uchar inline sha_uchar_to_hex (uchar digit)
{
    if (digit < 10) {
        return (digit + '0');
    } else if (digit < 16) {
        return (digit - 10 + 'a');
    }
    return ('0');
}