/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <errno.h>
#ifdef __linux__ // rb010123
  #include <mntent.h>
#endif
#include <dlfcn.h>

#ifdef __linux__
  #include <fpu_control.h> // bk001213 - force dumps on divide by zero
#endif

// FIXME TTimo should we gard this? most *nix system should comply?
#include <termios.h>

#include "../qcommon/qcommon.h"
#include "../qcommon/cmd.h"
#include "../qcommon/netchan.h"
#include "../universal/universal_public.h"

#include "linux_local.h" // bk001204

unsigned  sys_frame_time;

uid_t saved_euid;
qboolean stdin_active = qtrue;

// =============================================================
// tty console variables
// =============================================================

// enable/disabled tty input mode
// NOTE TTimo this is used during startup, cannot be changed during run
static dvar_t *ttycon = NULL;
// general flag to tell about tty console mode
static qboolean ttycon_on = qfalse;
// when printing general stuff to stdout stderr (Sys_Printf)
//   we need to disable the tty console stuff
// this increments so we can recursively disable
static int ttycon_hide = 0;
// some key codes that the terminal may be using
// TTimo NOTE: I'm not sure how relevant this is
static int tty_erase;
static int tty_eof;

static struct termios tty_tc;

static field_t tty_con;

// history
// NOTE TTimo this is a bit duplicate of the graphical console history
//   but it's safer and faster to write our own here
#define TTY_HISTORY 32
static field_t ttyEditLines[TTY_HISTORY];
static int hist_current = -1, hist_count = 0;

// =======================================================================
// General routines
// =======================================================================

// bk001207 
#define MEM_THRESHOLD 96*1024*1024

/*
==================
Sys_LowPhysicalMemory()
==================
*/
qboolean Sys_LowPhysicalMemory() {
  //MEMORYSTATUS stat;
  //GlobalMemoryStatus (&stat);
  //return (stat.dwTotalPhys <= MEM_THRESHOLD) ? qtrue : qfalse;
  return qfalse; // bk001207 - FIXME
}

/*
==================
Sys_FunctionCmp
==================
*/
int Sys_FunctionCmp(void *f1, void *f2) {
  return qtrue;
}

/*
==================
Sys_FunctionCheckSum
==================
*/
int Sys_FunctionCheckSum(void *f1) {
  return 0;
}

/*
==================
Sys_MonkeyShouldBeSpanked
==================
*/
int Sys_MonkeyShouldBeSpanked( void ) {
  return 0;
}

void Sys_BeginProfiling( void ) {
}

/*
=================
Sys_In_Restart_f

Restart the input subsystem
=================
*/
void Sys_In_Restart_f( void ) 
{
#ifdef GLWIMP
  IN_Shutdown();
  IN_Init();
#endif
}

// =============================================================
// tty console routines
// NOTE: if the user is editing a line when something gets printed to the early console then it won't look good
//   so we provide tty_Clear and tty_Show to be called before and after a stdout or stderr output
// =============================================================

// flush stdin, I suspect some terminals are sending a LOT of shit
// FIXME TTimo relevant?
void tty_FlushIn()
{
  char key;
  while (read(0, &key, 1)!=-1);
}

// do a backspace
// TTimo NOTE: it seems on some terminals just sending '\b' is not enough
//   so for now, in any case we send "\b \b" .. yeah well ..
//   (there may be a way to find out if '\b' alone would work though)
void tty_Back()
{
  char key;
  key = '\b';
  write(1, &key, 1);
  key = ' ';
  write(1, &key, 1);
  key = '\b';
  write(1, &key, 1);
}

// clear the display of the line currently edited
// bring cursor back to beginning of line
void tty_Hide()
{
  int i;
  assert(ttycon_on);
  if (ttycon_hide)
  {
    ttycon_hide++;
    return;
  }
  if (tty_con.cursor>0)
  {
    for (i=0; i<tty_con.cursor; i++)
    {
      tty_Back();
    }
  }
  ttycon_hide++;
}

