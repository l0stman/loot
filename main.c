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
  int mode = inter;

  envp = newenv();
  install("null", &null, envp);
  for (p = primlist; p < primlist+psiz; p++)
	install((*p)->label, proc(*p), envp);
  
  /* load the library */
  if ((pref = getenv(PREFIX)) == NULL) {
	warnx("%s not defined, can't load library", PREFIX);
	return envp;
  }
  snprintf(buf, BUFSIZ, "%s/%s", pref, LOOTRC);
  if ((fp = fopen(buf, "r")) == NULL) {
	warnx("Can't open %s", buf);
	return envp;
  }
  if (fgets(buf, BUFSIZ, fp) != NULL) {
	inter = 0;
	load(buf, envp);
	inter = mode;
  }
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
		exit(1);
  } else {
	load(NULL, envp);
	putchar('\n');
  }
  exit(0);
}
