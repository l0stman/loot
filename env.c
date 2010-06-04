#include "extern.h"
#include "exp.h"
#include "env.h"

/* inspired by K&R */

/* hash: form hash value for string s */
static unsigned
hash(const char *s, size_t hashsize)
{
        unsigned hashval;

        for (hashval = 0; *s != '\0'; s++)
                hashval = *s + 31 * hashval;
        return hashval % hashsize;
}

/* find: look for s in frame */
static struct nlist *
find(const char *s, frame_t *fp)
{
        struct nlist *np;

        for (np = fp->bucket[hash(s, fp->size)]; np != NULL; np = np->next)
                if (strcmp(s, np->name) == 0)
                        return np;    /* found */
        return NULL;          /* not found */
}

/* lookup: look for s in the environment */
struct nlist *
lookup(const char *s, env_t *ep)
{
        struct nlist *np;

        for ( ; ep != NULL; ep = eenv(ep))
                if ((np = find(s, fframe(ep))) != NULL)
                        return np;    /* found */
        return NULL;  /* not found */
}

/* install: put (name, defn) in the environment */
struct nlist *
install(const char *name, exp_t *defn, env_t *ep)
{
        struct nlist *np;
        frame_t *fp;
        unsigned hashval;

        fp = fframe(ep);
        if ((np = find(name, fp)) == NULL) {  /* not found */
                NEW(np);
                np->name = strtoatm(name);
                hashval = hash(name, fp->size);
                np->next = fp->bucket[hashval];
                fp->bucket[hashval] = np;
        }
        np->defn = defn;
        return np;
}

/* undef: remove the entry corresponding to name in frame */
void
undef(char *s, frame_t *fp)
{
        struct nlist *np;
        struct nlist *prev;
        unsigned hashval;

        hashval = hash(s, fp->size);
        np = fp->bucket[hashval];
        prev = NULL;

        while (np != NULL) {
                if (strcmp(s, np->name) == 0) { /* found */
                        if (prev == NULL)     /* we're at the beginning */
                                fp->bucket[hashval] = np->next;
                        else
                                prev->next = np->next;
                        free(np);
                        return;
                }
                prev = np;
                np = np->next;
        }
}

/* newframe: return a new frame pointer */
frame_t *
newframe(void)
{
        frame_t *fp;

        NEW(fp);
        fp->bucket = calloc(HASHSIZE, sizeof(*fp->bucket));
        fp->size = HASHSIZE;
        return fp;
}

/* newenv: return a new environment */
env_t *
newenv(void)
{
        env_t *ep;

        NEW(ep);
        ep->fp = newframe();
        ep->ep = NULL;
        return ep;
}

/* extenv: extend the environment with new bindings */
env_t *
extenv(exp_t *blist, env_t *envp)
{
        env_t *ep;
        exp_t *bind;

        ep = newenv();
        ep->ep = envp;        /* enclosing environment */
        for (; !isnull(blist); blist = cdr(blist)) {
                bind = car(blist);
                install(symp(car(bind)), cdr(bind), ep);
        }
        return ep;
}

/* Dump the frame content to the standard output */
void
fdump(frame_t *fp)
{
        struct nlist *np;
        char *s;
        int i;

        for (i = 0; i < fp->size; i++) {
                if (fp->bucket[i]) {
                        for (np = fp->bucket[i]; np; np = np->next) {
                                printf("%d: [%s, %s] ", i, np->name,
                                       s = tostr(np->defn));
                                free(s);
                        }
                        putchar('\n');
                }
        }
}
