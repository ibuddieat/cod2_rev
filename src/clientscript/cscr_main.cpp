#include "../qcommon/qcommon.h"
#include "clientscript_public.h"

#ifdef TESTING_LIBRARY
#define scrVarPub (*((scrVarPub_t*)( 0x08394000 )))
#else
extern scrVarPub_t scrVarPub;
#endif

#ifdef TESTING_LIBRARY
#define scrAnimPub (*((scrAnimPub_t*)( 0x08202440 )))
#else
extern scrAnimPub_t scrAnimPub;
#endif

#ifdef TESTING_LIBRARY
#define scrParserPub (*((scrParserPub_t*)( 0x08283ED4 )))
#else
extern scrParserPub_t scrParserPub;
#endif

qboolean Scr_IsInOpcodeMemory(const char *pos)
{
	return pos - scrVarPub.programBuffer < scrCompilePub.programLen;
}

void Scr_GetGenericField(const byte *data, int fieldtype, int offset)
{
	const char *model;
	unsigned short object;
	vec3_t vector;
	unsigned short stringValue;

	switch ( fieldtype )
	{
	case F_INT:
		Scr_AddInt(*(int *)&data[offset]);
		break;

	case F_FLOAT:
		Scr_AddFloat(*(float *)&data[offset]);
		break;

	case F_LSTRING:
		Scr_AddString((const char *)&data[offset]);
		break;

	case F_STRING:
		stringValue = *(unsigned short *)&data[offset];
		if ( stringValue )
			Scr_AddConstString(stringValue);
		break;

	case F_VECTOR:
		Scr_AddVector((const float *)&data[offset]);
		break;

	case F_ENTITY:
		if ( *(gentity_t **)&data[offset] )
			Scr_AddEntity(*(gentity_t **)&data[offset]);
		break;

	case F_VECTORHACK:
		vector[0] = 0.0;
		vector[1] = *(float *)&data[offset];
		vector[2] = 0.0;
		Scr_AddVector(vector);
		break;

	case F_OBJECT:
		object = *(unsigned short *)&data[offset];
		if ( object )
			Scr_AddObject(object);
		break;

	case F_MODEL:
		model = G_ModelName(data[offset]);
		Scr_AddString(model);
		break;

	default:
		break;
	}
}

void Scr_SetGenericField(byte *data, int fieldtype, int offset)
{
	unsigned int stringValue;
	vec3_t vector;

	switch ( fieldtype )
	{
	case F_INT:
		*(int *)&data[offset] = Scr_GetInt(0);
		break;

	case F_FLOAT:
		*(float *)&data[offset] = Scr_GetFloat(0);
		break;

	case F_STRING:
		stringValue = Scr_GetConstStringIncludeNull(0);
		Scr_SetString((unsigned short *)&data[offset], stringValue);
		break;

	case F_VECTOR:
		Scr_GetVector(0, vector);
		*(vec_t *)&data[offset] = vector[0];
		*(vec_t *)&data[offset + 4] = vector[1];
		*(vec_t *)&data[offset + 8] = vector[2];
		break;

	case F_ENTITY:
		*(gentity_t **)&data[offset] = Scr_GetEntity(0);
		break;

	case F_VECTORHACK:
		Scr_GetVector(0, vector);
		*(vec_t *)&data[offset] = vector[1];
		break;

	default:
		break;
	}
}

bool Scr_IsIdentifier(const char *token)
{
	while ( *token )
	{
		if ( !isalnum(*token) && *token != '_' )
			return false;

		++token;
	}

	return true;
}

int Scr_GetFunctionHandleInternal(const char *filename, const char *name)
{
	VariableValue val;
	unsigned int index;
	unsigned int object;
	const char *pos;
	unsigned int fileName;
	unsigned int str;
	unsigned int posId;
	unsigned int filePosId;
	unsigned int id;

	fileName = Scr_CreateCanonicalFilename(filename);
	id = FindVariable(scrCompilePub.scriptsPos, fileName);
	SL_RemoveRefToString(fileName);

	if ( !id )
		return 0;

	posId = FindObject(id);
	str = SL_FindLowercaseString(name);

	if ( !str )
		return 0;

	filePosId = FindVariable(posId, str);

	if ( !filePosId )
		return 0;

	if ( GetVarType(filePosId) != VAR_OBJECT )
		return 0;

	object = FindObject(filePosId);
	index = FindVariable(object, 1u);
	Scr_EvalVariable(&val, index);
	pos = val.u.codePosValue;

	if ( !Scr_IsInOpcodeMemory(val.u.codePosValue) )
		return 0;

	return pos - scrVarPub.programBuffer;
}

int Scr_GetFunctionHandle(const char *filename, const char *name, qboolean errorIfMissing)
{
	int handle;

	if ( !Scr_LoadScript(filename) && errorIfMissing )
		Com_Error(ERR_DROP, "Could not find script '%s'", filename);

	handle = Scr_GetFunctionHandleInternal(filename, name);

	if ( !handle && errorIfMissing )
		Com_Error(ERR_DROP, "Could not find label '%s' in script '%s'", name, filename);

	return handle;
}

