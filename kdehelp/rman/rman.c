static char rcsid[] = "$Header$";

/*
  RosettaMan

  Copyright (c) 1993-1996  T.A. Phelps (phelps@cs.Berkeley.EDU)
  All Rights Reserved.

     Permission to use, copy, modify, and distribute this software and its
     documentation for educational, research and non-profit purposes, 
     without fee, and without a written agreement is hereby granted, 
     provided that the above copyright notice and the following 
     paragraph appears in all copies.  

     Permission to incorporate this software into commercial products may 
     be obtained from the Office of Technology Licensing, 2150 Shattuck 
     Avenue, Suite 510, Berkeley, CA  94704.


  accept man pages as formatted by (10)
     Hewlett-Packard HP-UX, AT&T System V, SunOS, Sun Solaris, OSF/1, DEC Ultrix,
	SGI IRIX, Linux, FreeBSD, SCO

  output as (10)
     printable ASCII, section headers only, TkMan, [tn]roff, Ensemble, HTML,
	LaTeX, LaTeX2e, RTF, Perl pod, MIME, and soon SGML

     written March 24, 1993
	bs2tk transformed into RosettaMan November 4-5, 1993

   1993
    2-Apr  bullets, change bars, copyright symbol
    5      boldface, other SGI nicks
    7      skip unrecognized escape codes
   10      small caps
   13      underscores considered uppercase so show up
              in default small caps font
           screen out Ultrix junk (code getting pretty tangled now)
   14      until Tk text has better tab support, replace tabs by
           spaces until get to next tab stop (for Ultrix)
           -t gives tabstop spacing
   20      Solaris support (Larry Tsui)
    3-Jun  section subheading parsing (Per-Erik Martin)
   28      hyphenated man pages in SEE ALSO show up correctly in Links
           (Mike Steele)
   13-Jul  under FILES, fully qualified path names are added to Links,
              but this taken out immediately because not useful
   14      option to keep changebars on right (Warren Jessop)
    5-Aug  search for header, footer dynamically--
              no need to edit or search large list of patterns
   11      -m kicks in man page formatting beyond nroff backspace kludges
   27      handle double digit numbers better by trying again relative to end of line
   19-Sep  -T gives Tk extras (otherwise ASCII only)
           -H gives headers only (implies -T off)
   10-Oct  -r reverse compiles to [tn]roff source (as Geoff Collyer's nam and fontch,
           but leveraging existing analysis so only addition of ~60 lines)
           (The code is device-driver obscure now--obfuscated C contest next.)
   13      header and footer optionally available at bottom in Tk view
           (Marty Leisner)
   19      "reflected" odd and even page headers&footers zapped
   20      keep count of sections and subsections, using smaller font for larger numbers
    1-Nov  reverse compiles to Ensemble, except for character ranges

    4      started rman rewrite for cleaner support of multiple output targets,
           including: plain ascii, headers only, TkMan, [nt]roff, Ensemble, SGML, HTML
    5      line filtering separated from other logic
           despite greater sophistication, RosettaMan faster than bs2tk (!)
   28-Dec  man page reference recognition (Michael Harrison)

   1994
    1-Jan  identify descriptive lists by comparing scnt2 with s_avg
    3      tail-end table of contents in HTML documents
    5      -f <filter> and LaTeX output mode
   24      proof-of-concept RTF output mode
   26      handle man pages that don't have a header on the first page
   28      parse "handwritten" man pages
   22-Feb  alpha version released
    6-Mar  various bug fixes
   10      beta version released
   13-Jun  fixed surious generation on <DL>'s (the existence of which pointed out by David Sibley)
   22-Jul  table recognition experiment.
              works reasonably well, except for tables with centered headers
    3      allow for off-by-one (and -two) in identification of header and footer
           fixed problem with recurrent/leftover text with OSF/1 bold bullets (yeesh)
   12-Sep  2.0gamma released
   13      check for *third* header, possibly centered, possibly after blank lines (Charles Anderson)
           fixed tag ranges for lines following blank lines (just \n)
              of pages with global indentation (Owen Rees)
   19      fixed two small problems with LaTeX (^ => \^, \bullet => $\bullet$) (Neal Becker)
   24      simple check for erroneously being fed roff source
   26      deal with bold +- as in ksh (ugh)
   30      2.0delta released
    9-Oct  special check for OSF to guard against section head interpreted as footer
    8-Nov  Perl pod output format (result still needs work, but not much)
    7-Dec  2.0epsilon released (last one before final 2.0)
   22      Happy Winter Solstice!  2.0 released
           deprecated gets() replaced (Robert Withrow)
   25      TkMan module's $w.show => $t, saving about 9% in generated characters

   1995
    1-Jan  experiment with TkMan output to take advantage of my hack to Tk text
           (i.e., $t insert end "text" => $t insert end "text1" tag1 "text2" tag2 ...)
		 results => output size reduced about 25%, time reduced about 12-15%
   25-Mar  back to old mark command for Tk module
    8-May  hyphens in SEE ALSO section would confuse link-finder, so re-linebreak if necessary(!)
           (Greg Earle & Uri Guttman)
    4-Aug  put formats and options into tables (inspired by Steve Maguire's Writing Solid Code)
   19      -V accepts colon-separated list of valid volume names (Dag Nygren)
   22      MIME output format that's usable in Emacs 19.29 (just three hours at hacking HTML module)
           (Neal Becker)
    9-Sep  nits in HTML and better Solaris code snippets (Drazen Kacar)
   13-Nov  Macintosh port by Matthias Neeracher <neeri@iis.ee.ethz.ch>
   18-Dec  adapted to LaTeX2e, null manRef yields italicized man ref (H. Palme)
   28      allow long option names (Larry Schwimmer)

   1996
   22-Jan  fixed problem with hyphenation supression and tabs in man page--sheesh! (H. Palme)
   23-May  split TkMan format into Tk and TkMan (which calls Tk)
   25      in TkMan format, initial spaces converted to tabs
   ??-???  SGML output format (DTD found at long last), validated by sgmls
*/


/* TO DO ****

   output to SGML with Davenport DTD

   don't give SHORTLINE if just finished bullet of bultxt, ended section head, ... other cases?
   make sure text following bullet is aligned correctly

   output to WinHelp?  don't have specs (anybody interested?)
   collect header and footer until hit blank line?
   what to do about tables?   count second gap of spaces & average gap? ==>
      good idea but tables too variable for this to work
   internal, outline-like header section for HTML documents?  how to put this *first*?
   one line look ahead to enable better parsing (item lists, et cetera)
   alluc (==nonlc) flag, copy curline to last line vector (works well with lookahead cache)
   ??      collect sundry globals into vectors (i.e., arrays and enum indexes)
              (if compiler has good constant propagation, then shouldn't slow access)
   collect scattered globals into vectors (e.g., curline[ispcnt]): array + enum
      curline, lastline, flags, pending, bufs+lens
*/



#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
/* OSF seems to need this */
#ifdef I_UNISTD
#include <unistd.h>
#endif /* I_UNISTD */

/*** make #define's into consts? => can't because compilers not smart enough ***/
/* maximum number of tags per line */
#define MAXTAGS 50
#define MAXTOC 500
#define xputchar(c)		if (fcharout) putchar(c)
enum { c_rsquote='\x27', c_lsquote='\x60', c_dagger='\xa7', c_bullet='\xb7', c_plusminus='\xb1' };


/*** tag management ***/

enum tagtype { NOTAG, TITLE, ITALICS, BOLD, SYMBOL, SMALLCAPS, BOLDITALICS, MONO, MANREF };	/* MANREF last */

struct { enum tagtype type; int first; int last; } tags[MAXTAGS];
int tagc=0;
struct { char *text; int type; int line; } toc[MAXTOC];
int tocc=0;


/* characters in this list automatically prefixed by a backslash (set in output format function */
char *escchars="";
/* characters that need special handling in any output format, *more than just a backslash* */
/* characters in this list need a corresponding case statement in each output format */
char *trouble=".`'><&\\^|\xa7\xb7\xb1";
enum command {
	CHARPERIOD='.', CHARLSQUOTE='`', CHARRSQUOTE='\'', CHARGT='>', CHARLT='<',
	CHARAMP='&', CHARBACKSLASH='\\', CHARDASH='-', CHARHAT='^', CHARVBAR='|',
	CHARPLUSMINUS=0xb1,	CHARDAGGER=0xa7, CHARBULLET=0xb7,
	CHANGEBAR=0x100, CHARLQUOTE, CHARRQUOTE, 
	BEGINDOC, ENDDOC, BEGINBODY, ENDBODY,
	BEGINHEADER, ENDHEADER, BEGINFOOTER, ENDFOOTER, SHORTLINE,
	BEGINSECTION, ENDSECTION, BEGINSUBSECTION, ENDSUBSECTION,
	BEGINSECTHEAD, ENDSECTHEAD, BEGINSUBSECTHEAD, ENDSUBSECTHEAD,
	BEGINBOLD, ENDBOLD, BEGINITALICS, ENDITALICS, BEGINMANREF, ENDMANREF,
	BEGINSC, ENDSC, BEGINBOLDITALICS, ENDBOLDITALICS, BEGINY, ENDY,
	BEGINBULPAIR, ENDBULPAIR, BEGINBULLET, ENDBULLET, BEGINBULTXT, ENDBULTXT,
	BEGINLINE, ENDLINE, BEGINTABLELINE, ENDTABLELINE, BEGINTABLE, ENDTABLE
};
void (*fn)(enum command) = NULL;
enum command prevcmd = BEGINDOC;


/*** globals ***/
/* move all flags into an array?
enum { fSubsX, fLast };
int flags[fLast];
*/

int fPara=0;			/* line or paragraph groupings of text */
int fSubsections=0;	/* extract subsection titles too? */
int fChangeleft=0;	/* move change bars to left? (-1 => delete them) */
int fMan=1;		/* invoke agressive man page filtering? */
int fQS=0;		/* squeeze out spaces (scnt and interword)? */
int fIQS=0;		/* squeeze out initial spaces (controlled separately from fQS) */
int fILQS=0;		/* squeeze out spaces for usual indent */
int fHeadfoot=0;	/* show canonical header and footer at bottom? */
int falluc=0;
int fintable=0;
int fTable=0;
int fotable=0;
int TabStops=8;
int hanging=0;		/* location of hanging indent (if ==0, none) */
enum { NAME, SYNOPSIS, DESCRIPTION, SEEALSO, FILES, AUTHOR, RANDOM };	/* RANDOM last */
char *sectheadname[] = {
  "NAME", "SYNOPSIS", "DESCRIPTION", "SEE ALSO:RELATED INFORMATION", "FILES", "AUTHOR", "RANDOM"
};
int sectheadid = RANDOM;
int oldsectheadid = RANDOM;

int fCodeline=0;
int fNOHY=0;		/* re-linebreak so no words are hyphenated */
const char *TABLEOFCONTENTS = "Table of Contents";
const char *HEADERANDFOOTER = "Header and Footer";
char manName[80]="man page";
char manSect[10]="1";
const char *providence =
	"manual page source format generated by RosettaMan v" ROSETTAMANVERSION;
const char *anonftp =
	"available via anonymous ftp from ftp.cs.berkeley.edu:/ucb/people/phelps/tcltk/rman.tar.Z";

int linelen;			/* length of result in plain[] */
int spcsqz;			/* number of spaces squeezed out */
int ccnt=0;			/* # of changebars */
int scnt,scnt2;		/* counts of initial spaces in line */
int s_sum,s_cnt;
int bs_sum, bs_cnt;
int ncnt=0,oncnt=0;		/* count of interline newlines */
int CurLine=1;
int AbsLine=1-1;		/* absolute line number */
int indent=0;			/* global indentation */
int lindent=0;			/* usual local indent */
int auxindent=0;		/* aux indent */
int I;				/* index into line/paragraph */
int fcharout=1;		/* show text or not */
int lookahead;
/*int tabgram[BUFSIZ];*/	/* histogram of first character positions */
char buf[BUFSIZ];
char plain[BUFSIZ];		/* current text line with control characters stripped out */
char hitxt[BUFSIZ];		/* highlighted text (available at time of BEGIN<highlight> signal */
char header[BUFSIZ]="";		/* complete line */
char header2[BUFSIZ]="";		/* SGIs have two lines of headers and footers */
char header3[BUFSIZ]="";		/* GNU and some others have a third! */
char footer[BUFSIZ]="";
char footer2[BUFSIZ]="";
#define CRUFTS 5
char *cruft[CRUFTS] = { header, header2, header3, footer, footer2 };

int fIP=0;



/*** utility functions ***/


/* case insensitive versions of strcmp and strncmp */

int
stricmp(const char *s1, const char *s2) {
	assert(s1!=NULL && s2!=NULL);
	/*strincmp(s1, s2, strlen(s1)+1);*/

	while (tolower(*s1)==tolower(*s2)) {
		if (*s1=='\0') return 0;
		s1++; s2++;
	}

	if (tolower(*s1)<tolower(*s2)) return -1;
	else return 1;
}

int
strincmp(const char *s1, const char *s2, size_t n) {
	assert(s1!=NULL && s2!=NULL && n>0);

	while (n>0 && tolower(*s1)==tolower(*s2)) {
		if (*s1=='\0' || n==1) return 0;
		n--; s1++; s2++;
	}

	if (tolower(*s1)<tolower(*s2)) return -1;
	else return 1;
}