// show the current line
// FIXME TTimo need to position the cursor if needed??
void tty_Show()
{
  int i;
  assert(ttycon_on);
  assert(ttycon_hide>0);
  ttycon_hide--;
  if (ttycon_hide == 0)
  {
    if (tty_con.cursor)
    {
      for (i=0; i<tty_con.cursor; i++)
      {
        write(1, tty_con.buffer+i, 1);
      }
    }
  }
}

// never exit without calling this, or your terminal will be left in a pretty bad state
void Sys_ConsoleInputShutdown()
{
  if (ttycon_on)
  {
    Com_Printf("Shutdown tty console\n");
    tcsetattr (0, TCSADRAIN, &tty_tc);
  }
}

void Hist_Add(field_t *field)
{
  int i;
  assert(hist_count <= TTY_HISTORY);
  assert(hist_count >= 0);
  assert(hist_current >= -1);
  assert(hist_current <= hist_count);
  // make some room
  for (i=TTY_HISTORY-1; i>0; i--)
  {
    ttyEditLines[i] = ttyEditLines[i-1];
  }
  ttyEditLines[0] = *field;
  if (hist_count<TTY_HISTORY)
  {
    hist_count++;
  }
  hist_current = -1; // re-init
}

field_t *Hist_Prev()
{
  int hist_prev;
  assert(hist_count <= TTY_HISTORY);
  assert(hist_count >= 0);
  assert(hist_current >= -1);
  assert(hist_current <= hist_count);
  hist_prev = hist_current + 1;
  if (hist_prev >= hist_count)
  {
    return NULL;
  }
  hist_current++;
  return &(ttyEditLines[hist_current]);
}

field_t *Hist_Next()
{
  assert(hist_count <= TTY_HISTORY);
  assert(hist_count >= 0);
  assert(hist_current >= -1);
  assert(hist_current <= hist_count);
  if (hist_current >= 0)
  {
    hist_current--;
  }
  if (hist_current == -1)
  {
    return NULL;
  }
  return &(ttyEditLines[hist_current]);
}

// =============================================================
// general sys routines
// =============================================================

#if 0
// NOTE TTimo this is not used .. looks interesting though? protection against buffer overflow kind of stuff?
void Sys_Printf (char *fmt, ...)
{
  va_list   argptr;
  char    text[1024];
  unsigned char   *p;

  va_start (argptr,fmt);
  vsprintf (text,fmt,argptr);
  va_end (argptr);

  if (strlen(text) > sizeof(text))
    Sys_Error("memory overwrite in Sys_Printf");

  for (p = (unsigned char *)text; *p; p++)
  {
    *p &= 0x7f;
    if ((*p > 128 || *p < 32) && *p != 10 && *p != 13 && *p != 9)
      printf("[%02x]", *p);
    else
      putc(*p, stdout);
  }
}
#endif

// single exit point (regular exit or in case of signal fault)
void Sys_Exit( int ex ) {
  Sys_ConsoleInputShutdown();

#ifdef NDEBUG // regular behavior

  // We can't do this 
  //  as long as GL DLL's keep installing with atexit...
  //exit(ex);
  _exit(ex);
#else

  // Give me a backtrace on error exits.
  //assert( ex == 0 );
  exit(ex);
#endif
}


void Sys_Quit (void) {
  Dvar_Shutdown();
  Cmd_Shutdown();
  Hunk_Shutdown();
#ifndef DEDICATED
  CL_Shutdown ();
#endif
  fcntl (0, F_SETFL, fcntl (0, F_GETFL, 0) & ~FNDELAY);
  Sys_Exit(0);
}

