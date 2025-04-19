#include "../qcommon/qcommon.h"

// can't just use function pointers, or dll linkage can
// mess up when qcommon is included in multiple places
static short ( *_BigShort )( short l ) = NULL;
static short ( *_LittleShort )( short l ) = NULL;
static int ( *_BigLong )( int l ) = NULL;
static int ( *_LittleLong )( int l ) = NULL;
static int64_t ( *_BigLong64 )( int64_t l ) = NULL;
static int64_t ( *_LittleLong64 )( int64_t l ) = NULL;
static float ( *_BigFloat )( float l ) = NULL;
static float ( *_LittleFloat )( float l ) = NULL;

short LittleShort( short l )
{
	return _LittleShort( l );
}

int LittleLong( int l )
{
	return _LittleLong( l );
}

int64_t LittleLong64( int64_t l )
{
	return _LittleLong64( l );
}

float LittleFloat( float l )
{
	return _LittleFloat( l );
}

short BigShort( short l )
{
	return _BigShort( l );
}

int BigLong( int l )
{
	return _BigLong( l );
}

int64_t BigLong64( int64_t l )
{
	return _BigLong64( l );
}

float BigFloat( float l )
{
	return _BigFloat( l );
}

short ShortSwap( short l )
{
	byte b1,b2;

	b1 = l & 255;
	b2 = ( l >> 8 ) & 255;

	return ( b1 << 8 ) + b2;
}

short ShortNoSwap( short l )
{
	return l;
}

int LongSwap( int l )
{
	byte b1,b2,b3,b4;

	b1 = l & 255;
	b2 = ( l >> 8 ) & 255;
	b3 = ( l >> 16 ) & 255;
	b4 = ( l >> 24 ) & 255;

	return ( (int)b1 << 24 ) + ( (int)b2 << 16 ) + ( (int)b3 << 8 ) + b4;
}

int LongNoSwap( int l )
{
	return l;
}

int64_t Long64Swap (int64_t i)
{
	byte b1, b2, b3, b4,b5,b6,b7,b8;

	b1 = i & 255;
	b2 = ( i >> 8 ) & 255;
	b3 = ( i>>16 ) & 255;
	b4 = ( i>>24 ) & 255;
	b5 = ( i>>32 ) & 255;
	b6 = ( i>>40 ) & 255;
	b7 = ( i>>48 ) & 255;
	b8 = ( i>>56 ) & 255;

	return ((int64_t)b1 << 56) + ((int64_t)b2 << 48) + ((int64_t)b3 << 40) +((int64_t)b4 << 32) +((int64_t)b5 << 24) +((int64_t)b6 << 16) + ((int64_t)b7 << 8)+ b8;
}

int64_t Long64NoSwap( int64_t ll )
{
	return ll;
}

float FloatSwap( float f )
{
	union
	{
		float f;
		byte b[4];
	} dat1, dat2;

	dat1.f = f;
	dat2.b[0] = dat1.b[3];
	dat2.b[1] = dat1.b[2];
	dat2.b[2] = dat1.b[1];
	dat2.b[3] = dat1.b[0];
	return dat2.f;
}

float FloatNoSwap( float f )
{
	return f;
}

void Swap_Init( void )
{
	byte swaptest[2] = {1,0};

	if ( *(short *)swaptest == 1 )
	{
		_BigShort = ShortSwap;
		_LittleShort = ShortNoSwap;
		_BigLong = LongSwap;
		_LittleLong = LongNoSwap;
		_BigLong64 = Long64Swap;
		_LittleLong64 = Long64NoSwap;
		_BigFloat = FloatSwap;
		_LittleFloat = FloatNoSwap;
	}
	else
	{
		_BigShort = ShortNoSwap;
		_LittleShort = ShortSwap;
		_BigLong = LongNoSwap;
		_LittleLong = LongSwap;
		_BigLong64 = Long64NoSwap;
		_LittleLong64 = Long64Swap;
		_BigFloat = FloatNoSwap;
		_LittleFloat = FloatSwap;
	}
}

int I_isprint( int c )
{
	if ( c >= 0x20 && c <= 0x7E )
		return ( 1 );
	return ( 0 );
}

int I_islower( int c )
{
	if (c >= 'a' && c <= 'z')
		return ( 1 );
	return ( 0 );
}

