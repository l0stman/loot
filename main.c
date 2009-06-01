#include "loot.h"
#include "exp.h"
#include "env.h"
#include "reader.h"
#include "parser.h"
#include "eval.h"
#include "prim.h"

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

static void
print(struct exp *ep)
{
  char *s;

  printf("%s%s\n", OUTPR, s = tostr(ep));
  free(s);
}

int
main(int argc, char *argv[])
{
  FILE *fp;
  struct buf *bp;
  struct exp *ep;
  struct env *envp;

  if (argc > 1) {
	if ((fp = fopen(argv[1], "r")) == NULL)
	  err_sys("Can't open file %s", argv[1]);
	inter = 0;	/* Non interactive mode */
  } else
	fp = stdin;
  
  envp = initenv();
  while ((bp = read(fp)) != NULL) {
	ep = eval(parse(bp->buf, bp->len), envp);
	if (inter && ep != NULL)
	  print(ep);
	bfree(bp);
  }
  putchar('\n');
  exit(0);
}
