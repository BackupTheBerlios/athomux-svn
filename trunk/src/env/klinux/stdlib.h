#include <linux/kernel.h>
#include <linux/vmalloc.h>
#define printf(args...) athomux_printf(args)
extern int athomux_printf(const char *fmt, ...);
#define malloc vmalloc
#define free vfree
#define exit(r) panic("exit(%i) not implemented!",r)
#define abort() panic("abort() not implemented!")
