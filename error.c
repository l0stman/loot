#include <err.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "error.h"

/*
 * Fatal error unrelated to a system call.
 * Print a message and terminate.
 */
void
err_quit(const char *fmt, ...)
{
  char buf[MAXLINE];
  va_list ap;

  va_start(ap, fmt);
  vsnprintf(buf, MAXLINE, fmt, ap);
  errx(EXIT_FAILURE, "%s", buf);
  va_end(ap);
}

/*
 * Fatal error related to a system call.
 * Print a message and terminate.
 */
void
err_sys(const char *fmt, ...)
{
  char buf[MAXLINE];
  va_list ap;

  va_start(ap, fmt);
  vsnprintf(buf, MAXLINE, fmt, ap);
  err(EXIT_FAILURE, "%s", buf);
  va_end(ap);
}
