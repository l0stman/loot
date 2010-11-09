#ifndef READ_H
#define READ_H

#include "stream.h"

#define UNGETC(c, fp)	do {                            \
                if (c != EOF && ungetc(c, fp) == EOF)   \
                        err_sys("ungetc");              \
        } while (0)

extern const excpt_t read_error;
extern exp_t *read(stream *);

#endif /* !READ_H */
