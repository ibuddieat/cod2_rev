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

#include "../qcommon/qcommon.h"
#include "linux_local.h"
#include <errno.h>
#include <pthread.h>

threadid_t mainthread;
void *g_threadValues[THREAD_VALUE_COUNT];

/*
================
Sys_InitializeCriticalSections
================
*/
static pthread_mutex_t crit_sections[CRITSECT_COUNT];
static void Sys_InitializeCriticalSections( void )
{
	int i;
	pthread_mutexattr_t muxattr;

	pthread_mutexattr_init(&muxattr);
	pthread_mutexattr_settype(&muxattr, PTHREAD_MUTEX_RECURSIVE);

	for (i = 0; i < CRITSECT_COUNT; i++)
	{
		pthread_mutex_init( &crit_sections[i], &muxattr );

	}

	pthread_mutexattr_destroy(&muxattr);
}

/*
================
Sys_InitMainThread
================
*/
void Sys_InitMainThread()
{
	Sys_InitializeCriticalSections();
	mainthread = pthread_self();
	Com_InitThreadData(THREAD_CONTEXT_MAIN);
}

/*
================
Sys_IsMainThread
================
*/
qboolean Sys_IsMainThread( void )
{
	return mainthread == pthread_self();
}

/*
================
Sys_SetValue
================
*/
void Sys_SetValue( int valueIndex, void *data )
{
	g_threadValues[valueIndex] = data;
}

/*
================
Sys_GetValue
================
*/
void* Sys_GetValue( int valueIndex )
{
	return g_threadValues[valueIndex];
}

/*
================
Sys_CreateNewThread
================
*/
#define MIN_STACKSIZE 256*1024
qboolean Sys_CreateNewThread(void* (*ThreadMain)(void*), threadid_t *tid, void* arg)
{
	int err;
	pthread_attr_t tattr;

	err = pthread_attr_init(&tattr);
	if(err != 0)
	{
		Com_Printf("pthread_attr_init(): Thread creation failed with the following error: %s\n", strerror(errno));
		return qfalse;
	}

	err = pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
	if(err != 0)
	{
		pthread_attr_destroy(&tattr);
		Com_Printf("pthread_attr_setdetachstate(): Thread creation failed with the following error: %s\n", strerror(errno));
		return qfalse;
	}

	err = pthread_attr_setstacksize(&tattr, MIN_STACKSIZE);
	if(err != 0)
	{
		pthread_attr_destroy(&tattr);
		Com_Printf("pthread_attr_setstacksize(): Thread creation failed with the following error: %s\n", strerror(errno));
		return qfalse;
	}

	err = pthread_create(tid, &tattr, ThreadMain, arg);
	if(err != 0)
	{
		pthread_attr_destroy(&tattr);
		Com_Printf("pthread_create(): Thread creation failed with the following error: %s\n", strerror(errno));
		return qfalse;
	}
	pthread_attr_destroy(&tattr);
	return qtrue;
}

/*
================
Sys_ExitThread
================
*/
void Sys_ExitThread(int code)
{
	pthread_exit(&code);
}

/*
================
Sys_SleepMSec
================
*/
void Sys_SleepMSec(int msec)
{
	struct timespec ts;
	ts.tv_sec = msec / 1000;
	ts.tv_nsec = (msec % 1000) * 1000000;
	nanosleep(&ts, NULL);
}

/*
================
Sys_EnterCriticalSection
================
*/
void Sys_EnterCriticalSection(int section)
{
	pthread_mutex_lock(&crit_sections[section]);
}

/*
================
Sys_TryEnterCriticalSection
================
*/
int Sys_TryEnterCriticalSection(int section)
{
	return pthread_mutex_trylock(&crit_sections[section]);
}

/*
================
Sys_LeaveCriticalSection
================
*/
void Sys_LeaveCriticalSection(int section)
{
	pthread_mutex_unlock(&crit_sections[section]);
}
