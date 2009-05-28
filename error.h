#ifndef ERROR_H
#define ERROR_H

#define MAXLINE	100	/* Maximum length of an error message. */

void err_quit(const char *, ...);
void err_sys(const char *, ...);

#endif /* !ERROR_H */
