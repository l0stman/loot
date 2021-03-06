CC		= clang
CFLAGS		= -O0 -g -Wall -std=c99 -pedantic
#CFLAGS		= -O3 -Wall -std=c99 -pedantic -DNDEBUG
LDFLAGS		= -lm

OBJS		= main.o err.o read.o extern.o exp.o type.o eval.o env.o \
		  prim.o atom.o stream.o
PROGNAME	= loot

PREF		= ${HOME}
INSTDIR		= $(PREF)/bin
LIBDIR		= $(PREF)/lib/loot
LIBNAME		= lib.scm
LOOTRC		= $(PREF)/.lootrc

$(PROGNAME) : $(OBJS)
	$(CC) $(LDFLAGS) -o $(.TARGET) $(.ALLSRC)

.SUFFIXES : .o .c
.c.o :
	$(CC) $(CFLAGS) -c $(.IMPSRC)

install:
	install -d $(INSTDIR)
	install -S -C $(PROGNAME) $(INSTDIR)
	install -d $(LIBDIR)
	install -S -C $(LIBNAME) $(LIBDIR)
	echo -n "$(LIBDIR)/$(LIBNAME)" > $(LOOTRC)

depend:
	$(CC) -E -MM *.c > .depend

clean:
	rm -f *.o *.core *~ $(PROGNAME)

tags:
	find . -name '*.[ch]' -print | xargs etags