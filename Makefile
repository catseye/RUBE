# GNU Makefile for RUBE.

PROG=bin/rube
CC?=gcc
O?=.o
RM_F?=rm -f

WARNS=	-W -Wall -Wstrict-prototypes -Wmissing-prototypes \
	-Wpointer-arith	-Wno-uninitialized -Wreturn-type -Wcast-qual \
	-Wwrite-strings -Wswitch -Wshadow -Wcast-align -Wchar-subscripts \
	-Winline -Wnested-externs -Wredundant-decls

ifdef DJGPP
else
  ifdef ANSI
    CFLAGS+= -ansi -pedantic
  else
    CFLAGS+= -std=c99 -D_POSIX_C_SOURCE=200809L
  endif
endif

CFLAGS+= ${WARNS} ${EXTRA_CFLAGS}

ifdef DEBUG
  CFLAGS+= -g
endif

OBJ=src/rube$(O)
OBJS=$(OBJ)

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(OBJS) -o $(PROG) $(LIBS)

$(OBJ): src/rube.c
	$(CC) $(CFLAGS) -c src/rube.c -o $(OBJ)

clean:
	$(RM_F) $(OBJS)

distclean:
	$(RM_F) $(PROG)
