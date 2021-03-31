#ifndef _SYSCALL_WRAPPER_SYSCALLS_H
#define _SYSCALL_WRAPPER_SYSCALLS_H

#include <linux/kernel.h>

struct pt_regs;

asmlinkage long sys_open(const struct pt_regs *regs);
asmlinkage long sys_openat(const struct pt_regs *regs);

#endif
