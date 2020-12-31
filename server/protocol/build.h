#define __STRING_HACK(x) #x
#define __STRINGIFY(x) __STRING_HACK (x)
#define __VERSION_MAJOR__ 0
#define __VERSION_MINOR__ 2

#include "micro"
#define __BUILT_BY__ "Arsh"

#define __SERVER_VERSION__                                                                         \
    __STRINGIFY (__VERSION_MAJOR__)                                                                \
    "." __STRINGIFY (__VERSION_MINOR__) "." __STRINGIFY (__VERSION_MICRO__)

#define __SERVER_INFO__ "compiled by " __BUILT_BY__ " on [" __DATE__ ", " __TIME__ "]"

#define __COPYRIGHT__ "The Unlicense https://unlicense.org/"