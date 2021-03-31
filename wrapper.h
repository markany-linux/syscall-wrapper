#ifndef _SYSCALL_WRAPPER_WRAPPER_H
#define _SYSCALL_WRAPPER_WRAPPER_H

#include <linux/types.h>

struct pt_regs;

struct syscall_table {
  unsigned long **addr;
};

struct syscall_addr {
  long (*open)(const struct pt_regs *);
  long (*openat)(const struct pt_regs *);

  struct syscall_table table;
};

extern struct syscall_addr g_orig_syscalls;

bool toggle_wrap_syscalls(bool wrap);

#endif