void Sys_Init(void)
{
#ifdef GLWIMP
  Cmd_AddCommand ("in_restart", Sys_In_Restart_f);
#endif

#if defined __linux__
#if defined __i386__
  Dvar_RegisterString( "arch", "linux i386", DVAR_ROM );
#elif defined __alpha__
  Dvar_RegisterString( "arch", "linux alpha", DVAR_ROM );
#elif defined __sparc__
  Dvar_RegisterString( "arch", "linux sparc", DVAR_ROM );
#elif defined __FreeBSD__

#if defined __i386__ // FreeBSD
  Dvar_RegisterString( "arch", "freebsd i386", DVAR_ROM );
#elif defined __alpha__
  Dvar_RegisterString( "arch", "freebsd alpha", DVAR_ROM );
#else
  Dvar_RegisterString( "arch", "freebsd unknown", DVAR_ROM );
#endif // FreeBSD

#else
  Dvar_RegisterString( "arch", "linux unknown", DVAR_ROM );
#endif
#elif defined __sun__
#if defined __i386__
  Dvar_RegisterString( "arch", "solaris x86", DVAR_ROM );
#elif defined __sparc__
  Dvar_RegisterString( "arch", "solaris sparc", DVAR_ROM );
#else
  Dvar_RegisterString( "arch", "solaris unknown", DVAR_ROM );
#endif
#elif defined __sgi__
#if defined __mips__
  Dvar_RegisterString( "arch", "sgi mips", DVAR_ROM );
#else
  Dvar_RegisterString( "arch", "sgi unknown", DVAR_ROM );
#endif
#else
  Dvar_RegisterString( "arch", "unknown", DVAR_ROM );
#endif

  Dvar_RegisterString( "username", Sys_GetCurrentUser(), DVAR_ROM );

#ifdef GLWIMP
  IN_Init();
#endif
}

void  Sys_Error( const char *error, ...)
{ 
  va_list     argptr;
  char        string[1024];

  // change stdin to non blocking
  // NOTE TTimo not sure how well that goes with tty console mode
  fcntl (0, F_SETFL, fcntl (0, F_GETFL, 0) & ~FNDELAY);

  // don't bother do a show on this one heh
  if (ttycon_on)
  {
    tty_Hide();
  }

#ifndef DEDICATED
  CL_Shutdown ();
#endif

  va_start (argptr,error);
  vsprintf (string,error,argptr);
  va_end (argptr);
  fprintf(stderr, "Sys_Error: %s\n", string);

  Sys_Exit( 1 ); // bk010104 - use single exit point.
} 

void Sys_Warn (char *warning, ...)
{ 
  va_list     argptr;
  char        string[1024];

  va_start (argptr,warning);
  vsprintf (string,warning,argptr);
  va_end (argptr);

  if (ttycon_on)
  {
    tty_Hide();
  }

  fprintf(stderr, "Warning: %s", string);

  if (ttycon_on)
  {
    tty_Show();
  }
} 

/*
============
Sys_FileTime

returns -1 if not present
============
*/
int Sys_FileTime (char *path)
{
  struct  stat  buf;

  if (stat (path,&buf) == -1)
    return -1;

  return buf.st_mtime;
}

void floating_point_exception_handler(int whatever)
{
  signal(SIGFPE, floating_point_exception_handler);
}

/*
==================
Field_Clear
==================
*/
void Field_Clear( field_t *edit ) {
	memset( edit->buffer, 0, MAX_EDIT_LINE );
	edit->cursor = 0;
	edit->scroll = 0;
}

static const char *completionString;
static char shortestMatch[MAX_TOKEN_CHARS];
static int matchCount;
// field we are working on, passed to Field_CompleteCommand (&g_consoleCommand for instance)
static field_t *completionField;

/*
===============
FindMatches

===============
*/
static void FindMatches( const char *s ) {
	int i;

	if ( I_strnicmp( s, completionString, strlen( completionString ) ) ) {
		return;
	}
	matchCount++;
	if ( matchCount == 1 ) {
		Q_strncpyz( shortestMatch, s, sizeof( shortestMatch ) );
		return;
	}

	// cut shortestMatch to the amount common with s
	for ( i = 0 ; s[i] ; i++ ) {
		if ( tolower( shortestMatch[i] ) != tolower( s[i] ) ) {
			shortestMatch[i] = 0;
		}
	}
	shortestMatch[i] = 0;
}

