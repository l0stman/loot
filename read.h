#ifndef READ_H
#define READ_H

#include "stream.h"

extern const excpt_t read_error;
extern unsigned topexplin;
extern unsigned topexpcol;

extern exp_t *read(stream *);

#endif /* !READ_H */
