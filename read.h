#ifndef READ_H
#define READ_H

#include "stream.h"

extern const excpt_t read_error;
extern unsigned topexplin;
extern unsigned topexpcol;

extern exp_t *read(void);

#define RAISE1(e, ...)	raise(&(e), instream->name, topexplin, topexpcol, \
                              __VA_ARGS__)

#endif /* !READ_H */
