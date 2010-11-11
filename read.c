#include "extern.h"
#include "exp.h"
#include "read.h"
#include "type.h"

const excpt_t read_error = { "read" };

static exp_t *read_atm(char);
static exp_t *read_char(void);
static exp_t *read_sharp(void);
static exp_t *read_rparen(void);
static exp_t *read_dot(void);
static exp_t *read_str(void);
static exp_t *read_pair(unsigned);
static exp_t *read_quote(unsigned);
static exp_t *read_qquote(unsigned);
static exp_t *read_comma(unsigned);

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
getch(void)
{
        register char c;

        for (;;) {
                while (isspace(c = sgetchar()))
                        ;
                if (c != ';')
                        break;
                while ((c = sgetchar()) != '\n') /* comment */
                        ;
        }
        return c;
}

/* Position of the current top-level expression. */
unsigned topexplin;
unsigned topexpcol;

#define eoferr() 		RAISE(read_error, "unexpected end of file")
#define doterr(line, col)	raise(&read_error, instream->name, line, col, \
                                      "Illegal use of .")

/*
 * Read an expression from the input stream.
 */
static inline exp_t *
read0(unsigned level)
{
        exp_t *(*read_syn)();
        unsigned char c;

        c = getch();
        if (level == 0) {
                topexplin = instream->line;
                topexpcol = instream->col;
        }
        return (c < NELEMS(stab) && (read_syn = stab[c]) != NULL ?
                read_syn(level) : read_atm(c));
}

/* External interface to read. Use read0 internally. */
exp_t *
read(void)
{
        return read0(0);
}

/* Read a pair expression from the input stream. */
static exp_t *
read_pair(unsigned level)
{
        exp_t *car, *cdr;
        char c;

        TRY
                if ((c = getch()) == ')')
                        RETURN(null);
                sungetch(c);
                car = read0(level+1);
                cdr = NULL;
                if ((c = getch()) == '.') {
                        unsigned dotline, dotcol;
                        dotline = instream->line;
                        dotcol  = instream->col;
                        if (issep(c = sgetchar())) {
                                cdr = read0(level+1);
                                if (getch() != ')')
                                        doterr(dotline, dotcol);
                        } else {
                                sungetch(c);
                                c = '.';
                        }
                }
                if (!cdr) {
                        sungetch(c);
                        cdr = read_pair(level);
                }
        CATCH(eof_error)
                RAISE1(read_error, "too many open parenthesis");
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

/* Read an atom from the input stream. */
static exp_t *
read_atm(char c)
{
        buf_t *bp;
        exp_t *ep;

        bp = binit();
        TRY
                do
                        bputc(tolower(c), bp);
                while (!issep(c = sgetchar()));
        CATCH(eof_error);
        ENDTRY;
        if (!feof(instream->fp))
                sungetch(c);

        ep = parse_atm(bp->buf, bp->len);
        bfree(bp);
        return ep;
}

/* Read a string from the input stream.*/
static exp_t *
read_str(void)
{
        register char c;
        unsigned line, col;
        buf_t *bp;
        exp_t *ep;

        bp   = binit();
        line = instream->line;
        col  = instream->col;
        TRY
                while ((c = sgetchar()) != '"')
                        bputc(c, bp);
        CATCH(eof_error)
                raise(&read_error,instream->name,line,col,"unmatched quote");
        ENDTRY;

        ep = nstr(bp->buf, bp->len);
        bfree(bp);
        return ep;
}

static inline exp_t *
enclose(enum kindex ki, unsigned level)
{
        exp_t *ep;

        TRY
                ep = read0(level+1);
        CATCH(eof_error)
                eoferr();
        ENDTRY;
        return cons(keywords[ki], cons(ep, null));
}

/* Read a quote expression. */
static exp_t *
read_quote(unsigned level)
{
        return enclose(QUOTE, level);
}

/* Read a quasi-quote expression. */
static exp_t *
read_qquote(unsigned level)
{
        return enclose(QQUOTE, level);
}

/* Read a comma expression. */
static exp_t *
read_comma(unsigned level)
{
        char c;
        exp_t *exp;

        TRY
                if ((c = sgetchar()) != '@') {
                        sungetch(c);
                        exp = enclose(UNQUOTE, level);
                } else
                        exp = enclose(SPLICE, level);
        CATCH(eof_error)
                eoferr();
        ENDTRY;
        return exp;
}

/* Read a sharp expression. */
static exp_t *
read_sharp(void)
{
        exp_t *exp;
        char c, ch;

        TRY
                switch (c = sgetchar()) {
                case 't':
                case 'f':
                        if (!issep(ch = sgetchar()))
                                RAISE(read_error, "bad syntax #%c...", c);
                        sungetch(ch);
                        exp = (c == 't' ? true : false);
                        break;
                case '\\':      /* character? */
                        exp = read_char();
                        break;
                default:
                        RAISE(read_error, "bad syntax #%c", c);
                        break;
                }
        CATCH(eof_error)
                eoferr();
        ENDTRY;

        return exp;
}

/* Read a character from the input stream. */
static exp_t *
read_char()
{
        buf_t *bp;
        exp_t *exp;
        register char c;
        unsigned line, col;

        bp = binit();
        line = instream->line;
        col  = instream->col;

        TRY
                while (!issep(c = sgetchar()))
                        bputc(c, bp);
        CATCH(eof_error)
                eoferr();
        ENDTRY;
        sungetch(c);

        if (bp->len == 1 && isprint(bp->buf[0]))
                exp = nchar(bp->buf[0]);
        else if (bp->len == 7 && !strncmp("newline", bp->buf, 7))
                exp = nchar('\n');
        else if (bp->len == 5 && !strncmp("space", bp->buf, 5))
                exp = nchar(' ');
        else
                raise(&read_error, instream->name, line, col,
                      "bad character constant #\\.*%s", bp->buf, bp->len);
        bfree(bp);
        return exp;
}

/* Read a right parenthesis. */
static exp_t *
read_rparen()
{
        RAISE(read_error, "unexpected )");
        return NULL;            /* not reached */
}

/* Read a dot expression. */
static exp_t *
read_dot()
{
        char c;
        unsigned dotline, dotcol;

        dotline = instream->line;
        dotcol  = instream->col;
        TRY
                if (issep(c = sgetchar()))
                        doterr(dotline, dotcol);
        CATCH(eof_error)
                doterr(dotline, dotcol);
        ENDTRY;
        sungetch(c);
        return read_atm('.');
}
