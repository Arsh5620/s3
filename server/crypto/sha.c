#include "sha.h"

uchar *
sha_256_compute (string_s string, uchar *buffer, long max_len)
{
    if (max_len < SHA256LENGTH)
    {
        return (NULL);
    }
    uchar sha256[SHA256_DIGEST_LENGTH] = {0};
    uchar *hash = SHA256 ((uchar *) string.address, string.length, sha256);
    sha_256_copy_hex (buffer, sha256);
    return (hash);
}

void
sha_256_copy_hex (uchar *dest, uchar *source)
{
    for (long i = 0; i < SHA256LENGTH / 2; ++i)
    {
        *(dest + i * 2) = (*(source + i) >> 4) & 0xF;
        *(dest + i * 2 + 1) = *(source + i) & 0xF;
    }

    for (long i = 0; i < SHA256LENGTH; ++i)
    {
        *(dest + i) = sha_int_to_hex (*(dest + i));
    }
}

uchar inline sha_int_to_hex (uchar unit)
{
    if (unit < 10)
    {
        return (unit + '0');
    }
    else if (unit < 16)
    {
        return (unit - 10 + 'a');
    }
    return ('0');
}