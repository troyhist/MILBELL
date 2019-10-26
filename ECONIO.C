// =======================================================================
// Enhanced CONIO ver. 1.06 (C, CPP)
// Troy computing, Inc.
// Copyright (c) 1997-'2000
// -----------------------------------------------------------------------
// Extension of CONIO library.
// Used library: memc.h
// -----------------------------------------------------------------------
// File: econio.h
// -----------------------------------------------------------------------
//     Compiler: Borland C++ 3.1, 5.01
//  Application: DOS, Win32 console
// -----------------------------------------------------------------------
// Made date: 18/IIX.1997
// Last edit: 20/ VI.2000
//     Build: 2
// =======================================================================
// Precompile definitions ... ECONIO
// =======================================================================
#define _ECONIOS
#define _ECDOS16                // Don't allocate more than 8 Kb
#define _MILBELL_
// =======================================================================
// Debug defines ... Please, remark in release
// =======================================================================
//#define _ECDEBUG              // Enable output in estatus(), ehelp()
//#define _ECDEBUG00            // Show current command name on screen
#define _ECDEBUG01              // Make pause after _ECDEBUG00
// =======================================================================
#include "memc.h"
#include "milbell.h"
#include "econio.h"
// -----------------------------------------------------------------------
#include <dos.h>                        // geninterrupt
#include <stdio.h>                      // vsprintf
#include <string.h>                     // strnicmp
#include <stdarg.h>                     // va_list
// =======================================================================
#define ever            "1.06"          // Version
#define ever_d		0x0106		// Version (as a digit)
#define ECTRL           "#"             // Control character
#define EMAXC           50              // MAX quantity of commands
// =======================================================================
#ifdef _ECDOS16
#define _ECMEMALLOC     8162            // uses in eprintf(...)
#define _ECMAXBLOCK     8162            // max block in eputs(...)
#else
#define _ECWIN32
#define _ECMEMALLOC     65535           // uses in eprintf(...)
#define _ECMAXBLOCK     16384           // max block in eputs(...)
#endif
// =======================================================================
/*
        Syntax of eputs:

        ....#<...>....          - знак '#' является yпpавляющим
        ....##........          - вывод знака '#'

        Commands:

	---------------------------------------------------------------
	--- Managing commands (ver. 1.06)                           ---
	---------------------------------------------------------------
	#ver<vvv>		- minimal version of library to view
	#batch<m>               - batch mode switch
				  m=0 - batch mode off, otherwise on
				  in the batch mode skips 'g'
	#end			- stops output to screen
	#ec			- starts/stops commands translation
	#rst			- calls 'einit' procedure
	#rem                    - remarks (comments) start/stop flag
	---------------------------------------------------------------
	--- New Commands (these commands don't exist in ver. 1.05)  ---
	---------------------------------------------------------------
	#loc<xx><yy>            - locate cursor position at current window
	#win<xx><yy><wd><hg>    - creating of text window
				  xx, yy   - position
				  wd, hg   - width, height
	#wina<x1><y1><x2><y2>   - creating of text window in absolute coords
				  x1, y1   - upper left corner
				  x2, y2   - lower right corner
	---------------------------------------------------------------
	--- Old Commands (this commands exist in ver. 1.05)         ---
        ---------------------------------------------------------------
        #cls                    - clear screen
        #cur<p>                 - тип кypсоpа
        #bl<p>                  - включение или выключение мигания
        #br<p>                  - включение или выключение яpкости фона
        #c<ffbb>                - yстановка цвета символов и фона
        #f<ff>                  - yстановка цвета символов
        #b<bb>                  - yстановка цвета фона
        #g                      - паyза (остановка до нажатия клавиши)
*/
// =======================================================================
// Internal constants ... ECONIO
// =======================================================================
#define mBATCH  1               // Batch mode
#define mINTER  0               // Interactive mode
// -----------------------------------------------------------------------
#define eMAXVAR 100             // Max. quantity of variables
// -----------------------------------------------------------------------
#define eRADIX  100             // Radix*10 of digits
// =======================================================================
void _econtrol(char*);
void _eb(char* par);	void _ebatch(char*);	void _ebl(char* par);
void _ebr(char* par);	void _ec(char* par);	void _ecls(char* par);
void _ecur(char* par);	void _eec(char*);	void _eend(char*);
void _ef(char* par);    void _eg(char* par);	void _eloc(char* par);
void _erem(char* par);	void _erst(char*);	void _ever(char*);
void _ewin(char* par);	void _ewina(char* par);
// =======================================================================
// Econio manager's struct
// =======================================================================
struct  {
	int init;               // "init" flag
	int bmode;              // mode of work (default is mINTER)
	int work;		// "work" flag
	int skipec;		// "skips Econio's commands" flag
	int rem;                // "remark (comment)" flag
	int commands;           // quantity of commands
	int blink;              // blinking mode
	} _emgr={0};
