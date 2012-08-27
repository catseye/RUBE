/*
 * rube.c v1.3, Feb 2010, Chris Pressey
 * Interpreter/Debugger for the RUBE programming language
 *
 * (c)1997-2010 Cat's Eye Technologies.  All rights reserved.
 *
 * Freely redistributable unmodified for non-commmercial purposes.
 * THIS "AS-IS" SOFTWARE COMES WITH NO WARRANTY, EXPRESS OR IMPLIED.
 *
 * Usage :
 *
 * rube [-d] [-q] [-r input-file] [-w output-file]
 *      [-y delay] [-f frame-skip] [-o offset] <rube-source>
 *
 *  -d: disable debugging output
 *  -q: produce no output but program output
 *  -r: redirect input from a specified file instead of stdin
 *  -w: redirect output to a specified file instead of stdout
 *  -y: specify debugging delay in milliseconds (default 0)
 *  -f: specify debugging frame skip in frames (default 1)
 *
 * Compilation :
 *
 * MS-DOS: tested with Borland C++ v3.1.
 *         Load ANSI.SYS or compatible ANSI driver before using.
 * Windows: tested with GCC 3.4.4 under Cygwin.
 * Linux: tested with GCC 4.2.4 under Ubuntu 8.04.3 LTS.
 *
 * History :
 *
 * v1.00: May/Jun 97 original, minimal implementation
 * v1.01: Jun 97 added K gate, AV swinches, + packer, - unpacker
 * v1.02: Jul 97 fixed bug in WM winches, - unpacker, added . and C
 *	  doubled height of playfield and improved debugger.
 *	  added Ben Olmstead's cursor-turner-offer-thingy.
 *	  added -q option.
 * v1.3: Feb 110 made compilable in POSIX and strict ANSI C89.
 *        screen is cleared before drawing initial playfield.
 */

/********************************************************* #INCLUDE'S */

#define CURSORON

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#if __BORLANDC__
  #include <dos.h>
#else
  #define stricmp strcasecmp
#endif
#ifdef _POSIX_C_SOURCE
  #include <sys/time.h>
#endif

/********************************************************** #DEFINE'S */

#define LINEWIDTH    80
#define PAGEHEIGHT   50

#define SCREENWIDTH  79
#define SCREENHEIGHT 22

#define cur          pg[y * LINEWIDTH + x].c
#define curv         pg[y * LINEWIDTH + x].v
#define curd(dx,dy)  pg[(y+dy) * LINEWIDTH + (x+dx)].c
#define nex          pg2[y * LINEWIDTH + x].c
#define nexv         pg2[y * LINEWIDTH + x].v
#define nexd(dx,dy)  pg2[(y+dy) * LINEWIDTH + (x+dx)].c

#define shrink(s) s[strlen(s)-1]=0

typedef struct cellstruct
{
  char c;
  signed long int v;
} cell;

/*************************************************** GLOBAL VARIABLES */

cell pg[LINEWIDTH * PAGEHEIGHT]; /* playfield */
cell pg2[LINEWIDTH * PAGEHEIGHT];/* playfield' */

cell *head = NULL;

int x = 0, y = 0;                /* x and y looping */
int dx = 1, dy = 0;              /* direction of looping */
int debug = 1;                   /* flag : display ANSI debugging? */
int infile = 0, ia;              /* flag : use input file, and assoc arg? */
int outfile = 0, oa;             /* flag : use output file, and assoc arg? */
int deldur = 0;                  /* debugging delay in milliseconds */
int debskip = 1;		 /* frame skip in debug view */
int debopos = 1;       		 /* output column in debugger */
int frame = 1;
int quiet = 0;

/********************************************************* PROTOTYPES */

int isramp(char c);
int isblock(char c);
int issupport(char c);
int iscrate(char c);
int ctoh(char c);
char htoc(int i);

/******************************************************* MAIN PROGRAM */

