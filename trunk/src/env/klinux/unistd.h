#define __KERNEL_SYSCALLS__
#include <asm/param.h>
#include <linux/unistd.h>
#include <linux/sched.h>
#define SEEK_SET 0

static inline unsigned int sleep(unsigned int secs)
{
  set_current_state(TASK_UNINTERRUPTIBLE);
  schedule_timeout(HZ * secs);
  return 0;
}