/*
===============
PrintMatches

===============
*/
static void PrintMatches( const char *s ) {
	if ( !I_strnicmp( s, shortestMatch, strlen( shortestMatch ) ) ) {
		Com_Printf( "    %s\n", s );
	}
}

static void keyConcatArgs( void ) {
	int i;
	const char    *arg;

	for ( i = 1 ; i < Cmd_Argc() ; i++ ) {
		Q_strncat( completionField->buffer, sizeof( completionField->buffer ), " " );
		arg = Cmd_Argv( i );
		while ( *arg ) {
			if ( *arg == ' ' ) {
				Q_strncat( completionField->buffer, sizeof( completionField->buffer ),  "\"" );
				break;
			}
			arg++;
		}
		Q_strncat( completionField->buffer, sizeof( completionField->buffer ),  Cmd_Argv( i ) );
		if ( *arg == ' ' ) {
			Q_strncat( completionField->buffer, sizeof( completionField->buffer ),  "\"" );
		}
	}
}

static void ConcatRemaining( const char *src, const char *start ) {
	const char *str;

	str = strstr( src, start );
	if ( !str ) {
		keyConcatArgs();
		return;
	}

	str += strlen( start );
	Q_strncat( completionField->buffer, sizeof( completionField->buffer ), str );
}

/*
===============
Field_CompleteCommand

perform Tab expansion
NOTE TTimo this was originally client code only
  moved to common code when writing tty console for *nix dedicated server
===============
*/
void Field_CompleteCommand( field_t *field ) {
	field_t temp;
	completionField = field;

	// only look at the first token for completion purposes
	Cmd_TokenizeString( completionField->buffer );

	completionString = Cmd_Argv( 0 );
	if ( completionString[0] == '\\' || completionString[0] == '/' ) {
		completionString++;
	}
	matchCount = 0;
	shortestMatch[0] = 0;

	if ( strlen( completionString ) == 0 ) {
		return;
	}

	Cmd_ForEach( FindMatches );
	Dvar_ForEach( FindMatches );

	if ( matchCount == 0 ) {
		return; // no matches
	}

	Com_Memcpy( &temp, completionField, sizeof( field_t ) );

	if ( matchCount == 1 ) {
		Com_sprintf( completionField->buffer, sizeof( completionField->buffer ), "\\%s", shortestMatch );
		if ( Cmd_Argc() == 1 ) {
			Q_strncat( completionField->buffer, sizeof( completionField->buffer ), " " );
		} else {
			ConcatRemaining( temp.buffer, completionString );
		}
		completionField->cursor = strlen( completionField->buffer );
		return;
	}

	// multiple matches, complete to shortest
	Com_sprintf( completionField->buffer, sizeof( completionField->buffer ), "\\%s", shortestMatch );
	completionField->cursor = strlen( completionField->buffer );
	ConcatRemaining( temp.buffer, completionString );

	Com_Printf( "]%s\n", completionField->buffer );

	// run through again, printing matches
	Cmd_ForEach( PrintMatches );
	Dvar_ForEach( PrintMatches );
}

// initialize the console input (tty mode if wanted and possible)
void Sys_ConsoleInputInit()
{
  struct termios tc;

  // TTimo 
  // https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=390
  // ttycon 0 or 1, if the process is backgrounded (running non interactively)
  // then SIGTTIN or SIGTOU is emitted, if not catched, turns into a SIGSTP
  signal(SIGTTIN, SIG_IGN);
  signal(SIGTTOU, SIG_IGN);

  // FIXME TTimo initialize this in Sys_Init or something?
  ttycon = Dvar_RegisterBool("ttycon", 0, DVAR_LATCH);
  if (Dvar_GetBool("ttycon"))
  {
    if (isatty(STDIN_FILENO)!=1)
    {
      Com_Printf("stdin is not a tty, tty console mode failed\n");
      Dvar_SetBool(ttycon, 0);
      ttycon_on = qfalse;
      return;
    }
    Com_Printf("Started tty console (use +set ttycon 0 to disable)\n");
    Field_Clear(&tty_con);
    tcgetattr (0, &tty_tc);
    tty_erase = tty_tc.c_cc[VERASE];
    tty_eof = tty_tc.c_cc[VEOF];
    tc = tty_tc;
    /*
     ECHO: don't echo input characters
     ICANON: enable canonical mode.  This  enables  the  special
              characters  EOF,  EOL,  EOL2, ERASE, KILL, REPRINT,
              STATUS, and WERASE, and buffers by lines.
     ISIG: when any of the characters  INTR,  QUIT,  SUSP,  or
              DSUSP are received, generate the corresponding sig�
              nal
    */              
    tc.c_lflag &= ~(ECHO | ICANON);
    /*
     ISTRIP strip off bit 8
     INPCK enable input parity checking
     */
    tc.c_iflag &= ~(ISTRIP | INPCK);
    tc.c_cc[VMIN] = 1;
    tc.c_cc[VTIME] = 0;
    tcsetattr (0, TCSADRAIN, &tc);    
    ttycon_on = qtrue;
  } else
    ttycon_on = qfalse;
}

