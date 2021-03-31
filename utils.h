#ifndef _SYSCALL_WRAPPER_UTILS_H
#define _SYSCALL_WRAPPER_UTILS_H

struct task_struct;

char *get_proc_path(struct task_struct *task, char *buf, int buflen);
char *get_current_proc_path(char *buf, int buflen);

#endif
