#include "extern.h"

int   linenum  = 1;             /* line being read */
char *filename = NULL;          /* name of the file being read */

/* Safe malloc. */
void *
smalloc(size_t size)
{
        void *vp;

        if ((vp = malloc(size)) == NULL)
                err_sys("Not enough memory");
        return vp;
}

/* Safe calloc */
void *
scalloc(size_t num, size_t size)
{
        void *vp;

        if ((vp = calloc(num, size)) == NULL)
                err_sys("Not enough memory");
        return vp;
}

/* Safe realloc */
void *
srealloc(void *ptr, size_t size)
{
        void *vp;

        if ((vp = realloc(ptr, size)) == NULL)
                err_sys("Not enough memory");
        return vp;
}

/* Safe strdup */
char *
sstrdup(const char *s)
{
        char *d;

        d = smalloc(strlen(s)+1);
        strcpy(d, s);
        return d;
}

/* Safe strndup */
char *
sstrndup(const char *s, size_t n)
{
        char *d;

        d = smalloc(n+1);
        strncpy(d, s, n);
        *(d+n) = '\0';
        return d;
}