int main (int argc, char **argv)
{
  FILE *f;
  FILE *fi;
  FILE *fo;
  FILE *fs;
  int i;
  int done=0;
  int maxy=0; int maxx=0;
  short signed oldcursor;

#ifdef CURSOROFF
__asm
{
  mov al,0x03
  mov bl,0x00
  int 0x10
  mov oldcursor,cx
  mov al,0x01
  mov cx,0x0001
  int 0x10
}
#endif

  srand (time (0));

  if (argc < 2)
  {
    printf ("USAGE: rube [-d] [-q] [-r input] [-w output] [-y delay] [-f skip] foo.rub\n");
    exit (0);
  }
  for (i = 1; i < argc; i++)
  {
    if (!stricmp(argv[i], "-d")) { debug = 0; }
    if (!stricmp(argv[i], "-q")) { quiet = 1; debug = 0; }
    if (!stricmp(argv[i], "-r")) { infile = 1; ia = i + 1; }
    if (!stricmp(argv[i], "-w")) { outfile = 1; oa = i + 1; }
    if (!stricmp(argv[i], "-y")) { deldur = atoi(argv[i + 1]); }
    if (!stricmp(argv[i], "-f")) { debskip = atoi(argv[i + 1]); }
  }
  if (!quiet) printf ("Cat's Eye Technologies RUBE Interpreter v1.3\n");
  if ((f = fopen (argv[argc - 1], "r")) != NULL)             /*** Input Phase */
  {
    int x = 0, y = 0;
    while (!feof (f))
    {
      cur = fgetc (f);
      if (cur == '\n')
      {
	cur = ' ';
	x = 0;
	y++;
	if (y >= PAGEHEIGHT) break;
      } else
      {
	x++;
	if (x > maxx) maxx = x;
	if (x >= LINEWIDTH)
	{
	  x = 0;
	  y++;
	  if (y >= PAGEHEIGHT) break;
	}
      }
    }
    fclose (f);
    maxy = y;
  } else
  {
    printf ("Error : couldn't open '%s' for input.\n", argv[argc - 1]);
    exit (0);
  }

  if (infile)
  {
    if (!(fi = fopen (argv[ia], "r")))
    {
      printf ("Error : couldn't open '%s' for input.\n", argv[ia]);
      exit (0);
    }
  }

  if (outfile)
  {
    if (!(fo = fopen (argv[oa], "w")))
    {
      printf ("Error : couldn't open '%s' for output.\n", argv[oa]);
      exit (0);
    }
  }

#if __BORLANDC__
  setcbrk(1);
#endif

  if (debug)
  {
    printf ("%c[2J", 27);
  }
  while (!done)          /*** Intepreting Phase */
  {
    if ((debug) && (!(frame++ % debskip) || (!frame)))
    {
      printf ("%c[1;1H", 27);
      for(y = 0; (y <= maxy) && (y <= SCREENHEIGHT); y++)
      {
	for(x = 0; (x <= maxx) && (x <= SCREENWIDTH); x++)
	{
	  putc(isprint(cur) ? cur : ' ', stdout);
	}
	printf("\n");
      }
    } else
    {
      /* putc('.', stdout); */
    }
    fflush (stdout);
    fflush (stdin);
    for (x=0; x<=(maxx); x++)
    {
      for (y=0; y<=(maxy); y++)
      {
	switch (cur)
	{
	  case 0: case 1: case 2: case 3: case 4:
	  case 5: case 6: case 7: case 8: case 9:
	  case 10: case 11: case 12: case 13: case 14:
	  case 15: case 16: case 17: case 18: case 19:
	  case 20: case 21: case 22: case 23: case 24:
	  case 25: case 26: case 27: case 28: case 29:
	  case 30: case 31: case 32:
	    if (iscrate(curd(0,-1))) nex = curd(0,-1);    /* falling in from above */
	    if (curd(0,-1) == '(') nex = '(';
	    if (curd(0,-1) == ')') nex = ')';

	    if (curd(1,1) == 'W') nex = curd(2,2);
	    if (curd(-1,1) == 'W') nex = curd(-2,2);

	    if ((curd(1,1) == 'V') && iscrate(curd(2,1))) nex = curd(2,1);
	    if ((curd(-1,1) == 'V') && iscrate(curd(-2,1))) nex = curd(-2,1);

	    if (curd(1,-1) == 'M') nex = curd(2,-2);
	    if (curd(-1,-1) == 'M') nex = curd(-2,-2);

	    if ((curd(1,-1) == 'A') && iscrate(curd(2,-1))) nex = curd(2,-1);
	    if ((curd(-1,-1) == 'A') && iscrate(curd(-2,-1))) nex = curd(-2,-1);

	    if (curd(0,-1) == '~') nex = '~';
	    if ((curd(-1,0) == '~') && (issupport(curd(-1,1)))) nex = '~';
	    if ((curd(1,0) == '~') && (issupport(curd(1,1)))) nex = '~';

	    if (curd(1,-1) == '+')
	    {
	      if (iscrate(curd(1,0)) && iscrate(curd(2,0)))
	      {
		nex = htoc((ctoh(curd(1,0))+ctoh(curd(2,0))) % 16);
	      }
	    }

	    if (curd(-1,-1) == '+')
	    {
	      if (iscrate(curd(-1,0)) && iscrate(curd(-2,0)))
	      {
		nex = htoc((ctoh(curd(-1,0))+ctoh(curd(-2,0))) % 16);
	      }
	    }

	    if (curd(1,-1) == '-')
	    {
	      if (iscrate(curd(1,0)) && iscrate(curd(2,0)))
	      {
		int i;
		i = ctoh(curd(2,0)) - ctoh(curd(1,0));
		while (i < 0) i += 16;
		nex = htoc(i);
	      }
	    }

	    if (curd(-1,-1) == '-')
	    {
	      if (iscrate(curd(-1,0)) && iscrate(curd(-2,0)))
	      {
		int i;
		i = ctoh(curd(-2,0)) - ctoh(curd(-1,0));
		while (i < 0) i += 16;
		nex = htoc(i);
	      }
	    }

	    if ((curd(1,-1) == 'K') && (iscrate(curd(1,-2))))
	    {
	      if(ctoh(curd(1,-2)) < ctoh(curd(1,0))) nex = curd(1,-2);
	    }

	    if ((curd(-1,-1) == 'K') && (iscrate(curd(-1,-2))))
	    {
	      if(ctoh(curd(-1,-2)) >= ctoh(curd(-1,0))) nex = curd(-1,-2);
	    }

	    if ((iscrate(curd(-1,0))) && (curd(-1,1) == '>')) nex = curd(-1,0);
	    if ((iscrate(curd(1,0))) && (curd(1,1) == '<')) nex = curd(1,0);
	    if (curd(0,-1) == ':') nex = curd(0,-2);
	    if ((curd(0,-1) == ';') && (iscrate(curd(0,-2)))) nex = curd(0,-2);
	    if ((curd(0,1) == '.') && (iscrate(curd(0,2)))) nex = curd(0,2);
	    if ((curd(-1,0) == '(') && (curd(1,0) == ')')) /* collision */
	    {
	      nex = ' ';
	    } else
	    {
	      if ((curd(-1,0) == '(') && (issupport(curd(-1,1)))) nex = '(';
	      if ((curd(1,0) == ')') && (issupport(curd(1,1)))) nex = ')';
	      if ((curd(0,1) == '/') || (curd(0,1) == '\\'))
	      {
		if ((curd(-1,1) == '(') && (issupport(curd(-1,2)))) nex = '(';
		if ((curd(1,1) == ')') && (issupport(curd(1,2)))) nex = ')';
	      }
	    }
	    if (iscrate(curd(-1,0)))
	    { /* shift crates */
	      int bx=-1;
	      while ((iscrate(curd(bx,0))) && (issupport(curd(bx,1))))
	      {
		if (curd(bx-1,0) == '(')
		{
		  nex = curd(-1,0);
		}
		bx--;
	      }
	    }
	    if (iscrate(curd(1,0)))
	    {
	      int bx=1;
	      while ((iscrate(curd(bx,0))) && (issupport(curd(bx,1))))
	      {
		if (curd(bx+1,0) == ')')
		{
		  nex = curd(1,0);
		}
		bx++;
	      }
	    }
	    break;
	  case '(':
	    if (((curd(1,0) == '(') ||
		 (curd(1,0) <= ' ') ||
		 (curd(0,1) <= ' ') ||
		 (curd(0,1) == '('))) nex = ' ';
	    if (isramp(curd(0,1))) nex = ' ';
	    if (isramp(curd(1,0))) nex = ' ';
	    if (isramp(curd(-1,0))) nex = ' ';
	    if ((isblock(curd(1,0))) ||
		(curd(1,-1) == ',') ||
		(curd(1,0) == '*')) nex = ')';
	    if (iscrate(curd(1,0)))
	    {
	      int bx=1;
	      while ((iscrate(curd(bx,0))) && (issupport(curd(bx,1))))
	      {
		if (isblock(curd(bx+1,0)))
		{
		  nex = ')';
		}
		bx++;
	      }
	    }
	    break;
	  case ')':
	    if (((curd(-1,0) == ')') ||
		 (curd(-1,0) <= ' ') ||
		 (curd(0,1) <= ' ') ||
		 (curd(0,1) == ')'))) nex = ' ';
	    if (isramp(curd(0,1))) nex = ' ';
	    if (isramp(curd(1,0))) nex = ' ';
	    if (isramp(curd(-1,0))) nex = ' ';
	    if ((isblock(curd(-1,0))) ||
		(curd(-1,-1) == ',') ||
		(curd(-1,0) == '*')) nex = '(';
	    if (iscrate(curd(-1,0)))
	    {
	      int bx=-1;
	      while ((iscrate(curd(bx,0))) && (issupport(curd(bx,1))))
	      {
		if (isblock(curd(bx-1,0)))
		{
		  nex = '(';
		}
		bx--;
	      }
	    }
	    break;
	  case 'O':
	    if ((iscrate(curd(0,-1))) && (iscrate(curd(0,-2))))
	    {
	      int d;

	      d = ctoh(curd(0,-1)) + ctoh(curd(0,-2)) * 16;
	      if (curd(0, 1) == 'b')
	      {
		if (debug)
		{
		  char s[80];
		  printf ("%c[%d;%dH", 27, 25, debopos);
		  sprintf(s, "%d ", (int)d);
		  debopos += strlen(s);
		  if (debopos > SCREENWIDTH)
		  {
		    debopos = 1;
		    printf ("%c[%d;%dH%c[K", 27, 25, 1, 27);
		    debopos += strlen(s);
		  }
		  printf("%s", s);
		} else
		{
		  printf("%d ", (int)d);
		}
	      }
	      if (curd(0, 1) == 'c')
	      {
		if (debug)
		{
		  printf ("%c[%d;%dH", 27, 25, debopos++);
		  if (debopos > SCREENWIDTH)
		  {
		    debopos = 1;
		    printf ("%c[%d;%dH%c[K", 27, 25, 1, 27);
		    debopos++;
		  }
		  printf ("%c", (char)d);
		} else
		{
		  putc((char)d, stdout);
		}
	      }
	    }
	  case 'A':
	    if (iscrate(curd(-1,0))  || iscrate(curd(1,0))) nex = 'V'; else nex = cur;
	    break;
	  case 'V':
	    if (iscrate(curd(-1,0))  || iscrate(curd(1,0))) nex = 'A'; else nex = cur;
	    break;
	  default: nex = cur;
	}
	if (iscrate(cur))
	{
	  if (issupport(curd(0,1))) nex = cur; else nex = ' ';
	  if ((curd(1,0) <= ' ') && (curd(0,1) == '>')) nex = ' ';
	  if ((curd(-1,0) <= ' ') && (curd(0,1) == '<')) nex = ' ';
	  if ((curd(1,-1) == 'W') && (curd(2,-2) == cur)) nex = ' ';
          if ((curd(-1,-1) == 'W') && (curd(2,-2) == cur)) nex = ' ';
          if ((curd(1,1) == 'M') && (curd(2,2) == cur)) nex = ' ';
	  if ((curd(-1,1) == 'M') && (curd(-2,2) == cur)) nex = ' ';
	  if (curd(1,0) == 'V') nex = ' ';
	  if (curd(-1,0) == 'V') nex = ' ';
	  if (curd(1,0) == 'A') nex = ' ';
	  if (curd(-1,0) == 'A') nex = ' ';
	  if (iscrate(curd(-1,0)) && ((curd(-1,-1) == '+')
	      || (curd(-1,-1) == '-'))) nex = ' ';
	  if (iscrate(curd(1,0)) && ((curd(1,-1) == '+')
	      || (curd(1,-1) == '-'))) nex = ' ';
	  if ((iscrate(curd(-1,0)) || iscrate(curd(1,0))) && ((curd(0,-1) == '+')
	      || (curd(0,-1) == '-'))) nex = ' ';
	}
      }
    }
    /* fix nex array */
    for (x=0; x<=(maxx); x++)
    {
      for (y=0; y<=(maxy); y++)
      {
	switch (cur)
	{
	  case '*':
	    if (curd(-1,0) == ')') nex = ' ';
	    if (curd(1,0) == '(') nex = ' ';
	    break;
	  case 'O':
	    if ((iscrate(curd(0,-1))) && (iscrate(curd(0,-2))))
	    {
	      nexd(0,-1)=' ';
	      nexd(0,-2)=' ';
	    }
	    break;
	}
	if (iscrate(cur))
	{
	  if (curd(1,0) == ')')
	  {
	    int bx=0; int flag=0;
	    while ((iscrate(curd(bx,0))) && (issupport(curd(bx,1))))
	    {
	      if (curd(bx-1,0) <= ' ')
	      {
		flag = 1;
	      }
	      bx--;
	    }
	    if (flag)
	    {
	      bx=0;
	      while ((iscrate(curd(bx,0))) && (issupport(curd(bx,1))))
	      {
		nexd(bx-1,0) = curd(bx,0);
		bx--;
	      }
	      nex = ')'; nexd(1,0) = ' ';
	    }
	  }
	  if (curd(-1,0) == '(')
	  {
	    int bx=0; int flag=0;
	    while (iscrate(curd(bx,0)) && (issupport(curd(bx,1))))
	    {
	      if (curd(bx+1,0) <= ' ')
	      {
		flag=1;
	      }
	      bx++;
	    }
	    if (flag)
	    {
	      bx=0;
	      while ((iscrate(curd(bx,0))) && (issupport(curd(bx,1))))
	      {
		nexd(bx+1,0) = curd(bx,0);
		bx++;
	      }
	      nex = '('; nexd(-1,0)= ' ';
	    }
	  }
	  if ((curd(-1,0)=='C') ||
	      (curd(1,0)=='C') ||
	      (curd(0,-1)=='C') ||
	      (curd(0,1)=='C')) nex = ' ';
	}
	if ((curd(-1,0)=='F') ||
	    (curd(1,0)=='F') ||
	    (curd(0,-1)=='F') ||
	    (curd(0,1)=='F')) nex = ' ';
      }
    }
    if (deldur > 0)
      rube_delay (deldur);
    memcpy(pg, pg2, LINEWIDTH * PAGEHEIGHT * sizeof(cell));
  }
  if (fi) fclose (fi);
  if (fo) fclose (fo);
  if (fs) fclose (fs);
  if (debug) printf ("%c[22;1H", 27);
#if CURSOROFF
__asm
{
  mov al,0x01
  mov cx,oldcursor
  int 0x10
}
#endif
  exit (0);
}

