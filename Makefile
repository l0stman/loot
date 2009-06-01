CFLAGS = -g -Wall -ansi -pedantic
OBJS = main.o error.o reader.o loot.o exp.o parser.o type.o eval.o\
env.o prim.o
INSTDIR = /home/l0stman/bin
PROGNAME = loot

$(PROGNAME) : $(OBJS)
	$(CC) $(CFLAGS) -o $(.TARGET) $(.ALLSRC)

.SUFFIXES : .o .c
.c.o :
	$(CC) $(CFLAGS) -c $(.IMPSRC)

install:
	install -d $(INSTDIR)
	install -S -C $(PROGNAME) $(INSTDIR) 

depend:
	$(CC) -E -MM *.c > .depend

clean:
	rm -f *.o *.core *~ $(PROGNAME)
