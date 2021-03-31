#include "utils.h"

#include <linux/file.h>
#include <linux/fs.h>
#include <linux/sched/mm.h>
#include <linux/version.h>

char *get_proc_path(struct task_struct *task, char *buf, int buflen) {
  struct file *exe_file;
  char *path = ERR_PTR(-ENOENT);
  struct mm_struct *mm;

  mm = get_task_mm(task);
  if (!mm) goto out;

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 8, 0)
  down_read(&mm->mmap_sem);
#else
  down_read(&mm->mmap_lock);
#endif
  exe_file = mm->exe_file;
  if (exe_file) {
    get_file(exe_file);
    path_get(&exe_file->f_path);
  }
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 8, 0)
  up_read(&mm->mmap_sem);
#endif
  mmput(mm);

  if (exe_file) {
    path = d_path(&exe_file->f_path, buf, buflen);
    path_put(&exe_file->f_path);
    fput(exe_file);
  }

out:
  return path;
}

char *get_current_proc_path(char *buf, int buflen) {
  return get_proc_path(current, buf, buflen);
}
