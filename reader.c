#include "extern.h"
#include "exp.h"
#include "reader.h"
#include "type.h"

const excpt_t read_error = { "read" };

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

        while ((c = fgetc(fp)) != EOF)
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

/*
 * Read an expression from a file descriptor skipping blanks and comments.
 */

static exp_t *read_atm(FILE *, int);
static exp_t *read_pair(FILE *);
static exp_t *read_quote(FILE *);

exp_t *
read(FILE *fp)
{
        int c;
        exp_t *exp;

        skip(fp);
        switch (c = fgetc(fp)) {
        case EOF:
                exp = NULL;
                break;
        case '(':               /* compound expression */
                exp = read_pair(fp);
                break;
        case ')':
                RAISE(read_error, "unexpected )");
                break;
        case '\'':              /* quoted expression */
                exp = read_quote(fp);
                break;
        case '.':
                if ((c = fgetc(fp)) == EOF || issep(c))
                        RAISE(read_error, "Illegal use of .");
                ungetc(c, fp);
                c = '.';
        default:                /* atom */
                exp = read_atm(fp, c);
                break;
        }
        return exp;
}

/* Return the next non-blank character from fp. */
static inline int
nextc(FILE *fp, int ln)
{
        skip(fp);
        if (feof(fp))
                raise(&read_error, filename, ln, "too many open parenthesis");
        return fgetc(fp);
}

#define doterr(line)	raise(&read_error, filename, line, "Illegal use of .")

/* Read a pair expression from fp. */
static exp_t *
read_pair(FILE *fp)
{
        int c, ln;
        exp_t *car, *cdr;

        ln = linenum;
        if ((c = nextc(fp, ln)) == ')')
                return null;
        ungetc(c, fp);
        car = read(fp);
        cdr = NULL;
        if ((c = nextc(fp, ln)) == '.')
                if (issep(c = fgetc(fp))) {
                        cdr = read(fp);
                        if (nextc(fp, ln) != ')')
                                doterr(ln);
                } else {
                        ungetc(c, fp);
                        c = '.';
                }
        if (!cdr) {
                UNGETC(c, fp);
                cdr = read_pair(fp);
        }

        return cons(car, cdr);
}

/* Parse a non-pair expression */
static exp_t *
parse_atm(char *s, int len)
{
        exp_t *ep;
        char *p, *endp;
        int n, d;

        p = sstrndup(s, len);
        if (isfloatstr(p, len))
                ep = nfloat(atof(p));
        else if (isratstr(p, len)) {
                n = strtol(p, &endp, 10);
                if (*endp == '/') {
                        d = strtol(++endp, NULL, 10);
                        ep = nrat(n, d);
                } else
                        ep = nfixnum(n);
        } else
                ep = atom(p);
        free(p);
        return ep;
}

/* Read an atom from fp and write to buf. */
static exp_t *
read_atm(FILE *fp, int ch)
{
        int ln = linenum, c = ch;
        buf_t *bp;
        exp_t *ep;

        bp = binit();
        do
                bputc(c, bp);
        while ((c = fgetc(fp)) != EOF && !isstop(ch, c));
        if (ch == '"')
                if (c == EOF)
                        raise(&read_error, filename, ln, "unmatched quote");
                else
                        bputc('"', bp);       /* writing the closing quote */
        else
                ungetc(c, fp);
        ep = parse_atm(bp->buf, bp->len);
        bfree(bp);
        return ep;
}

/* Transform 'exp to (quote exp). */
static exp_t *
read_quote(FILE *fp)
{
        exp_t *ep;

        if (!(ep = read(fp)))
                RAISE(read_error, "unexpected end of file");
        return cons(keywords[QUOTE], cons(ep, null));
}
