#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "wrapper.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MaBling");
MODULE_DESCRIPTION("A simple example Linux syscall wrapper");
MODULE_VERSION("0.0.1");

static int __init wrapper_module_init(void) {
  printk(KERN_INFO "Hello, World!\n");
  if (!toggle_wrap_syscalls(true)) return -1;
  return 0;
}

static void __exit wrapper_module_exit(void) {
  printk(KERN_INFO "Goodbye, World!\n");
  toggle_wrap_syscalls(false);
}

module_init(wrapper_module_init);
module_exit(wrapper_module_exit);