/* compare string and a colon-separated list of strings */
int
strcoloncmp2(char *candidate, int end, char *list, int sen) {
	char *l = list;
	char *c,c2;

	assert(candidate!=NULL && list!=NULL);
	assert(end>=-1 && end<=255);
	assert(sen==0 || sen==1);

	if (*l==':') l++;	/* tolerate a leading colon */

	/* invariant: c and v point to start of strings to compare */
	while (*l) {
		assert(l==list || l[-1]==':');
		for (c=candidate; *c && *l; c++,l++)
			if ((sen && *c!=*l) || (!sen && tolower(*c)!=tolower(*l)))
				break;

		/* if candidate matches a valid one as far as valid goes, it's a keeper */
		if ((*l=='\0' || *l==':') && (*c==end || end==-1)) {
		  if (*c=='\b') {
		    c2 = c[-1];
		    while (*c=='\b' && c[1]==c2) c+=2;
		  }
		  /* no volume qualifiers with digits */
		  if (!isdigit(*c)) return 1;
		}

		/* bump to start of next valid */
		while (*l && *l++!=':') /* nada */;
	}

	return 0;
}

int
strcoloncmp(char *candidate, int end, char *list) {
	int sen=1;
	char *l = list;

	assert(candidate!=NULL && list!=NULL);
	assert(end>=-1 && end<=255);

	if (*l=='=') l++; else end=-1;
	if (*l=='i') { sen=0; l++; }

	return strcoloncmp2(candidate, end, l, sen);
}



/* add an attribute tag to a range of characters */

void
addtag(enum tagtype type, int first, int last) {
	assert(type!=NOTAG);

	if (tagc<MAXTAGS) {
		tags[tagc].type = type;
		tags[tagc].first = first;
		tags[tagc].last = last;
		tagc++;
	}
}


/*
   collect all saves to string table one one place, so that
   if decide to go with string table instead of multiple malloc, it's easy
   (probably few enough malloc's that more sophistication is unnecessary)
*/

void
addtoc(char *text, enum command type, int line) {
	char *r;

	assert(text!=NULL && strlen(text)>0);
	assert(type==BEGINSECTION || type==BEGINSUBSECTION);

	if (tocc<MAXTOC) {
		r = malloc(strlen(text)+1); if (r==NULL) return;
		strcpy(r,text);
		toc[tocc].text = r;
		toc[tocc].type = type;
		toc[tocc].line = line;
		tocc++;
	}
}



char *vollist = VOLLIST;
char phrase[BUFSIZ];	/* first "phrase" (space of >=3 spaces) */
int phraselen;

void
filterline(char *buf, char *plain) {
	char *p,*q,*r;
	char *ph;
	int iq;
	int i,j;
	int hl=-1, hl2=-1;
	int iscnt=0;	/* interword space count */
	int tagci;
	int I0;
	int etype;
	int efirst;
	enum tagtype tag = NOTAG;

	assert(buf!=NULL && plain!=NULL);

	etype=NOTAG;
	efirst=-1;
	tagci=tagc;
	ph=phrase; phraselen=0;
	scnt=scnt2=0;
	s_sum=s_cnt=0;
	bs_sum=bs_cnt=0;
	ccnt=0;
	spcsqz=0;

	/* strip only certain \x1b's and only at very beginning of line */
	for (p=buf; *p=='\x1b' && (p[1]=='8'||p[1]=='9'); p+=2)
		/* nop */;

	strcpy(plain,p);
	q=&plain[strlen(p)];

	/*** spaces and change bars ***/
	for (scnt=0,p=plain; *p==' '; p++) scnt++;	/* initial space count */

	assert(*q=='\0');
	q--;
	if (fChangeleft)
		for (; q-40>plain && *q=='|'; q--)	{	/* change bars */
			if (fChangeleft!=-1) ccnt++;
			while (q-2>=plain && q[-1]=='\b' && q[-2]=='|') q-=2;	/* boldface changebars! */
		}

	if (q!=&plain[scnt-1])			/* zap trailing spaces */
		for (; *q==' '; q--) /* nop */;

	q[1]='\0';


	/* set I for tags below */
	if (indent>=0 && scnt>=indent) scnt-=indent;
	if (!fPara && !fIQS) {
		if (fChangeleft) I+=(scnt>ccnt)?scnt:ccnt;
		else I+=scnt;
	}
	I0=I;

	/*** tags and filler spaces ***/

	iq=0; falluc=1;
	for (q=plain; *p; p++) {

		iscnt=0;
		if (*p==' ') {
			for (r=p; *r==' '; r++) { iscnt++; spcsqz++; }
			s_sum+=iscnt; s_cnt++;
			if (iscnt>1 && !scnt2 && *p==' ') scnt2=iscnt;
			if (iscnt>2) { bs_cnt++; bs_sum+=iscnt; }	/* keep track of large gaps */
			iscnt--;		/* leave last space for tail portion of loop */

			if (fQS && iscnt<3) { p=r-1;	iscnt=0; } /* reduce strings of <3 spaces to 1 */
			/* else if (fQS && iscnt>=3) { replace with tab? } */
			else {
				for (i=0; i<iscnt; i++) { p++; *q++=' '; }
			}
		} /* need to go through if chain for closing off annotations */

		/** backspace-related filtering **/

		/* else */ if (*p=='\b' && p[1]=='_' && q>plain && q[-1]=='+') {
			/* bold plus/minus(!) */
			q[-1]=c_plusminus;
			while (*p=='\b' && p[1]=='_') p+=2;
			continue;
		} else if ((*p=='_' && p[1]=='\b' && p[2]!='_' && p[3]!='\b')
			|| (*p=='\b' && p[1]=='_')) {
			/* italics */
			/* start tag only if not already in one */
			if (hl==-1) { hl=I+iq; tag=ITALICS; }
			p+=2;
		} else if (*p=='_' && p[2]==p[4] && p[1]=='\b' && p[3]=='\b' && p[2]!='_') {
			/* bold italics (for Solaris) */
			for (p+=2; *p==p[2] && p[1]=='\b';)
				p+=2;
			if (hl==-1) { hl=I+iq; tag=BOLDITALICS; }
		} else if (*p==p[2] && p[1]=='\b') {
			/* boldface */
			while (*p==p[2] && p[1]=='\b')
				p+=2;
			if (hl==-1) { hl=I+iq; tag=BOLD; }
		} else if (p[1]=='\b' &&
				 ((*p=='o' && p[2]=='+') ||
				  (*p=='+' && p[2]=='o')) ) {
			/* bullets */
			p+=2;
			while (p[1]=='\b' &&		/* bold bullets(!) */
				 (*p=='o' || p[2]=='+') )
				p+=2;
			*q++=c_bullet; iq++;
			continue;
		} else if (*p=='\b' && p>plain && p[-1]=='o' && p[1]=='+') {
			/* OSF bullets */
			while (*p=='\b' && p[1]=='+') p+=2;	/* bold bullets(!) */
			q[-1]=c_bullet; p--;
			continue;
		} else if (p[1]=='\b' && *p=='+' && p[2]=='_') {
			/* plus/minus */
			p+=2;
			*q++=c_plusminus; iq++;
			continue;
		} else if (p[1]=='\b' && *p=='|' && p[2]=='-') {
			/* dagger */
			*q++=c_dagger; iq++;
			p+=2; continue;
		} else if (*p=='\b') {
			/* supress unattended backspaces */
			continue;
		} else if (*p=='\x1b' /*&& (p[1]=='9'||p[1]=='8')*/) {
			p++;
			if (*p=='[') {
				p++;
				if (*p=='1' && hl==-1) {
					/* stash attributes in "invalid" array element */
					efirst=I+iq; etype=BOLD;
					/*hl=I+iq; tag=BOLD; -- faces immediate end of range */
				} else if (*p=='0' /*&& hl>=0 && hl2==-1 && tags[MAXTAGS].first<I+iq*/) {
					/* doesn't catch tag if spans line -- just make tag and hl static? */
					/*addtag(tags[MAXTAGS].type, tags[MAXTAGS].first, I+iq);*/
					if (hl==-1 && hl2==-1 && efirst!=-1/*<I+iq*/)
						addtag(etype, efirst, I+iq);
					efirst=-1;
				} /* else skip unrecognized escape codes like 8/9 */
			}
			p+=2;	/* gobble 0/1 and mysterious following 'm' */
			/* following 'm' (why?) gobbled in overarching for */
			/*continue;*/
		} else if ((isupper(*p) /*|| *p=='_' || *p=='&'*/) &&
				 (hl>=0 || isupper(p[1]) || (p[1]=='_' && p[2]!='\b') || p[1]=='&')) {
			if (hl==-1 && efirst==-1) { hl=I+iq; tag=SMALLCAPS; }
		} else {
			/* end of tag, one way or another */
			/* collect tags in this pass, interspersed later if need be */
			/* can't handle overlapping tags */
			if (hl>=0) {
				if (hl2==-1) addtag(tag, hl, I+iq);
				hl=-1;
			}
		}

		/** non-backspace related filtering **/
		/* case statement here in place of if chain? */
/* Tk 3.x's text widget tabs too crazy
		if (*p==' ' && strncmp("     ",p,5)==0) {
			xputchar('\t'); i+=5-1; ci++; continue;
		} else
*/
/* copyright symbol: too much work for so little
		if (p[i]=='o' && (strncmp("opyright (C) 19",&p[i],15)==0
				    || strncmp("opyright (c) 19",&p[i],15)==0)) {
			printf("opyright \xd3 19");
			addtag(SYMBOL, ci+9, ci+10);
			i+=15-1; ci+=13; continue;
		} else
*/
		if (*p=='(' && q>plain && (isalnum(q[-1])||strchr("._-+",q[-1])!=NULL)
		    && strcoloncmp(&p[1],')',vollist)
		    /* && p[1]!='s' && p[-1]!='`' && p[-1]!='\'' && p[-1]!='"'*/ ) {
			hl2=I+iq;
			for (r=q-1; r>=plain && (isalnum(*r)||strchr("._-+:",*r)!=NULL); r--)
				hl2--;
			/* else ref to a function? */
			/* maybe save position of opening paren so don't highlight it later */
		} else if (*p==')' && hl2!=-1) {
			/* don't overlap tags on man page referenes */
			while (tagc>0 && tags[tagc-1].last>hl2) tagc--;
			addtag(MANREF, hl2, I+iq+1);
			hl2=hl=-1;
		} else if (hl2!=-1) {
			if (!isalnum(*p)) hl2=-1;
		}


		/*assert(*p!='\0');*/
		if (!*p) break;	/* not just safety check -- check out sgmls.1 */

		*q++=*p;
		falluc = falluc && (isupper(*p) || isspace(*p) || strchr("-+&_'",*p)!=NULL);
		if (!scnt2) { *ph++=*p; phraselen++; }
		iq+=iscnt+1;
	}
	if (hl>=0) addtag(tag, hl, I+iq);
	else if (efirst>=0) addtag(etype, efirst, I+iq);
	*q=*ph='\0';
	linelen=iq+ccnt;


	/* special case for Solaris:
	   if line has ONLY <CODE> tags AND they SPAN line, convert to one tag */
	fCodeline=0;
	if (tagc && tags[0].first==0 && tags[tagc-1].last==linelen) {
		fCodeline=1;
		j=0;
		/* invariant: at start of a tag */
		for (i=0; fCodeline && i<tagc; i++) {
			if (tags[i].type!=BOLDITALICS /*&& tags[i].type!=BOLD*/) fCodeline=0;
			else if ((j=tags[i].last)<linelen) {
			  for (; j< tags[i+1].first ; j++)
			  	if (!isspace(plain[j])) { fCodeline=0; break; }
			}
		}
	}


	/* verify tag lists -- compiler has dead code elimination, I hope */
	for (i=tagci; i<tagc; i++) {
		/* verify valid ranges */
		assert(tags[i].type>NOTAG && tags[i].type<=MANREF);
		assert(tags[i].first>=I0 && tags[i].last<=linelen+I0);
		assert(tags[i].first<=tags[i].last);

		/* verify for no overlap with other tags */
		for (j=i+1; j<tagc; j++) {
			assert(tags[i].last<=tags[j].first /*|| tags[i].first>=tags[j].last*/);
		}
	}
}




/*
 * OUTPUT FORMATS
 *    *** break these out so can selectively include them in the binary ***
 *    *** does this save significant space? ***
 */



/*
 * Tk -- just emit list of text-tags pairs
 */

