#ifndef ERROR_H
#define ERROR_H

#define MAXLINE 100     /* Maximum length of an error message. */

extern void err_quit(const char *, ...);
extern void err_sys(const char *, ...);

#endif /* !ERROR_H */
