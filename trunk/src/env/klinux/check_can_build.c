#define DRIVER_DESC "ATHOMUX kernelspace build test"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

#include <asm/bitops.h>
#include <asm/current.h>
#include <asm/div64.h>
#include <asm/page.h>
#include <asm/semaphore.h>
#include <asm/uaccess.h>
#include <asm/unistd.h>
#include <errno.h>
#include <linux/bio.h>
#include <linux/blkdev.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/gfp.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>



int __init init_module(void)
{
  printk(DRIVER_DESC "\n");
  return(0);
}


void __exit cleanup_module(void)
{
  init_module();
}

MODULE_AUTHOR("Jens-Christian Korth");
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");
