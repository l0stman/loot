#ifndef READ_H
#define READ_H

#define UNGETC(c, fp)	do {                            \
                if (c != EOF && ungetc(c, fp) == EOF)   \
                        err_sys("ungetc");              \
        } while (0)

extern const excpt_t read_error;
extern exp_t *read(FILE *fp);

#endif /* !READ_H */