int I_isupper( int c )
{
	if (c >= 'A' && c <= 'Z')
		return ( 1 );
	return ( 0 );
}

int I_isalpha( int c )
{
	if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
		return ( 1 );
	return ( 0 );
}

int I_isalphanum( int c )
{
	if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
		return ( 1 );
	return ( 0 );
}

qboolean I_isanumber( const char *s )
{
	char *p;

	if( *s == '\0' )
		return qfalse;

	strtod( s, &p );

	return *p == '\0';
}

qboolean I_isintegral( float f )
{
	return (int)f == f;
}

qboolean I_isprintstring( char* s )
{
	char* a = s;
	while( *a )
	{
		if ( *a < 0x20 || *a > 0x7E ) return 0;
		a++;
	}
	return 1;
}

void I_strncpyz( char *dest, const char *src, int destsize )
{
	strncpy( dest, src, destsize-1 );
	dest[destsize-1] = 0;
}

int I_strnicmp(const char *s0, const char *s1, int n)
{
	int c1;
	int c0;

	assert(s0);
	assert(s1);

	if (!s0 || !s1)
	{
		return s1 - s0;
	}

	do
	{
		c0 = *s0++;
		c1 = *s1++;

		if (!n--)
		{
			return 0;       // strings are equal until end point
		}

		if (c0 != c1)
		{
			if (I_isupper(c0))
			{
				c0 += 32;
			}
			if (I_isupper(c1))
			{
				c1 += 32;
			}
			if (c0 != c1)
			{
				return c0 < c1 ? -1 : 1;
			}
		}
	}
	while (c0);

	return 0;
}

int I_strncmp(const char *s0, const char *s1, int n)
{
	int c1;
	int c0;

	assert(s0);
	assert(s1);

	if (!s0 || !s1)
	{
		return s1 - s0;
	}

	do
	{
		c0 = *s0++;
		c1 = *s1++;

		if (!--n)       // strings are equal until end point
		{
			return 0;
		}
		if (c0 != c1)
		{
			return c0 < c1 ? -1 : 1;
		}
	}
	while (c0);

	return 0;      // strings are equal
}

int I_stricmp(const char *s0, const char *s1)
{
	assert(s0);
	assert(s1);
	return I_strnicmp(s0, s1, 0x7FFFFFFF);
}

char *I_strlwr(char *s)
{
	char* iter;

	for (iter = s; *iter; ++iter)
	{
		if (I_isupper(*iter))
		{
			*iter += 32;
		}
	}
	return s;
}

char *I_strupr(char *s)
{
	char* iter;

	for (iter = s; *iter; ++iter)
	{
		if (I_islower(*iter))
		{
			*iter -= 32;
		}
	}
	return s;
}

void I_strncat( char *dest, int size, const char *src )
{
	int		l1;

	l1 = strlen( dest );

	if ( l1 >= size )
	{
		Com_Error( ERR_FATAL, "I_strncat: already overflowed" );
	}

	I_strncpyz( dest + l1, src, size - l1 );
}

int Com_sprintf(char *dest, size_t size, const char *format, ...)
{
	int result;
	va_list va;

	va_start(va, format);
	result = I_vsnprintf(dest, size, format, va);
	va_end(va);

	dest[size - 1] = '\0';

	return result;
}

void Com_DefaultExtension( char *path, int maxSize, const char *extension )
{
	char oldPath[MAX_QPATH];
	char    *src;

//
// if path doesn't have a .EXT, append extension
// (extension should include the .)
//
	src = path + strlen( path ) - 1;

	while ( *src != '/' && src != path )
	{
		if ( *src == '.' )
		{
			return;                 // it has an extension
		}
		src--;
	}

	I_strncpyz( oldPath, path, sizeof( oldPath ) );
	Com_sprintf( path, maxSize, "%s%s", oldPath, extension );
}

const char *Com_StringContains( const char *str1, const char *str2, int casesensitive)
{
	int len, i, j;

	len = strlen(str1) - strlen(str2);
	for (i = 0; i <= len; i++, str1++)
	{
		for (j = 0; str2[j]; j++)
		{
			if (casesensitive)
			{
				if (str1[j] != str2[j])
				{
					break;
				}
			}
			else
			{
				if (toupper(str1[j]) != toupper(str2[j]))
				{
					break;
				}
			}
		}
		if (!str2[j])
		{
			return str1;
		}
	}
	return NULL;
}

