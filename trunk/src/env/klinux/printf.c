//  parts taken from print_string.c and printk.c - both GPLed

#include <linux/kernel.h>
#include <linux/sched.h>    // For current
#include <linux/tty.h>      // For the tty declarations


int athomux_printf(const char *fmt, ...)
{
  va_list args;
  int printed_len;
  char *p;
  static char printf_buf[512];
  struct tty_struct *my_tty = NULL;

  if(likely(current->signal))
    my_tty = current->signal->tty;

  /* Emit the output into the temporary buffer */
  va_start(args, fmt);
  printed_len = vscnprintf(printf_buf, sizeof(printf_buf), fmt, args);
  va_end(args);

  /* If my_tty is NULL, the current task has no tty you can print to (this is possible,
   * for example, if it's a daemon).  If so, we use printk.
   */
  if(unlikely(my_tty == NULL)) {
    printk("%s", printf_buf);
  } else {
#if 0
    (*my_tty->driver->write)(
      my_tty,                 // The tty itself
      0,                      // We don't take the string from user space
      printf_buf,             // String
      strlen(printf_buf));    // Length
#else
    for (p = printf_buf; *p; p++) {
      if (unlikely(*p == '\n'))
	(*my_tty->driver->write)(my_tty, 0, "\015\012", 2);
      else
	(*my_tty->driver->write)(my_tty, 0, p, 1);
    }
#endif
  }
  return printed_len;
}
