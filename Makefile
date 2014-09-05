# GNU Makefile for RUBE.

PROG=bin/rube$(EXE)
CC?=gcc
O?=.o
EXE?=

WARNS=	-W -Wall -Wstrict-prototypes -Wmissing-prototypes \
	-Wpointer-arith	-Wno-uninitialized -Wreturn-type -Wcast-qual \
	-Wwrite-strings -Wswitch -Wshadow -Wcast-align -Wchar-subscripts \
	-Winline -Wnested-externs -Wredundant-decls

ifdef ANSI
  CFLAGS+= -ansi -pedantic -D_BSD_SOURCE
else
  CFLAGS+= -std=c99 -D_POSIX_C_SOURCE=200809L
endif

CFLAGS+= ${WARNS} ${EXTRA_CFLAGS}

ifdef DEBUG
  CFLAGS+= -g
endif

OBJ=src/rube$(O)
OBJS=$(OBJ)

all: $(PROG)

bin/.exists:
	mkdir -p bin
	touch bin/.exists

$(PROG): bin/.exists $(OBJS)
	$(CC) $(OBJS) -o $(PROG) $(LIBS)

$(OBJ): src/rube.c
	$(CC) $(CFLAGS) -c src/rube.c -o $(OBJ)

clean:
	rm -f $(OBJS)

distclean:
	rm -f $(PROG)