int Com_Filter( const char *filter, const char *name, int casesensitive)
{
	char buf[MAX_TOKEN_CHARS];
	const char *ptr;
	int i, found;

	while(*filter)
	{
		if (*filter == '*')
		{
			filter++;
			for (i = 0; *filter; i++)
			{
				if (*filter == '*' || *filter == '?') break;
				buf[i] = *filter;
				filter++;
			}
			buf[i] = '\0';
			if (strlen(buf))
			{
				ptr = Com_StringContains(name, buf, casesensitive);
				if (!ptr) return qfalse;
				name = ptr + strlen(buf);
			}
		}
		else if (*filter == '?')
		{
			filter++;
			name++;
		}
		else if (*filter == '[' && *(filter+1) == '[')
		{
			filter++;
		}
		else if (*filter == '[')
		{
			filter++;
			found = qfalse;
			while(*filter && !found)
			{
				if (*filter == ']' && *(filter+1) != ']') break;
				if (*(filter+1) == '-' && *(filter+2) && (*(filter+2) != ']' || *(filter+3) == ']'))
				{
					if (casesensitive)
					{
						if (*name >= *filter && *name <= *(filter+2)) found = qtrue;
					}
					else
					{
						if (toupper(*name) >= toupper(*filter) &&
						        toupper(*name) <= toupper(*(filter+2))) found = qtrue;
					}
					filter += 3;
				}
				else
				{
					if (casesensitive)
					{
						if (*filter == *name) found = qtrue;
					}
					else
					{
						if (toupper(*filter) == toupper(*name)) found = qtrue;
					}
					filter++;
				}
			}
			if (!found) return qfalse;
			while(*filter)
			{
				if (*filter == ']' && *(filter+1) != ']') break;
				filter++;
			}
			filter++;
			name++;
		}
		else
		{
			if (casesensitive)
			{
				if (*filter != *name) return qfalse;
			}
			else
			{
				if (toupper(*filter) != toupper(*name)) return qfalse;
			}
			filter++;
			name++;
		}
	}
	return qtrue;
}

int Com_FilterPath(const char *filter, const char *name, int casesensitive)
{
	int i;
	char new_filter[MAX_QPATH];
	char new_name[MAX_QPATH];

	for (i = 0; i < MAX_QPATH-1 && filter[i]; i++)
	{
		if ( filter[i] == '\\' || filter[i] == ':' )
		{
			new_filter[i] = '/';
		}
		else
		{
			new_filter[i] = filter[i];
		}
	}
	new_filter[i] = '\0';
	for (i = 0; i < MAX_QPATH-1 && name[i]; i++)
	{
		if ( name[i] == '\\' || name[i] == ':' )
		{
			new_name[i] = '/';
		}
		else
		{
			new_name[i] = name[i];
		}
	}
	new_name[i] = '\0';
	return Com_Filter(new_filter, new_name, casesensitive);
}

qboolean isFloat(const char* string, int size)
{
	const char* ptr;
	int i;
	qboolean dot = qfalse;
	qboolean sign = qfalse;
	qboolean whitespaceended = qfalse;

	if(size == 0) //If we have given a length compare the whole string
		size = 0x10000;

	for(i = 0, ptr = string; i < size && *ptr != '\0' && *ptr != '\n'; i++, ptr++)
	{

		if(*ptr == ' ')
		{
			if(whitespaceended == qfalse)
				continue;
			else
				return qtrue;
		}
		whitespaceended = qtrue;

		if(*ptr == '-' && sign ==0)
		{
			sign = qtrue;
			continue;
		}
		if(*ptr == '.' && dot == 0)
		{
			dot = qtrue;
			continue;
		}
		if(*ptr < '0' || *ptr > '9') return qfalse;
	}
	return qtrue;
}

qboolean isInteger(const char* string, int size)
{
	const char* ptr;
	int i;
	qboolean sign = qfalse;
	qboolean whitespaceended = qfalse;

	if(size == 0) //If we have given a length compare the whole string
		size = 0x10000;

	for(i = 0, ptr = string; i < size && *ptr != '\0' && *ptr != '\n' && *ptr != '\r'; i++, ptr++)
	{

		if(*ptr == ' ')
		{
			if(whitespaceended == qfalse)
				continue;
			else
				return qtrue;
		}
		whitespaceended = qtrue;

		if(*ptr == '-' && sign ==0)
		{
			sign = qtrue;
			continue;
		}
		if(*ptr < '0' || *ptr > '9')
		{
			return qfalse;
		}
	}
	return qtrue;
}

