#include <linux/string.h>
#define strerror(errnum) sprintf("Error %i",errnum)
