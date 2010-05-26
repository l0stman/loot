#include "loot.h"
#include "reader.h"

/* skip spaces in the input stream. */
static inline void
skip_spa(FILE *fp)
{
        register int c;

        while ((c = fgetc(fp)) != EOF && isspace(c))
                if (c == '\n')
                        ++line;
        ungetc(c, fp);
}

/* skip the current line in the input stream. */
static inline void
skip_line(FILE *fp)
{
        register int c;

        while ((c = fgetc(fp)) != EOF && c != '\n')
                ;
        if (c == '\n')
                ++line;
}

/* skip blanks and comments from fp. */
static void
skip(FILE *fp)
{
        register int c;

        while ((c = fgetc(fp)) != EOF) {
                if (isspace(c)) {
                        if (c == '\n')
                                ++line;
                        skip_spa(fp);
                } else if (c == ';')    /* comment */
                        skip_line(fp);
                else {
                        ungetc(c, fp);
                        break;
                }
        }
}

/*
 * Read an expression from a file descriptor skipping blanks and comments.
 * Write the result into a buffer.
 */

static buf_t *read_atm(FILE *, int);
static buf_t *read_pair(FILE *);
static buf_t *read_quote(FILE *);

buf_t *
read(FILE *fp)
{
        int c;
        extern int inter;

        if (inter) {
                printf("%s", INPR);
                fflush(stdout);
        }
        skip(fp);
        switch (c = fgetc(fp)) {
        case EOF:
                return NULL;
                break;
        case '(':     /* compound expression */
                skip(fp);
                return read_pair(fp);
                break;
        case '\'':/* quoted experssion */
                return read_quote(fp);
                break;
        default:      /* atom */
                return read_atm(fp, c);
                break;
        }
}

/* Read a pair from fp and write to buf. */
static buf_t *
read_pair(FILE *fp)
{
        int c, ln, pn;
        buf_t *bp, *q;

        pn = 1;               /* number of open parenthesis */
        ln = line;

        bp = binit();
        bputc('(', bp);
        while ((c = fgetc(fp)) != EOF && pn) {
                if (isspace(c) || c == ';') {
                        ungetc(c, fp);
                        skip(fp);
                        if (!issep(c = fgetc(fp)))
                                bputc(' ', bp);
                        ungetc(c, fp);
                        continue;
                }
                switch (c) {
                case '(':
                        ++pn;
                        break;
                case ')':
                        --pn;
                        break;
                default:
                        break;
                }
                if (c == '\'') {        /* it's a quoted expression */
                        q = read_quote(fp);
                        bwrite(bp, q->buf, q->len);
                        bfree(q);
                } else
                        bputc(c, bp);
                if (issep(c) && pn)
                        skip(fp);
        }
        if (pn)
                err_quit("Too many open parenthesis at line %d.", ln);
        else
                ungetc(c, fp);
        return bp;
}

/* Read an atom from fp and write to buf. */
static buf_t *
read_atm(FILE *fp, int ch)
{
        int ln = line, c = ch;
        buf_t *bp;

        bp = binit();
        do {
                bputc(c, bp);
        } while ((c = fgetc(fp)) != EOF && !isstop(ch, c));
        if (ch == '"')
                if (c == EOF)
                        err_quit("Unmatched quote at line %d.", ln);
                else
                        bputc('"', bp);       /* writing the closing quote */
        else
                ungetc(c, fp);
        return bp;
}

/* Transform 'exp to (quote exp). */
static buf_t *
read_quote(FILE *fp)
{
        int mode = inter;
        buf_t *bp, *res;

        inter = 0;    /* Passing in non-interactive mode */
        bp = read(fp);
        inter = mode; /* Restore the previous mode */

        res = binit();
        bwrite(res, "(quote", 6);
        if (!issep(*bp->buf))
                bputc(' ', res);
        bwrite(res, bp->buf, bp->len);
        bputc(')', res);
        bfree(bp);
        return res;
}