char *Sys_ConsoleInput(void)
{
  // we use this when sending back commands
  static char text[256];
  int i;
  int avail;
  char key;
  field_t *history;

  if (Dvar_GetBool("ttycon"))
  {
    avail = read(0, &key, 1);
    if (avail != -1)
    {
      // we have something
      // backspace?
      // NOTE TTimo testing a lot of values .. seems it's the only way to get it to work everywhere
      if ((key == tty_erase) || (key == 127) || (key == 8))
      {
        if (tty_con.cursor > 0)
        {
          tty_con.cursor--;
          tty_con.buffer[tty_con.cursor] = '\0';
          tty_Back();
        }
        return NULL;
      }
      // check if this is a control char
      if ((key) && (key) < ' ')
      {
        if (key == '\n')
        {
          // push it in history
          Hist_Add(&tty_con);
          strcpy(text, tty_con.buffer);
          Field_Clear(&tty_con);
          key = '\n';
          write(1, &key, 1);
          return text;
        }
        if (key == '\t')
        {
          tty_Hide();
          Field_CompleteCommand( &tty_con );
          // Field_CompleteCommand does weird things to the string, do a cleanup
          //   it adds a '\' at the beginning of the string
          //   cursor doesn't reflect actual length of the string that's sent back
          tty_con.cursor = strlen(tty_con.buffer);
          if (tty_con.cursor>0)
          {
            if (tty_con.buffer[0] == '\\')
            {
              for (i=0; i<=tty_con.cursor; i++)
              {
                tty_con.buffer[i] = tty_con.buffer[i+1];
              }
              tty_con.cursor--;
            }
          }
          tty_Show();
          return NULL;
        }
        avail = read(0, &key, 1);
        if (avail != -1)
        {
          // VT 100 keys
          if (key == '[' || key == 'O')
          {
            avail = read(0, &key, 1);
            if (avail != -1)
            {
              switch (key)
              {
              case 'A':
                history = Hist_Prev();
                if (history)
                {
                  tty_Hide();
                  tty_con = *history;
                  tty_Show();
                }
                tty_FlushIn();
                return NULL;
                break;
              case 'B':
                history = Hist_Next();
                tty_Hide();
                if (history)
                {
                  tty_con = *history;
                } else
                {
                  Field_Clear(&tty_con);
                }
                tty_Show();
                tty_FlushIn();
                return NULL;
                break;
              case 'C':
                return NULL;
              case 'D':
                return NULL;
              }
            }
          }
        }
        Com_DPrintf("droping ISCTL sequence: %d, tty_erase: %d\n", key, tty_erase);
        tty_FlushIn();
        return NULL;
      }
      // push regular character
      tty_con.buffer[tty_con.cursor] = key;
      tty_con.cursor++;
      // print the current line (this is differential)
      write(1, &key, 1);
    }
    return NULL;
  } else
  {
    int     len;
    fd_set  fdset;
    struct timeval timeout;

    if (!com_dedicated || !com_dedicated->current.integer)
      return NULL;

    if (!stdin_active)
      return NULL;

    FD_ZERO(&fdset);
    FD_SET(0, &fdset); // stdin
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    if (select (1, &fdset, NULL, NULL, &timeout) == -1 || !FD_ISSET(0, &fdset))
    {
      return NULL;
    }

    len = read (0, text, sizeof(text));
    if (len == 0)
    { // eof!
      stdin_active = qfalse;
      return NULL;
    }

    if (len < 1)
      return NULL;
    text[len-1] = 0;    // rip off the /n and terminate

    return text;
  }
}

