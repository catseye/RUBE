# GNU Makefile for RUBE.
# $Id: Makefile 64 2008-02-16 18:59:19Z catseye $

PROG=rube
CC?=gcc
STRIP?=strip
O?=.o
EXE?=

WARNS=	-Werror -W -Wall -Wstrict-prototypes -Wmissing-prototypes \
	-Wpointer-arith	-Wno-uninitialized -Wreturn-type -Wcast-qual \
	-Wwrite-strings -Wswitch -Wshadow -Wcast-align -Wchar-subscripts \
	-Winline -Wnested-externs -Wredundant-decls

ifdef ANSI
  CFLAGS+=-ansi -pedantic
else
  CFLAGS+=-std=c99 -D_POSIX_C_SOURCE=200112L
endif

OBJS=rube${O}

all: ${PROG}${EXE}

${PROG}${EXE}: $(OBJS)
	$(CC) $(OBJS) -o ${PROG} $(LIBS)

rube${O}: rube.c
	$(CC) $(CFLAGS) -c rube.c -o rube${O}

clean:
	rm -f *.o *.core *.exe ${PROG}${EXE}