void
Tk(enum command cmd) {
	static int skip=0;	/* skip==1 when line has no text */
	/*static int markcnt=0;*/
	int i;

	/* invariant: always ready to insert text */

	switch (cmd) {
	   case BEGINDOC:
		I=0; CurLine=1;
		escchars = "\"[]$";
		printf(/*$t insert end */ "\"");
		break;
	   case ENDDOC:
		if (fHeadfoot) {
/*	grr, should have +mark syntax for Tk text widget! -- maybe just just +sect#, +subsect#
		printf("\\n\\n\" {} \"%s\\n\" {+headfoot h2}\n",HEADERANDFOOTER);
*/
			printf("\\n\\n\" {} \"%s\\n\" h2\n",HEADERANDFOOTER);
			/*printf("$t mark set headfoot %d.0\n",CurLine);*/
			CurLine++;

			for (i=0; i<CRUFTS; i++) {
				if (*cruft[i]) {
					printf(/*$t insert end */" {%s} sc \\n\n",cruft[i]);
					CurLine++;
				}
			}
		} else printf("\"\n");
		break;

	   case BEGINLINE:
		/*I=0; -- need to do this at end of line so set for filterline() */
		/* nothing to do at start of line except catch up on newlines */
		for (i=0; i<ncnt; i++) printf("\\n");
		CurLine+=ncnt;
		break;
	   case ENDLINE:
		if (!skip) printf("\\n");
		/* reflist collected from manref-tagged text -- no need for explicit list
		   easy to collect refs from entire page for Links menu
		if (!skip) {
			printf("\\n");
			if (sectheadid==SEEALSO) {
				printf("\"\n");
				printf("append manx(links) {%s,}\n", plain);
				printf("$t insert end \"");
			}
		}
		*/

		tagc=0;
		skip=0;
		CurLine++; I=0;
		/*if ((CurLine&0x3f)==0x3f) printf("\"\nupdate idletasks\n$t insert end \"");*/
		break;

	   case ENDSECTHEAD:
		/*printf("\\n\" {h2 +js%d}\n$t insert end \"",++markcnt); skip=1;*/
		printf("\\n\" h2 \"");
		tagc=0;
		/*
		printf("$t mark set js%d %d.0\n", ++markcnt, CurLine);
		printf("$t insert end \""); skip=1;
		*/
		skip=1;
		break;
	   case ENDSUBSECTHEAD:
		/*printf("\\n\" {+jss%d}\n$t insert end \"",++markcnt); skip=1;*/
		printf("\\n\" {} \"");	/* add h3? */
		tagc=0;
		/*
		printf("$t mark set jss%d %d.0\n", ++markcnt, CurLine);
		printf("$t insert end \""); skip=1;
		*/
		skip=1;
		break;
	   case BEGINTABLELINE:
		break;
	   case ENDTABLELINE:
		printf("\" tt \"");
		/*addtag(MONO, 0, I);*/
		break;

	   case CHANGEBAR:		putchar('|'); I++; break;
	   case CHARLQUOTE:
	   case CHARRQUOTE:
		putchar('\\'); putchar('"'); I++;
		break;
	   case CHARLSQUOTE:
	   case CHARRSQUOTE:
	   case CHARPERIOD:
	   case CHARDASH:
	   case CHARLT:
	   case CHARGT:
	   case CHARHAT:
	   case CHARVBAR:
	   case CHARAMP:
	   case CHARDAGGER:
	   case CHARPLUSMINUS:
		putchar(cmd); I++; break;
	   case CHARBACKSLASH:	printf("\\\\"); I++; break;
	   case CHARBULLET:		printf("\" {} %c symbol \"",c_bullet); I++; break;


	   case BEGINSECTHEAD:
	   case BEGINSUBSECTHEAD:
		tagc=0;	/* section and subsection formatting controlled descriptively */
		/* no break;*/

	   case BEGINBOLD:
	   case BEGINITALICS:
	   case BEGINBOLDITALICS:
	   case BEGINY:
	   case BEGINSC:
	   case BEGINMANREF:
		/* end text, begin attributed text */
		printf("\" {} \"");
		break;

	   /* rely on the fact that no more than one tag per range of text */
	   case ENDBOLD:		printf("\" b \""); break;
	   case ENDITALICS:		printf("\" i \""); break;
	   case ENDBOLDITALICS:	printf("\" bi \""); break;
	   case ENDY:			printf("\" symbol \""); break;
	   case ENDSC:			printf("\" sc \""); break;
	   case ENDMANREF:		printf("\" manref \""); break;
		/* presentation attributes dealt with at end of line */

	   case BEGINBODY: case ENDBODY:
	   case SHORTLINE:
	   case BEGINBULPAIR: case ENDBULPAIR:
	   case BEGINBULLET: case ENDBULLET:
	   case BEGINBULTXT: case ENDBULTXT:
	   case BEGINSECTION: case ENDSECTION:
	   case BEGINSUBSECTION: case ENDSUBSECTION:
	   case BEGINHEADER: case ENDHEADER:
	   case BEGINFOOTER: case ENDFOOTER:
	   case BEGINTABLE: case ENDTABLE:
		/* no action */
		break;
	}
}




/*
 * TkMan -- Tk format wrapped with commands
 */

void
TkMan(enum command cmd) {
	static int markcnt=0;
	int i;

	/* invariant: always ready to insert text */

	switch (cmd) {
	   case BEGINDOC:
		printf("$t insert end ");	/* opening quote supplied in Tk() below */
		Tk(cmd);
		break;
	   case ENDDOC:
		if (fHeadfoot) {
/*	grr, should have +mark syntax for Tk text widget!
		printf("\\n\\n\" {} \"%s\\n\" {+headfoot h2}\n",HEADERANDFOOTER);
*/
			printf("\\n\\n\" {} \"%s\\n\" h2\n",HEADERANDFOOTER);
			printf("$t mark set headfoot %d.0\n",CurLine);
			CurLine++;

			for (i=0; i<CRUFTS; i++) {
				if (*cruft[i]) {
					printf("$t insert end {%s} sc \\n\n",cruft[i]);
					CurLine++;
				}
			}
		} else printf("\"\n");


/*
		printf("$t insert 1.0 {");
		for (i=0; i<BUFSIZ; i++) if (tabgram[i]) printf("%d=%d, ", i, tabgram[i]);
		printf("\\n\\n}\n");
*/

		break;

	   case BEGINLINE:
	     Tk(cmd);
		for (i=0; i<ccnt; i++) putchar('|');
		for (i=0; i<(scnt/5); i++) putchar('\t');
		for (i=0; i<(scnt%5); i++) putchar(' ');
		/*if (fCodeline) printf("CODE");*/
		break;
	   case CHANGEBAR:
		break;
	   case ENDLINE:
		/*tagc=0; CurLine++; I=0;*/ Tk(cmd);
		if ((CurLine&0x3f)==0x3f) printf("\"\nupdate idletasks\n$t insert end \"");
		break;

	   case ENDSECTHEAD:
		Tk(cmd);
		printf("\"\n"); /* close off current stream */
		printf("$t mark set js%d %d.0\n", ++markcnt, CurLine);
		printf("$t insert end \"");
		break;

	   case ENDSUBSECTHEAD:
		Tk(cmd);
		/*printf("\\n\" {+jss%d}\n$t insert end \"",++markcnt); skip=1;*/
		printf("\"\n"); /* close off current stream */
		printf("$t mark set jss%d %d.0\n", ++markcnt, CurLine);
		printf("$t insert end \"");
		break;

	   default:	/* if not caught above, it's the same as Tk */
		Tk(cmd);
	}
}




/*
 * ASCII
 */

void
ASCII(enum command cmd) {
	int i;

	switch (cmd) {
	   case ENDDOC:
		if (fHeadfoot)	{
			printf("\n%s\n", HEADERANDFOOTER);
			for (i=0; i<CRUFTS; i++) if (*cruft[i]) printf("%s\n",cruft[i]);
		}
		break;
	   case CHARRQUOTE:
	   case CHARLQUOTE:
		putchar('"');
		break;
	   case CHARLSQUOTE:
	   case CHARRSQUOTE:
		putchar('\'');
		break;
	   case CHARPERIOD:
	   case CHARDASH:
	   case CHARLT:
	   case CHARAMP:
	   case CHARBACKSLASH:
	   case CHARGT:
	   case CHARHAT:
	   case CHARVBAR:
		putchar(cmd); break;
	   case CHARDAGGER:	putchar('+'); break;
	   case CHARBULLET:	putchar('*'); break;
	   case CHARPLUSMINUS: printf("+-"); break;
	   case CHANGEBAR:	putchar('|'); break;

	   case BEGINLINE:
		for (i=0; i<ncnt; i++) putchar('\n');
		break;
	   case ENDLINE:
		putchar('\n');
		CurLine++;
		tagc=0;
		break;

	   case BEGINDOC:
	   case BEGINBODY: case ENDBODY:
	   case BEGINHEADER: case ENDHEADER:
	   case BEGINFOOTER: case ENDFOOTER:
	   case BEGINSECTION: case ENDSECTION:
	   case BEGINSECTHEAD: case ENDSECTHEAD:
	   case BEGINSUBSECTHEAD: case ENDSUBSECTHEAD:
	   case BEGINBULPAIR: case ENDBULPAIR:
	   case BEGINBULLET: case ENDBULLET:
	   case BEGINBULTXT: case ENDBULTXT:
	   case BEGINSUBSECTION: case ENDSUBSECTION:

	   case SHORTLINE:
	   case BEGINTABLE: case ENDTABLE:
	   case BEGINTABLELINE: case ENDTABLELINE:
	   case BEGINBOLD: case ENDBOLD:
	   case BEGINITALICS: case ENDITALICS:
	   case BEGINMANREF: case ENDMANREF:
	   case BEGINBOLDITALICS: case ENDBOLDITALICS:
	   case BEGINY: case ENDY:
	   case BEGINSC: case ENDSC:
		/* nothing */
		break;
	}
}



/*
 * Perl 5 pod ("plain old documentation")
 */

void
pod(enum command cmd) {
	static int curindent=0;
	int i;

	if (hanging==-1) {
		if (curindent) hanging=curindent; else hanging=5;
	}


	if (cmd==BEGINBULPAIR) {
		if (curindent && hanging!=curindent) printf("\n=back\n\n");
		if (hanging!=curindent) printf("\n=over %d\n\n",hanging);
		curindent=hanging;
	} else if (cmd==ENDBULPAIR) {
		/* nothing--wait until next command */
	} else if (cmd==BEGINLINE && !scnt) {
		if (curindent) printf("\n=back\n\n");
		curindent=0;
	} else if (cmd==BEGINBODY) {
		if (curindent) {
			printf("\n=back\n\n");
			curindent=0;
			auxindent=0;
		}
	}
/*
	   case BEGINBULPAIR:
		printf("=over %d\n\n", hanging);
		break;
	   case ENDBULPAIR:
		printf("\n=back\n\n");
		break;
*/
	switch (cmd) {
	   case BEGINDOC: I=0; break;

	   case CHARRQUOTE:
	   case CHARLQUOTE:
		putchar('"');
		break;
	   case CHARLSQUOTE:
	   case CHARRSQUOTE:
		putchar('\'');
		break;
	   case CHARPERIOD:
	   case CHARDASH:
	   case CHARLT:
	   case CHARAMP:
	   case CHARBACKSLASH:
	   case CHARGT:
	   case CHARHAT:
	   case CHARVBAR:
		putchar(cmd); break;
	   case CHARDAGGER:	putchar('+'); break;
	   case CHARPLUSMINUS: printf("+-"); break;
	   case CHANGEBAR:	putchar('|'); break;
	   case CHARBULLET:	putchar('*'); break;

	   case BEGINLINE:
		for (i=0; i<ncnt; i++) putchar('\n');
		CurLine+=ncnt;
		break;
	   case ENDLINE:
		putchar('\n');
		CurLine++;
		tagc=0;
		I=0;
		break;

	   case BEGINSECTHEAD:	printf("=head1 "); break;
	   case BEGINSUBSECTHEAD:	printf("=head2 "); break;

	   case ENDSECTHEAD:
	   case ENDSUBSECTHEAD:
		printf("\n");
		break;

	   case BEGINBOLD:	printf("B<"); break;
	   case BEGINITALICS:	printf("I<"); break;
	   case BEGINMANREF:	printf("L<"); break;

	   case ENDBOLD:
	   case ENDITALICS:
	   case ENDMANREF:
		printf(">");
		break;

	   case BEGINBULLET:
		printf("\n=item ");
		break;
	   case ENDBULLET:
		printf("\n\n");
		fcharout=0;
		break;
	   case BEGINBULTXT:
		fcharout=1;
		auxindent=hanging;
		break;
	   case ENDBULTXT:
		auxindent=0;
		break;


	   case ENDDOC:
	   case BEGINBODY: case ENDBODY:
	   case BEGINHEADER: case ENDHEADER:
	   case BEGINFOOTER: case ENDFOOTER:
	   case BEGINSECTION: case ENDSECTION:
	   case BEGINSUBSECTION: case ENDSUBSECTION:
	   case BEGINBULPAIR: case ENDBULPAIR:

	   case SHORTLINE:
	   case BEGINTABLE: case ENDTABLE:
	   case BEGINTABLELINE: case ENDTABLELINE:
	   case BEGINBOLDITALICS: case ENDBOLDITALICS:
	   case BEGINY: case ENDY:
	   case BEGINSC: case ENDSC:
		/* nothing */
		break;
	}
}



void
Sections(enum command cmd) {

	switch (cmd) {
	   case ENDSECTHEAD:
	   case ENDSUBSECTHEAD:
		putchar('\n');
	   case BEGINDOC:
		fcharout=0;
		break;
	   case BEGINSUBSECTHEAD:
		printf("  ");
		/* no break */
	   case BEGINSECTHEAD:
		fcharout=1;
		break;
	   case CHARRQUOTE:
	   case CHARLQUOTE:
		xputchar('"');
		break;
	   case CHARLSQUOTE:
	   case CHARRSQUOTE:
		xputchar('\'');
		break;
	   case BEGINTABLE: case ENDTABLE:
	   case BEGINTABLELINE: case ENDTABLELINE:
		break;
	   case CHARPERIOD:
	   case CHARDASH:
	   case CHARBACKSLASH:
	   case CHARLT:
	   case CHARGT:
	   case CHARHAT:
	   case CHARVBAR:
	   case CHARAMP:
		xputchar(cmd); break;
	   case CHARDAGGER:	xputchar('+'); break;
	   case CHARBULLET:	xputchar('*'); break;
	   case CHARPLUSMINUS: xputchar('+'); xputchar('-'); break;
	   default:
		/* nothing */
		break;
	}
}