/*
========================================================================

BACKGROUND FILE STREAMING

========================================================================
*/

#if 1

void Sys_InitStreamThread( void ) {
}

void Sys_ShutdownStreamThread( void ) {
}

void Sys_BeginStreamedFile( fileHandle_t f, int readAhead ) {
}

void Sys_EndStreamedFile( fileHandle_t f ) {
}

int Sys_StreamedRead( void *buffer, int size, int count, fileHandle_t f ) {
  return FS_Read( buffer, size * count, f );
}

void Sys_StreamSeek( fileHandle_t f, int offset, int origin ) {
  FS_Seek( f, offset, origin );
}

#else

typedef struct
{
  fileHandle_t file;
  byte  *buffer;
  qboolean  eof;
  int   bufferSize;
  int   streamPosition; // next byte to be returned by Sys_StreamRead
  int   threadPosition; // next byte to be read from file
} streamState_t;

streamState_t stream;

/*
===============
Sys_StreamThread

A thread will be sitting in this loop forever
================
*/
void Sys_StreamThread( void ) 
{
  int   buffer;
  int   count;
  int   readCount;
  int   bufferPoint;
  int   r;

  // if there is any space left in the buffer, fill it up
  if ( !stream.eof )
  {
    count = stream.bufferSize - (stream.threadPosition - stream.streamPosition);
    if ( count )
    {
      bufferPoint = stream.threadPosition % stream.bufferSize;
      buffer = stream.bufferSize - bufferPoint;
      readCount = buffer < count ? buffer : count;
      r = FS_Read ( stream.buffer + bufferPoint, readCount, stream.file );
      stream.threadPosition += r;

      if ( r != readCount )
        stream.eof = qtrue;
    }
  }
}

/*
===============
Sys_InitStreamThread

================
*/
void Sys_InitStreamThread( void ) 
{
}

/*
===============
Sys_ShutdownStreamThread

================
*/
void Sys_ShutdownStreamThread( void ) 
{
}


/*
===============
Sys_BeginStreamedFile

================
*/
void Sys_BeginStreamedFile( fileHandle_t f, int readAhead ) 
{
  if ( stream.file )
  {
    Com_Error( ERR_FATAL, "Sys_BeginStreamedFile: unclosed stream");
  }

  stream.file = f;
  stream.buffer = Z_Malloc( readAhead );
  stream.bufferSize = readAhead;
  stream.streamPosition = 0;
  stream.threadPosition = 0;
  stream.eof = qfalse;
}

/*
===============
Sys_EndStreamedFile

================
*/
void Sys_EndStreamedFile( fileHandle_t f ) 
{
  if ( f != stream.file )
  {
    Com_Error( ERR_FATAL, "Sys_EndStreamedFile: wrong file");
  }

  stream.file = 0;
  Z_Free( stream.buffer );
}


/*
===============
Sys_StreamedRead

================
*/
int Sys_StreamedRead( void *buffer, int size, int count, fileHandle_t f ) 
{
  int   available;
  int   remaining;
  int   sleepCount;
  int   copy;
  int   bufferCount;
  int   bufferPoint;
  byte  *dest;

  dest = (byte *)buffer;
  remaining = size * count;

  if ( remaining <= 0 )
  {
    Com_Error( ERR_FATAL, "Streamed read with non-positive size" );
  }

  sleepCount = 0;
  while ( remaining > 0 )
  {
    available = stream.threadPosition - stream.streamPosition;
    if ( !available )
    {
      if (stream.eof)
        break;
      Sys_StreamThread();
      continue;
    }

    bufferPoint = stream.streamPosition % stream.bufferSize;
    bufferCount = stream.bufferSize - bufferPoint;

    copy = available < bufferCount ? available : bufferCount;
    if ( copy > remaining )
    {
      copy = remaining;
    }
    memcpy( dest, stream.buffer + bufferPoint, copy );
    stream.streamPosition += copy;
    dest += copy;
    remaining -= copy;
  }

  return(count * size - remaining) / size;
}