int isramp(char c)
{
  return ((c=='/')||(c=='\\'));
}

int isblock(char c)
{
  return ((c=='='));
}

int issupport(char c)
{
  return ((c=='=')||iscrate(c)||(c=='(')||(c==')')||(c==';')||
	  (c=='/')||(c=='\\')||(c==':')||(c=='*')||(c==',')||
	  (c=='>')||(c=='<')||(c=='O')||(c=='W')||(c=='M')||
	  (c=='A')||(c=='V')||(c=='~')||(c=='.'));
}

int iscrate(char c)
{
  return ((c=='0')||(c=='1')||(c=='2')||(c=='3')||
	  (c=='4')||(c=='5')||(c=='6')||(c=='7')||
	  (c=='8')||(c=='9')||(c=='a')||(c=='b')||
	  (c=='c')||(c=='d')||(c=='e')||(c=='f'));
}

int ctoh(char c)
{
  if((c>='0') && (c<='9')) return (c-'0'); else return ((c-'a')+10);
}

char htoc(int i)
{
  if((i>=0) && (i<=9)) return ((char)(i+'0')); else return ((char)(i+'a')-10);
}

int rube_delay(int msec)
{
#if __BORLANDC__
  delay (msec);
#elsif _POSIX_C_SOURCE
  struct timespec d;

  d.tv_sec = msec / 1000;
  msec %= 1000;
  d.tv_nsec = msec * 1000000;
  nanosleep(&d, NULL);
#else
  sleep(msec / 1000);
#endif
}