void
Roff(enum command cmd) {
	int i;

	switch (cmd) {
	   case BEGINDOC:
		I=1;
		printf(".TH %s %s \"generated by RosettaMan\" UCB\n",manName,manSect);
		printf(".\\\"  %s,\n",providence);
		printf(".\\\"  %s\n",anonftp);
		CurLine=1;
		break;
	   case BEGINBODY:		/*printf(".LP\n");*/ break;
	   case BEGINSECTHEAD:	printf(".SH "); break;
	   case BEGINSUBSECTHEAD:printf(".SS "); break;
	   case BEGINBULPAIR:	printf(".IP "); break;
	   case SHORTLINE:		printf("\n.br"); break;
	   case BEGINBOLD:		printf("\\fB"); break;	/* \n.B -- grr! */
	   case ENDBOLD:		printf("\\fR"); break;	/* putchar('\n'); */
	   case BEGINITALICS:	printf("\\fI"); break;
	   case ENDITALICS:		printf("\\fR"); break;
	   case BEGINBOLDITALICS:printf("\\f4"); break;
	   case ENDBOLDITALICS:	printf("\\fR"); break;

	   case CHARLQUOTE:		printf("\\*(rq"); break;
	   case CHARRQUOTE:		printf("\\*(lq"); break;
	   case CHARLSQUOTE:
	   case CHARRSQUOTE:
		putchar('\'');
		break;
	   case CHARPERIOD:		if (I==1) printf("\\&"); putchar('.'); I++; break;
	   case CHARDASH:		printf("\\-"); break;
	   case CHARLT:
	   case CHARGT:
	   case CHARHAT:
	   case CHARVBAR:
	   case CHARAMP:
		putchar(cmd); break;
	   case CHARBULLET:		printf("\\(bu"); break;
	   case CHARDAGGER:		printf("\\(dg"); break;
	   case CHARPLUSMINUS:	printf("\\(+-"); break;
	   case CHANGEBAR:		putchar('|'); break;
	   case CHARBACKSLASH:	printf("\\\\"); break;  /* correct? */

	   case BEGINLINE:
		for (i=0; i<ncnt; i++) putchar('\n');
		break;

	   case BEGINBULLET:	putchar('"'); break;
	   case ENDBULLET:		printf("\"\n"); break;

	   case ENDLINE:
		tagc=0;
		CurLine++;
		I=1;
		/* no break */
	   case ENDSUBSECTHEAD:
	   case ENDSECTHEAD:
	   case ENDDOC:
		putchar('\n');
		break;
	   case ENDBODY:
	   case ENDBULPAIR:
	   case BEGINBULTXT: case ENDBULTXT:
	   case BEGINSECTION: case ENDSECTION:
	   case BEGINSUBSECTION: case ENDSUBSECTION:
	   case BEGINY: case ENDY:
	   case BEGINSC: case ENDSC:
	   case BEGINTABLE: case ENDTABLE:
	   case BEGINTABLELINE: case ENDTABLELINE:
	   case BEGINHEADER: case ENDHEADER:
	   case BEGINFOOTER: case ENDFOOTER:
	   case BEGINMANREF: case ENDMANREF:
		/* nothing */
		break;
	}
}



/*
 * Ensemble
 */

void
EnsembleDumpTags(void) {
	int i,tag;
	int fI=0, fB=0;

	if (!tagc) return;

	printf("}{}{");		/* header */

	/* italics */
	for (i=0; i<tagc; i++) {
		tag = tags[i].type;
		if (tag==ITALICS||tag==BOLDITALICS) {
			if (!fI) {printf("ITALIC=("); fI=1;}
			printf("(%d,%d,[T])", tags[i].first, tags[i].last);
		}
	}
	if (fI) printf(")");

	/* bold */
	for (i=0; i<tagc; i++) {
		tag = tags[i].type;
		if (tag==BOLD||tag==BOLDITALICS) {
			if (!fB) {printf(",BOLD=("); fB=1;}
			printf("(%d,%d,[T])", tags[i].first, tags[i].last);
		}
	}
	if (fB) printf(")");

	/* man ref? */
/*
	for (i=0; i<tagc; i++) {
		tag = tags[i].type;
		if (tag==MANREF) {
			if (!fH) {printf(",HYPER=("); fH=1;}
			printf("(%d,%d,[???])", tags[i].first, tags[i].last);
		}
	}
	if (fH) printf(")");
*/

	/* don't put  printf("}");  here as trailer -- controlling function expects to close it */

	tagc=0;
}

void
Ensemble(enum command cmd) {

	switch (cmd) {
	   case BEGINDOC:
		I=0;
		printf("DOCUMENT MANPAGE\n<MANPAGE>\n");
		escchars = "{}\\";
		break;
	   case ENDDOC:	printf("</MANPAGE>\n"); break;
	   case BEGINBODY:
		printf("<SUBSECTIONBODY><BODY>{");
		break;
	   case ENDBODY:
		CurLine++;
		EnsembleDumpTags(); printf("}</BODY></SUBSECTIONBODY>\n");
		tagc=0;
		break;
	   case BEGINSECTION:		printf("<SECTION>"); break;
	   case ENDSECTION:			printf("</SECTION>\n"); break;
	   case BEGINSECTHEAD:		printf("<SECTHEAD>{"); break;
	   case ENDSECTHEAD:		tagc=0; I=0; printf("}</SECTHEAD>\n"); break;
	   case BEGINSUBSECTHEAD:	printf("<SUBSECTHEAD>{"); break;
	   case ENDSUBSECTHEAD:		tagc=0; I=0; printf("}</SUBSECTHEAD>\n"); break;
	   case BEGINBULPAIR:		printf("<SUBSECTIONBODY><LISTELEMENT>"); break;
	   case ENDBULPAIR:			printf("</LISTELEMENT></SUBSECTIONBODY>\n"); break;
	   case BEGINBULLET:		printf("<BULLET>{"); break;
	   case ENDBULLET:			tagc=0; I=0; printf("}</BULLET>"); break;
	   case BEGINBULTXT:		printf("<BULLETTEXT>{"); break;
	   case ENDBULTXT:
		EnsembleDumpTags();
		CurLine++;
		printf("}</BULLETTEXT>");
		break;
	   case BEGINSUBSECTION:	printf("<SUBSECTIONBODY><SUBSECTION>\n"); break;
	   case ENDSUBSECTION:	printf("</SUBSECTION></SUBSECTIONBODY>\n"); break;
	   case SHORTLINE:		/*poppush(prevcmd);*/ break;

	   case CHARRQUOTE:
	   case CHARLQUOTE:
		putchar('"'); I++;
		break;
	   case CHARLSQUOTE:
	   case CHARRSQUOTE:
		putchar('\'');
		break;
	   case CHARPERIOD:
	   case CHARDASH:
	   case CHARBACKSLASH:
	   case CHARLT:
	   case CHARGT:
	   case CHARHAT:
	   case CHARVBAR:
	   case CHARAMP:
	   case CHARBULLET:
	   case CHARDAGGER:
	   case CHARPLUSMINUS:
		putchar(cmd); I++; break;

	   case ENDLINE:		putchar(' '); I++; break;
	   case CHANGEBAR:
		/* maybe something later */
	   case BEGINLINE: 
	   case BEGINY: case ENDY:
	   case BEGINHEADER: case ENDHEADER:
	   case BEGINFOOTER: case ENDFOOTER:
	   case BEGINBOLD: case ENDBOLD:
	   case BEGINITALICS: case ENDITALICS:
	   case BEGINBOLDITALICS: case ENDBOLDITALICS:
	   case BEGINSC: case ENDSC:
	   case BEGINTABLE: case ENDTABLE:
	   case BEGINTABLELINE: case ENDTABLELINE:

	   case BEGINMANREF:
	   case ENDMANREF:
		/* easy strike for hypertext--want to dynamically generate, though */

		/* nothing */
		break;
	}
}



char *manTitle = MANTITLEPRINTF;
char *manRef = MANREFPRINTF;
int fmanRef=1;	/* valid man ref? */

/*
 * HTML
 */

void
HTML(enum command cmd) {
	static int pre=0;
	int i,j;
	int lasttoc;
	char *p, *p0;

	/* always respond to these signals */
	switch (cmd) {
	   case CHARLQUOTE:
	   case CHARRQUOTE:
		printf("&quot;");
		break;
	   case CHARLSQUOTE:
	   case CHARRSQUOTE:
	   case CHARPERIOD:
	   case CHARDASH:
	   case CHARBACKSLASH:
	   case CHARHAT:
	   case CHARVBAR:
	   case CHARBULLET:
	   case CHARDAGGER:
	   case CHARPLUSMINUS:
		putchar(cmd); break;
	   case CHARGT:		printf("&gt;"); break;
	   case CHARLT:		printf("&lt;"); break;
	   case CHARAMP:		printf("&amp;"); break;
	   default:
		break;
	}

	/* while in pre mode... */
	if (pre) {
		switch (cmd) {
		   case ENDLINE:	I=0; tagc=0; CurLine++; if (!fPara && scnt) printf("<BR>"); printf("\n"); break;
		   case ENDTABLE:	printf("</PRE><br>\n"); pre=0; fQS=fIQS=fPara=1; break;
		   default:
			/* nothing */
			break;
		}
		return;
	}

	/* usual operation */
	switch (cmd) {
	   case BEGINDOC:
		/* escchars = ...  => HTML doesn't backslash-quote metacharacters */
		printf("<!-- %s, -->\n",providence);
		printf("<!-- %s -->\n",anonftp);
		printf("<HTML>\n<HEAD>\n");
/*		printf("<ISINDEX>\n");*/
		/* better title possible? */
		printf("<TITLE>"); printf(manTitle, manName, manSect); printf("</TITLE>\n");
		printf("</HEAD>\n<BODY>\n");
		printf("<A HREF=\"#toc\">%s</A><P>\n", TABLEOFCONTENTS);
		I=0;
		break;
	   case ENDDOC:
		/* header and footer wanted? */
		printf("<P>\n");
		if (fHeadfoot) {
			printf("<HR><H2>%s</H2>\n", HEADERANDFOOTER);
			for (i=0; i<CRUFTS; i++) if (*cruft[i]) printf("%s<BR>\n",cruft[i]);
		}

		if (!tocc) {
			printf("\n<H1>ERROR: Empty man page</H1>\n");
		} else {
			printf("\n<HR><P>\n");
			printf("<A NAME=\"toc\"><B>%s</B></A><P>\n", TABLEOFCONTENTS);
			printf("<UL>\n");
			for (i=0, lasttoc=BEGINSECTION; i<tocc; lasttoc=toc[i].type, i++) {
				if (lasttoc!=toc[i].type) {
					if (toc[i].type==BEGINSUBSECTION) printf("<UL>\n");
					else printf("</UL>\n");
				}
				printf("<LI><A NAME=\"toc%d\" HREF=\"#sect%d\">%s</A></LI>\n", i+1, i+1, toc[i].text);
			}
			if (lasttoc==BEGINSUBSECTION) printf("</UL>");
			printf("</UL>\n");
		}
		printf("</BODY></HTML>\n");
		break;
	   case BEGINBODY:		break;
	   case ENDBODY:		break;

	   case BEGINSECTHEAD:
		printf("\n<H2><A NAME=\"sect%d\" HREF=\"#toc%d\">", tocc, tocc);
		break;
	   case ENDSECTHEAD:
		printf("</A></H2>\n");
		/* useful extraction from FILES, ENVIRONMENT? */
		break;
	   case BEGINSUBSECTHEAD:
		printf("\n<H3><A NAME=\"sect%d\" HREF=\"#toc%d\">", tocc, tocc);
		break;
	   case ENDSUBSECTHEAD:
		printf("</A></H3>\n");
		break;
	   case BEGINSECTION:	break;
	   case ENDSECTION:		break;
	   case BEGINSUBSECTION:	break;
	   case ENDSUBSECTION:	break;

	   case BEGINBULPAIR:	printf("<DL>\n"); break;
	   case ENDBULPAIR:		printf("</DL>\n"); break;
	   case BEGINBULLET:	printf("<DT>"); break;
	   case ENDBULLET:		break;
	   case BEGINBULTXT:	printf("<DD>"); break;
	   case ENDBULTXT:		printf("</DD>\n"); break;

	   case BEGINLINE:
		if (ncnt) printf("<P>\n");

		/* trailing spaces already trimmed off, so look for eol now */
		if (fCodeline) {
		  printf("<CODE>");
		  for (i=0; i<scnt-indent; i++) printf("&nbsp;");		/* ? */
		  tagc=0;

		  /* already have .tag=BOLDITALICS, .first=0 */
		  /* would be more elegant, but can't print initial spaces before first tag
		  tags[0].last = linelen;
		  tagc=1;
		  fIQS=0;
		  */
		}

		break;

	   case ENDLINE:
		/*if (fCodeline) { fIQS=1; fCodeline=0; }*/
		if (fCodeline) { printf("</CODE><BR>"); fCodeline=0; }
		I=0; tagc=0; CurLine++; if (!fPara && scnt) printf("<BR>"); printf("\n");
		break;

	   case SHORTLINE:
		if (fCodeline) { printf("</CODE>"); fCodeline=0; }
		if (!fIP) printf("<BR>\n");
		break;

	   case BEGINTABLE:		printf("<BR><PRE>\n"); pre=1; fQS=fIQS=fPara=0; break;
	   case ENDTABLE:		printf("</PRE><BR>\n"); pre=0; fQS=fIQS=fPara=1; break;
		/* could use a new list type */

	   case BEGINBOLD:		printf("<B>"); break;
	   case ENDBOLD:		printf("</B>"); break;
	   case BEGINITALICS:	printf("<I>"); break;
	   case ENDITALICS:		printf("</I>"); break;
	   case BEGINBOLDITALICS:printf("<CODE>"); break;
	   case ENDBOLDITALICS:	printf("</CODE>"); break;
	   case BEGINMANREF:
		/* put separation of name and number into caller */
		for (p=hitxt; *p && *p!='('; p++) /* empty */;
		*p++='\0'; p0=p;
		for (; *p && *p!=')'; p++) /* empty */;
		*p='\0';
		if (fmanRef) { printf("<A HREF=\""); printf(manRef, hitxt, p0); printf("\">"); }
		else printf("<I>");
		break;
	   case ENDMANREF:
		if (fmanRef) printf("</A>"); else printf("</I>");
		break;

	   case BEGINSC: case ENDSC:
	   case BEGINY: case ENDY:
	   case BEGINHEADER: case ENDHEADER:
	   case BEGINFOOTER: case ENDFOOTER:
	   case BEGINTABLELINE: case ENDTABLELINE:
	   case CHANGEBAR:
	   default:
		/* nothing */
		break;
	}
}