/*
===============
Sys_StreamSeek

================
*/
void Sys_StreamSeek( fileHandle_t f, int offset, int origin ) {
  // clear to that point
  FS_Seek( f, offset, origin );
  stream.streamPosition = 0;
  stream.threadPosition = 0;
  stream.eof = qfalse;
}

#endif

/*
========================================================================

EVENT LOOP

========================================================================
*/

// bk000306: upped this from 64
#define	MAX_QUED_EVENTS		256
#define	MASK_QUED_EVENTS	( MAX_QUED_EVENTS - 1 )

sysEvent_t  eventQue[MAX_QUED_EVENTS];
// bk000306: initialize
int   eventHead = 0;
int             eventTail = 0;
byte    sys_packetReceived[MAX_MSGLEN];

/*
================
Sys_QueEvent

A time of 0 will get the current time
Ptr should either be null, or point to a block of data that can
be freed by the game later.
================
*/
void Sys_QueEvent( int time, sysEventType_t type, int value, int value2, int ptrLength, void *ptr ) {
  sysEvent_t  *ev;

  ev = &eventQue[ eventHead & MASK_QUED_EVENTS ];

  // bk000305 - was missing
  if ( eventHead - eventTail >= MAX_QUED_EVENTS )
  {
    Com_Printf("Sys_QueEvent: overflow\n");
    // we are discarding an event, but don't leak memory
    if ( ev->evPtr )
    {
      Z_Free( ev->evPtr );
    }
    eventTail++;
  }

  eventHead++;

  if ( time == 0 )
  {
    time = Sys_Milliseconds();
  }

  ev->evTime = time;
  ev->evType = type;
  ev->evValue = value;
  ev->evValue2 = value2;
  ev->evPtrLength = ptrLength;
  ev->evPtr = ptr;
}

/*
================
Sys_GetEvent

================
*/
sysEvent_t Sys_GetEvent( void ) {
  sysEvent_t  ev;
  char    *s;
  msg_t   netmsg;
  netadr_t  adr;

  // return if we have data
  if ( eventHead > eventTail )
  {
    eventTail++;
    return eventQue[ ( eventTail - 1 ) & MASK_QUED_EVENTS ];
  }

  // pump the message loop
  // in vga this calls KBD_Update, under X, it calls GetEvent
#ifdef GLWIMP
  Sys_SendKeyEvents ();
#endif

  // check for console commands
  s = Sys_ConsoleInput();
  if ( s )
  {
    char  *b;
    int   len;

    len = strlen( s ) + 1;
    b = (char *)Z_Malloc( len );
    strcpy( b, s );
    Sys_QueEvent( 0, SE_CONSOLE, 0, 0, len, b );
  }

#ifdef GLWIMP
  // check for other input devices
  IN_Frame();
#endif

  // check for network packets
  MSG_Init( &netmsg, sys_packetReceived, sizeof( sys_packetReceived ) );
  if ( Sys_GetPacket ( &adr, &netmsg ) )
  {
    netadr_t    *buf;
    int       len;

    // copy out to a seperate buffer for qeueing
    len = sizeof( netadr_t ) + netmsg.cursize;
    buf = (netadr_t *)Z_Malloc( len );
    *buf = adr;
    memcpy( buf+1, netmsg.data, netmsg.cursize );
    Sys_QueEvent( 0, SE_PACKET, 0, 0, len, buf );
  }

  // return if we have data
  if ( eventHead > eventTail )
  {
    eventTail++;
    return eventQue[ ( eventTail - 1 ) & MASK_QUED_EVENTS ];
  }

  // create an empty event to return

  memset( &ev, 0, sizeof( ev ) );
  ev.evTime = Sys_Milliseconds();

  return ev;
}

