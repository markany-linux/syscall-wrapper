#include "syscalls.h"

#include <linux/limits.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#include "utils.h"
#include "wrapper.h"

#define FD_START (3)

static void print_target_proc_action(const char *filename) {
  const char kTargetProc[] = "/home/swma/develop/simple-test/build/simple-test";
  char buf[PATH_MAX];
  char *proc_path = get_current_proc_path(buf, PATH_MAX);

  if (!IS_ERR(proc_path) && !strcmp(kTargetProc, proc_path)) {
    printk(KERN_INFO "Target proccess file access: %s\n", proc_path);
    if (!strncpy_from_user(buf, filename, PATH_MAX)) {
      printk(KERN_INFO "filename: %s\n", buf);
    }
  }
}

asmlinkage long sys_open(const struct pt_regs *regs) {
  int fd;

  fd = g_orig_syscalls.open(regs);
  if (FD_START > fd) return fd;

  print_target_proc_action((char *)regs->di);
  return fd;
}

asmlinkage long sys_openat(const struct pt_regs *regs) {
  int fd;

  fd = g_orig_syscalls.openat(regs);
  if (FD_START > fd) return fd;

  print_target_proc_action((char *)regs->si);
  return fd;
}