/*
 * SGML
 */

/* same as HTML but just has man page-specific DTD */
/* follows the Davenport DocBook DTD v2.3, availble from ftp.ora.com */

/*char *docbookpath = "docbook.dtd";*/

void
SGML(enum command cmd) {
	static int pre=0;
	int i,j;
	int lasttoc;
	char *p, *p0;
	static int fRefEntry=0;
	static int fRefPurpose=0;
	/*static char *bads => SGML doesn't backslash-quote metacharacters */

/*
	fprintf(stderr,
		   "The framework for SGML is in place but not done.  If you\n"
		   "are familiar with the DocBook DTD, however, it shouldn't be\n"
 		   "too difficult to finish it.  If you do so, please send your\n"
		   "code to me so that I may share the wealth in the next release.\n"
		   );
	exit(1);
*/

	/* always respond to these signals */
	switch (cmd) {
	   case CHARLQUOTE:
	   case CHARRQUOTE:
		printf("&quot;");
		break;
	   case CHARLSQUOTE:
	   case CHARRSQUOTE:
	   case CHARPERIOD:
	   case CHARHAT:
	   case CHARVBAR:
	   case CHARBULLET:
	   case CHARDAGGER:
	   case CHARPLUSMINUS:
		putchar(cmd); break;
	   case CHARAMP:		printf("&amp;"); break;
	   case CHARDASH:
		if (sectheadid==NAME && !fRefPurpose) {
			printf("</RefEntry><RefPurpose>");
			fRefPurpose=1;
		} else putchar('-');
		break;
	   case CHARBACKSLASH:	putchar('\\'); break;
	   case CHARGT:		printf("&gt;"); break;
	   case CHARLT:		printf("&lt;"); break;
	   default:
		break;
	}

	/* while in pre mode... */
	if (pre) {
		switch (cmd) {
		   case ENDLINE:	I=0; tagc=0; CurLine++; if (!fPara && scnt) printf("<BR>"); printf("\n"); break;
		   case ENDTABLE:	printf("</PRE><br>\n"); pre=0; fQS=fIQS=fPara=1; break;
		   default:
			/* nothing */
			break;
		}
		return;
	}

	/* usual operation */
	switch (cmd) {
	   case BEGINDOC:
		/*printf("<!DOCTYPE chapter SYSTEM \"%s\">\n", docbookpath);*/
		printf("<!--\n\n\tI am looking for help to finish SGML.\n\n-->\n");

		printf("<!-- %s\n",providence);
		printf("     %s -->\n\n",anonftp);
		/* better title possible? */
		printf("<RefEntry ID=%s.%s>\n", manName, manSect);
		printf("<RefMeta><RefEntryTitle>%s</RefEntryTitle>", manName);
		printf("<ManVolNum>%s</ManVolNum></RefMeta>\n\n", manSect);

		I=0;
		break;

	   case ENDDOC:
		/* header and footer wanted? */
		if (fHeadfoot) {
			printf("<RefSect1><Title>%s</Title>\n", HEADERANDFOOTER);
			for (i=0; i<CRUFTS; i++) if (*cruft[i]) printf("<Para>%s\n",cruft[i]);
			printf("</RefSect1>");
		}

		/* table of contents, such as found in HTML, can be generated automatically by SGML software */

		printf("</RefEntry>\n");
		break;
	   case BEGINBODY:		printf("\n\n<Para>"); break;
	   case ENDBODY:		break;

	   case BEGINSECTHEAD:
	   case BEGINSUBSECTHEAD:
		printf("<Title>");
		break;
	   case ENDSECTHEAD:
	   case ENDSUBSECTHEAD:
		printf("</Title>");
		break;

	   case BEGINSECTION:
		if (sectheadid==NAME) printf("<RefNameDiv>");
			/*printf("<RefEntry>");  -- do lotsa parsing here for RefName, RefPurpose*/
		else if (sectheadid==SYNOPSIS) printf("<RefSynopsisDiv>");
		else printf("<RefSect1>");
		break;
	   case ENDSECTION:
		if (oldsectheadid==NAME) printf("</RefNameDiv>\n\n");
		else if (oldsectheadid==SYNOPSIS) printf("</RefSynopsisDiv>\n\n");
		else printf("</RefSect1>\n\n");
		break;

	   case BEGINSUBSECTION:	printf("<RefSect2>"); break;
	   case ENDSUBSECTION:	printf("</RefSect2>"); break;

	   case BEGINBULPAIR:	printf("<ItemizedList MARK=Bullet>\n"); break;
	   case ENDBULPAIR:		printf("</ItemizedList>\n"); break;
	   case BEGINBULLET:	printf("<Term>"); break;
	   case ENDBULLET:		printf("</Term>"); break;
	   case BEGINBULTXT:	printf("<ListItem><Para>"); break;
	   case ENDBULTXT:		printf("</Para></ListItem>\n"); break;

	   case BEGINLINE:
		/* remember, get BEGINBODY call at start of paragraph */
		if (fRefEntry) {
			if (fRefPurpose) {
				for (p=plain; *p!='-'; p++) {
					/* nothing?! */
				}
			}
		}

		break;

	   case ENDLINE:
		/*if (fCodeline) { fIQS=1; fCodeline=0; }*/
		if (fCodeline) { printf("</CODE><BR>"); fCodeline=0; }
		I=0; tagc=0; CurLine++; if (!fPara && scnt) printf("<BR>"); printf("\n");
		break;

	   case SHORTLINE:
		if (fCodeline) { printf("</CODE>"); fCodeline=0; }
		if (!fIP) printf("<BR>\n");
		break;

	   case BEGINTABLE:		printf("<BR><PRE>\n"); pre=1; fQS=fIQS=fPara=0; break;
	   case ENDTABLE:		printf("</PRE><BR>\n"); pre=0; fQS=fIQS=fPara=1; break;
		/* could use a new list type */

	   /* have to make some guess about bold and italics */
	   case BEGINBOLD:		printf("<B>"); break;
	   case ENDBOLD:		printf("</B>"); break;
	   case BEGINITALICS:	printf("<I>"); break;
	   case ENDITALICS:		printf("</I>"); break;
	   case BEGINBOLDITALICS:printf("<CODE>"); break;
	   case ENDBOLDITALICS:	printf("</CODE>"); break;
	   case BEGINMANREF:
/*
		for (p=hitxt; *p && *p!='('; p++) /* empty * /;
		*p++='\0'; p0=p;
		for (; *p && *p!=')'; p++) /* empty * /;
		*p='\0';
		if (fmanRef) { printf("<LINK LINKEND=\""); printf(manRef, hitxt, p0); printf("\">"); }
		else printf("<I>");
*/
		printf("<Command>");
		break;
	   case ENDMANREF:
/*		if (fmanRef) printf("</LINK>"); else printf("</I>");*/
		printf("</Command>");
		break;

	   case BEGINSC: case ENDSC:
	   case BEGINY: case ENDY:
	   case BEGINHEADER: case ENDHEADER:
	   case BEGINFOOTER: case ENDFOOTER:
	   case BEGINTABLELINE: case ENDTABLELINE:
	   case CHANGEBAR:
	   default:
		/* nothing */
		break;
	}
}



/* generates MIME compliant to RFC 1563 */

void
MIME(enum command cmd) {
	static int pre=0;
	int i;
	int lasttoc;
	char *p, *p0;

	/* always respond to these signals */
	switch (cmd) {
	   case CHARDASH:
	   case CHARAMP:
	   case CHARPERIOD:
		putchar(cmd); break;
	   case CHARLSQUOTE:	putchar('`'); break;
	   case CHARRSQUOTE:	putchar('\''); break;
	   case CHARBULLET:		putchar('*'); break;
	   case CHARDAGGER:		putchar('|'); break;
	   case CHARPLUSMINUS:	printf("+-"); break;
	   case CHARLQUOTE:
	   case CHARRQUOTE:
		putchar('"');
		break;
	   case CHARBACKSLASH:	/* these should be caught as escaped chars */
	   case CHARGT:
	   case CHARLT:
		assert(1);
		break;
	   default:
		break;
	}

	/* while in pre mode... */
	if (pre) {
		switch (cmd) {
		   case ENDLINE:	I=0; tagc=0; CurLine++; if (!fPara && scnt) printf("\n\n"); break;
		   case ENDTABLE:	printf("</fixed>\n\n"); pre=0; fQS=fIQS=fPara=1; break;
		   default:
			/* nothing */
			break;
		}
		return;
	}

	/* usual operation */
	switch (cmd) {
	   case BEGINDOC:
		printf("Content-Type: text/enriched\n");
		printf("Text-Width: 60\n");
		escchars = "<>\\";

		I=0;
		break;
	   case ENDDOC:
		/* header and footer wanted? */
		printf("\n\n");
		if (fHeadfoot) {
			printf("\n");
			MIME(BEGINSECTHEAD); printf("%s",HEADERANDFOOTER); MIME(ENDSECTHEAD);
			for (i=0; i<CRUFTS; i++) if (*cruft[i]) printf("\n%s\n",cruft[i]);
		}

/*
		printf("\n<comment>\n");
		printf("%s\n%s\n", providence, anonftp);
		printf("</comment>\n\n");
*/

/*
		printf("\n<HR><P>\n");
		printf("<A NAME=\"toc\"><B>%s</B></A><P>\n", TABLEOFCONTENTS);
		printf("<UL>\n");
		for (i=0, lasttoc=BEGINSECTION; i<tocc; lasttoc=toc[i].type, i++) {
		  if (lasttoc!=toc[i].type) {
		    if (toc[i].type==BEGINSUBSECTION) printf("<UL>\n");
		    else printf("</UL>\n");
		  }
		  printf("<LI><A NAME=\"toc%d\" HREF=\"#sect%d\">%s</A></LI>\n", i+1, i+1, toc[i].text);
		}
		if (lasttoc==BEGINSUBSECTION) printf("</UL>");
		printf("</UL>\n");
		printf("</BODY></HTML>\n");
*/
		break;
	   case BEGINBODY:		break;
	   case ENDBODY:		break;

	   case BEGINSECTHEAD:
		printf("\n<bigger><bigger><underline>");
		/*A NAME=\"sect%d\" HREF=\"#toc%d\"><H2>", tocc, tocc);*/
		break;
	   case ENDSECTHEAD:
		printf("</underline></bigger></bigger>\n\n<indent>");
		/* useful extraction from files, environment? */
		break;
	   case BEGINSUBSECTHEAD:
		printf("<bigger>");
		/*\n<A NAME=\"sect%d\" HREF=\"#toc%d\"><H3>", tocc, tocc);*/
		break;
	   case ENDSUBSECTHEAD:
		printf("</bigger>\n\n</indent>");
		break;
	   case BEGINSECTION:
	   case BEGINSUBSECTION:
		break;
	   case ENDSECTION:
	   case ENDSUBSECTION:
		printf("</indent>\n");
		break;

	   case BEGINBULPAIR:	break;
	   case ENDBULPAIR:		break;
	   case BEGINBULLET:	printf("<bold>"); break;
	   case ENDBULLET:		printf("</bold>\t"); break;
	   case BEGINBULTXT:	printf("<indent>"); break;
	   case ENDBULTXT:		printf("</indent>\n"); break;

	   case BEGINLINE:		if (ncnt) printf("\n\n"); break;
	   case ENDLINE:		I=0; tagc=0; CurLine++; printf("\n"); break;
	   case SHORTLINE:		if (!fIP) printf("\n\n"); break;
	   case BEGINTABLE:		printf("<nl><fixed>\n"); pre=1; fQS=fIQS=fPara=0; break;
	   case ENDTABLE:		printf("</fixed><nl>\n"); pre=0; fQS=fIQS=fPara=1; break;
		/* could use a new list type */

	   case BEGINBOLD:		printf("<bold>"); break;
	   case ENDBOLD:		printf("</bold>"); break;
	   case BEGINITALICS:	printf("<italics>"); break;
	   case ENDITALICS:		printf("</italics>"); break;
	   case BEGINBOLDITALICS:printf("<bold><italics>"); break;
	   case ENDBOLDITALICS:	printf("</bold></italics>"); break;
	   case BEGINMANREF:
		printf("<x-color><param>blue</param>");
/* how to make this hypertext?
		for (p=hitxt; *p && *p!='('; p++) /* empty * /;
		*p++='\0'; p0=p;
		for (; *p && *p!=')'; p++) /* empty * /;
		*p='\0';
		printf("<A HREF=\""); printf(manRef, hitxt, p0); printf("\">");
*/
		break;
	   case ENDMANREF:
		printf("</x-color>");
		break;

	   case BEGINSC: case ENDSC:
	   case BEGINY: case ENDY:
	   case BEGINHEADER: case ENDHEADER:
	   case BEGINFOOTER: case ENDFOOTER:
	   case BEGINTABLELINE: case ENDTABLELINE:
	   case CHANGEBAR:
	   default:
		/* nothing */
		break;
	}
}



