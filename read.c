#include "extern.h"
#include "exp.h"
#include "read.h"
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

static exp_t *read_atm(FILE *, int);
static exp_t *read_pair(FILE *);
static exp_t *read_quote(FILE *, exp_t *);
static exp_t *read_char(FILE *);

#define doterr(line)	raise(&read_error, filename, line, "Illegal use of .")
#define eoferr()	RAISE(read_error, "unexpected end of file")
#define readerr(fmt, s) raise(&read_error, filename, linenum, fmt, s)

/*
 * Read an expression from a file descriptor skipping blanks and comments.
 */

exp_t *
read(FILE *fp)
{
        int c, ch;
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
                exp = read_quote(fp, keywords[QUOTE]);
                break;
        case '`':
                exp = read_quote(fp, keywords[QQUOTE]);
                break;
        case ',':
                switch (c = fgetc(fp)) {
                case EOF:
                        eoferr();
                        break;
                case '@':
                        exp = read_quote(fp, keywords[SPLICE]);
                        break;
                default:
                        ungetc(c, fp);
                        exp = read_quote(fp, keywords[UNQUOTE]);
                        break;
                }
                break;
        case '#':
                switch (c = fgetc(fp)) {
                case EOF:
                        eoferr();
                        break;
                case 't':
                case 'f':
                        if ((ch = fgetc(fp)) == EOF)
                                eoferr();
                        if (!issep(ch))
                                readerr("bad syntax #%c...", ch);
                        ungetc(ch, fp);
                        exp = (c == 't' ? true : false);
                        break;
                case '\\':      /* character? */
                        exp = read_char(fp);
                        break;
                default:
                        readerr("bad syntax #%c", c);
                        break;
                }
                break;
        case '.':
                if ((c = fgetc(fp)) == EOF || issep(c))
                        doterr(linenum);
                ungetc(c, fp);
                c = '.';
        default:                /* atom */
                exp = read_atm(fp, c);
                break;
        }
        return exp;
}

/* Read a character from fp. */
static exp_t *
read_char(FILE *fp)
{
        buf_t *bp;
        exp_t *exp;
        register int c;

        if ((c = fgetc(fp)) == EOF)
                eoferr();
        bp = binit();
        do
                bputc(c, bp);
        while ((c = fgetc(fp)) != EOF && !issep(c));
        ungetc(c, fp);
        if (bp->len == 1 && isprint(bp->buf[0]))
                exp = nchar(bp->buf[0]);
        else if (bp->len == 7 &&
                 !strncmp("newline", bp->buf, 7))
                exp = nchar('\n');
        else if (bp->len == 5 &&
                 !strncmp("space", bp->buf, 5))
                exp = nchar(' ');
        else {
                bputc('\0', bp);
                readerr("bad character constant #\\%s",
                        bp->buf);
        }
        bfree(bp);
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
                bputc(tolower(c), bp);
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

/* Enclose the following expression exp into (keyword exp). */
static exp_t *
read_quote(FILE *fp, exp_t *keyword)
{
        exp_t *ep;

        if (!(ep = read(fp)))
                eoferr();
        return cons(keyword, cons(ep, null));
}
