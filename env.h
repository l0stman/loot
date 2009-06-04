#ifndef ENV_H
#define ENV_H

#define HASHSIZE	101

struct nlist {	/* table entry */
  struct nlist *next;
  char *name;		/* defined name */
  exp_t *defn;	/* replacement expression */
};

struct frame {	/* a frame is an array of pointers of nlist */
  struct nlist **bucket;
  size_t size;
};

struct env {	/* an environment is a list of frames */
  struct frame *fp;	/* first frame of the environment */
  struct env *ep;	/* enclosing environment */
};
typedef struct env env_t;

struct frame *newframe(void);
struct nlist *lookup(char *, env_t *);
struct nlist *install(char *, exp_t *, env_t *);
env_t *newenv(void);
env_t *extenv(exp_t *, env_t *);
void undef(char *, struct frame *);

/* fframe: return the first frame in the environment */
static __inline__ struct frame *
fframe(env_t *ep)
{
  return ep->fp;
}

/* eenv: return the enclosing environment */
static __inline__ env_t *
eenv(env_t *ep)
{
  return ep->ep;
}
#endif /* !ENV_H */
