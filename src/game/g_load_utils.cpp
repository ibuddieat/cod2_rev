#include "../qcommon/qcommon.h"
#include "g_shared.h"

/*
=============
VectorToString
This is just a convenience function
for printing vectors
=============
*/
const char *vtos( const vec3_t v )
{
	static int index;
	static char str[8][32];
	char    *s;

	// use an array so that multiple vtos won't collide
	s = str[index];
	index = ( index + 1 ) & 7;

	Com_sprintf( s, 32, "(%i %i %i)", (int)v[0], (int)v[1], (int)v[2] );

	return s;
}

const char *vtosf( const vec3_t v )
{
	static int index;
	static char str[8][64];
	char    *s;

	// use an array so that multiple vtos won't collide
	s = str[index];
	index = ( index + 1 ) & 7;

	Com_sprintf( s, 64, "(%f %f %f)", v[0], v[1], v[2] );

	return s;
}

/*
=============
G_SpawnStringInternal
=============
*/
qboolean G_SpawnStringInternal( const SpawnVar *spawnVar, const char *key, const char *defaultString, const char **out )
{
	assert(spawnVar->spawnVarsValid);
	for ( int i = 0; i < spawnVar->numSpawnVars; i++ )
	{
		if ( !I_stricmp(key, spawnVar->spawnVars[i][0]) )
		{
			*out = spawnVar->spawnVars[i][1];
			return qtrue;
		}
	}
	*out = defaultString;
	return qfalse;
}

/*
=============
G_NewString

Builds a copy of the string, translating \n to real linefeeds
so message texts can be multi-line
=============
*/
unsigned int G_NewString( const char *string )
{
#define MAX_STRING_LEN 16384
	char *new_p;
	char newb[MAX_STRING_LEN];
	int i,l;

	l = strlen( string ) + 1;

	if ( l > MAX_STRING_LEN )
	{
		Com_Error( ERR_DROP, "G_NewString: len = %i > %i", l, MAX_STRING_LEN );
	}

	new_p = newb;

	// turn \n into a real linefeed
	for ( i = 0 ; i < l ; i++ )
	{
		if ( string[i] == '\\' && i < l - 1 )
		{
			i++;
			if ( string[i] == 'n' )
			{
				*new_p++ = '\n';
			}
			else
			{
				*new_p++ = '\\';
			}
		}
		else
		{
			*new_p++ = string[i];
		}
	}

	return SL_GetString(newb, 0);
}

/*
====================
G_ParseSpawnVars

Parses a brace bounded set of key / value pairs out of the
level's entity strings into level.spawnVar[]

This does not actually spawn an entity.
====================
*/
qboolean G_ParseSpawnVars( SpawnVar *spawnVar )
{
	char keyname[MAX_TOKEN_CHARS];
	char com_token[MAX_TOKEN_CHARS];

	spawnVar->spawnVarsValid = 0;
	spawnVar->numSpawnVars = 0;
	spawnVar->numSpawnVarChars = 0;

	// parse the opening brace
	if ( !G_GetEntityToken( com_token, sizeof( com_token ) ) )
	{
		// end of spawn string
		return qfalse;
	}
	if ( com_token[0] != '{' )
	{
		Com_Error( ERR_DROP, "G_ParseSpawnVars: found %s when expecting {",com_token );
	}

	// go through all the key / value pairs
	while ( 1 )
	{
		// parse key
		if ( !G_GetEntityToken( keyname, sizeof( keyname ) ) )
		{
			Com_Error( ERR_DROP, "G_ParseSpawnVars: EOF without closing brace" );
		}

		if ( keyname[0] == '}' )
		{
			break;
		}

		// parse value
		if ( !G_GetEntityToken( com_token, sizeof( com_token ) ) )
		{
			Com_Error( ERR_DROP, "G_ParseSpawnVars: EOF without closing brace" );
		}

		if ( com_token[0] == '}' )
		{
			Com_Error( ERR_DROP, "G_ParseSpawnVars: closing brace without data" );
		}
		if ( spawnVar->numSpawnVars == MAX_SPAWN_VARS )
		{
			Com_Error( ERR_DROP, "G_ParseSpawnVars: MAX_SPAWN_VARS" );
		}
		spawnVar->spawnVars[ spawnVar->numSpawnVars ][0] = G_AddSpawnVarToken( keyname, spawnVar );
		spawnVar->spawnVars[ spawnVar->numSpawnVars ][1] = G_AddSpawnVarToken( com_token, spawnVar );
		spawnVar->numSpawnVars++;
	}

	spawnVar->spawnVarsValid = qtrue;
	return qtrue;
}

/*
====================
G_AddSpawnVarToken
====================
*/
char *G_AddSpawnVarToken( const char *string, SpawnVar *spawnVar )
{
	int l;
	char    *dest;

	l = strlen( string );
	if ( spawnVar->numSpawnVarChars + l + 1 > MAX_SPAWN_VARS_CHARS )
	{
		Com_Error( ERR_DROP, "G_AddSpawnVarToken: MAX_SPAWN_VARS" );
	}

	dest = spawnVar->spawnVarChars + spawnVar->numSpawnVarChars;
	memcpy( dest, string, l + 1 );

	spawnVar->numSpawnVarChars += l + 1;

	return dest;
}

/*
============
G_GetEntityToken
============
*/
qboolean G_GetEntityToken( char *buffer, int bufferSize )
{
	const char* s;

	// Parse the next token from sv.entityParsePoint
	s = Com_Parse(&sv.entityParsePoint);

	// Copy the parsed token into the destination buffer
	Q_strncpyz(buffer, s, bufferSize);

	// If sv.entityParsePoint is NULL and the parsed token is empty,
	// we’ve reached the end (return qfalse), otherwise qtrue
	if (!sv.entityParsePoint && !s[0])
	{
		return qfalse;
	}

	return qtrue;
}
