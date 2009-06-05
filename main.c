#include "loot.h"
#include "exp.h"
#include "env.h"
#include "prim.h"

#define PSIZ	sizeof(plst)/sizeof(plst[0])

/* List of primitive procedures */
static struct {
  char *n;
  exp_t *(*pp)();
} plst[] = {
  /* arimthmetic */
  {"+", prim_add},
  {"-", prim_sub},
  {"*", prim_prod},
  /* pair */
  {"cons", prim_cons},
  {"car", prim_car},
  {"cdr", prim_cdr},
  /* test */
  {"eq?", prim_eq},
  {"symbol?", prim_sym},
  {"pair?", prim_pair},
  /* misc */
  {"eval", prim_eval},
  {"load", prim_load},
};

static env_t *initenv(void);

int
main(int argc, char *argv[])
{
  env_t *envp;
	
  envp = initenv();
  if (--argc) {
	inter = 0;	/* Non interactive mode */
	while (argc--)
	  if (load(*++argv, envp))
		exit(EXIT_FAILURE);
  } else {
	load(NULL, envp);
	putchar('\n');
  }
  exit(EXIT_SUCCESS);
}

/* Initialize the global environment */
static env_t *
initenv(void)
{
  env_t *envp;
  char buf[BUFSIZ], *pref;
  FILE *fp;
  int mode = inter, ret, i;

  envp = newenv();
  for (i = 0; i < PSIZ; i++)
	install(plst[i].n, proc(prim(plst[i].n, plst[i].pp)), envp);
  
  /* load the library */
  if ((ret = (pref = getenv(PREFIX)) != NULL)) {
	snprintf(buf, BUFSIZ, "%s/%s", pref, LOOTRC);
	if ((ret = (fp = fopen(buf, "r")) != NULL))
	  fgets(buf, BUFSIZ, fp);
  } else
	warnx("environment variable %s not defined", PREFIX);
  if (!ret)
	snprintf(buf, BUFSIZ, "%s", LIBNAM);
  inter = 0;
  load(buf, envp);
  inter = mode;
  return envp;
}