/*
 * LaTeX
 */

void
LaTeX(enum command cmd) {

	switch (cmd) {
	   case BEGINDOC:
		escchars = "$&%#_{}"; /* and more to come? */
		printf("%% %s,\n", providence);
		printf("%% %s\n\n", anonftp);
		/* definitions */
		printf(
		  "\\documentstyle{article}\n"
		  "\\def\\thefootnote{\\fnsymbol{footnote}}\n"
		  "\\setlength{\\parindent}{0pt}\n"
		  "\\setlength{\\parskip}{0.5\\baselineskip plus 2pt minus 1pt}\n"
		  "\\begin{document}\n"
			  );
		I=0;
		break;
	   case ENDDOC:
		/* header and footer wanted? */
		printf("\n\\end{document}\n");

		break;
	   case BEGINBODY:		break;
	   case ENDBODY:		break;
	   case BEGINSECTION:	break;
	   case ENDSECTION:		break;
	   case BEGINSECTHEAD:	printf("\\section{"); tagc=0; break;
	   case ENDSECTHEAD:
		printf("}");
/*
		if (CurLine==1) printf("\\footnote{"
		  "\\it conversion to \\LaTeX\ format by RosettaMan "
		  "available via anonymous ftp from {\\tt ftp.berkeley.edu:/ucb/people/phelps/tcltk}}"
			  );
*/
		/* useful extraction from files, environment? */
		printf("\n");
		break;
	   case BEGINSUBSECTHEAD:printf("\\subsection{"); break;
	   case ENDSUBSECTHEAD:
		printf("}");
		break;
	   case BEGINSUBSECTION:	break;
	   case ENDSUBSECTION:	break;
	   case BEGINBULPAIR:	printf("\\begin{itemize}\n"); break;
	   case ENDBULPAIR:		printf("\\end{itemize}\n"); break;
	   case BEGINBULLET:	printf("\\item ["); break;
	   case ENDBULLET:		printf("] "); break;
	   case BEGINLINE:		if (ncnt) printf("\n\n"); break;
	   case ENDLINE:		I=0; tagc=0; putchar('\n'); CurLine++; break;
	   case BEGINTABLE:		printf("\\begin{verbatim}\n"); break;
	   case ENDTABLE:		printf("\\end{verbatim}\n"); break;
	   case SHORTLINE:		if (!fIP) printf("\n\n"); break;
	   case BEGINBULTXT:	break;
	   case ENDBULTXT:		putchar('\n'); break;

	   case CHARLQUOTE:		printf("``"); break;
	   case CHARRQUOTE:		printf("''"); break;
	   case CHARLSQUOTE:
	   case CHARRSQUOTE:
	   case CHARPERIOD:
	   case CHARDASH:
		putchar(cmd); break;
	   case CHARBACKSLASH:	printf("$\\backslash$"); break;
	   case CHARGT:		printf("$>$"); break;
	   case CHARLT:		printf("$<$"); break;
	   case CHARHAT:		printf("$\\char94{}$"); break;
	   case CHARVBAR:		printf("$|$"); break;
	   case CHARAMP:		printf("\\&"); break;
	   case CHARBULLET:		printf("$\\bullet$ "); break;
	   case CHARDAGGER:		printf("\\dag "); break;
	   case CHARPLUSMINUS:	printf("\\pm "); break;

	   case BEGINBOLD:		printf("{\\bf "); break;
	   case BEGINSC:		printf("{\\sc "); break;
	   case BEGINITALICS:	printf("{\\it "); break;
	   case BEGINBOLDITALICS:printf("{\\bf\\it "); break;
	   case BEGINMANREF:	printf("{\\sf "); break;
	   case ENDBOLD:
	   case ENDSC:
	   case ENDITALICS:
	   case ENDBOLDITALICS:
	   case ENDMANREF:
		putchar('}');
		break;

	   case BEGINY: case ENDY:
	   case BEGINHEADER: case ENDHEADER:
	   case BEGINFOOTER: case ENDFOOTER:
	   case BEGINTABLELINE: case ENDTABLELINE:
	   case CHANGEBAR:
		/* nothing */
		break;
	}
}


void
LaTeX2e(enum command cmd) {

	switch (cmd) {
		/* replace selected commands ... */
	   case BEGINDOC:
		escchars = "$&%#_{}";
		printf("%% %s,\n", providence);
		printf("%% %s\n\n", anonftp);
		/* definitions */
		printf(
		  "\\documentclass{article}\n"
		  "\\def\\thefootnote{\\fnsymbol{footnote}}\n"
		  "\\setlength{\\parindent}{0pt}\n"
		  "\\setlength{\\parskip}{0.5\\baselineskip plus 2pt minus 1pt}\n"
		  "\\begin{document}\n"
			  );
		I=0;
		break;
	   case BEGINBOLD:		printf("\\textbf{"); break;
	   case BEGINSC:		printf("\\textsc{"); break;
	   case BEGINITALICS:	printf("\\textit{"); break;
	   case BEGINBOLDITALICS:printf("\\textbf{\\textit{"); break;
	   case BEGINMANREF:	printf("\\textsf{"); break;
	   case ENDBOLDITALICS:	printf("}}"); break;

		/* ... rest same as old LaTeX */
	   default:
		LaTeX(cmd);
	}
}



/*
 * Rich Text Format (RTF)
 */

/* RTF could use more work */

void
RTF(enum command cmd) {

	switch (cmd) {
	   case BEGINDOC:
		escchars = "{}";
		/* definitions */
		printf(
		  /* fonts */
		  "{\\rtf1\\deff2 {\\fonttbl"
		  "{\\f20\\froman Times;}{\\f150\\fnil I Times Italic;}"
		  "{\\f151\\fnil B Times Bold;}{\\f152\\fnil BI Times BoldItalic;}"
		  "{\\f22\\fmodern Courier;}{\\f23\\ftech Symbol;}"
		  "{\\f135\\fnil I Courier Oblique;}{\\f136\\fnil B Courier Bold;}{\\f137\\fnil BI Courier BoldOblique;}"
		  "{\\f138\\fnil I Helvetica Oblique;}{\\f139\\fnil B Helvetica Bold;}}"
		  "\n"

		  /* style sheets */
		  "{\\stylesheet{\\li720\\sa120 \\f20 \\sbasedon222\\snext0 Normal;}"
		  "{\\s2\\sb200\\sa120 \\b\\f3\\fs20 \\sbasedon0\\snext2 section head;}"
		  "{\\s3\\li180\\sa120 \\b\\f20 \\sbasedon0\\snext3 subsection head;}"
		  "{\\s4\\fi-1440\\li2160\\sa240\\tx2160 \\f20 \\sbasedon0\\snext4 detailed list;}}"
		  "\n"

/* more header to come--do undefined values default to nice values? */
			   );
		I=0;
		break;
	   case ENDDOC:
		/* header and footer wanted? */
		printf("\\par{\\f150 %s,\n%s}", providence, anonftp);
		printf("}\n");
		break;
	   case BEGINBODY:		break;
	   case ENDBODY:
		CurLine++;
		printf("\\par\n");
		tagc=0;
		break;
	   case BEGINSECTION:	break;
	   case ENDSECTION:		printf("\n\\par\n"); break;
	   case BEGINSECTHEAD:	printf("{\\s2 "); tagc=0; break;
	   case ENDSECTHEAD:
		printf("}\\par");
		/* useful extraction from files, environment? */
		printf("\n");
		break;
	   case BEGINSUBSECTHEAD:printf("{\\s3 "); break;
	   case ENDSUBSECTHEAD:
		printf("}\\par\n");
		break;
	   case BEGINSUBSECTION:	break;
	   case ENDSUBSECTION:	break;
	   case BEGINLINE:		/*if (ncnt) printf("\n\n");*/ break;
	   case ENDLINE:		I=0; tagc=0; putchar(' '); /*putchar('\n'); CurLine++;*/ break;
	   case SHORTLINE:		if (!fIP) printf("\\line\n"); break;
	   case BEGINBULPAIR:	printf("{\\s4 "); break;
	   case ENDBULPAIR:		printf("}\\par\n"); break;
	   case BEGINBULLET:	break;
	   case ENDBULLET:		printf("\\tab "); fcharout=0; break;
	   case BEGINBULTXT:	fcharout=1; break;
	   case ENDBULTXT:		break;

	   case CHARLQUOTE:		printf("``"); break;
	   case CHARRQUOTE:		printf("''"); break;
	   case CHARLSQUOTE:
	   case CHARRSQUOTE:
	   case CHARPERIOD:
	   case CHARDASH:
	   case CHARBACKSLASH:
	   case CHARGT:
	   case CHARLT:
	   case CHARHAT:
	   case CHARVBAR:
	   case CHARAMP:
		putchar(cmd); break;
	   case CHARBULLET:		printf("\\bullet "); break;
	   case CHARDAGGER:		printf("\\dag "); break;
	   case CHARPLUSMINUS:	printf("\\pm "); break;

	   case BEGINBOLD:		printf("{\\b "); break;
	   case BEGINSC:		printf("{\\fs20 "); break;
	   case BEGINITALICS:	printf("{\\i "); break;
	   case BEGINBOLDITALICS:printf("{\\b \\i "); break;
	   case BEGINMANREF:	printf("{\\f22 "); break;
	   case ENDBOLD:
	   case ENDSC:
	   case ENDITALICS:
	   case ENDBOLDITALICS:
	   case ENDMANREF:
		putchar('}');
		break;

	   case BEGINY: case ENDY:
	   case BEGINHEADER: case ENDHEADER:
	   case BEGINFOOTER: case ENDFOOTER:
	   case BEGINTABLE: case ENDTABLE:
	   case BEGINTABLELINE: case ENDTABLELINE:
	   case CHANGEBAR:
		/* nothing */
		break;
	}
}



/*
 * pointers to existing tools
 */

void
PostScript(enum command cmd) {
	fprintf(stderr, "Use groff or psroff to generate PostScript.\n");
	exit(1);
}


void
FrameMaker(enum command cmd) {
	fprintf(stderr, "FrameMaker comes with filters that convert from roff to MIF.\n");
	exit(1);
}




/*** Kong ***/
/*
  I hope the compiler has good common subexpression elimination
     for all the pointer arithmetic.
*/

/*
 level 0: DOC - need match
 level 1: SECTION - need match
 level 2: SUBSECTION | BODY | BULLETPAIR
 level 3: BODY (within SUB) | BULLETPAIR (within SUB) | BULTXT (within BULLETPAIR)
 level 4: BULTXT (within BULLETPAIR within SUBSECTION)

 never see: SECTHEAD, SUBSECTHEAD, BULLET
*/

int Psect=0, Psub=0, Pbp=0, Pbt=0, Pb=0;

void
pop(enum command cmd) {
	assert(cmd==BEGINBULTXT || cmd==BEGINBULPAIR || cmd==BEGINBODY || cmd==BEGINSECTION || cmd==BEGINSUBSECTION || cmd==ENDDOC);
/*
	int i;
	int p;
	int match;

	p=cmdp-1;
	for (i=cmdp-1;i>=0; i--)
		if (cmd==cmdstack[i]) { match=i; break; }
*/

	/* if match, pop off all up to and including match */
	/* otherwise, pop off one level*/

	if (Pbt) { (*fn)(ENDBULTXT); Pbt=0; }
	if (cmd==BEGINBULTXT) return;

	if (Pb && cmd==BEGINBULPAIR) { (*fn)(ENDBODY); Pb=0; }	/* special */
	if (Pbp) { (*fn)(ENDBULPAIR); Pbp=0; }
	if (cmd==BEGINBULPAIR) return;

	if (Pb) { (*fn)(ENDBODY); Pb=0; }
	if (cmd==BEGINBODY) return;

	if (Psub) { (*fn)(ENDSUBSECTION); Psub=0; }
	if (cmd==BEGINSUBSECTION) return;

	if (Psect) { (*fn)(ENDSECTION); Psect=0; }
	if (cmd==BEGINSECTION) return;
}


void
poppush(enum command cmd) {
	assert(cmd==BEGINBULTXT || cmd==BEGINBULPAIR || cmd==BEGINBODY || cmd==BEGINSECTION || cmd==BEGINSUBSECTION);

	pop(cmd);

	switch (cmd) {
	   case BEGINBULTXT: Pbt=1; break;
	   case BEGINBULPAIR: Pbp=1; break;
	   case BEGINBODY: Pb=1; break;
	   case BEGINSECTION: Psect=1; break;
	   case BEGINSUBSECTION: Psub=1; break;
	   default:
		fprintf(stderr, "poppush: unrecognized code %d\n", cmd);
	}

	(*fn)(cmd);
	prevcmd = cmd;
}



/* wrapper for getchar() that expands tabs */
/* the compiler inlines this, one hopes */

int
getchartab(void) {
	static int tabexp = 0;
	static int charinline = 0;
	int c;

	c = lookahead;
	if (tabexp) tabexp--;
	else if (c=='\n') charinline=0;
	else if (c=='\t') {
		tabexp = TabStops-(charinline%TabStops); if (tabexp==TabStops) tabexp=0;
		lookahead = c = ' ';
	}

	if (!tabexp && lookahead!=EOF) lookahead=getchar();
	if (c=='\b') charinline--; else charinline++;
	return c;
}


