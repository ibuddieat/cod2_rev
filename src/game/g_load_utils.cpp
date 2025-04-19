#include "../qcommon/qcommon.h"
#include "g_shared.h"

unsigned int G_NewString(const char *string)
{
	size_t len;
	signed int i;
	char *chr;
	char str[16384];

	len = strlen(string) + 1;

	if ( len > 16384 )
		Com_Error(ERR_DROP, "G_NewString: len = %i > %i", len, 16384);

	chr = str;

	for ( i = 0; i < (int)len; ++i )
	{
		if ( string[i] == 92 && i < (int)(len - 1) )
		{
			if ( string[++i] == 110 )
				*chr = 10;
			else
				*chr = 92;

			++chr;
		}
		else
		{
			*chr++ = string[i];
		}
	}

	return SL_GetString(str, 0);
}

int G_SpawnStringInternal(const SpawnVar *spawnVar, const char *key, const char *defaultString, const char **out)
{
	int i;

	for ( i = 0; i < spawnVar->numSpawnVars; ++i )
	{
		if ( !strcmp(key, spawnVar->spawnVars[i].key) )
		{
			*out = spawnVar->spawnVars[i].value;
			return 1;
		}
	}

	*out = defaultString;
	return 0;
}



int G_GetEntityToken(char *buffer, int bufferSize)
{
	char *token;

	token = Com_Parse(&sv.entityParsePoint);
	I_strncpyz(buffer, token, bufferSize);

	if ( sv.entityParsePoint || *token )
		return 1;

	return 0;
}

char* G_AddSpawnVarToken(char *string, SpawnVar *spawnVar)
{
	char *dest;
	int len;

	len = strlen(string);

	if ( spawnVar->numSpawnVarChars + len + 1 > 2048 )
		Com_Error(ERR_DROP, "G_AddSpawnVarToken: MAX_SPAWN_VARS");

	dest = &spawnVar->spawnVarChars[spawnVar->numSpawnVarChars];
	memcpy(dest, string, len + 1);
	spawnVar->numSpawnVarChars += len + 1;

	return dest;
}

int G_ParseSpawnVars(SpawnVar *spawnVar)
{
	char com_token[1024];
	char key_name[1024];

	spawnVar->spawnVarsValid = 0;
	spawnVar->numSpawnVars = 0;
	spawnVar->numSpawnVarChars = 0;

	if ( !G_GetEntityToken(com_token, 1024) )
		return 0;

	if ( com_token[0] != 123 )
		Com_Error(ERR_DROP, "G_ParseSpawnVars: found %s when expecting {", com_token);

	while ( 1 )
	{
		if ( !G_GetEntityToken(key_name, 1024) )
			Com_Error(ERR_DROP, "G_ParseSpawnVars: EOF without closing brace");

		if ( key_name[0] == 125 )
			break;

		if ( !G_GetEntityToken(com_token, 1024) )
			Com_Error(ERR_DROP, "G_ParseSpawnVars: EOF without closing brace");

		if ( com_token[0] == 125 )
			Com_Error(ERR_DROP, "G_ParseSpawnVars: closing brace without data");

		if ( spawnVar->numSpawnVars == 64 )
			Com_Error(ERR_DROP, "G_ParseSpawnVars: MAX_SPAWN_VARS");

		spawnVar->spawnVars[spawnVar->numSpawnVars].key = G_AddSpawnVarToken(key_name, spawnVar);
		spawnVar->spawnVars[spawnVar->numSpawnVars].value = G_AddSpawnVarToken(com_token, spawnVar);
		++spawnVar->numSpawnVars;
	}

	spawnVar->spawnVarsValid = 1;
	return 1;
}


/*
=============
VectorToString
This is just a convenience function
for printing vectors
=============
*/
const char    *vtos( const vec3_t v )
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

const char    *vtosf( const vec3_t v )
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