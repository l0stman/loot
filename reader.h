#ifndef READER_H
#define READER_H

#define UNGETC(c, fp)	do {                            \
                if (c != EOF && ungetc(c, fp) == EOF)   \
                        err_sys("ungetc");              \
        } while (0)

extern const excpt_t read_error;
extern buf_t *read(FILE *fp);

#endif /* !READER_H */
