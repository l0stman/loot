#include "extern.h"
#include "exp.h"
#include "env.h"
#include "prim.h"

static void initenv(void);
const char *progname;

int
main(int argc, char *argv[])
{
        progname = sstrdup(basename(argv[0]));
        initenv();
        if (--argc) {
                while (argc--)
                        if (load(*++argv, NINTER))
                                exit(EXIT_FAILURE);
        } else {
                load(NULL, INTER);
                putchar('\n');
        }
        free((void *)progname);

        exit(EXIT_SUCCESS);
}

/* Initialize the global environment */
static void
initenv(void)
{
        char buf[BUFSIZ], *pref;
        FILE *fp;
        int ret;

        initkeys();
        globenv = newenv();
        instcst(globenv);
        instprim(globenv);

        /* load the library */
        if ((ret = (pref = getenv(PREFIX)) != NULL)) {
                snprintf(buf, BUFSIZ, "%s/%s", pref, LOOTRC);
                if ((ret = (fp = fopen(buf, "r")) != NULL)) {
                        fgets(buf, BUFSIZ, fp);
                        fclose(fp);
                }
        } else
                warnx("environment variable %s not defined", PREFIX);
        if (!ret)
                snprintf(buf, BUFSIZ, "%s", LIBNAM);
        load(buf, NINTER);
}
