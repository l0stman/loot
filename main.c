#include "loot.h"
#include "exp.h"
#include "env.h"
#include "prim.h"

/* Initialize the global environment */
static struct env *
initenv(void)
{
  struct env *envp;
  struct proc **p;

  envp = newenv();
  install("null", &null, envp);
  for (p = primlist; p < primlist+psiz; p++)
	install((*p)->label, proc(*p), envp);
  return envp;
}

int
main(int argc, char *argv[])
{
  struct env *envp;
  
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
