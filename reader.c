#include "loot.h"
#include "reader.h"

const excpt_t read_error = { "reader error" };

/* skip spaces in the input stream. */
static inline void
skip_spa(FILE *fp)
{
        register int c;

        while ((c = fgetc(fp)) != EOF && isspace(c))
                if (c == '\n')
                        ++linenum;
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
                ++linenum;
}

/* skip blanks and comments from fp. */
static void
skip(FILE *fp)
{
        register int c;

        while ((c = fgetc(fp)) != EOF) {
                if (isspace(c)) {
                        if (c == '\n')
                                ++linenum;
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
        buf_t *exp;

        skip(fp);
        switch (c = fgetc(fp)) {
        case EOF:
                exp = NULL;
                break;
        case '(':     /* compound expression */
                skip(fp);
                exp = read_pair(fp);
                break;
        case ')':
                RAISE(read_error, "unexpected )");
                break;
        case '\'':/* quoted expression */
                exp = read_quote(fp);
                break;
        default:      /* atom */
                exp = read_atm(fp, c);
                break;
        }
        return exp;
}

/* Read a pair from fp and write to buf. */
static buf_t *
read_pair(FILE *fp)
{
        int c, ln;
        buf_t *bp, *exp;

        ln = linenum;
        bp = binit();
        bputc('(', bp);
        while ((c = fgetc(fp)) != EOF && c != ')') {
                if (c == '\n')
                        ++linenum;
                if (!isspace(c))
                        ungetc(c, fp);
                exp = read(fp);
                if (bp->len > 1 && !issep(*exp->buf))
                        bputc(' ', bp);
                bwrite(bp, exp->buf, exp->len);
                bfree(exp);
                skip(fp);
        }
        if (c == EOF)
                raise(&read_error, filename, ln, "too many open parenthesis");
        else
                bputc(')', bp);

#ifdef DEBUG_READER
        printf("%.*s", bp->len, bp->buf);
#endif

        return bp;
}

/* Read an atom from fp and write to buf. */
static buf_t *
read_atm(FILE *fp, int ch)
{
        int ln = linenum, c = ch;
        buf_t *bp;

        bp = binit();
        do {
                bputc(c, bp);
        } while ((c = fgetc(fp)) != EOF && !isstop(ch, c));
        if (ch == '"')
                if (c == EOF)
                        raise(&read_error, filename, ln, "unmatched quote");
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
        buf_t *bp, *res;

        bp = read(fp);

        res = binit();
        bwrite(res, "(quote", 6);
        if (!issep(*bp->buf))
                bputc(' ', res);
        bwrite(res, bp->buf, bp->len);
        bputc(')', res);
        bfree(bp);
        return res;
}
