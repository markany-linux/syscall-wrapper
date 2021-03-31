#include "wrapper.h"

#include <linux/unistd.h>
#include <linux/version.h>

#include "syscalls.h"

#define MAX_WRAP_TARGETS (16)

#define WRAP_OFFSET(m) (offsetof(struct syscall_addr, m))

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0)
#define REQUIRE_KPROBE (1)
#include <linux/kprobes.h>
static struct kprobe kp = {.symbol_name = "kallsyms_lookup_name"};
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
#define KALLSYMS (1)
#include <linux/kallsyms.h>
#else
#include <linux/syscalls.h>
#endif

struct wrap_data {
  int syscall;
  void *handler;
  off_t offset;
};

struct wrap_info {
  struct wrap_data targets[MAX_WRAP_TARGETS];
  struct syscall_table table;
};

static const struct wrap_info g_wrap_info = {
    {{__NR_open, sys_open, WRAP_OFFSET(open)},
     {__NR_openat, sys_openat, WRAP_OFFSET(openat)},
     {0}},
    {0}};

struct syscall_addr g_orig_syscalls = {0};

static void set_syscall_addr(unsigned long *addr, unsigned long **backup,
                             const struct syscall_table *table, int syscall) {
  if (table && addr) {
    if (backup) *backup = table->addr[syscall];
    table->addr[syscall] = addr;
  }
}

static void disable_page_protection(void) {
  unsigned long value;

  /// 1. 현재 CR0 레지스터의 값을 획득
  asm volatile("mov %%cr0,%0" : "=r"(value));

  /// 2. 현재 CR0 레지스터가 읽기 전용 설정 상태인지 확인
  ///		이미 쓰기 가능 상태라면 별도의 작업을 수행하지 않음
  if (value & 0x00010000) {
    /// 3. 읽기 전용을 쓰기 가능으로 변경하기 위해 읽기/쓰기 비트를 반전
    value &= ~0x00010000;
    /// 4. CR0 레지스터를 쓰기 가능으로 변경
    asm volatile("mov %0, %%cr0" : : "r"(value));
  }
}

static void enable_page_protection(void) {
  unsigned long value;

  /// 1. 현재 CR0 레지스터의 값을 획득
  asm volatile("mov %%cr0,%0" : "=r"(value));

  /// 2. 획득된 CR0 레지스터 값이 쓰기 가능 상태인지 확인
  ///		이미 읽기 전용으로 변경되어 있다면 별도의 수정이 필요하지 않음
  if (!(value & 0x00010000)) {
    /// 3. 쓰기 가능을 읽기 전용으로 변경하기 위한 비트 반전
    value |= 0x00010000;

    /// 4. CR0 레지스터를 일기 전용으로 변경
    asm volatile("mov %0,%%cr0" : : "r"(value));
  }
}

static bool get_syscall_table_addr(struct syscall_table *table) {
#ifdef KALLSYMS

#ifdef REQUIRE_KPROBE
  typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);
  kallsyms_lookup_name_t kallsyms_lookup_name;

  if (0 > register_kprobe(&kp)) {
    printf(KERN_ERR "Failed to register_kprobe() for kallsyms_lookup_name\n");
    return false;
  }

  kallsyms_lookup_name = (kallsyms_lookup_name_t)kp.addr;
  unregister_kprobe(&kp);
#endif  // #ifdef REQUIRE_KPROBE

  table->addr = (unsigned long **)kallsyms_lookup_name("sys_call_table");
#else   // #ifdef KALLSYMS
  long i = 0;
  unsigned long **tmp;

  for (i = (unsigned long)sys_close; i < ULONG_MAX; i += sizeof(void *)) {
    tmp = (unsigned long **)i;
    if (tmp[__NR_close] == (unsigned long *)sys_close) {
      table->addr = tmp;
      break;
    }
  }
#endif  // #ifdef KALLSYMS

  if (!table->addr) {
    printk(KERN_ERR "Failed to get syscall table address.\n");
    return false;
  }

  printk(KERN_INFO "Success to get syscall table: %p\n", table->addr);
  return true;
}

bool toggle_wrap_syscalls(bool wrap) {
  struct wrap_data *targets = (struct wrap_data *)&g_wrap_info.targets;
  struct syscall_table *table = &g_orig_syscalls.table;
  int i;
  unsigned long **addr;

  if (!table->addr && !get_syscall_table_addr(table)) return false;

  disable_page_protection();
  for (i = 0; i < MAX_WRAP_TARGETS; ++i) {
    if (!targets[i].handler) break;

    addr = ((void *)&g_orig_syscalls) + targets[i].offset;
    if (wrap) {
      if (*addr) continue;
      set_syscall_addr(targets[i].handler, addr, table, targets[i].syscall);
    } else {
      if (!*addr) continue;
      set_syscall_addr(*addr, NULL, table, targets[i].syscall);
    }

    if (!wrap) *addr = NULL;
  }
  enable_page_protection();

  if (wrap) {
    printk(KERN_INFO "Success to wrap syscalls. (%d syscalls)\n", i);
  } else {
    printk(KERN_INFO "Success to unwrap syscalls.\n");
  }

  return true;
}
