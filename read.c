#include "extern.h"
#include "exp.h"
#include "read.h"
#include "type.h"

const excpt_t read_error = { "read" };

static exp_t *read_atm(FILE *, int);
static exp_t *read_pair(FILE *);
static exp_t *read_char(FILE *);
static exp_t *read_quote(FILE *);
static exp_t *read_qquote(FILE *);
static exp_t *read_splice(FILE *);
static exp_t *read_unquote(FILE *);

/* Syntax table. */
static exp_t *(*stab[128])() = {
        /* 0  NUL */ NULL,
        /* 1  SOH */ NULL,
        /* 2  STX */ NULL,
        /* 3  ETX */ NULL,
        /* 4  EOT */ NULL,
        /* 5  ENQ */ NULL,
        /* 6  ACK */ NULL,
        /* 7  BEL */ NULL,
        /* 8  BS  */ NULL,
        /* 9  HT  */ NULL,
        /* 10 NL  */ NULL,
        /* 11 VT  */ NULL,
        /* 12 NP  */ NULL,
        /* 13 CR  */ NULL,
        /* 14 SO  */ NULL,
        /* 15 SI  */ NULL,
        /* 16 DLE */ NULL,
        /* 17 DC1 */ NULL,
        /* 18 DC2 */ NULL,
        /* 19 DC3 */ NULL,
        /* 20 DC4 */ NULL,
        /* 21 NAK */ NULL,
        /* 22 SYN */ NULL,
        /* 23 ETB */ NULL,
        /* 24 CAN */ NULL,
        /* 25 EM  */ NULL,
        /* 26 SUB */ NULL,
        /* 27 ESC */ NULL,
        /* 28 FS  */ NULL,
        /* 29 GS  */ NULL,
        /* 30 RS  */ NULL,
        /* 31 US  */ NULL,
        /* 32 SP  */ NULL,
        /* 33 !   */ NULL,
        /* 34 "   */ NULL,
        /* 35 #   */ NULL,
        /* 36 $   */ NULL,
        /* 37 %   */ NULL,
        /* 38 &   */ NULL,
        /* 39 '   */ NULL,
        /* 40 (   */ read_pair,
        /* 41 )   */ NULL,
        /* 42 *   */ NULL,
        /* 43 +   */ NULL,
        /* 44 ,   */ NULL,
        /* 45 -   */ NULL,
        /* 46 .   */ NULL,
        /* 47 /   */ NULL,
        /* 48 0   */ NULL,
        /* 49 1   */ NULL,
        /* 50 2   */ NULL,
        /* 51 3   */ NULL,
        /* 52 4   */ NULL,
        /* 53 5   */ NULL,
        /* 54 6   */ NULL,
        /* 55 7   */ NULL,
        /* 56 8   */ NULL,
        /* 57 9   */ NULL,
        /* 58 :   */ NULL,
        /* 59 ;   */ NULL,
        /* 60 <   */ NULL,
        /* 61 =   */ NULL,
        /* 62 >   */ NULL,
        /* 63 ?   */ NULL,
        /* 64 @   */ NULL,
        /* 65 A   */ NULL,
        /* 66 B   */ NULL,
        /* 67 C   */ NULL,
        /* 68 D   */ NULL,
        /* 69 E   */ NULL,
        /* 70 F   */ NULL,
        /* 71 G   */ NULL,
        /* 72 H   */ NULL,
        /* 73 I   */ NULL,
        /* 74 J   */ NULL,
        /* 75 K   */ NULL,
        /* 76 L   */ NULL,
        /* 77 M   */ NULL,
        /* 78 N   */ NULL,
        /* 79 O   */ NULL,
        /* 80 P   */ NULL,
        /* 81 Q   */ NULL,
        /* 82 R   */ NULL,
        /* 83 S   */ NULL,
        /* 84 T   */ NULL,
        /* 85 U   */ NULL,
        /* 86 V   */ NULL,
        /* 87 W   */ NULL,
        /* 88 X   */ NULL,
        /* 89 Y   */ NULL,
        /* 90 Z   */ NULL,
        /* 91 [   */ NULL,
        /* 92 \   */ NULL,
        /* 93 ]   */ NULL,
        /* 94 ^   */ NULL,
        /* 95 _   */ NULL,
        /* 96 `   */ NULL
};

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
                exp = read_quote(fp);
                break;
        case '`':
                exp = read_qquote(fp);
                break;
        case ',':
                switch (c = fgetc(fp)) {
                case EOF:
                        eoferr();
                        break;
                case '@':
                        exp = read_splice(fp);
                        break;
                default:
                        ungetc(c, fp);
                        exp = read_unquote(fp);
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
                                readerr("bad syntax #%c...", c);
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
static inline exp_t *
enclose(FILE *fp, exp_t *keyword)
{
        exp_t *ep;

        if (!(ep = read(fp)))
                eoferr();
        return cons(keyword, cons(ep, null));
}

/* Read a quote expression. */
static exp_t *
read_quote(FILE *fp)
{
        return enclose(fp, keywords[QUOTE]);
}

/* Read a quasi-quote expression. */
static exp_t *
read_qquote(FILE *fp)
{
        return enclose(fp, keywords[QQUOTE]);
}

/* Read an unquote expression. */
static exp_t *
read_unquote(FILE *fp)
{
        return enclose(fp, keywords[UNQUOTE]);
}

/* Read an unquote-splicing expression. */
static exp_t *
read_splice(FILE *fp)
{
        return enclose(fp, keywords[SPLICE]);
}
