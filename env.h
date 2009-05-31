#ifndef ENV_H
#define ENV_H

#define HASHSIZE	101

struct nlist {	/* table entry */
  struct nlist *next;
  char *name;		/* defined name */
  struct exp *defn;	/* replacement expression */
};

struct frame {	/* a frame is an array of pointers of nlist */
  struct nlist **bucket;
  size_t size;
};

struct env {	/* an environment is a list of frames */
  struct frame *fp;	/* first frame of the environment */
  struct env *ep;	/* enclosing environment */
};

struct frame *newframe(void);
struct nlist *lookup(char *, struct env *);
struct nlist *install(char *, struct exp *, struct env *);
struct env *newenv(void);
struct env *extenv(struct exp *, struct env *);
void undef(char *, struct frame *);

/* fframe: return the first frame in the environment */
static __inline__ struct frame *
fframe(struct env *ep)
{
  return ep->fp;
}

/* eenv: return the enclosing environment */
static __inline__ struct env *
eenv(struct env *ep)
{
  return ep->ep;
}
#endif /* !ENV_H */
