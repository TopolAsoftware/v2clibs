CC=gcc
CFLAGS= -O2 -Wall
LIBS=

PROGNAME=e_readcfg

SOURCES := $(wildcard *.c)
OBJ := $(patsubst %.c, %.o, $(SOURCES))

.c.o:
	$(CC) -c $(CFLAGS) $(MACHINE) $<

all: $(PROGNAME)

$(PROGNAME): $(OBJ) Makefile
	$(CC) $(CFLAGS) $(MACHINE) -o $(PROGNAME) $(LIBS) $(OBJ)

clean:
	rm -f *.o *~ core *.b $(PROGNAME)

v2_util.o: v2_util.c v2_util.h
e_readcfg.o: e_readcfg.c v2_util.o