/* replace gets.  handles hyphenation too */
char *
la_gets(char *buf) {
	static char la_buf[BUFSIZ];	/* can lookahead a full line, but nobody does now */
	static int fla=0, hy=0;
	char *ret,*p;
	int c,i;

	assert(buf!=NULL);

	if (fla) {
		/* could avoid copying if callers used return value */
		strcpy(buf,la_buf); fla=0;
		ret=buf;	/* correct? */
	} else {
		/*ret=gets(buf); -- gets is deprecated (since it can read too much?) */
		/* could do this...
		ret=fgets(buf, BUFSIZ, stdin);
		buf[strlen(buf)-1]='\0';
		... but don't want to have to rescan line with strlen, so... */

		i=0; p=buf;

		/* recover spaces if re-linebreaking */
		for (; hy; hy--) { *p++=' '; i++; }

		while (lookahead!=EOF && (c=getchartab())!='\n' && i<BUFSIZ) { *p++=c; i++; }
		assert(i<BUFSIZ);

		/*lookahead=ungetc(getchar(), stdin);	/* only looking ahead one character for now */

		/* very special case: if in SEE ALSO section, re-linebreak so references aren't linebroken
		   (also do this if fNOHY flag is set) -- doesn't affect lookahead */
		/* 0xad is an en dash on Linux? */
		if (p>buf && (p[-1]=='-'||p[-1]=='\xAD') && (fPara || sectheadid==SEEALSO || fNOHY) && isspace(lookahead)) {
			p--;	/* zap hyphen */
			/* zap boldfaced hyphens, gr! */
			while (p[-1]=='\b' && p[-2]=='-') p-=2;

			/* start getting next line, spaces first ... */
			while (lookahead!=EOF && isspace(lookahead) && lookahead!='\n') { getchartab(); hy++; }

			/* ... append next nonspace string to previous ... */
			while (lookahead!=EOF && !isspace(lookahead) && i++<BUFSIZ) *p++=getchartab();

			/* gobble following spaces (until, perhaps including, end of line) */
			while (lookahead!=EOF && isspace(lookahead) && lookahead!='\n') getchartab();
			if (lookahead=='\n') { getchartab(); hy=0; }
		}

		*p='\0';
		ret=(lookahead!=EOF)?buf:NULL;
	}

	AbsLine++;
	return ret;	/* change this to line length? */
}



/*
  buf[] == input text (read only)
  plain[] == output (initial, trailing spaces stripped; tabs=>spaces;
     underlines, overstrikes => tag array; spaces squeezed, if requested)
  ccnt = count of changebars
  scnt = count of initial spaces
  linelen = length result in plain[]
*/

/*#define MAXINDENT 15*/
/*#define HEADFOOTMATCH 20*/

int fHead=0;
int fFoot=0;

void
filter(void) {
	const int MINRM=50;		/* minimum column for right margin */
	const int MINMID=20;
	const int HEADFOOTSKIP=20;
	const int HEADFOOTMAX=25;
	const enum command tagbeginend[][2] = {	/* parallel to enum tagtype */
		{ -1,-1 },
		{ -1,-1 },
		{ BEGINITALICS, ENDITALICS },
		{ BEGINBOLD, ENDBOLD },
		{ BEGINY, ENDY },
		{ BEGINSC, ENDSC },
		{ BEGINBOLDITALICS, ENDBOLDITALICS },
		{ -1,-1 },
		{ BEGINMANREF, ENDMANREF }
	};
	int curtag;
	char *p,*r;
	char head[BUFSIZ]="";		/* first "word" */
	char foot[BUFSIZ]="";
	int header_m=0, footer_m=0;
	int headlen=0, footlen=0;
/*	int line=1-1; */
	int i,j,k,l,off;
	int sect=0,subsect=0,bulpair=0,osubsect=0;
	int title=1;
	int oscnt=-1;
	int empty=0,oempty;
	int fcont=0;
	int Pnew=0,I0;
	float s_avg=0.0;
	int spaceout;

	/* try to keep tabeginend[][] in parallel with enum tagtype */
	assert(tagbeginend[ITALICS][0]==BEGINITALICS);
	assert(tagbeginend[MANREF][1]==ENDMANREF);

	/*	for (i=0; i<BUFSIZ; i++) tabgram[i]=0;*/

	if (fMan) indent=-1;
	I=1;
	CurLine=1;
	(*fn)(BEGINDOC); I0=I;

	/* run through each line */
	while (la_gets(buf)!=NULL) {
		if (title) I=I0;
		filterline(buf,plain);	/* ALL LINES ARE FILTERED */
		fintable = fTable &&
			((!ncnt && fotable) ||
			(ncnt && bs_cnt>=2 && bs_cnt<=5 && ((float) bs_sum / (float) bs_cnt)>3.0));
		if (fintable) {
			if (!fotable) (*fn)(BEGINTABLE);
		} else if (fotable) {
			(*fn)(ENDTABLE);
			I=I0; tagc=0; filterline(buf,plain);	/* rescan first line out of table */
		}

		s_avg=(float) s_sum;
		if (s_cnt>=2) {
			/* don't count large second space gap */
			if (scnt2) s_avg= (float) (s_sum - scnt2) / (float) (s_cnt-1);
			else s_avg= (float) (s_sum) / (float) (s_cnt);
		}

		p=plain;	/* points to current character in plain */

		/*** determine header and global indentation ***/
		if (fMan && (!fHead || indent==-1)) {
			if (!linelen) continue;
			if (*header=='\0') {
				/* check for missing first header--but this doesn't catch subsequent pages */
				if (stricmp(p,"NAME")==0) {
					indent=scnt; /*filterline(buf,plain);*/ scnt=0; I=I0; fHead=1;
				} else {
					fHead=1;
					(*fn)(BEGINHEADER);
					/* grab header and its first word */
					strcpy(header,p);
					if ((header_m=HEADFOOTSKIP)>linelen) header_m=0;
					strcpy(head,phrase); headlen=phraselen;
					la_gets(buf); filterline(buf,plain);
					if (linelen) {
						strcpy(header2,plain);
						if (strincmp(plain,"Digital",7)==0 || strincmp(plain,"OSF",3)==0) {
							fFoot=1;
							fSubsections=0;
						}
					}
					(*fn)(ENDHEADER); tagc=0;
					continue;
				}
			} else {
				/* some idiot pages have a *third* header line, possibly after a null line */
				if (*header && scnt>MINMID) { strcpy(header3,p); ncnt=0; continue; }
				/* indent of first line ("NAME") after header sets global indent */
				/* check '<' for Plan 9(?) */
				if (*p!='<') {
					indent=scnt; I=I0; scnt=0;
				} else continue;
			}
/*			if (indent==-1) continue;*/
		}
		if (!lindent && scnt) lindent=scnt;
/*printf("lindent = %d, scnt=%d\n",lindent,scnt);*/


		/**** for each ordinary line... *****/

		/*** skip over global indentation */
		oempty=empty; empty=(linelen==0);
		if (empty) {ncnt++; continue;}

		/*** strip out per-page titles ***/

		if (fMan && (scnt==0 || scnt>MINMID)) {
/*printf("***ncnt = %d, fFoot = %d, line = %d***", ncnt,fFoot,AbsLine);*/
			if (!fFoot && !isspace(*p) && (scnt>5 || (*p!='-' && *p!='_')) &&
			    /* don't add ncnt -- AbsLine gets absolute line number */
			    (((ncnt>=2 && AbsLine/*+ncnt*/>=61/*was 58*/ && AbsLine/*+ncnt*/<70)
				 || (ncnt>=4 && AbsLine/*+ncnt*/>=59 && AbsLine/*+ncnt*/<74)
				 || (ncnt && AbsLine/*+ncnt*/>=61 && AbsLine/*+ncnt*/<=66))
				 && (/*lookahead!=' ' ||*/ (s_cnt>=1 && s_avg>1.1) || !falluc) )
			    ) {
				(*fn)(BEGINFOOTER);
				/* grab footer and its first word */
				strcpy(footer,p);
/*				if ((footer_m=linelen-HEADFOOTSKIP)<0) footer_m=0;*/
				if ((footer_m=HEADFOOTSKIP)>linelen) footer_m=0;
				/*grabphrase(p);*/ strcpy(foot,phrase); footlen=phraselen;
				/* permit variations at end, as for SGI "Page N", but keep minimum length */
				if (footlen>3) footlen--;
				la_gets(buf); filterline(buf,plain); if (linelen) strcpy(footer2,plain);
				title=1;
				(*fn)(ENDFOOTER); tagc=0;

				/* if no header on first page, try again after first footer */
				if (!fFoot && *header=='\0') fHead=0;	/* this is dangerous */
				fFoot=1;
				continue;
			} else
				/* a lot of work, but only for a few lines (about 4%) */
				if (fFoot && (scnt==0 || scnt+indent>MINMID) &&
					 (   (headlen && strncmp(head,p,headlen)==0)
					  || strcmp(header2,p)==0 || strcmp(header3,p)==0
					  || (footlen && strncmp(foot,p,footlen)==0)
					  || strcmp(footer2,p)==0
					  /* try to recognize lines with dates and page numbers */
					  /* skip into line */
					  || (header_m && header_m<linelen &&
						 strncmp(&header[header_m],&p[header_m],HEADFOOTMAX)==0)
					  || (footer_m && footer_m<linelen &&
						 strncmp(&footer[footer_m],&p[footer_m],HEADFOOTMAX)==0)
					  /* skip into line allowing for off-by-one */
					  || (header_m && header_m<linelen &&
						 strncmp(&header[header_m],&p[header_m+1],HEADFOOTMAX)==0)
					  || (footer_m && footer_m<linelen &&
						 strncmp(&footer[footer_m],&p[footer_m+1],HEADFOOTMAX)==0)
					  /* or two */
					  || (header_m && header_m<linelen &&
						 strncmp(&header[header_m],&p[header_m+2],HEADFOOTMAX)==0)
					  || (footer_m && footer_m<linelen &&
						 strncmp(&footer[footer_m],&p[footer_m+2],HEADFOOTMAX)==0)
					  /* or with reflected odd and even pages */
					  || (headlen && headlen<linelen &&
						 strncmp(head,&p[linelen-headlen],headlen)==0)
					  || (footlen && footlen<linelen &&
						 strncmp(foot,&p[linelen-footlen],footlen)==0)
					  )) {
				tagc=0; title=1; continue;
			}

			/* page numbers at end of line */
			for(i=0; p[i] && isdigit(p[i]); i++)
				/* empty */;
			if (&p[i]!=plain && !p[i]) {title=1; fFoot=1; continue;}
		}

		/*** interline spacing ***/
		/* multiple \n: paragraph mode=>new paragraph, line mode=>blank lines */
		/* need to chop up lines for Roff */

		/*tabgram[scnt]++;*/
		if (title) ncnt=(scnt!=oscnt || (/*scnt<4 &&*/ isupper(*p)));
		if (CurLine==1) {ncnt=0; tagc=0;} /* gobble all newlines before first text line */
		(*fn)(BEGINLINE);
		if (/*fPara &&*/ ncnt) Pnew=1;
		title=0; /*ncnt=0;--moved down*/
		if (fintable) (*fn)(BEGINTABLELINE);
		oscnt=scnt; fotable=fintable;

/* let output modules decide what to do at the start of a paragraph
		if (fPara && !Pnew && (prevcmd==BEGINBODY || prevcmd==BEGINBULTXT)) {
			putchar(' '); I++;
		}
*/

		/*** identify structural sections and notify fn */

		if (fMan) {
			sect = (scnt==0 && isupper(*p));
			subsect=(fSubsections && (scnt==2||scnt==3));
/*			bulpair = (scnt<7 && (*p==c_bullet || *p=='-'));*/
			/* decode the below */
			bulpair = ((!auxindent || scnt!=lindent+auxindent) /*!bulpair*/
					 && ((scnt>=2 && scnt2>5) || scnt>=5 || (tagc>0 && tags[0].first==scnt) ) /* scnt>=2?? */
					 && (((*p==c_bullet || strchr("-+.",*p)!=NULL || falluc) && (ncnt || scnt2>4)) || 
					  (scnt2-s_avg>=2 && phrase[phraselen-1]!='.') ||
					  (scnt2>3 && s_cnt==1)
					  ));
			if (bulpair) {
				if (tagc>0 && tags[0].first==scnt) {
					k=tags[0].last;
					for (l=1; l<tagc; l++) {
						if (tags[l].first - k <=3)
							k=tags[l].last;
						else break;
					}
					phraselen=k-scnt;
					for (k=phraselen; plain[k]==' ' && k<linelen; k++) /* nothing */;
					if (k>=5 && k<linelen) hanging=k; else hanging=-1;
				} else if (scnt2) hanging=phraselen+scnt2;
				else hanging=5;
			} else hanging=0;

/*			hanging = bulpair? phraselen+scnt2 : 0;*/
/*if (bulpair) printf("hanging = %d\n",hanging);*/
			/* maybe, bulpair=0 would be best */
		}

		/* certain sections (subsections too?) special, like SEE ALSO */
		/* to make canonical name as plain, all lowercase */
		if (sect /*||subsect -- no widespread subsection names*/) {
		  sectheadid = RANDOM;
		  for (j=0; j<RANDOM; j++) {
		    if (strcoloncmp2(plain,'\0',sectheadname[j],0)) {
			 sectheadid=j;
			 break;
		    }
		  }
		}

		if (sect) {
			addtoc(plain, BEGINSECTION, CurLine);
			poppush(BEGINSECTION); (*fn)(BEGINSECTHEAD);
		} else if (subsect && !osubsect) {
			addtoc(plain, BEGINSUBSECTION, CurLine);
			poppush(BEGINSUBSECTION); (*fn)(BEGINSUBSECTHEAD);
		} else if (bulpair) {
			poppush(BEGINBULPAIR); (*fn)(BEGINBULLET);
			fIP=1; /*grabphrase(plain);*/
		} else if (Pnew) {
			poppush(BEGINBODY);
		}
		Pnew=0;
		oldsectheadid = sectheadid;


		/* move change bars to left */
		if (fChangeleft) {
			if (fPara) (*fn)(CHANGEBAR);
			/* replace initial spaces with changebars */
			else for (i=0; i<ccnt; i++) { /*xputchar('|'); */ (*fn)(CHANGEBAR); }
		}

		/* show initial spaces */
		if (!fIQS && fcharout) {
			spaceout = (scnt>ccnt)?(scnt-ccnt):0;
			if (fILQS) { if (spaceout>=lindent) spaceout-=lindent; else spaceout=0; }
			if (auxindent) { if (spaceout>=auxindent) spaceout-=auxindent; else spaceout=0; }
			printf("%*s",spaceout,"");
		}


		/*** iterate over each character in line, ***/
		/*** handling underlining, tabbing, copyrights ***/

		off=(!fIQS&&!fPara)?scnt:0;
		for (i=0, p=plain, curtag=0, fcont=0; *p; p++,i++,fcont=0) {
			/* interspersed presentation signals */
			/* start tags in reverse order of addition (so structural first) */
			if (curtag<tagc && i+I0+off==tags[curtag].first) {
				for (r=hitxt, j=tags[curtag].last-tags[curtag].first, hitxt[j]='\0'; j; j--)
					hitxt[j-1]=p[j-1];
				(*fn)(tagbeginend[tags[curtag].type][0]);
			}

			/* special characters */
			switch(*p) {
			   case '"':
				if (p==plain || isspace(p[-1])) { (*fn)(CHARLQUOTE); fcont=1; }
				else if (isspace(p[1])) { (*fn)(CHARRQUOTE); fcont=1; }
				break;
			   case '\'':
				if (p==plain || isspace(p[-1])) { (*fn)(CHARLSQUOTE); fcont=1; }
				else if (isspace(p[1])) { (*fn)(CHARRSQUOTE); fcont=1; }
				break;
			   case '-':
				/* check for -opt => \-opt */
				if (p==plain || (isspace(p[-1]) && !isspace(p[1]))) {
				  (*fn)(CHARDASH); fcont=1;
				}
				break;
			}

			/* troublemaker characters */
			if (!fcont && fcharout) {
				if (strchr(escchars,*p)!=NULL) {
					putchar('\\'); putchar(*p); I++;
				} else if (strchr(trouble,*p)!=NULL) {
					(*fn)(*p); fcont=1;
				} else {
					putchar(*p); I++;
				}
			}

/*default:*/
			if (curtag<tagc && i+I0+off+1==tags[curtag].last) {
				(*fn)(tagbeginend[tags[curtag].type][1]);
				curtag++;
			}

			if (fIP && ((*p==' ' && i==phraselen) || *p=='\0')) {
				p++;  /* needed but why? */
				(*fn)(ENDBULLET); fIP=0;
				if (*p!='\0') {
					/*oscnt+=phraselen;*/
					oscnt+=i;
					for (r=p; *r==' '; r++) {
						oscnt++;
/*
						i++;
						if (fQS || !fcharout) p++;
*/
					}
				}
				p--;	/* to counteract increment in loop */

				poppush(BEGINBULTXT);
			}
		}


		/*** end of line in buf[] ***/
		/*** deal with section titles, hyperlinks ***/

		if (sect) { (*fn)(ENDSECTHEAD); Pnew=1; }
		else if (subsect) { (*fn)(ENDSUBSECTHEAD); Pnew=1; }
		else if (fIP) { (*fn)(ENDBULLET); fIP=0; poppush(BEGINBULTXT); }
/* oscnt not right here */
		else if (scnt+linelen+spcsqz<MINRM /*&& ncnt*/ && lookahead!='\n'
			    && prevcmd!=BEGINBULTXT && prevcmd!=ENDSUBSECTHEAD && prevcmd!=ENDSUBSECTHEAD)
			(*fn)(SHORTLINE);
		osubsect=subsect;

		if (fintable) (*fn)(ENDTABLELINE);
		/*if (!fPara)*/ (*fn)(ENDLINE);
		ncnt=0;
		I0=I;	/* save I here in case skipping lines screws it up */
	}

	/* wrap up at end */
	pop(ENDDOC); (*fn)(ENDDOC);
}



