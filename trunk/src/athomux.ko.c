#define DRIVER_DESC "ATHOMUX subsystem for Linux"
#define ATHOMUX_BOOTSTRAP
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include "init.h"


static char *init = "athload_help";
static char *param = "";


int __init init_module(void)
{
   int result;
   printk("Loading " DRIVER_DESC "...\n");
   ATHOMUX_INIT_INIT();
   result = !ATHOMUX_INIT(init, param);
   if(result)
     printk(DRIVER_DESC " NOT loaded.\n");
   else
     printk(DRIVER_DESC " loaded.\n");
   return(result);
}


void __exit cleanup_module(void)
{
   printk("Unloading " DRIVER_DESC "...\n");
   ATHOMUX_CLEANUP(init, param);
   printk(DRIVER_DESC " unloaded.\n");
}


MODULE_AUTHOR("Jens-Christian Korth");
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");
module_param(init, charp, 0);
module_param(param, charp, 0);
MODULE_PARM_DESC(init, "name of the brick which is being loaded after bootstrapping");
MODULE_PARM_DESC(param, "parameter string for the brick referenced by init");