/*****************************************************************************/

qboolean Sys_CheckCD( void ) {
  return qtrue;
}

void Sys_AppActivate (void)
{
}

char *Sys_GetClipboardData(void)
{
  return NULL;
}

void  Sys_Print( const char *msg )
{
  if (ttycon_on)
  {
    tty_Hide();
  }
  fputs(msg, stderr);
  if (ttycon_on)
  {
    tty_Show();
  }
}


void    Sys_ConfigureFPU() { // bk001213 - divide by zero
#ifdef __linux__
#ifdef __i386
#ifndef NDEBUG

  // bk0101022 - enable FPE's in debug mode
  static int fpu_word = _FPU_DEFAULT & ~(_FPU_MASK_ZM | _FPU_MASK_IM);
  int current = 0;
  _FPU_GETCW(current);
  if ( current!=fpu_word)
  {
#if 0
    Com_Printf("FPU Control 0x%x (was 0x%x)\n", fpu_word, current );
    _FPU_SETCW( fpu_word );
    _FPU_GETCW( current );
    assert(fpu_word==current);
#endif
  }
#else // NDEBUG
  static int fpu_word = _FPU_DEFAULT;
  _FPU_SETCW( fpu_word );
#endif // NDEBUG
#endif // __i386 
#endif // __linux
}

void Sys_PrintBinVersion( const char* name ) {
  const char* date = __DATE__;
  const char* time = __TIME__;
  const char* sep = "==============================================================";
  fprintf( stdout, "\n\n%s\n", sep );
#ifdef DEDICATED
  fprintf( stdout, "Linux Quake3 Dedicated Server [%s %s]\n", date, time );  
#else
  fprintf( stdout, "Linux Quake3 Full Executable  [%s %s]\n", date, time );  
#endif
  fprintf( stdout, " local install: %s\n", name );
  fprintf( stdout, "%s\n\n", sep );
}

void Sys_ParseArgs( int argc, char* argv[] ) {

  if ( argc==2 )
  {
    if ( (!strcmp( argv[1], "--version" ))
         || ( !strcmp( argv[1], "-v" )) )
    {
      Sys_PrintBinVersion( argv[0] );
      Sys_Exit(0);
    }
  }
}

int main ( int argc, char* argv[] )
{
  // int 	oldtime, newtime; // bk001204 - unused
  int   len, i;
  char  *cmdline;
  void Sys_SetDefaultCDPath(const char *path);

  // go back to real user for config loads
  saved_euid = geteuid();
  seteuid(getuid());

  Sys_InitMainThread();
  Dvar_Init();
  Sys_ParseArgs( argc, argv );  // bk010104 - added this for support
  Sys_SetDefaultCDPath(argv[0]);

  // merge the command line, this is kinda silly
  for (len = 1, i = 1; i < argc; i++)
    len += strlen(argv[i]) + 1;
  cmdline = (char *)malloc(len);
  *cmdline = 0;
  for (i = 1; i < argc; i++)
  {
    if (i > 1)
      strcat(cmdline, " ");
    strcat(cmdline, argv[i]);
  }

  // bk000306 - clear queues
  memset( &eventQue[0], 0, MAX_QUED_EVENTS*sizeof(sysEvent_t) ); 
  memset( &sys_packetReceived[0], 0, MAX_MSGLEN*sizeof(byte) );

  Sys_Milliseconds();
  Com_Init(cmdline);

  Sys_ConsoleInputInit();

  fcntl(0, F_SETFL, fcntl (0, F_GETFL, 0) | FNDELAY);
	
#ifdef GLWIMP
	// init here for dedicated, as we don't have GLimp_Init
	InitSig();
#endif

  while (1)
  {
#ifdef __linux__
    Sys_ConfigureFPU();
#endif
	usleep(5000);
    Com_Frame();
  }
}
