#pragma once

#include <float.h>
#include <limits.h>

#include "com_math.h"

#define	MAX_DVARS 1280
#define FILE_HASH_SIZE 256

enum DvarSetSource
{
	DVAR_SOURCE_INTERNAL = 0x0,
	DVAR_SOURCE_EXTERNAL = 0x1,
	DVAR_SOURCE_SCRIPT = 0x2,
	DVAR_SOURCE_DEVGUI = 0x3,
};

enum DvarType : char
{
	DVAR_TYPE_BOOL = 0x00,
	DVAR_TYPE_FLOAT = 0x01,
	DVAR_TYPE_VEC2 = 0x02,
	DVAR_TYPE_VEC3 = 0x03,
	DVAR_TYPE_VEC4 = 0x04,
	DVAR_TYPE_INT = 0x05,
	DVAR_TYPE_ENUM = 0x06,
	DVAR_TYPE_STRING = 0x07,
	DVAR_TYPE_COLOR = 0x08
};

union DvarLimits
{
	struct
	{
		int stringCount;
		const char **strings;
	} enumeration;
	struct
	{
		int min;
		int max;
	} integer;
	struct
	{
		float min;
		float max;
	} decimal;
};

typedef struct
{
	union
	{
		bool boolean;
		int integer;
		float decimal;
		vec2_t vec2;
		vec3_t vec3;
		vec4_t vec4;
		const char *string;
		unsigned char color[4];
	};
} DvarValue;

typedef struct dvar_s
{
	const char *name;
	unsigned short flags;
	DvarType type;
	bool modified;
	DvarValue current;
	DvarValue latched;
	DvarValue reset;
	DvarLimits domain;
	dvar_s *next;
	dvar_s *hashNext;
} dvar_t;

#define DVAR_NOFLAG				0				// 0x0000
#define DVAR_ARCHIVE            (1 << 0)        // 0x0001
#define DVAR_USERINFO           (1 << 1)        // 0x0002
#define DVAR_SERVERINFO         (1 << 2)        // 0x0004
#define DVAR_SYSTEMINFO         (1 << 3)        // 0x0008
#define DVAR_INIT               (1 << 4)        // 0x0010
#define DVAR_LATCH              (1 << 5)        // 0x0020
#define DVAR_ROM                (1 << 6)        // 0x0040
#define DVAR_CHEAT              (1 << 7)        // 0x0080
#define DVAR_DEVELOPER          (1 << 8)        // 0x0100
#define DVAR_SAVED              (1 << 9)        // 0x0200
#define DVAR_NORESTART          (1 << 10)       // 0x0400
#define DVAR_CHANGEABLE_RESET   (1 << 12)       // 0x1000
#define DVAR_EXTERNAL           (1 << 14)       // 0x4000
#define DVAR_AUTOEXEC           (1 << 15)       // 0x8000
#define DVAR_INVALID_ENUM_INDEX -1337

extern dvar_t *sortedDvars;
extern int dvarCount;