qboolean isVector(const char* string, int size, int dim)
{
	const char* ptr;
	int i;

	if(size == 0) //If we have given a length compare the whole string
		size = 0x10000;

	for(i = 0, ptr = string; i < size && *ptr != '\0' && *ptr != '\n' && dim > 0; i++, ptr++)
	{

		if(*ptr == ' ')
		{
			continue;
		}
		dim = dim -1;

		if(isFloat(ptr, size -i) == qfalse)
			return qfalse;

		while(*ptr != ' ' && *ptr != '\0' && *ptr != '\n' && i < size)
		{
			ptr++;
			i++;
		}

		if(*ptr != ' ')
		{
			break;
		}

	}
	if(dim != 0)
		return qfalse;

	return qtrue;
}

qboolean strToVect(const char* string, float *vect, int dim)
{
	const char* ptr;
	int i;

	for(ptr = string, i = 0; *ptr != '\0' && *ptr != '\n' && i < dim; ptr++)
	{

		if(*ptr == ' ')
		{
			continue;
		}

		vect[i] = atof(ptr);

		i++;

		while(*ptr != ' ' && *ptr != '\0' && *ptr != '\n')
		{
			ptr++;
		}


		if(*ptr != ' ')
		{
			break;
		}

	}
	if(i != dim)
		return qfalse;

	return qtrue;
}

char ColorIndex(unsigned char c)
{
	if ((int)(c - 48) >= 17)
	{
		return 7;
	}
	else
	{
		return c - 48;
	}
}

const char *Com_GetFilenameSubString(const char *pathname)
{
	const char* last;

	last = pathname;
	while (*pathname)
	{
		if (*pathname == '/' || *pathname == '\\')
		{
			last = pathname + 1;
		}
		++pathname;
	}
	return last;
}

const char *Com_GetExtensionSubString(const char *filename)
{
	const char* substr;

	substr = NULL;
	while (*filename)
	{
		if (*filename == '.')
		{
			substr = filename;
		}
		else if (*filename == '/' || *filename == '\\')
		{
			substr = NULL;
		}
		++filename;
	}
	if (!substr)
	{
		substr = filename;
	}
	return substr;
}

void Com_StripExtension(const char *in, char *out)
{
	const char* extension;

	extension = Com_GetExtensionSubString(in);
	while (in != extension)
	{
		*out++ = *in++;
	}
	*out = 0;
}

char *va(const char *format, ...)
{
	struct va_info_t *info;
	int index;
	va_list va;

	info = (va_info_t*)Sys_GetValue(THREAD_VALUE_VA);
	index = info->index;
	info->index = (info->index + 1) % MAX_VASTRINGS;

	va_start(va, format);
	I_vsnprintf(info->va_string[index], sizeof(info->va_string[0]), format, va);
	va_end(va);

	info->va_string[index][MAX_STRING_CHARS - 1] = 0;

	return info->va_string[index];
}