// =======================================================================
struct  {
	int parl;               // parameter's length
	void (*f)(char*);       // function address
	char* cname;            // command name
	} _ecmds[]=
	{{0, &_econtrol, "#"},	// WARNING: This command must be FIRST !!!
	{2, &_eb,	"b"},	{1, &_ebatch, 	"batch"},
	{1, &_ebl,	"bl"}, 	{1, &_ebr, 	"br"},
	{4, &_ec,	"c"},	{0, &_ecls,	"cls"},
	{1, &_ecur, 	"cur"},	{0, &_eec, 	"ec"},
	{0, &_eend,	"end"},	{2, &_ef, 	"f"},
	{0, &_eg, 	"g"},	{4, &_eloc, 	"loc"},
	{0, &_erem, 	"rem"},	{0, &_erst, 	"rst"},
	{3, &_ever,	"ver"},	{8, &_ewin, 	"win"},
	{8, &_ewina, 	"wina"},
	{0, NULL, NULL}};	// WARNING: This command must be LAST !!!
// =======================================================================
struct  {
	int enumb;              // Number of the command in sorted list
	int ecoml;              // Length of mnemonic name
	int eparl;              // Length of parameter list
	void (*f)(char*);       // Function address
        char* ecmds;            // Command
        } edata[EMAXC];
