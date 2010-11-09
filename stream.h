#ifndef STREAM_H
#define STREAM_H

typedef struct stream {
        char     *name;
        FILE     *fp;
        unsigned  line;
        unsigned  col;
} stream;

extern stream *sstdin;
extern const excpt_t  eof_error;

stream *nstream(char *, FILE *);
stream *sopen(const char *path);
void    sclose(stream *);

/* Get a character from the input stream. */
static inline char
sgetc(stream *sp)
{
        int c;

        if ((c = fgetc(sp->fp)) == EOF)
                raise(&eof_error, sp->name, sp->line, "end of file");
        if (c == '\n') {
                sp->line++;
                sp->col = 0;
        } else
                sp->col++;

        return c;
}

/* Push-back a non white-space character into the input stream. */
static inline void
sungetc(int c, stream *sp)
{
        if (!isspace(c)) {
                if (ungetc(c, sp->fp) == EOF)
                        err_sys("sungetc");
                sp->col--;
        }
}

#endif  /* !STREAM_H */