int
setFilterDefaults(char *outputformat) {
	struct {
		void (*fn)(enum command);
		int fPara; int fQS; int fIQS; int fNOHY; int fChangeleft; char *names;
		/* (could set parameters at BEGINDOC call...) */
	} format[] = {
		/*{default	0	0	0	0	0 },*/
		{ ASCII,		0,	0,	0,	0,	0,	"ASCII" },
		{ TkMan,		0,	0,	/*0*/1,	0,	0,	"TkMan" },
		{ Tk,		0,	0,	0,	0,	0,	"Tk:Tcl" },
		{ Sections,	0,	0,	0,	0,	0,	"sections" },
		{ Roff,		0,	1,	1,	1,	-1,	"roff:troff:nroff" },
		{ HTML,		1,	1,	1,	1,	1,	"HTML:WWW" },
		{ SGML,		1,	1,	1,	1,	1,	"SGML" },
		{ MIME,		1,	1,	1,	1,	1,	"MIME:Emacs:enriched" },
		{ LaTeX,		1,	1,	1,	1,	1,	"LaTeX:LaTeX209:209:TeX" },
		{ LaTeX2e,	1,	1,	1,	1,	1,	"LaTeX2e:2e" },
		{ RTF,		1,	1,	1,	1,	1,	"RTF" },
		{ pod,		0,	0,	1,	0,	-1,	"pod:Perl" },
		{ Ensemble,	1,	1,	1,	1,	1,	"Ensemble" },
		{ PostScript,	0,	0,	0,	0,	0,	"PostScript:ps" },
		{ FrameMaker,	0,	0,	0,	0,	0,	"FrameMaker:Frame:Maker:MIF" },
		{ NULL,		0,	0,	0,	0,	0,	NULL }
	};
	/*	assert(format[0].fn==ASCII);*/

	int i, found=0;
	for (i=0; format[i].fn!=NULL; i++) {
		if (strcoloncmp2(outputformat,'\0',format[i].names,0)) {
			fn = format[i].fn;
			fPara = format[i].fPara;
			fQS = format[i].fQS;
			fIQS = format[i].fIQS;
			fNOHY = format[i].fNOHY;
			fChangeleft = format[i].fChangeleft;
			found=1;
			break;
		}
	}

	return (found==0);
}


int
main(int argc, char *argv[]) {
	int c;
	int i,j;
	char *p;
	int fname=0;
	const int helpbreak=75;
	const int helpispace=4;
	int helplen=0;
	int desclen;
	char **argvch;	/* remapped argv */
	char *argvbuf;
	extern char *optarg;
	extern int optind, opterr;

	char strgetopt[80];
	/* options with an arg must have a '<' in the description */
	struct { char letter; int arg; char *longnames; char *desc; } option[] = {
		{ 'f', 1, "filter", " <ASCII|roff|TkMan|Tk|Ensemble|Sections|HTML|SGML|MIME|LaTeX|LaTeX2e|RTF|pod>" },
		{ 'r', 1, "reference:manref:ref", " <man reference printf string>" },
		{ 'l', 1, "title", " <title printf string>" },
		{ 'b', 0, "subsections:sub", " (show subsections)" },
		{ 'n', 1, "name", "(ame of man page) <string>" },
		{ 's', 1, "section:sect", "(ection) <string>" },
		{ 'k', 0, "keep:head:foot:header:footer", "(eep head/foot)" },
		{ 'V', 1, "volumes:vol", "(olume) <colon-separated list>" },
		{ 'c', 0, "changeleft:changebar", "(hangebarstoleft toggle)" },
		{ 'p', 0, "paragraph:para", "(aragraph mode toggle)" },
		{ 't', 1, "tabstop:tabstops", "(abstops spacing) <number>" },
		{ 'y', 0, "zap:nohyphens", " (zap hyphens toggle)" },
		{ 'K', 0, "nobreak", " (declare that page has no breaks)" },
		{ 'm', 0, "notaggressive", "(an page aggressive parsing OFF)" },
		{ 'T', 0, "tables", "(able agressive parsing ON)" },
		{ 'h', 0, "help", "(elp)" },
		/* { '?', 1, "", " (help)" }, -- getopt returns '?' as error flag */
		{ 'v', 0, "version", "(ersion)" },
		{ '\0', 0, "", NULL }
	};

	/* calculate strgetopt from options list */
	for (i=0,p=strgetopt; option[i].letter!='\0'; i++) {
		*p++ = option[i].letter;
		/* check for duplicate option letters */
		assert(strchr(strgetopt,option[i].letter)==&p[-1]);
		if (option[i].arg) *p++=':';
	}
	*p='\0';

	/* spot check construction of strgetopt */
	assert(p<strgetopt+80);
	assert(strlen(strgetopt)>10);
	assert(strchr(strgetopt,'f')!=NULL);
	assert(strchr(strgetopt,'v')!=NULL);
	assert(strchr(strgetopt,':')!=NULL);

#ifdef macintosh
	extern void InitToolbox();
	InitToolbox();
#endif


	/* map long option names to single letters for switching */
	/* (GNU probably has a reusable function to do this...) */
	/* deep six getopt in favor of integrated long names + letters? */
	argvch = malloc(argc * sizeof(char*));
	p = argvbuf = malloc(argc*3 * sizeof(char));	/* either -<char>'\0' or no space used */
	for (i=0; i<argc; i++) argvch[i]=argv[i];
	for (i=1; i<argc; i++) {
		if (argv[i][0]=='-' && argv[i][1]=='-') {
			if (argv[i][2]=='\0') break;	/* end of options */
			for (j=0; option[j].letter!='\0'; j++) {
				if (strcoloncmp2(&argv[i][2],'\0',option[j].longnames,0)) {
					argvch[i] = p;
					*p++ = '-'; *p++ = option[j].letter; *p++ = '\0';
					if (option[j].arg) i++;	/* skip arguments of options */
					break;
				}
			}
			if (option[j].letter=='\0') fprintf(stderr, "%s: unknown option %s\n", argv[0], argv[i]);
		}
	}


	/* first pass through options to set defaults for chosen format */

	setFilterDefaults("ASCII");		/* default to ASCII (used by TkMan's Glimpse indexing */

	while ((c=getopt(argc,argvch,strgetopt))!=-1) {

		switch (c) {
		   case 'k': fHeadfoot=1; break;
		   case 'b': fSubsections=1; break;
		   case 'c': fChangeleft=1; break;
		   case 'n': strcpy(manName,optarg); fname=1; break;	/* name & section for when using stdin */
		   case 's': strcpy(manSect,optarg); break;
		   /*case 'D': docbookpath = optarg; break;*/
		   case 'V': vollist = optarg; break;
		   case 'l': manTitle = optarg; break;
		   case 'r': manRef = optarg;
			if (strlen(manRef)==0 || strcmp(manRef,"-")==0 || strcmp(manRef,"off")==0) fmanRef=0;
			break;
		   case 't': TabStops=atoi(optarg); break;
		   case 'm': fMan=0; break;
		   case 'T': fTable=1; break;
		   case 'p': fPara=!fPara; break;
		   case 'K': fFoot=1; break;
		   case 'y': fNOHY=1; break;

		   case 'f': /* set format */
			if (setFilterDefaults(optarg)) {
				fprintf(stderr, "%s: unknown format: %s\n", argvch[0], optarg);
				exit(1);
			}
			
			break;
		   case 'v':
			printf("RosettaMan v" ROSETTAMANVERSION " of $Date$\n");
			exit(0);
			break;
		   case 'h': /*case '?':*/
			printf("rman"); helplen=strlen("rman");

			/* linebreak options */
			assert(helplen>0);
			for (i=0; option[i].letter!='\0'; i++) {
				desclen = strlen(option[i].desc);
				if (helplen+desclen+5 > helpbreak) { printf("\n%*s",helpispace,""); helplen=helpispace; }
				printf(" [-%c%s]", option[i].letter, option[i].desc);
				helplen += desclen+5;
			}
			if (helplen>helpispace) printf("\n");
			printf("%*s [<filename>]\n",helpispace,"");
			exit(0);
			break;

		   /*case '?':*/
		   default:
			fprintf(stderr, "%s: unidentified option -%c (-h for help)\n",argvch[0],c);
			exit(2);
		}
	}


	/* read from given file name(s) */
	if (optind<argc) {
		if (!fname) {  /* if no name given, create from file name */
			/* take name from tail of path */
			if ((p=strrchr(argvch[optind],'/'))!=NULL) p++; else p=argvch[optind];
			strcpy(manName,p);

			/* search backward from end for final dot. split there */
			if ((p=strrchr(manName,'.'))!=NULL) {
				strcpy(manSect,p+1);
				*p='\0';
			}
		}

		if (freopen(argvch[optind], "r", stdin)==NULL) {
			fprintf(stderr, "%s: can't open %s\n", argvch[0],argvch[optind]);
			exit(1);
		}
	}

	/* minimal check for roff source: first character dot command or apostrophe comment */
	/* MUST initialize lookahead here, BEFORE first call to la_gets */
	lookahead = ungetc(getchar(), stdin);
	if (lookahead=='.' || lookahead=='\'') {
		fprintf(stderr, "%s:\tInput looks like [tn]roff source--RosettaMan needs formatted text\n"
					 "\tfrom `nroff -man' or from */man/cat[1-8oln] directories.\n", argvch[0]);
		exit(1);
	}

	filter();
	return 0;
}