dvar_t *Dvar_RegisterBool(const char *dvarName, bool value, unsigned int flags);
dvar_t *Dvar_RegisterInt(const char *dvarName, int value, int min, int max, unsigned int flags);
dvar_t *Dvar_RegisterFloat(const char *dvarName, float value, float min, float max, unsigned int flags);
dvar_t *Dvar_RegisterString(const char *dvarName, const char *value, unsigned int flags);
dvar_t *Dvar_RegisterEnum(const char *dvarName, const char **valueList, int defaultIndex, unsigned int flags);
dvar_t *Dvar_RegisterVec2(const char *dvarName, float x, float y, float min, float max, unsigned int flags);
dvar_t *Dvar_RegisterVec3(const char *dvarName, float x, float y, float z, float min, float max, unsigned int flags);
dvar_t *Dvar_RegisterVec4(const char *dvarName, float x, float y, float z, float w, float min, float max, unsigned int flags);
dvar_t *Dvar_RegisterColor(const char *dvarName, float r, float g, float b, float a, unsigned int flags);
void Dvar_SetBoolFromSource(dvar_t *dvar, bool value, DvarSetSource source);
void Dvar_SetFloatFromSource(dvar_t *dvar, float value, DvarSetSource source);
void Dvar_SetIntFromSource(dvar_t *dvar, int value, DvarSetSource source);
void Dvar_SetFromStringFromSource(dvar_t *dvar, const char *string, DvarSetSource source);
bool Dvar_StringToBool(const char *string);
int Dvar_StringToInt(const char *string);
float Dvar_StringToFloat(const char *string);
void Dvar_StringToVec2(const char *string, vec2_t vector);
void Dvar_StringToVec3(const char *string, vec3_t vector);
void Dvar_StringToVec4(const char *string, vec4_t vector);
void Dvar_StringToColor(const char *string, unsigned char *color);
int Dvar_StringToEnum(const DvarLimits *domain, const char *string);
qboolean Dvar_ValuesEqual(DvarType type, DvarValue val0, DvarValue val1);
DvarValue Dvar_StringToValue(const DvarType type, const DvarLimits domain, const char *string);
void Dvar_SetLatchedValue(dvar_t *dvar, DvarValue value);
void Dvar_SetVariant(dvar_t *dvar, DvarValue value, DvarSetSource source);
void Dvar_WeakCopyString(const char *string, DvarValue *value);
void Dvar_CopyString(const char *string, DvarValue *value);
void Dvar_AssignCurrentStringValue(dvar_t *dvar, DvarValue *dest, const char *string);
qboolean Dvar_ShouldFreeCurrentString(dvar_t *dvar);
qboolean Dvar_ShouldFreeResetString(dvar_t *dvar);
qboolean Dvar_ShouldFreeLatchedString(dvar_t *dvar);
void Dvar_FreeString(DvarValue *value);
void Dvar_UpdateResetValue(dvar_t *dvar, DvarValue value);
void Dvar_PerformUnregistration(dvar_t *dvar);
int Dvar_GetCombinedString(char *dest, int arg);
const char *Dvar_IndexStringToEnumString(const dvar_t *dvar, const char *indexString);
void Dvar_SetCommand(const char *dvarName, const char *string);
qboolean Dvar_HasLatchedValue(dvar_t *var);
qboolean Dvar_IsValidName(const char *dvarName);
void Dvar_Reset(dvar_t *dvar, DvarSetSource setSource);
void Dvar_AddFlags(dvar_t* var, unsigned short flags);
void Dvar_ClearFlags(dvar_t* var, unsigned short flags);
void Dvar_SetFromStringByName(const char *dvarName, const char *string);
const char *Dvar_DisplayableLatchedValue(dvar_t *var);
const char *Dvar_DisplayableResetValue(dvar_t *var);
const char *Dvar_DisplayableValue(dvar_t *var);
void Dvar_PrintDomain(DvarType type, DvarLimits domain);
dvar_t* Dvar_FindVar(const char* dvarName);
void Dvar_SetBool(dvar_t *dvar, bool value);
void Dvar_SetInt(dvar_t *dvar, int value);
void Dvar_SetFloat(dvar_t *dvar, float value);
void Dvar_SetString(dvar_t *dvar, const char *value);
void Dvar_SetIntByName(const char *dvarName, int value);
qboolean Dvar_GetBool(const char *dvarName);
int Dvar_GetInt(const char *dvarName);
float Dvar_GetFloat(const char *dvarName);
const char* Dvar_GetString(const char *dvarName);
const char* Dvar_GetVariantString(const char *dvarName);
void Dvar_ForEach( void(*callback)(const char *s) );
void Dvar_SetInAutoExec(qboolean inAutoExec);
void Dvar_WriteVariables(fileHandle_t f);
void Dvar_WriteDefaults(fileHandle_t f);
void Dvar_ClearModified(dvar_t* var);
char *Dvar_InfoString(unsigned short bit);
char *Dvar_InfoString_Big(unsigned short bit);
void Dvar_Set_f(void);
void Dvar_SetA_f(void);
void Dvar_SetS_f(void);
void Dvar_SetU_f(void);
qboolean Dvar_Command();
void Dvar_AddCommands();
void Dvar_Shutdown();
int Dvar_IsSystemActive();
void Dvar_Init();