unsigned int Scr_LoadScript(const char *filename)
{
	const char *fileName;
	const char *codePos;
	const char *name;
	unsigned int var;
	unsigned int id;
	unsigned int scriptPosIdIndex;
	unsigned int loadedScriptsIdIndex;
	const char *sourceBuf;
	const char *scriptfilename;
	char scriptName[64];
	unsigned int index;
	sval_u parseData;
	char *sourceBuffer;

	index = Scr_CreateCanonicalFilename(filename);

	if ( FindVariable(scrCompilePub.loadedscripts, index) )
	{
		SL_RemoveRefToString(index);
		id = FindVariable(scrCompilePub.scriptsPos, index);
		if ( id )
			return FindObject(id);
		else
			return 0;
	}
	else
	{
		loadedScriptsIdIndex = GetNewVariable(scrCompilePub.loadedscripts, index);
		SL_RemoveRefToString(index);
		fileName = SL_ConvertToString(index);
		Com_sprintf(scriptName, sizeof(scriptName), "%s.gsc", fileName);
		sourceBuf = scrParserPub.sourceBuf;
		codePos = (const char *)TempMalloc(0);
		name = SL_ConvertToString(index);
		sourceBuffer = Scr_AddSourceBuffer(name, scriptName, codePos, 1);

		if ( sourceBuffer )
		{
			scrAnimPub.animTreeNames = 0;
			scrCompilePub.far_function_count = 0;
			scriptfilename = scrParserPub.scriptfilename;
			scrParserPub.scriptfilename = scriptName;
			scrCompilePub.in_ptr = "+";
			scrCompilePub.parseBuf = sourceBuffer;
			ScriptParse(&parseData, 0);
			var = GetVariable(scrCompilePub.scriptsPos, index);
			scriptPosIdIndex = GetObjectA(var);
			ScriptCompile(parseData, scriptPosIdIndex, loadedScriptsIdIndex);
			scrParserPub.scriptfilename = scriptfilename;
			scrParserPub.sourceBuf = sourceBuf;
			return scriptPosIdIndex;
		}
		else
		{
			return 0;
		}
	}
}

void Scr_EndLoadScripts()
{
	Scr_ClearStrings();
	SL_ShutdownSystem(2u);
	scrCompilePub.script_loading = 0;
	ClearObject(scrCompilePub.loadedscripts);
	RemoveRefToObject(scrCompilePub.loadedscripts);
	scrCompilePub.loadedscripts = 0;
	ClearObject(scrCompilePub.scriptsPos);
	RemoveRefToObject(scrCompilePub.scriptsPos);
	scrCompilePub.scriptsPos = 0;
	ClearObject(scrCompilePub.builtinFunc);
	RemoveRefToObject(scrCompilePub.builtinFunc);
	scrCompilePub.builtinFunc = 0;
	ClearObject(scrCompilePub.builtinMeth);
	RemoveRefToObject(scrCompilePub.builtinMeth);
	scrCompilePub.builtinMeth = 0;
}

void Scr_PostCompileScripts()
{
	Hunk_ConvertTempToPermLowInternal();
}

void Scr_BeginLoadScripts()
{
	scrCompilePub.script_loading = 1;
	Scr_InitOpcodeLookup();
	scrCompilePub.loadedscripts = Scr_AllocArray();
	scrCompilePub.scriptsPos = Scr_AllocArray();
	scrCompilePub.builtinFunc = Scr_AllocArray();
	scrCompilePub.builtinMeth = Scr_AllocArray();
	scrVarPub.programBuffer = (const char *)Hunk_AllocLowInternal(0);
	scrCompilePub.programLen = 0;
	scrVarPub.endScriptBuffer = 0;
	Scr_AllocStrings();
	scrVarPub.fieldBuffer = 0;
	scrCompilePub.value_count = 0;
	Scr_ClearErrorMessage();
	scrCompilePub.func_table_size = 0;
	Scr_AllocAnims(1);
	TempMemoryReset();
}

void Scr_PrecacheAnimTrees(void *(*Alloc)(int), int user)
{
	int index;

	for ( index = 1; index <= scrAnimPub.xanim_num[user]; ++index )
		Scr_LoadAnimTreeAtIndex(index, Alloc, user);
}

void Scr_EndLoadAnimTrees()
{
	ClearObject(scrAnimPub.animtrees);
	RemoveRefToObject(scrAnimPub.animtrees);
	scrAnimPub.animtrees = 0;

	if ( scrAnimPub.animtree_node )
		RemoveRefToObject(scrAnimPub.animtree_node);

	SL_ShutdownSystem(2u);
	scrVarPub.endScriptBuffer = (const char *)Hunk_AllocLowInternal(0);
	scrAnimPub.animtree_loading = 0;
}