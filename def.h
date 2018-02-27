/* Digger Remastered
   Copyright (c) Andrew Jenner 1998-2004 */

#ifdef GNU32
#ifndef _WINDOWS
#define _WINDOWS
#endif
#ifndef WIN32
#define WIN32
#endif
#endif

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#define TRUE -1
#define FALSE 0
typedef int bool;

typedef signed char Sint3;
typedef unsigned char Uint3;
typedef signed short int Sint4;
typedef unsigned short int Uint4;
typedef signed int Sint;
typedef unsigned int Uint;
typedef signed long int Sint5;
typedef unsigned long int Uint5;

#define DIR_NONE -1
#define DIR_RIGHT 0
#define DIR_UP 2
#define DIR_LEFT 4
#define DIR_DOWN 6

#define TYPES 5

#define BONUSES 1
#define BAGS 50
#define MONSTERS 6
#define DIGGERS 2

#define FIREBALLS DIGGERS
#define SPRITES (BONUSES+BAGS+MONSTERS+FIREBALLS+DIGGERS)

/* Sprite order is figured out here. By LAST I mean last+1. */

#define FIRSTBONUS 0
#define LASTBONUS (FIRSTBONUS+BONUSES)
#define FIRSTBAG LASTBONUS
#define LASTBAG (FIRSTBAG+BAGS)
#define FIRSTMONSTER LASTBAG
#define LASTMONSTER (FIRSTMONSTER+MONSTERS)
#define FIRSTFIREBALL LASTMONSTER
#define LASTFIREBALL (FIRSTFIREBALL+FIREBALLS)
#define FIRSTDIGGER LASTFIREBALL
#define LASTDIGGER (FIRSTDIGGER+DIGGERS)

#define MWIDTH 15
#define MHEIGHT 10
#define MSIZE MWIDTH*MHEIGHT

#define MAX_REC_BUFFER 262144l
           /* I reckon this is enough for about 36 hours of continuous play. */

#define INI_GAME_SETTINGS "Game"
#define INI_GRAPHICS_SETTINGS "Graphics"
#define INI_SOUND_SETTINGS "Sound"
#ifdef _WINDOWS
#define INI_KEY_SETTINGS "Win Keys"
#else
#define INI_KEY_SETTINGS "Keys"
#endif

#if defined(_WINDOWS) || defined(__OS2__)
#define DEFAULT_BUFFER 2048
#define DEF_SND_DEV 1
#else
#define DEFAULT_BUFFER 128
#ifdef ARM
#define DEF_SND_DEV 1
#else
#define DEF_SND_DEV 0
#endif
#endif

#if !defined (_MSVC) && defined (WIN32)
#define _int64 LARGE_INTEGER
#endif

#define FILENAME_BUFFER_SIZE 512

#if defined ARM || defined WIN32 || defined __OS2__
#define FLATFILE
#endif

#ifdef FLATFILE
#define near
#define far
#define huge
#define farmalloc malloc
#define farfree free
#define farcoreleft coreleft
#define farmemset memset
#define farmemcpy memcpy
#endif

#ifdef ARM
#define ININAME "Digger:Settings"
#else
#define ININAME "DIGGER.INI"
#endif

#if defined _WINDOWS
#define DIGGER_VERSION "TD WIN 20020310"
#elif defined ARM
#define DIGGER_VERSION "JB ARM 19990320"
#elif defined __OS2__
#define DIGGER_VERSION "VA OS2 20140620"
#else
#define DIGGER_VERSION "AJ DOS 20010825"
#endif

/* Version string:
  First word: your initials if you have changed anything.
  Second word: platform.
  Third word: compilation date in yyyymmdd format. */
