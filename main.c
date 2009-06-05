#include "loot.h"
#include "exp.h"
#include "env.h"
#include "prim.h"

/* Initialize the global environment */
static env_t *
initenv(void)
{
  env_t *envp;
  proc_t **p;
  char buf[BUFSIZ], *pref;
  FILE *fp;
  int mode = inter, ret;

  envp = newenv();
  install("null", &null, envp);
  for (p = primlist; p < primlist+psiz; p++)
	install((*p)->label, proc(*p), envp);
  
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