const char *Info_ValueForKey( const char *s, const char *key )
{
	char	pkey[BIG_INFO_KEY];
	static	char value[2][BIG_INFO_VALUE];	// use two buffers so compares
	// work without stomping on each other
	static	int	valueindex = 0;
	char	*o;

	if ( !s || !key )
	{
		return "";
	}

	if ( strlen( s ) >= BIG_INFO_STRING )
	{
		Com_Error( ERR_DROP, "Info_ValueForKey: oversize infostring" );
	}

	valueindex ^= 1;
	if (*s == '\\')
		s++;
	while (1)
	{
		o = pkey;
		while (*s != '\\')
		{
			if (!*s)
				return "";
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value[valueindex];

		while (*s != '\\' && *s)
		{
			*o++ = *s++;
		}
		*o = 0;

		if (!I_stricmp (key, pkey) )
			return value[valueindex];

		if (!*s)
			break;
		s++;
	}

	return "";
}

void Info_NextPair( const char **head, char *key, char *value )
{
	char	*o;
	const char	*s;

	s = *head;

	if ( *s == '\\' )
	{
		s++;
	}
	key[0] = 0;
	value[0] = 0;

	o = key;
	while ( *s != '\\' )
	{
		if ( !*s )
		{
			*o = 0;
			*head = s;
			return;
		}
		*o++ = *s++;
	}
	*o = 0;
	s++;

	o = value;
	while ( *s != '\\' && *s )
	{
		*o++ = *s++;
	}
	*o = 0;

	*head = s;
}

void Info_RemoveKey( char *s, const char *key )
{
	char	*start;
	char	pkey[MAX_INFO_KEY];
	char	value[MAX_INFO_VALUE];
	char	*o;

	if ( strlen( s ) >= MAX_INFO_STRING )
	{
		Com_Error( ERR_DROP, "Info_RemoveKey: oversize infostring" );
	}

	if (strchr (key, '\\'))
	{
		return;
	}

	while (1)
	{
		start = s;
		if (*s == '\\')
			s++;
		o = pkey;
		while (*s != '\\')
		{
			if (!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value;
		while (*s != '\\' && *s)
		{
			if (!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;

		if (!strcmp (key, pkey) )
		{
			strcpy (start, s);	// remove this part
			return;
		}

		if (!*s)
			return;
	}

}

void Info_RemoveKey_Big( char *s, const char *key )
{
	char	*start;
	char	pkey[BIG_INFO_KEY];
	char	value[BIG_INFO_VALUE];
	char	*o;

	if ( strlen( s ) >= BIG_INFO_STRING )
	{
		Com_Error( ERR_DROP, "Info_RemoveKey_Big: oversize infostring" );
	}

	if (strchr (key, '\\'))
	{
		return;
	}

	while (1)
	{
		start = s;
		if (*s == '\\')
			s++;
		o = pkey;
		while (*s != '\\')
		{
			if (!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value;
		while (*s != '\\' && *s)
		{
			if (!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;

		if (!strcmp (key, pkey) )
		{
			strcpy (start, s);	// remove this part
			return;
		}

		if (!*s)
			return;
	}

}

void Info_SetValueForKey(char *s, const char *key, const char *value)
{
	int j;
	char c;
	char cleanValue[MAX_INFO_STRING];
	int len;
	char newi[MAX_INFO_STRING];
	int i;

	assert(value);
	if (strlen(s) >= MAX_INFO_STRING)
	{
		Com_Printf("Info_SetValueForKey: oversize infostring");
		return;
	}

	j = 0;
	for (i = 0; i < MAX_INFO_STRING - 1; ++i)
	{
		c = value[i];
		if (!c)
		{
			break;
		}
		if (c != '\\' && c != ';' && c != '\"')
		{
			assert(j < MAX_INFO_STRING);
			cleanValue[j++] = c;
		}
	}

	assert(j < MAX_INFO_STRING);
	cleanValue[j] = 0;
	if (strchr(key, '\\'))
	{
		Com_Printf("Can\'t use keys with a \\\nkey: '%s'\nvalue: '%s'", key, value);
		return;
	}

	if (strchr(key, ';'))
	{
		Com_Printf("Can\'t use keys with a semicolon\nkey: '%s'\nvalue: '%s'", key, value);
		return;
	}

	if (strchr(key, '\"'))
	{
		Com_Printf("Can\'t use keys with a \"\nkey: '%s'\nvalue: '%s'", key, value);
		return;
	}

	Info_RemoveKey(s, key);
	if (!cleanValue[0])
	{
		return;
	}

	len = Com_sprintf(newi, sizeof(newi), "\\%s\\%s", key, cleanValue);
	if (len <= 0)
	{
		Com_Printf("Info buffer length exceeded, not including key/value pair inresponse\n");
		return;
	}

	if (strlen(newi) + strlen(s) > MAX_INFO_STRING)
	{
		Com_Printf("Info string length exceeded\nkey: '%s'\nvalue: '%s'\nInfo string:\n%s\n", key, value, s);
		return;
	}

	strcat(s, newi);
}

void Info_SetValueForKey_Big(char *s, const char *key, const char *value)
{
	int j;
	char c;
	char cleanValue[BIG_INFO_STRING];
	int len;
	char newi[BIG_INFO_STRING];
	int i;

	assert(value);
	if (strlen(s) >= BIG_INFO_STRING)
	{
		Com_Printf("Info_SetValueForKey: oversize infostring");
		return;
	}

	j = 0;
	for (i = 0; i < BIG_INFO_STRING - 1; ++i)
	{
		c = value[i];
		if (!c)
		{
			break;
		}
		if (c != 92 && c != 59 && c != 34)
		{
			assert(j < BIG_INFO_STRING);
			cleanValue[j++] = c;
		}
	}

	assert(j < BIG_INFO_STRING);
	cleanValue[j] = 0;

	if (strchr(key, '\\'))
	{
		Com_Printf("Can\'t use keys with a \\\nkey: '%s'\nvalue: '%s'", key, value);
		return;
	}

	if (strchr(key, ';'))
	{
		Com_Printf("Can\'t use keys with a semicolon\nkey: '%s'\nvalue: '%s'", key, value);
		return;
	}

	if (strchr(key, '\"'))
	{
		Com_Printf("Can\'t use keys with a \"\nkey: '%s'\nvalue: '%s'", key, value);
		return;
	}

	Info_RemoveKey_Big(s, key);
	if (!cleanValue[0])
	{
		return;
	}

	len = Com_sprintf(newi, sizeof(newi), "\\%s\\%s", key, cleanValue);
	if (len <= 0)
	{
		Com_Printf("Info buffer length exceeded, not including key/value pair in response\n");
		return;
	}

	if (strlen(newi) + strlen(s) > BIG_INFO_STRING)
	{
		Com_Printf("Big info string length exceeded\nkey: '%s'\nvalue: '%s'\nInfo string:\n%s\n",
		           key, value, s);
		return;
	}

	strcat(s, newi);
}

char *I_CleanStr(char *string)
{
	char* d;
	char c;
	char* s;
	int keep_cleaning;

	do
	{
		s = string;
		d = string;
		keep_cleaning = 0;

		while (1)
		{
			c = *s;
			if (!*s)
			{
				break;
			}
			if (s && *s == '^' && s[1] && s[1] != '^' && s[1] >= '0' && s[1] <= '@')
			{
				++s;
				keep_cleaning = 1;
			}
			else if (c >= 0x20 && c != 0x7F)
			{
				*d++ = c;
			}
			++s;
		}
		*d = '\0';
	}
	while (keep_cleaning);

	return string;
}

char I_CleanChar(char character)
{
	if ((unsigned char)character == 146)
	{
		return 39;
	}
	else
	{
		return character;
	}
}

bool Com_ValidXModelName(const char *name)
{
	return !I_strnicmp(name, "xmodel", 6) && name[6] == 47;
}

qboolean Info_Validate( const char *s )
{
	if ( strchr( s, '\"' ) )
	{
		return qfalse;
	}

	if ( strchr( s, ';' ) )
	{
		return qfalse;
	}

	return qtrue;
}

/*
==================
Com_BitCheck

  Allows bit-wise checks on arrays with more than one item (> 32 bits)
==================
*/
bool Com_BitCheck( const int array[], int bitNum )
{
	return (array[bitNum >> 5] >> (bitNum & 31)) & 1;
}

/*
==================
Com_BitSet

  Allows bit-wise SETS on arrays with more than one item (> 32 bits)
==================
*/
void Com_BitSet( int array[], int bitNum )
{
	array[bitNum >> 5] |= 1 << (bitNum & 31);
}

/*
==================
Com_BitClear

  Allows bit-wise CLEAR on arrays with more than one item (> 32 bits)
==================
*/
void Com_BitClear( int array[], int bitNum )
{
	array[bitNum >> 5] &= ~(1 << (bitNum & 31));
}

float GetLeanFraction(const float fFrac)
{
	return (2.0f - I_fabs(fFrac)) * fFrac;
}

float UnGetLeanFraction(const float fFrac)
{
	return 1.0f - sqrtf(1.0f - fFrac);
}

void AddLeanToPosition(float *position, float fViewYaw, float fLeanFrac, float fViewRoll, float fLeanDist)
{
	float fLean;
	vec3_t vRight;
	vec3_t vAng;

	if ( fLeanFrac != 0.0 )
	{
		fLean = GetLeanFraction(fLeanFrac);
		vAng[0] = 0.0;
		vAng[1] = fViewYaw;
		vAng[2] = fLean * fViewRoll;
		AngleVectors(vAng, 0, vRight, 0);
		fLean = fLean * fLeanDist;
		position[0] = (float)(fLean * vRight[0]) + position[0];
		position[1] = (float)(fLean * vRight[1]) + position[1];
		position[2] = (float)(fLean * vRight[2]) + position[2];
	}
}

void SetConfigString(char **ppszConfigString, const char *pszKeyValue)
{
	size_t len;
	char *dest;

	if ( *pszKeyValue )
	{
		len = strlen(pszKeyValue);
		dest = (char *)Hunk_AllocLowAlignInternal(len + 1, 1);
		strcpy(dest, pszKeyValue);
		*ppszConfigString = dest;
	}
	else
	{
		*ppszConfigString = "";
	}
}

void SetConfigString2(unsigned char *pMember, const char *pszKeyValue)
{
	SetConfigString((char **)pMember, pszKeyValue);
}

qboolean ParseConfigStringToStruct(unsigned char *pStruct, const cspField_t *pFieldList, int iNumFields, const char *pszBuffer, int iMaxFieldTypes, int (*parseSpecialFieldType)(unsigned char *, const char *, const int), void (*parseStrCpy)(unsigned char *, const char *))
{
	const char *pszKeyValue;
	int iField;

	for ( iField = 0; iField < iNumFields; ++iField )
	{
		pszKeyValue = Info_ValueForKey(pszBuffer, pFieldList->szName);

		if ( *pszKeyValue )
		{
			if ( pFieldList->iFieldType >= CSPFT_NUM_BASE_FIELD_TYPES )
			{
				if ( iMaxFieldTypes <= 0 || pFieldList->iFieldType >= iMaxFieldTypes )
					Com_Error(ERR_DROP, "Bad field type %i", pFieldList->iFieldType);

				if ( !parseSpecialFieldType(pStruct, pszKeyValue, pFieldList->iFieldType) )
					return 0;
			}
			else
			{
				switch ( pFieldList->iFieldType )
				{
				case CSPFT_STRING:
					parseStrCpy(&pStruct[pFieldList->iOffset], pszKeyValue);
					break;

				case CSPFT_STRING_MAX_STRING_CHARS:
					I_strncpyz((char *)&pStruct[pFieldList->iOffset], pszKeyValue, MAX_STRING_CHARS);
					break;

				case CSPFT_STRING_MAX_QPATH:
					I_strncpyz((char *)&pStruct[pFieldList->iOffset], pszKeyValue, MAX_QPATH);
					break;

				case CSPFT_STRING_MAX_OSPATH:
					I_strncpyz((char *)&pStruct[pFieldList->iOffset], pszKeyValue, MAX_OSPATH);
					break;

				case CSPFT_INT:
					*(int *)&pStruct[pFieldList->iOffset] = atoi(pszKeyValue);
					break;

				case CSPFT_QBOOLEAN:
					*(qboolean *)&pStruct[pFieldList->iOffset] = atoi(pszKeyValue) != 0;
					break;

				case CSPFT_FLOAT:
					*(float *)&pStruct[pFieldList->iOffset] = atof(pszKeyValue);
					break;

				case CSPFT_MILLISECONDS:
					*(int *)&pStruct[pFieldList->iOffset] = (int)(atof(pszKeyValue) * 1000.0);
					break;

				default:
					break;
				}
			}
		}

		++pFieldList;
	}

	return iField == iNumFields;
}

int I_DrawStrlen(const char *str)
{
	const char* s;
	int count;

	s = str;
	count = 0;

	while (*s)
	{
		if (s && *s == '^' && s[1] && s[1] != '^' && s[1] >= '0' && s[1] <= '@')
		{
			s += 2;
		}
		else
		{
			++count;
			++s;
		}
	}

	return count;
}

va_info_t va_info[NUMTHREADS];
jmp_buf g_com_error[NUMTHREADS];
TraceThreadInfo g_traceThreadInfo[NUMTHREADS];

void Com_InitThreadData(int threadContext)
{
	Sys_SetValue(THREAD_VALUE_VA, &va_info[threadContext]);
	Sys_SetValue(THREAD_VALUE_COM_ERROR, &g_com_error[threadContext]);
	Sys_SetValue(THREAD_VALUE_TRACE, &g_traceThreadInfo[threadContext]);
}

void Sys_UnimplementedFunctionInternal(const char *function)
{
	Com_Printf("FUNCTION NOT IMPLEMENTED!!! (%s)\n", function);
}

snd_alias_list_t* Com_FindSoundAlias(const char *name)
{
	return NULL; // Not supported
}
