SRCNAME=jsonread
BINNAME=$(SRCNAME)

CC=gcc
# CFLAGS= -O2 -Wall -I/usr/include/libxml2
CFLAGS= -O2 -Wall
# LIBS= -lxml2
LIBS=

SOURCES := $(wildcard *.c)
OBJ := $(patsubst %.c, %.o, $(SOURCES))

.c.o:
	$(CC) -c $(CFLAGS) $<

all: $(BINNAME)

$(BINNAME): $(OBJ) Makefile
	$(CC) -o $(BINNAME) $(CFLAGS) $(OBJ) $(LIBS)

Makefile.dep:
	echo \# > Makefile.dep

clean:
	rm -f *.o *.cgi *~ core *.b $(BINNAME)

dep: clean
	$(CC) -MM $(CFLAGS) *.c > Makefile.dep

check: all
	./jsonread test.json

-include Makefile.dep
