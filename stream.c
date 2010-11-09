#include "extern.h"
#include "stream.h"

stream *sstdin;           /* standard input stream */
const excpt_t eof_error = { "eof" };

/* Return a new stream. */
stream *
nstream(char *name, FILE *fp)
{
        stream *sp;

        NEW(sp);
        sp->name = sstrdup(name);
        sp->fp = fp;
        sp->line = 1;
        sp->col = 0;

        return sp;
}

/* Open a file and associate a stream with it. */
stream *
sopen(const char *path)
{
        FILE *fp;

        if ((fp = fopen(path, "r")) == NULL) {
                warn("Can't open file %s", path);
                return NULL;
        }

        return nstream(basename(path), fp);
}

/* Close a stream. */
void
sclose(stream *sp)
{
        fclose(sp->fp);
        free(sp->name);
        free(sp);
}
