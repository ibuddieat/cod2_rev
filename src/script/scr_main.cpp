#include "../qcommon/qcommon.h"
#include "script_public.h"

qboolean Scr_IsInOpcodeMemory(const char *pos)
{
	return pos - scrVarPub.programBuffer < scrCompilePub.programLen;
}

unsigned int Scr_GetCanonicalStringIndex(unsigned int index)
{
	SL_TransferRefToUser(index, 2);

	if ( scrCompilePub.canonicalStrings[index] )
		return scrCompilePub.canonicalStrings[index];

	scrCompilePub.canonicalStrings[index] = ++scrVarPub.canonicalStrCount;
	return scrVarPub.canonicalStrCount;
}

unsigned int SL_GetCanonicalString(const char *str)
{
	unsigned int stringValue;
	unsigned int index;

	stringValue = SL_FindString(str);

	if ( scrCompilePub.canonicalStrings[stringValue] )
		return scrCompilePub.canonicalStrings[stringValue];

	index = SL_GetString_(str, 0);
	return Scr_GetCanonicalStringIndex(index);
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

int Scr_GetFunctionHandle(const char *filename, const char *name)
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

	if ( Scr_GetObjectType(filePosId) != VAR_OBJECT )
		return 0;

	object = FindObject(filePosId);
	index = FindVariable(object, 1u);
	Scr_EvalVariable(&val, index);
	pos = val.u.codePosValue;

	if ( !Scr_IsInOpcodeMemory(val.u.codePosValue) )
		return 0;

	return pos - scrVarPub.programBuffer;
}


unsigned int Scr_LoadScript(const char *filename)
{
	const char *fileName;
	const char *codePos;
	const char *name;
	unsigned int filePosPtr;
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
		filePosPtr = FindVariable(scrCompilePub.scriptsPos, index);

		if ( filePosPtr )
			return FindObject(filePosPtr);
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
			scriptPosIdIndex = GetObjectA(GetVariable(scrCompilePub.scriptsPos, index));
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

void Scr_FreeScripts()
{
	if ( scrCompilePub.script_loading )
	{
		scrCompilePub.script_loading = 0;
		Scr_EndLoadScripts();
	}

	if ( scrAnimPub.animtree_loading )
	{
		scrAnimPub.animtree_loading = 0;
		Scr_EndLoadAnimTrees();
	}

	SL_ShutdownSystem(1u);
	Scr_ShutdownOpcodeLookup();
	scrVarPub.programBuffer = 0;
	scrCompilePub.programLen = 0;
	scrVarPub.endScriptBuffer = 0;
	scrVarPub.checksum = 0;
}