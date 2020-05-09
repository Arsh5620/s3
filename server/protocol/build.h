#define __PRE_STRING(x) #x
#define __PRE_STRING2(x) __PRE_STRING(x)
#define __VERSION_MAJOR__	0
#define __VERSION_MINOR__	2
#include "micro"
#define __BUILD_AUTHOR__ "Arshdeep Singh (arsh5620@outlook.com)"

#define	__DBP_VERSION__\
	__PRE_STRING2(__VERSION_MAJOR__) "." \
	__PRE_STRING2(__VERSION_MINOR__) "." \
	"build_" __PRE_STRING2(__VERSION_MICRO__)

#define __DBP_INFO__ "this version of dbp was built by " \
	__BUILD_AUTHOR__ " on date (" __DATE__ ", " __TIME__ ")"

#define __DBP_COPYRIGHT_STATUS__	"GPL v2 copyright (c) Arshdeep Singh"