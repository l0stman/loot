#include "extern.h"
#include "exp.h"
#include "env.h"
#include "prim.h"

static env_t *initenv(void);
const char *progname;

int
main(int argc, char *argv[])
{
        env_t *envp;

        progname = sstrdup(basename(argv[0]));
        envp = initenv();
        if (--argc) {
                while (argc--)
                        if (load(*++argv, envp, NINTER))
                                exit(EXIT_FAILURE);
        } else {
                load(NULL, envp, INTER);
                putchar('\n');
        }
        free((void *)progname);
        exit(EXIT_SUCCESS);
}

/* Initialize the global environment */
static env_t *
initenv(void)
{
        env_t *envp;
        char buf[BUFSIZ], *pref;
        FILE *fp;
        int ret;

        initkeys();
        envp = newenv();
        instcst(envp);
        instprim(envp);

        /* load the library */
        if ((ret = (pref = getenv(PREFIX)) != NULL)) {
                snprintf(buf, BUFSIZ, "%s/%s", pref, LOOTRC);
                if ((ret = (fp = fopen(buf, "r")) != NULL))
                        fgets(buf, BUFSIZ, fp);
        } else
                warnx("environment variable %s not defined", PREFIX);
        if (!ret)
                snprintf(buf, BUFSIZ, "%s", LIBNAM);
        load(buf, envp, NINTER);

        return envp;
}
