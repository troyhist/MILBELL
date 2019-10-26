// =======================================================================
// PC speaker MUSIC ver. 1.00 (C)
// Troy computing, Inc.
// Copyright (c) 1997-2000
// -----------------------------------------------------------------------
// PC speaker music player.
// Used library: <none>
// -----------------------------------------------------------------------
// File: pcmusic.c
// -----------------------------------------------------------------------
//     Compiler: Borland C++ 3.1
//  Application: DOS16
// -----------------------------------------------------------------------
// Made date: 23/VII.2000
// Last edit: 28/VII.2000
//     Build: 1
// =======================================================================
#include <dos.h>		// sound, delay
#include <stdio.h>		// sscanf
#include <string.h>		// strlen
// -----------------------------------------------------------------------
#include "pcmusic.h"		//
// =======================================================================
// Global music constants
// =======================================================================
struct tagmusstat
	{
	int tacts;		// tacts per minute [20...250]
	int octave;		// octave number [0...6]
	int note;		// note number [0...6]
	int longing;		// note's longing [1...32]
	int clonging;		// current note's longing [1...32]
	int playflag;		// playing flag
	} musstat ={110, 2, 0, 4, 0, 0};
// =======================================================================
musnot octave[7]={
	{0,0,0,0,0,0,0},	// my edition
	{130.810,146.830,164.810,174.610,196.000,220.000,246.940},
	{261.630,293.660,329.630,349.230,392.000,440.000,493.880},
	{523.250,587.330,659.260,698.460,783.990,880.000,987.770},
	{1046.500,1174.700,1318.500,1396.900,1568.000,1760.000,1975.500},
	{0,0,0,0,0,0,0},        // my edition
	{0,0,0,0,0,0,0},        // my edition
	};
// -----------------------------------------------------------------------
#pragma warn -sus
musnotn* octaven=&octave;
#pragma warn +sus
// =======================================================================
// Internal Functions & Procedures ... PCMUSIC
// =======================================================================
char* filterdigits(char* str)
{
int i, l=strlen(str);
for (i=0; i<l; i++) if (str[i]<'0' || str[i]>'9') str[i]=0;
return (str);
}
// =======================================================================
char* parsemelody(char* melody)
{
char temp[5];
int i, longing=0;
switch(melody[0])
	{
	case '>': if (musstat.octave<6) musstat.octave++; break;
	case '<': if (musstat.octave>0) musstat.octave--; break;
	case 'T':
		strncpy (temp, melody+1, 3); temp[4]=0;
		sscanf (temp, "%d", &musstat.tacts);
		break;
	case 'O':
		strncpy (temp, melody+1, 2); temp[4]=0;
		sscanf (temp, "%d", &musstat.octave);
		break;
	case 'L':
		strncpy (temp, melody+1, 2); temp[4]=0;
		sscanf (filterdigits(temp), "%d", &musstat.longing);
		break;
	case 'C':case 'D':case 'E':case 'F':
	case 'G':case 'A':case 'B':case '.':
		switch (melody[0])
			{
			case 'C': musstat.note=0; break;
			case 'D': musstat.note=1; break;
			case 'E': musstat.note=2; break;
			case 'F': musstat.note=3; break;
			case 'G': musstat.note=4; break;
			case 'A': musstat.note=5; break;
			case 'B': musstat.note=6; break;
			case '.': musstat.note=0xff; break;	// pause
			}
		strncpy (temp, melody+1, 2); temp[4]=0;
		sscanf (filterdigits(temp), "%d", &longing);
		musstat.clonging=(longing>0)?longing:musstat.clonging;
		musstat.playflag=1;
		break;
	default: musstat.playflag=0; break;
	}
melody=melody+1;
return (melody);
}
// =======================================================================
// Functions & Procedures ... PCMUSIC
// =======================================================================
void pcsound (unsigned freq, unsigned del)
{ sound (freq); delay(del); nosound ();}
// =======================================================================
void pcmusic (char* mel)
{
char* melody=mel;
int done=0;
int l;		// temp longing
if (!strlen(mel)) return; strupr(mel);
do	{
	if (!strlen(melody)) done=1;
	melody=parsemelody(melody);
	if (musstat.playflag)
		{
		l=musstat.clonging?musstat.clonging:musstat.longing;
		if (musstat.note==0xff)
			delay (60000L/(musstat.tacts*(l+1)));
		else
			pcsound (octave[musstat.octave].note[musstat.note],
			60000L/(musstat.tacts*(l+1)));
		if (musstat.clonging) musstat.clonging=0;
		}
	} while (!done);
}
// =======================================================================
