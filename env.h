#ifndef ENV_H
#define ENV_H

#define HASHSIZE        101

typedef struct frame {  /* a frame is an array of pointers of nlist */
        struct nlist **bucket;
        size_t size;
} frame_t;

typedef struct env {    /* an environment is a list of frames */
        frame_t *fp;  /* first frame of the environment */
        struct env *ep;       /* enclosing environment */
} env_t;

struct nlist {  /* table entry */
        struct nlist *next;
        const char *name;             /* defined name */
        exp_t *defn;                  /* replacement expression */
};

extern frame_t *newframe(void);
extern void fdump(frame_t *);
extern struct nlist *lookup(symb_t *, env_t *);
extern struct nlist *install(symb_t *, exp_t *, env_t *);
extern env_t *newenv(void);
extern env_t *extenv(exp_t *, env_t *);
extern void undef(symb_t *, frame_t *);

/* fframe: return the first frame in the environment */
static inline frame_t *
fframe(env_t *ep)
{
        return ep->fp;
}

/* eenv: return the enclosing environment */
static inline env_t *
eenv(env_t *ep)
{
        return ep->ep;
}
#endif /* !ENV_H */