// =======================================================================
// Manager functions
// =======================================================================
int efindnumb (int numb)
{
// -----------------------------------------------------------------------
// Description: Searchs command with given number
//   Parametrs: numb    - number of command
//      Return: number of command
// -----------------------------------------------------------------------
unsigned i;
for (i=0; i<_emgr.commands; i++)
        if (edata[i].enumb==numb)
                return(i);
return(0);
}
// =======================================================================
int einit(void)
{
// -----------------------------------------------------------------------
// Description: Init's all econio system. Must be called FIRST
//   Parametrs: <none>
//      Return: NULL - SUCCESS ...
// -----------------------------------------------------------------------
int i, j, done, tempnums[EMAXC];
_emgr.init=1;           // inited
_emgr.bmode=mINTER;     // interactive mode
_emgr.rem=0;            // remark = FALSE
_emgr.skipec=0;		// translation = TRUE
_emgr.work=1;		// working = TRUE
_emgr.commands=0;       // no commands yet
_emgr.blink=0;          // no blinking
// -----------------------------------------------------------------------
// Calculates commands quantity
// -----------------------------------------------------------------------
for (i=0; i<EMAXC; i++)
        if (_ecmds[i].cname)
                { _emgr.commands++; edata[i].enumb=i; tempnums[i]=i;
                edata[i].ecoml=strlen(_ecmds[i].cname);
                edata[i].eparl=_ecmds[i].parl; edata[i].f=_ecmds[i].f;
                edata[i].ecmds=_ecmds[i].cname;}
        else    break;
// -----------------------------------------------------------------------
// Sorts commands by mnemonic length (max->min)
// -----------------------------------------------------------------------
do      {
        done=1;
	for (i=1; i<_emgr.commands-1; i++) // i=1, for skipping the '#'
                if(edata[tempnums[i]].ecoml<edata[tempnums[i+1]].ecoml)
                        { int temp=tempnums[i];
                        tempnums[i]=tempnums[i+1];
                        tempnums[i+1]=temp; done=0;
                        }
        } while (!done);
// -----------------------------------------------------------------------
for (i=0; i<_emgr.commands; i++)
        for (j=0; j<_emgr.commands; j++)
                if (tempnums[j]==i) edata[i].enumb=j;
// -----------------------------------------------------------------------
return (NULL);
}
// =======================================================================
// Econio Commands
// =======================================================================
void _econtrol(char* par)
{
#ifdef _ECDEBUG00
gotoxy(1,1); printf ("_ECDEBUG: econtrol (par=%s)", par);
#ifdef _ECDEBUG01
getch();
#endif
#endif
if (_emgr.rem || _emgr.skipec) return; cputs(ECTRL); if (par); }
// =======================================================================
void _eb(char* par)
{
int p;
#ifdef _ECDEBUG00
gotoxy(1,1); printf ("_ECDEBUG: _eb (par=%s)", par);
#ifdef _ECDEBUG01
getch();
#endif
#endif
if (_emgr.rem || _emgr.skipec) return;
sscanf(par, "%d", &p);
eback(p);
#ifdef _ECDEBUG00
gotoxy(1,2); printf ("_ECDEBUG: _eb (b=%d)", p);
#ifdef _ECDEBUG01
getch();
#endif
#endif
}
// -----------------------------------------------------------------------
void _ebatch(char* par)
{
#ifdef _ECDEBUG00
gotoxy(1,1); printf ("_ECDEBUG: _ebatch (par=%s)", par);
#ifdef _ECDEBUG01
getch();
#endif
#endif
if (_emgr.rem || _emgr.skipec) return;
sscanf(par, "%d", &_emgr.bmode);
}
// -----------------------------------------------------------------------
void _ebl(char* par)
{
int p;
struct text_info ti;
#ifdef _ECDEBUG00
gotoxy(1,1); printf ("_ECDEBUG: _ebl (par=%s)", par);
#ifdef _ECDEBUG01
getch();
#endif
#endif
if (_emgr.rem || _emgr.skipec) return;
sscanf(par, "%d", &p); _emgr.blink=p?BLINK:0;
gettextinfo(&ti); efore(ti.attribute&0x0f);
}
// -----------------------------------------------------------------------
void _ebr(char* par)
{
int p;
#ifdef _ECDEBUG00
gotoxy(1,1); printf ("_ECDEBUG: _ebr (par=%s)", par);
#ifdef _ECDEBUG01
getch();
#endif
#endif
if (_emgr.rem || _emgr.skipec) return;
sscanf(par, "%d", &p); ebright(p);
}
// -----------------------------------------------------------------------
void _ever(char* par)
{
int ver;
#ifdef _ECDEBUG00
gotoxy(1,1); printf ("_ECDEBUG: _ever (par=%s)", par);
#ifdef _ECDEBUG01
getch();
#endif
#endif
if (_emgr.rem || _emgr.skipec) return;
sscanf(par, "%x", &ver);
#ifdef _ECDEBUG00
gotoxy(1,2); printf ("_ECDEBUG: _ever (ver=%#x)", ver);
#ifdef _ECDEBUG01
getch();
#endif
#endif
if (ver>ever_d)
	{printf("Error: Unknown version %x (current is %x)!!!\n",
	ver,ever_d); _emgr.skipec=1; }
}
// -----------------------------------------------------------------------
void _ecls(char* par)
{
#ifdef _ECDEBUG00
gotoxy(1,1); printf ("_ECDEBUG: _ecls (par=%s)", par);
#ifdef _ECDEBUG01
getch();
#endif
#endif
if (_emgr.rem || _emgr.skipec) return; clrscr(); if (par); }
// -----------------------------------------------------------------------
void _ecur(char* par)
{
int p;
#ifdef _ECDEBUG00
gotoxy(1,1); printf ("_ECDEBUG: _ecur (par=%s)", par);
#ifdef _ECDEBUG01
getch();
#endif
#endif
if (_emgr.rem || _emgr.skipec) return;
sscanf(par, "%d", &p);
if (p==0) _setcursortype(_NOCURSOR);
if (p==1) _setcursortype(_NORMALCURSOR);
if (p==2) _setcursortype(_SOLIDCURSOR);
}
// -----------------------------------------------------------------------
void _erem(char* par)
{
#ifdef _ECDEBUG00
gotoxy(1,1); printf ("_ECDEBUG: _erem (par=%s)", par);
#ifdef _ECDEBUG01
getch();
#endif
#endif
if (_emgr.skipec) return; _emgr.rem=!_emgr.rem; if (par); }
// -----------------------------------------------------------------------
void _eend(char* par)
{
#ifdef _ECDEBUG00
gotoxy(1,1); printf ("_ECDEBUG: _eend (par=%s)", par);
#ifdef _ECDEBUG01
getch();
#endif
#endif
if (_emgr.rem || _emgr.skipec) return; _emgr.work=0; if (par);
}
// -----------------------------------------------------------------------
void _eec(char* par)
{
int p;
#ifdef _ECDEBUG00
gotoxy(1,1); printf ("_ECDEBUG: _eec (par=%s)", par);
#ifdef _ECDEBUG01
getch();
#endif
#endif
if (_emgr.rem) return;
_emgr.skipec=!_emgr.skipec; if (par); }
// -----------------------------------------------------------------------
void _erst(char* par)
{
#ifdef _ECDEBUG00
gotoxy(1,1); printf ("_ECDEBUG: _erst (par=%s)", par);
#ifdef _ECDEBUG01
getch();
#endif
#endif
if (_emgr.rem || _emgr.skipec) return; einit(); if (par);
}
// -----------------------------------------------------------------------
void _ewin(char* par)
{
int p0, p1, p2, p3;
long p;
#ifdef _ECDEBUG00
gotoxy(1,1); printf ("_ECDEBUG: _ewin (par=%s)", par);
#ifdef _ECDEBUG01
getch();
#endif
#endif
if (_emgr.rem || _emgr.skipec) return;
sscanf(par, "%ld", &p); p3=p%eRADIX; p/=eRADIX;
p2=p%eRADIX; p/=eRADIX; p1=p%eRADIX; p0=p/eRADIX;
window(p0, p1, p0+p2, p1+p3);
#ifdef _ECDEBUG00
gotoxy(1,2);
printf ("_ECDEBUG: _ewin (l,r,t,b=%d, %d, %d, %d)",p0,p1,p0+p2,p1+p3);
#ifdef _ECDEBUG01
getch();
#endif
#endif
}
// -----------------------------------------------------------------------
void _ewina(char* par)
{
int p0, p1, p2, p3;
long p;
#ifdef _ECDEBUG00
gotoxy(1,1); printf ("_ECDEBUG: _ewina (par=%s)", par);
#ifdef _ECDEBUG01
getch();
#endif
#endif
if (_emgr.rem || _emgr.skipec) return;
sscanf(par, "%ld", &p); p3=p%eRADIX; p/=eRADIX;
p2=p%eRADIX; p/=eRADIX; p1=p%eRADIX; p0=p/eRADIX;
window(p0, p1, p2, p3);
#ifdef _ECDEBUG00
gotoxy(1,2);
printf ("_ECDEBUG: _ewina (l,r,t,b=%d, %d, %d, %d)",p0,p1,p2,p3);
#ifdef _ECDEBUG01
getch();
#endif
#endif
}
// -----------------------------------------------------------------------
void _eloc(char* par)
{
int p0, p1;
#ifdef _ECDEBUG00
gotoxy(1,1); printf ("_ECDEBUG: _eloc (par=%s)", par);
#ifdef _ECDEBUG01
getch();
#endif
#endif
if (_emgr.rem || _emgr.skipec) return;
sscanf(par, "%d", &p0); p1=p0%eRADIX; p0/=eRADIX;
gotoxy(p0, p1);
#ifdef _ECDEBUG00
gotoxy(1,2); printf ("_ECDEBUG: _eloc (x=%d, y=%d)", p0, p1);
#ifdef _ECDEBUG01
getch();
#endif
#endif
}
// -----------------------------------------------------------------------
void _ec(char* par)
{
int p0, p1;
#ifdef _ECDEBUG00
gotoxy(1,1); printf ("_ECDEBUG: _ec (par=%s)", par);
#ifdef _ECDEBUG01
getch();
#endif
#endif
if (_emgr.rem || _emgr.skipec) return;
sscanf(par, "%d", &p0); p1=p0%eRADIX; p0/=eRADIX;
ecolor(p0, p1);
#ifdef _ECDEBUG00
gotoxy(1,2); printf ("_ECDEBUG: _ec (f=%d, b=%d)", p0, p1);
#ifdef _ECDEBUG01
getch();
#endif
#endif
}
// -----------------------------------------------------------------------
void _ef(char* par)
{
int p;
#ifdef _ECDEBUG00
gotoxy(1,1); printf ("_ECDEBUG: _ef (par=%s)", par);
#ifdef _ECDEBUG01
getch();
#endif
#endif
if (_emgr.rem || _emgr.skipec) return;
sscanf(par, "%d", &p);
efore(p);
#ifdef _ECDEBUG00
gotoxy(1,2); printf ("_ECDEBUG: _ef (f=%d)", p);
#ifdef _ECDEBUG01
getch();
#endif
#endif
}
// -----------------------------------------------------------------------
void _eg(char* par)
{
int p;
#ifdef _ECDEBUG00
gotoxy(1,1); printf ("_ECDEBUG: _eg (par=%s)", par);
#ifdef _ECDEBUG01
getch();
#endif
#endif
if(_emgr.rem || _emgr.skipec) return;
if(_emgr.bmode)return;
getch(); if(par);}
// =======================================================================
// Interface functions (Standart functions)
// =======================================================================
void ebright (int par)
{
par=!par;
_AX=0x1003;
_BL=(char)par;
#ifndef _ECWIN32
geninterrupt(0x10);
#endif
}
// =======================================================================
void ecolor (int fg, int bg)
{
efore (fg); eback (bg);
}
// =======================================================================
int eprintf (const char* format, ...)
{
char *buffer, *tempbuf;
int i, blks, ablks, len;
va_list arglist;
va_start(arglist, format);
if ((buffer=newm(char, _ECMEMALLOC))==NULL) return(EOF);
if ((tempbuf=newm(char, _ECMAXBLOCK))==NULL)
	{ deletem(buffer); return(EOF); }
vsprintf (buffer, format, arglist);
// -----------------------------------------------------------------------
// Calculates quantity of blocks
// -----------------------------------------------------------------------
len=strlen(buffer)+1; blks=len/_ECMAXBLOCK; ablks=len%_ECMAXBLOCK;
// -----------------------------------------------------------------------
// Output by blocks
// -----------------------------------------------------------------------
for (i=0; i<blks; i++)
        {
        strncpy(tempbuf, buffer+i*_ECMAXBLOCK, _ECMAXBLOCK);
        eputs (tempbuf);
        }
if (ablks)
        {
        strncpy(tempbuf, buffer+blks*_ECMAXBLOCK, len-blks*_ECMAXBLOCK);
        eputs (tempbuf);
        }
// -----------------------------------------------------------------------
deletem(buffer); deletem(tempbuf);
return (EPOSITIVE);
}
// =======================================================================
int eputs (char *s)
{
// -----------------------------------------------------------------------
char temp[256], buffer[_ECMEMALLOC];
char *cptr;
int spos, cpos;         // Start & control position of text block
int i, k;
int len=strlen(s);
// -----------------------------------------------------------------------
if (!_emgr.init) return(printf("Error: Init Econio first !\n"));
// -----------------------------------------------------------------------
spos=0;
// -----------------------------------------------------------------------
do      {
        // ---------------------------------------------------------------
        // Searchs the control code '#'
	// ---------------------------------------------------------------
        cptr=strstr(s+spos, ECTRL);
        if (cptr)
                {
                // -------------------------------------------------------
                // Outputs a text block w/o control code
                // -------------------------------------------------------
                cptr=cptr+1;
                cpos=cptr-s;
                strncpy(buffer, s+spos, cpos-spos-1);
		buffer[cpos-spos-1]=0;
		if (!_emgr.rem)
		#ifdef _MILBELL_
		mus_cputs(buffer);
		#else
		cputs(buffer);
		#endif
                // -------------------------------------------------------
                // Compares all commands
                // -------------------------------------------------------
                for (i=0; i<_emgr.commands; i++)
                        {
                        k=efindnumb(i);
                        if (!strnicmp (cptr,edata[k].ecmds, edata[k].ecoml))
                                {
				strncpy(temp,cptr+edata[k].ecoml,
                                        edata[k].eparl);
                                temp[edata[k].eparl]=NULL;
				edata[k].f(temp);
				spos=cpos+edata[k].ecoml+edata[k].eparl;
				if (_emgr.skipec) spos=cpos;
                                break;
                                }
                        }
                // -------------------------------------------------------
                }
        // ---------------------------------------------------------------
        else    {
                if (!_emgr.rem) cputs (s+spos);
                spos=len;
                }
        // ---------------------------------------------------------------
	} while (spos<len && _emgr.work);
return (len);
}
// =======================================================================
int estatus(void)
{
// -----------------------------------------------------------------------
// Description: Prints Econio status
//   Parametrs: <none>
//      Return: NULL - SUCCESS ...
// -----------------------------------------------------------------------
#ifdef _ECDEBUG
int i;
printf ("\n"
        "Econio status information:\n"
        "version        = %s\n"
        "_emgr.init     = %d\n"
        "_emgr.bmode    = %d\tmode of work (default is mINTER)\n"
	"_emgr.work     = %d\t\"work\" flag\n"
	"_emgr.skipec   = %d\t\"skip EC commands\" flag\n"
	"_emgr.rem      = %d\t\"remark\" flag\n"
        "_emgr.commands = %d\tquantity of commands\n"
        "_emgr.blink    = %d\tblinking mode\n\n",
	ever, _emgr.init, _emgr.bmode, _emgr.work, _emgr.skipec, _emgr.rem,
	_emgr.commands, _emgr.blink);
#endif
return(!_emgr.init);
}
// =======================================================================
int ehelp(void)
{
// -----------------------------------------------------------------------
// Description: Prints Econio's help page
//   Parametrs: <none>
//      Return: NULL - SUCCESS ...
// -----------------------------------------------------------------------
#ifdef _ECDEBUG
int i;
printf ("\n"
	"Econio's help page (Available commands):\n");
for (i=0; i<_emgr.commands; i++)
	printf ("%2d. %-8s - %2d. #%-8s coml=%2d parl=%2d\n",
		edata[efindnumb(i)].enumb,      edata[efindnumb(i)].ecmds,
		efindnumb(i),                   edata[efindnumb(i)].ecmds,
                edata[efindnumb(i)].ecoml,      edata[efindnumb(i)].eparl);
#endif
return(NULL);
}
// =======================================================================