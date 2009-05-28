#include "loot.h"
#include "reader.h"

int
main(int argc, char *argv[])
{
  FILE *fp;
  struct buf *bp;

  if (argc > 1) {
	if ((fp = fopen(argv[1]), "r") == NULL)
	  err_sys("Can't open file %s", argv[1]);
	inter = 0;	/* Non interactive mode */
  } else
	fp = stdin;

  while ((bp = read(fp)) != NULL)
	printf("%s %s\n", OUTPR, bp->buf);
  exit(0);
}
