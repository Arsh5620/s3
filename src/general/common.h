#ifndef SRC_GENERAL_COMMON_
#define SRC_GENERAL_COMMON_

#define FAILED 1
#define SUCCESS 0

#define FALSE 0
#define TRUE 1

#define ZERO 0

#define KB(x) (x * 1024)
#define MB(x) (KB (x) * 1024)

#define NETWORK_QUEQUE 16
#define NETWORK_PORT 5665
#define APP_NAME "s3"
#define SHA256_HEXLENGTH 64
#define SHA256_LENGTH 32

#define max(x, y) x > y ? x : y
#define min(x, y) x < y ? x : y

typedef char boolean;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

uchar
uchar_to_hex (uchar digit);
void
string_hexify (uchar *dest, uchar *source, long strlen);

#endif // SRC_GENERAL_COMMON_