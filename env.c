#include "loot.h"
#include "exp.h"
#include "env.h"

/* inspired by K&R */

/* hash: form hash value for string s */
static unsigned
hash(char *s, size_t hashsize)
{
  unsigned hashval;

  for (hashval = 0; *s != '\0'; s++)
	hashval = *s + 31 * hashval;
  return hashval % hashsize;
}

/* find: look for s in frame */
static struct nlist *
find(char *s, struct frame *fp)
{
  struct nlist *np;

  for (np = fp->bucket[hash(s, fp->size)]; np != NULL; np = np->next)
	if (strcmp(s, np->name) == 0)
	  return np;	/* found */
  return NULL;		/* not found */
}

/* lookup: look for s in the environment */
struct nlist *
lookup(char *s, struct env *ep)
{
  struct nlist *np;

  for ( ; ep != NULL; ep = eenv(ep))
	if ((np = find(s, fframe(ep))) != NULL)
	  return np;	/* found */
  return NULL;	/* not found */
}
	
/* install: put (name, defn) in the environment */
struct nlist *
install(char *name, struct exp *defn, struct env *ep)
{
  struct nlist *np;
  struct frame *fp;
  unsigned hashval;
  
  fp = fframe(ep);
  if ((np = find(name, fp)) == NULL) {	/* not found */
	np = smalloc(sizeof(*np));
	if (np == NULL || (np->name = sstrdup(name)) == NULL)
	  return NULL;
	hashval = hash(name, fp->size);
	np->next = fp->bucket[hashval];
	fp->bucket[hashval] = np;
  }
  np->defn = defn;
  return np;
}

/* undef: remove the entry corresponding to name in frame */
void
undef(char *s, struct frame *fp)
{
  struct nlist *np;
  struct nlist *prev;
  unsigned hashval;

  hashval = hash(s, fp->size);
  np = fp->bucket[hashval];
  prev = NULL;

  while (np != NULL) {
	if (strcmp(s, np->name) == 0) {	/* found */
	  if (prev == NULL)	/* we're at the beginning */
		fp->bucket[hashval] = np->next;
	  else
		prev->next = np->next;
	  free(np->name);
	  free(np);
	  return;
	}
	prev = np;
	np = np->next;
  }
}

/* newframe: return a new frame pointer */
struct frame *
newframe(void)
{
  struct frame *fp;
  
  fp = smalloc(sizeof(*fp));
  fp->bucket = smalloc(sizeof(*fp->bucket)*HASHSIZE);
  fp->size = HASHSIZE;
  return fp;
}

/* newenv: return a new environment */
struct env *
newenv(void)
{
  struct env *ep;

  ep = smalloc(sizeof(*ep));
  ep->fp = newframe();
  ep->ep = NULL;
  return ep;
}
