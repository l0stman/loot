CFLAGS=	-ggdb3 -Wall -ansi -pedantic
OBJS=	main.o error.o reader.o loot.o exp.o parser.o type.o eval.o\
env.o prim.o
PREF=	${HOME}
INSTDIR= $(PREF)/bin
LIBDIR=	$(PREF)/lib/loot
LIBNAME= lib.lt
LOOTRC=	$(PREF)/.lootrc
PROGNAME= loot

$(PROGNAME) : $(OBJS)
	$(CC) $(CFLAGS) -o $(.TARGET) $(.ALLSRC)

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
