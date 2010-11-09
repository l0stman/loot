#include "extern.h"
#include "exp.h"
#include "read.h"
#include "type.h"

const excpt_t read_error = { "read" };

static exp_t *read_atm(stream *, char);
static exp_t *read_pair(stream *);
static exp_t *read_char(stream *);
static exp_t *read_quote(stream *);
static exp_t *read_qquote(stream *);
static exp_t *read_comma(stream *);
static exp_t *read_sharp(stream *);
static exp_t *read_rparen(stream *);
static exp_t *read_dot(stream *);
static exp_t *read_str(stream *);

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
        /* 34 "   */ read_str,
        /* 35 #   */ read_sharp,
        /* 36 $   */ NULL,
        /* 37 %   */ NULL,
        /* 38 &   */ NULL,
        /* 39 '   */ read_quote,
        /* 40 (   */ read_pair,
        /* 41 )   */ read_rparen,
        /* 42 *   */ NULL,
        /* 43 +   */ NULL,
        /* 44 ,   */ read_comma,
        /* 45 -   */ NULL,
        /* 46 .   */ read_dot,
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
        /* 96 `   */ read_qquote
};

/*
 * Return the next character from the input stream skipping comments
 * and white-spaces.
 */
static char
getch(stream *sp)
{
        register char c;

        for (;;) {
                while (isspace(c = sgetc(sp)))
                        ;
                if (c != ';')
                        break;
                while ((c = sgetc(sp)) != '\n') /* comment */
                        ;
        }
        return c;
}

#define eoferr()	RAISE(read_error, "unexpected end of file")
#define readerr(fmt, s) raise(&read_error, instream->name, instream->line, \
                              instream->col, fmt, s)

/*
 * Read an expression from an input stream.
 */
exp_t *
read(stream *sp)
{
        exp_t *(*read_syn)();
        unsigned char c;

        return (c = getch(sp)) < NELEMS(stab) && (read_syn = stab[c]) != NULL ?
                read_syn(sp) : read_atm(sp, c);
}

/* Read a pair expression from the input stream. */
static exp_t *
read_pair(stream *sp)
{
        exp_t *car, *cdr;
        char c;
        unsigned line, col;

        line = sp->line;
        col  = sp->col;
        TRY
                if ((c = getch(sp)) == ')')
                        RETURN(null);
                sungetc(c, sp);
                car = read(sp);
                cdr = NULL;
                if ((c = getch(sp)) == '.')
                        if (issep(c = sgetc(sp))) {
                                cdr = read(sp);
                                if (getch(sp) != ')')
                                        RAISE(read_error, "should be a )");
                        } else {
                                sungetc(c, sp);
                                c = '.';
                        }
                if (!cdr) {
                        sungetc(c, sp);
                        cdr = read_pair(sp);
                }
        CATCH(eof_error)
                raise(&read_error, sp->name, line, col,
                      "too many open parenthesis");
        ENDTRY;

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

/* Read an atom from sp. */
static exp_t *
read_atm(stream *sp, char c)
{
        buf_t *bp;
        exp_t *ep;

        bp = binit();
        TRY
                do
                        bputc(tolower(c), bp);
                while (!issep(c = sgetc(sp)));
        CATCH(eof_error);
        ENDTRY;
        sungetc(c, sp);

        ep = parse_atm(bp->buf, bp->len);
        bfree(bp);
        return ep;
}

/* Read a string from the input stream.*/
static exp_t *
read_str(stream *sp)
{
        register char c;
        unsigned line, col;
        buf_t *bp;
        exp_t *ep;

        bp   = binit();
        line = sp->line;
        col  = sp->col;
        TRY
                while ((c = sgetc(sp)) != '"')
                        bputc(c, bp);
        CATCH(eof_error)
                raise(&read_error, sp->name, line, col, "unmatched quote");
        ENDTRY;

        ep = nstr(bp->buf, bp->len);
        bfree(bp);
        return ep;
}

static inline exp_t *
enclose(stream *sp, enum kindex ki)
{
        exp_t *ep;

        TRY
                ep = read(sp);
        CATCH(eof_error)
                eoferr();
        ENDTRY;
        return cons(keywords[ki], cons(ep, null));
}

/* Read a quote expression. */
static exp_t *
read_quote(stream *sp)
{
        return enclose(sp, QUOTE);
}

/* Read a quasi-quote expression. */
static exp_t *
read_qquote(stream *sp)
{
        return enclose(sp, QQUOTE);
}

/* Read a comma expression. */
static exp_t *
read_comma(stream *sp)
{
        char c;
        exp_t *exp;

        TRY
                if ((c = sgetc(sp)) != '@') {
                        sungetc(c, sp);
                        exp = enclose(sp, UNQUOTE);
                } else
                        exp = enclose(sp, SPLICE);
        CATCH(eof_error)
                eoferr();
        ENDTRY;
        return exp;
}

/* Read a sharp expression. */
static exp_t *
read_sharp(stream *sp)
{
        exp_t *exp;
        char c, ch;

        TRY
                switch (c = sgetc(sp)) {
                case 't':
                case 'f':
                        if (!issep(ch = sgetc(sp)))
                                readerr("bad syntax #%c...", c);
                        sungetc(ch, sp);
                        exp = (c == 't' ? true : false);
                        break;
                case '\\':      /* character? */
                        exp = read_char(sp);
                        break;
                default:
                        readerr("bad syntax #%c", c);
                        break;
                }
        CATCH(eof_error)
                eoferr();
        ENDTRY;

        return exp;
}

/* Read a character from the input stream. */
static exp_t *
read_char(stream *sp)
{
        buf_t *bp;
        exp_t *exp;
        register char c;

        bp = binit();
        TRY
                while (!issep(c = sgetc(sp)))
                        bputc(c, bp);
        CATCH(eof_error)
                eoferr();
        ENDTRY;
        sungetc(c, sp);
        if (bp->len == 1 && isprint(bp->buf[0]))
                exp = nchar(bp->buf[0]);
        else if (bp->len == 7 && !strncmp("newline", bp->buf, 7))
                exp = nchar('\n');
        else if (bp->len == 5 && !strncmp("space", bp->buf, 5))
                exp = nchar(' ');
        else {
                bputc('\0', bp);
                readerr("bad character constant #\\%s", bp->buf);
        }
        bfree(bp);
        return exp;
}

/* Read a right parenthesis. */
static exp_t *
read_rparen(stream *sp)
{
        RAISE(read_error, "unexpected )");
        return NULL;            /* not reached */
}

#define doterr()	RAISE(read_error, "Illegal use of .")

/* Read a dot expression. */
static exp_t *
read_dot(stream *sp)
{
        char c;

        TRY
                if (issep(c = sgetc(sp)))
                        doterr();
        CATCH(eof_error)
                doterr();
        ENDTRY;
        sungetc(c, sp);
        return read_atm(sp, '.');
}
