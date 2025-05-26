#include "../qcommon/qcommon.h"
#include "script_public.h"

scrVmGlob_t scrVmGlob;
scrVmPub_t scrVmPub;

int g_script_error_level;
jmp_buf g_script_error[MAX_VM_STACK_DEPTH + 1];

/*
==============
Scr_Allign2
==============
*/
const char* Scr_Allign2( char const *pos )
{
	UNIMPLEMENTED(__FUNCTION__);
	return NULL;
}

/*
==============
Scr_Allign4
==============
*/
const char* Scr_Allign4( char const *pos )
{
	UNIMPLEMENTED(__FUNCTION__);
	return NULL;
}

/*
==============
Scr_Allign4Strict
==============
*/
const char* Scr_Allign4Strict( char const *pos )
{
	UNIMPLEMENTED(__FUNCTION__);
	return NULL;
}

/*
==============
Scr_GetLocalVarAtIndex
==============
*/
unsigned int Scr_GetLocalVarAtIndex( int index )
{
	return scrVmPub.localVars[-index];
}

/*
==============
Scr_GetLocalVar
==============
*/
unsigned int Scr_GetLocalVar( const char *pos )
{
	return Scr_GetLocalVarAtIndex( *(unsigned char *)pos );
}

/*
==============
Scr_ReadShort
==============
*/
short Scr_ReadShort( const char **pos )
{
	short value = *(reinterpret_cast<const short *>(*pos));
	*pos += sizeof(short);

	return value;
}

/*
==============
Scr_ReadInt
==============
*/
int Scr_ReadInt( const char **pos )
{
	int value = *(reinterpret_cast<const int *>(*pos));
	*pos += sizeof(int);

	return value;
}

/*
==============
Scr_ReadVector
==============
*/
const float* Scr_ReadVector( const char **pos )
{
	const float *value = reinterpret_cast<const float *>(*pos);
	*pos += sizeof(vec3_t);

	return value;
}

/*
==============
Scr_ReadIntArray
==============
*/
const int* Scr_ReadIntArray( const char **pos, int count )
{
	const int *value = reinterpret_cast<const int *>(*pos);
	*pos += sizeof(const int) * count;

	return value;
}

/*
==============
Scr_ReadUnsignedShort
==============
*/
unsigned short Scr_ReadUnsignedShort( const char **pos )
{
	unsigned short value = *(reinterpret_cast<const unsigned short *>(*pos));
	*pos += sizeof(unsigned short);

	return value;
}

/*
==============
Scr_ReadUnsigned
==============
*/
unsigned int Scr_ReadUnsigned( const char **pos )
{
	unsigned int value = *(reinterpret_cast<const unsigned int *>(*pos));
	*pos += sizeof(unsigned int);

	return value;
}

/*
==============
Scr_ReadFloat
==============
*/
float Scr_ReadFloat( const char **pos )
{
	float value = *(reinterpret_cast<const float *>(*pos));
	*pos += sizeof(float);

	return value;
}

/*
==============
Scr_ReadCodePos
==============
*/
const char* Scr_ReadCodePos( const char **pos )
{
	const char *value = *(reinterpret_cast<const char**>(const_cast<char *>(*pos)));
	*pos += sizeof(const char *);

	return value;
}

/*
==============
Scr_StackClear
==============
*/
void Scr_StackClear( void )
{
	scrVmPub.top = scrVmPub.stack;
}

/*
==============
Scr_DecTime
==============
*/
void Scr_DecTime()
{
	assert(!(scrVarPub.time & ~VAR_NAME_LOW_MASK));
	--scrVarPub.time;
	scrVarPub.time &= VAR_NAME_LOW_MASK;
}

/*
==============
Scr_GetNumParam
==============
*/
unsigned int Scr_GetNumParam()
{
	return scrVmPub.outparamcount;
}

/*
==============
Scr_IsSystemActive
==============
*/
int Scr_IsSystemActive()
{
	return scrVarPub.timeArrayId != 0;
}

/*
==============
Scr_GetReturnPos
==============
*/
char* Scr_GetReturnPos( unsigned int *localId )
{
	char *pos;

	if ( scrVmPub.function_count < 2 )
	{
		return 0;
	}

	pos = (char *)scrVmPub.function_frame[-1].fs.pos;

	if ( pos == &g_EndPos )
	{
		return 0;
	}

	*localId = scrVmPub.function_frame[-1].fs.localId;
	return pos;
}

/*
==============
Scr_GetNumScriptThreads
==============
*/
unsigned int Scr_GetNumScriptThreads()
{
	UNIMPLEMENTED(__FUNCTION__);
	return 0;
}

/*
==============
Scr_SetLoading
==============
*/
void Scr_SetLoading( int bLoading )
{
	scrVmGlob.loading = bLoading;
}

/*
==============
Scr_Abort
==============
*/
void Scr_Abort()
{
	scrVarPub.timeArrayId = 0;
	scrVarPub.bInited = false;
}

/*
==============
Scr_Settings
==============
*/
void Scr_Settings( int developer, int developer_script, int abort_on_error )
{
	assert(!abort_on_error || developer);

	scrVarPub.developer = developer != 0;
	scrVarPub.developer_script = developer_script != 0;

	scrVmPub.abort_on_error = abort_on_error != 0;
}

/*
==============
Scr_ClearErrorMessage
==============
*/
void Scr_ClearErrorMessage()
{
	scrVarPub.error_message = NULL;
	scrVmGlob.dialog_error_message = NULL;

	scrVarPub.error_index = 0;
}

/*
==============
Scr_ResetTimeout
==============
*/
void Scr_ResetTimeout()
{
	scrVmGlob.starttime = Sys_MilliSeconds();
}

/*
==============
Scr_ErrorWithDialogMessage
==============
*/
void Scr_ErrorWithDialogMessage( const char *error, const char *dialog_error )
{
	scrVarPub.error_message = error;
	scrVmGlob.dialog_error_message = dialog_error;

	Scr_ErrorInternal();
}

/*
==============
Scr_Error
==============
*/
void Scr_Error( const char *error )
{
	if ( scrVarPub.error_message == NULL )
	{
		scrVarPub.error_message = error;
	}

	Scr_ErrorInternal();
}

/*
==============
Scr_GetPointerType
==============
*/
int Scr_GetPointerType( unsigned int index )
{
	VariableValue *value;

	if ( index >= scrVmPub.outparamcount )
	{
		Scr_Error(va("parameter %d does not exist", index + 1));
		return 0;
	}

	value = &scrVmPub.top[-index];

	if ( value->type != VAR_POINTER )
	{
		Scr_Error(va("type %s is not a pointer", var_typename[value->type]));
		return 0;
	}

	return GetObjectType(value->u.pointerValue);
}

/*
==============
Scr_GetTypeName
==============
*/
const char* Scr_GetTypeName( unsigned int index )
{
	VariableValue *value;

	if ( index >= scrVmPub.outparamcount )
	{
		Scr_Error(va("parameter %d does not exist", index + 1));
		return NULL;
	}

	value = &scrVmPub.top[-index];
	return var_typename[value->type];
}

/*
==============
Scr_GetType
==============
*/
int Scr_GetType( unsigned int index )
{
	VariableValue *value;

	if ( index >= scrVmPub.outparamcount )
	{
		Scr_Error(va("parameter %d does not exist", index + 1));
		return 0;
	}

	value = &scrVmPub.top[-index];
	return value->type;
}

/*
==============
Scr_GetObject
==============
*/
unsigned int Scr_GetObject( unsigned int index )
{
	VariableValue *value;

	if ( index >= scrVmPub.outparamcount )
	{
		Scr_Error(va("parameter %d does not exist", index + 1));
		return 0;
	}

	value = &scrVmPub.top[-index];

	if ( value->type != VAR_POINTER )
	{
		scrVarPub.error_index = index + 1;
		Scr_Error(va("type %s is not an object", var_typename[value->type]));
		return 0;
	}

	return value->u.pointerValue;
}

/*
==============
Scr_GetEntityRef
==============
*/
scr_entref_t Scr_GetEntityRef( unsigned int index )
{
	VariableValue *value;
	unsigned int id;
	scr_entref_t entref;

	// init empty variable
	entref.entnum = 0;
	entref.classnum = 0;

	if ( index >= scrVmPub.outparamcount )
	{
		Scr_Error(va("parameter %d does not exist", index + 1));
		return entref;
	}

	value = &scrVmPub.top[-index];

	if ( value->type != VAR_POINTER )
	{
		scrVarPub.error_index = index + 1;
		Scr_Error(va("type %s is not an entity", var_typename[value->type]));
		return entref;
	}

	id = value->u.pointerValue;

	if ( GetObjectType(value->u.pointerValue) != VAR_ENTITY )
	{
		scrVarPub.error_index = index + 1;
		Scr_Error(va("type %s is not an entity", var_typename[GetObjectType(id)]));
		return entref;
	}

	return Scr_GetEntityIdRef(id);
}

/*
==============
Scr_GetFunc
==============
*/
unsigned int Scr_GetFunc( unsigned int index )
{
	VariableValue *value;

	if ( index >= scrVmPub.outparamcount )
	{
		Scr_Error(va("parameter %d does not exist", index + 1));
		return 0;
	}

	value = &scrVmPub.top[-index];

	if ( value->type != VAR_FUNCTION )
	{
		scrVarPub.error_index = index + 1;
		Scr_Error(va("type %s is not a function", var_typename[value->type]));
		return 0;
	}

	return value->u.codePosValue - scrVarPub.programBuffer;
}

/*
==============
Scr_GetVector
==============
*/
void Scr_GetVector( unsigned int index, vec3_t vectorValue )
{
	VariableValue *value;

	if ( index >= scrVmPub.outparamcount )
	{
		Scr_Error(va("parameter %d does not exist", index + 1));
		return;
	}

	value = &scrVmPub.top[-index];

	if ( value->type != VAR_VECTOR )
	{
		scrVarPub.error_index = index + 1;
		Scr_Error(va("type %s is not a vector", var_typename[value->type]));
		return;
	}

	VectorCopy(value->u.vectorValue, vectorValue);
}

/*
==============
Scr_GetConstIString
==============
*/
unsigned int Scr_GetConstIString( unsigned int index )
{
	VariableValue *value;

	if ( index >= scrVmPub.outparamcount )
	{
		Scr_Error(va("parameter %d does not exist", index + 1));
		return 0;
	}

	value = &scrVmPub.top[-index];

	if ( value->type != VAR_ISTRING )
	{
		scrVarPub.error_index = index + 1;
		Scr_Error(va("type %s is not a localized string", var_typename[value->type]));
		return 0;
	}

	return value->u.stringValue;
}

/*
==============
Scr_GetFloat
==============
*/
float Scr_GetFloat( unsigned int index )
{
	VariableValue *value;

	if ( index >= scrVmPub.outparamcount )
	{
		Scr_Error(va("parameter %d does not exist", index + 1));
		return 0;
	}

	value = &scrVmPub.top[-index];

	// cast to float
	if ( value->type == VAR_INTEGER )
	{
		return (float)value->u.intValue;
	}

	if ( value->type != VAR_FLOAT )
	{
		scrVarPub.error_index = index + 1;
		Scr_Error(va("type %s is not a float", var_typename[value->type]));
		return 0;
	}

	return value->u.floatValue;
}

/*
==============
Scr_GetInt
==============
*/
int Scr_GetInt( unsigned int index )
{
	VariableValue *value;

	if ( index >= scrVmPub.outparamcount )
	{
		Scr_Error(va("parameter %d does not exist", index + 1));
		return 0;
	}

	value = &scrVmPub.top[-index];

	if ( value->type != VAR_INTEGER )
	{
		scrVarPub.error_index = index + 1;
		Scr_Error(va("type %s is not an int", var_typename[value->type]));
	}

	return value->u.intValue;
}

/*
==============
Scr_ObjectError
==============
*/
void Scr_ObjectError( const char *error )
{
	scrVarPub.error_index = -1;
	Scr_Error(error);
}

/*
==============
Scr_ParamError
==============
*/
void Scr_ParamError( unsigned int index, const char *error )
{
	scrVarPub.error_index = index + 1;
	Scr_Error(error);
}

/*
==============
Scr_GetIString
==============
*/
const char* Scr_GetIString( unsigned int index )
{
	return SL_ConvertToString( Scr_GetConstIString( index ) );
}

/*
==============
Scr_FindAllThreads
==============
*/
int Scr_FindAllThreads( unsigned int selfId, unsigned int *threads, unsigned int localId )
{
	unsigned int timeId, notifyListId, stackId, threadId;
	int count = 0;

	if ( localId && selfId == Scr_GetSelf(localId) )
	{
		if ( threads )
		{
			*threads = localId;
		}

		count = 1;
	}

	for ( threadId = FindNextSibling(scrVarPub.timeArrayId); threadId; threadId = FindNextSibling(threadId) )
	{
		count = Scr_FindAllThreadsInternal(selfId, threadId, count, true, threads);
	}

	notifyListId = FindVariable(selfId, OBJECT_NOTIFY_LIST);

	if ( notifyListId )
	{
		timeId = FindObject(notifyListId);
		assert(timeId);

		for ( stackId = FindNextSibling(timeId); stackId; stackId = FindNextSibling(stackId) )
		{
			count = Scr_FindAllThreadsInternal(selfId, stackId, count, false, threads);
		}
	}

	return count;
}

/*
==============
Scr_TraverseScript
==============
*/
void Scr_TraverseScript( char const *pos )
{
	UNIMPLEMENTED(__FUNCTION__);
}

/*
==============
Scr_IsEndonThread
==============
*/
bool Scr_IsEndonThread( unsigned int localId )
{
	unsigned int stackId;
	int type;

	if ( GetObjectType(localId) != VAR_NOTIFY_THREAD )
	{
		return false;
	}

	if ( GetStartLocalId(localId) != localId )
	{
		return false;
	}

	stackId = Scr_GetWaittillThreadStackId(localId, localId);
	type = GetObjectType(stackId);

	assert((type == VAR_UNDEFINED) || (type == VAR_STACK));
	return type == VAR_UNDEFINED;
}

/*
==============
Scr_TerminalError
==============
*/
void Scr_TerminalError( const char *error )
{
	Scr_DumpScriptThreads();
	Scr_DumpScriptVariablesDefault();

	scrVmPub.terminal_error = true;
	Scr_Error(error);
}

/*
==============
Scr_InitSystem
==============
*/
void Scr_InitSystem()
{
	assert(!scrVarPub.timeArrayId);
	scrVarPub.timeArrayId = AllocObject();

	assert(!scrVarPub.pauseArrayId);
	scrVarPub.pauseArrayId = Scr_AllocArray();

	assert(!scrVarPub.levelId);
	scrVarPub.levelId = AllocObject();

	assert(!scrVarPub.animId);
	scrVarPub.animId = AllocObject();

	scrVarPub.time = 0;
	g_script_error_level = -1;
}

/*
==============
Scr_AddArrayStringIndexed
==============
*/
void Scr_AddArrayStringIndexed( unsigned int stringValue )
{
	unsigned int id;

	assert(scrVmPub.inparamcount);

	--scrVmPub.top;
	--scrVmPub.inparamcount;

	assert(scrVmPub.top->type == VAR_POINTER);

	id = GetNewVariable(scrVmPub.top->u.pointerValue, stringValue);
	SetNewVariableValue(id, scrVmPub.top + 1);
}

/*
==============
Scr_AddArray
==============
*/
void Scr_AddArray()
{
	unsigned int id;

	assert(scrVmPub.inparamcount);

	--scrVmPub.top;
	--scrVmPub.inparamcount;

	assert(scrVmPub.top->type == VAR_POINTER);

	id = GetNewArrayVariable(scrVmPub.top->u.pointerValue, GetArraySize(scrVmPub.top->u.pointerValue));
	SetNewVariableValue(id, scrVmPub.top + 1);
}

/*
==============
Scr_GetAnimTree
==============
*/
scr_animtree_t Scr_GetAnimTree( unsigned int index ) // untested
{
	VariableValue *value;
	scr_animtree_t tree;

	if ( index >= scrVmPub.outparamcount )
	{
		Scr_Error(va("parameter %d does not exist", index + 1));

		tree.anims = NULL;
		return tree;
	}

	value = &scrVmPub.top[-index];

	if ( value->type != VAR_INTEGER )
	{
		scrVarPub.error_message = va("type %s is not an animtree", var_typename[value->type]);

		RemoveRefToValue(value);
		value->type = VAR_UNDEFINED;

		scrVarPub.error_index = index + 1;
		Scr_ErrorInternal();

		tree.anims = NULL;
		return tree;
	}

	if ( value->u.intValue > scrAnimPub.xanim_num[SCR_XANIM_SERVER] || !Scr_GetAnims(value->u.intValue) )
	{
		scrVarPub.error_message = "bad anim tree";

		RemoveRefToValue(value);
		value->type = VAR_UNDEFINED;

		scrVarPub.error_index = index + 1;
		Scr_ErrorInternal();

		tree.anims = NULL;
		return tree;
	}

	tree.anims = Scr_GetAnims(value->u.intValue);
	return tree;
}

/*
==============
Scr_GetAnim
==============
*/
scr_anim_s Scr_GetAnim( unsigned int index, XAnimTree_s *tree )
{
	VariableValue *value;
	scr_anim_s anim;

	if ( index >= scrVmPub.outparamcount )
	{
		Scr_Error(va("parameter %d does not exist", index + 1));

		anim.index = 0;
		anim.tree = 0;

		return anim;
	}

	value = &scrVmPub.top[-index];

	if ( value->type != VAR_ANIMATION )
	{
		scrVarPub.error_message = va("type %s is not an anim", var_typename[value->type]);

		RemoveRefToValue(value);
		value->type = VAR_UNDEFINED;

		scrVarPub.error_index = index + 1;
		Scr_ErrorInternal();

		anim.index = 0;
		anim.tree = 0;

		return anim;
	}

	anim.linkPointer = value->u.codePosValue;

	if ( !tree )
	{
		return anim;
	}

	if ( Scr_GetAnims(anim.tree) != XAnimGetAnims(tree) )
	{
		scrVarPub.error_message = va("anim '%s' in animtree '%s' does not belong to the entity's animtree '%s'",
		                             XAnimGetAnimDebugName(Scr_GetAnims(anim.tree), anim.index),
		                             XAnimGetAnimTreeDebugName(Scr_GetAnims(anim.tree)),
		                             XAnimGetAnimTreeDebugName(XAnimGetAnims(tree)));

		RemoveRefToValue(value);
		value->type = VAR_UNDEFINED;

		scrVarPub.error_index = index + 1;
		Scr_ErrorInternal();
	}

	return anim;
}

/*
==============
Scr_ClearOutParams
==============
*/
void Scr_ClearOutParams()
{
	while ( scrVmPub.outparamcount )
	{
		RemoveRefToValue(scrVmPub.top);

		--scrVmPub.top;
		--scrVmPub.outparamcount;
	}
}

/*
==============
Scr_GetConstLowercaseString
==============
*/
unsigned int Scr_GetConstLowercaseString( unsigned int index )
{
	unsigned int stringValue;
	VariableValue *value;
	const char *string;
	int i;
	char tempString[8192];

	if ( index >= scrVmPub.outparamcount )
	{
		Scr_Error(va("parameter %d does not exist", index + 1));
		return 0;
	}

	value = &scrVmPub.top[-index];

	if ( !Scr_CastString(value) )
	{
		scrVarPub.error_index = index + 1;
		Scr_ErrorInternal();
		return 0;
	}

	assert(value->type == VAR_STRING);
	stringValue = value->u.stringValue;
	string = SL_ConvertToString(stringValue);

	for ( i = 0; ; i++ )
	{
		tempString[i] = tolower(string[i]);

		if ( !string[i] )
		{
			break;
		}
	}

	assert(value->type == VAR_STRING);
	value->u.stringValue = SL_GetString(tempString, 0);
	SL_RemoveRefToString(stringValue);

	return value->u.stringValue;
}

/*
==============
Scr_GetConstString
==============
*/
unsigned int Scr_GetConstString( unsigned int index )
{
	VariableValue *value;

	if ( index >= scrVmPub.outparamcount )
	{
		Scr_Error(va("parameter %d does not exist", index + 1));
		return 0;
	}

	value = &scrVmPub.top[-index];

	if ( !Scr_CastString(value) )
	{
		scrVarPub.error_index = index + 1;
		Scr_ErrorInternal();
		return 0;
	}

	assert(value->type == VAR_STRING);
	return value->u.stringValue;
}

/*
==============
Scr_GetNextCodepos
==============
*/
const char* Scr_GetNextCodepos( VariableValue *top, const char *pos, int opcode, int mode, unsigned int *localId )
{
	UNIMPLEMENTED(__FUNCTION__);
	return NULL;
}

/*
==============
Scr_Shutdown
==============
*/
void Scr_Shutdown()
{
	if ( !scrVarPub.bInited )
	{
		return;
	}

	scrVarPub.bInited = false;

	Scr_ShutdownVariables();
	VM_Shutdown();
	SL_Shutdown();
}

/*
==============
Scr_GetDebugString
==============
*/
const char* Scr_GetDebugString( unsigned int index )
{
	VariableValue *value;

	if ( index >= scrVmPub.outparamcount )
	{
		Scr_Error(va("parameter %d does not exist", index + 1));
		return 0;
	}

	value = &scrVmPub.top[-index];

	Scr_CastDebugString(value);

	assert(value->type == VAR_STRING);
	return SL_ConvertToString(value->u.stringValue);
}

/*
==============
Scr_GetStringIncludeNull
==============
*/
const char* Scr_GetStringIncludeNull( unsigned int index )
{
	return SL_ConvertToString( Scr_GetConstString( index ) );
}

/*
==============
Scr_GetConstStringIncludeNull
==============
*/
unsigned int Scr_GetConstStringIncludeNull( unsigned int index )
{
	if ( index < scrVmPub.outparamcount && scrVmPub.top[-index].type == VAR_UNDEFINED )
	{
		return 0;
	}

	return Scr_GetConstString(index);
}

/*
==============
Scr_GetString
==============
*/
const char* Scr_GetString( unsigned int index )
{
	return SL_ConvertToString( Scr_GetConstString( index ) );
}

/*
==============
Scr_FreeThread
==============
*/
void Scr_FreeThread( unsigned short handle )
{
	assert(scrVarPub.timeArrayId);
	assert(handle);

	RemoveRefToObject(handle);
}

/*
==============
VM_CancelNotify
==============
*/
void VM_CancelNotify( unsigned int notifyListOwnerId, unsigned int startLocalId )
{
	unsigned int notifyNameListId, notifyListId, stringValue;

	notifyListId = FindObject( FindVariable( notifyListOwnerId, OBJECT_NOTIFY_LIST ) );
	stringValue = Scr_GetThreadNotifyName(startLocalId);
	assert(stringValue);
	notifyNameListId = FindObject( FindVariable( notifyListId, stringValue ) );

	VM_CancelNotifyInternal(notifyListOwnerId, startLocalId, notifyListId, notifyNameListId, stringValue);
}

/*
==============
SetEntityFieldValue
==============
*/
bool SetEntityFieldValue( unsigned int classnum, int entnum, int offset, VariableValue *value )
{
	assert(!scrVmPub.inparamcount);
	assert(!scrVmPub.outparamcount);

	scrVmPub.outparamcount = 1;
	scrVmPub.top = value;

	if ( !Scr_SetObjectField(classnum, entnum, offset) )
	{
		assert(!scrVmPub.inparamcount);
		assert(scrVmPub.outparamcount == 1);

		scrVmPub.outparamcount = 0;
		return false;
	}

	assert(!scrVmPub.inparamcount);

	if ( scrVmPub.outparamcount )
	{
		assert(scrVmPub.outparamcount == 1);

		RemoveRefToValue(scrVmPub.top);
		--scrVmPub.top;

		scrVmPub.outparamcount = 0;
	}

	return true;
}

/*
==============
Scr_CancelNotifyList
==============
*/
void Scr_CancelNotifyList( unsigned int notifyListOwnerId )
{
	unsigned int selfStartLocalId, selfLocalId, startLocalId, notifyListId, notifyNameListId, stackId;
	VariableStackBuffer *stackValue;

	while ( 1 )
	{
		notifyListId = FindVariable(notifyListOwnerId, OBJECT_NOTIFY_LIST);

		if ( !notifyListId )
		{
			break;
		}

		notifyNameListId = FindNextSibling(FindObject(notifyListId));

		if ( !notifyNameListId )
		{
			break;
		}

		stackId = FindNextSibling(FindObject(notifyNameListId));

		if ( !stackId )
		{
			break;
		}

		startLocalId = GetVariableKeyObject(stackId);
		assert(startLocalId);

		if ( GetObjectType(stackId) == VAR_STACK )
		{
			stackValue = GetVariableValueAddress(stackId)->stackValue;
			Scr_CancelWaittill(startLocalId);

			VM_TrimStack(startLocalId, stackValue, false);
			continue;
		}

		AddRefToObject(startLocalId);
		Scr_CancelWaittill(startLocalId);

		selfLocalId = Scr_GetSelf(startLocalId);
		selfStartLocalId = GetStartLocalId(selfLocalId);

		stackId = FindVariable(selfStartLocalId, OBJECT_STACK);

		if ( stackId )
		{
			stackValue = GetVariableValueAddress(stackId)->stackValue;

			assert(!Scr_GetThreadNotifyName( selfStartLocalId ));
			assert(GetObjectType( stackId ) == VAR_STACK);
			assert(!stackValue->pos);

			VM_TrimStack(selfStartLocalId, stackValue, true);
		}

		Scr_KillEndonThread(startLocalId);
		RemoveRefToEmptyObject(startLocalId);
	}
}

/*
==============
Scr_SetStructField
==============
*/
void Scr_SetStructField( unsigned int structId, unsigned int index )
{
	unsigned int fieldValueId;

	assert(!scrVmPub.outparamcount);
	assert(scrVmPub.inparamcount == 1);

	fieldValueId = Scr_GetVariableField(structId, index);

	assert(scrVmPub.inparamcount == 1);
	scrVmPub.inparamcount = 0;

	SetVariableFieldValue(fieldValueId, scrVmPub.top);

	assert(!scrVmPub.inparamcount);
	assert(!scrVmPub.outparamcount);

	scrVmPub.top--;
}

/*
==============
Scr_ShutdownSystem
==============
*/
void Scr_ShutdownSystem( int bComplete )
{
	unsigned int id, parentId;

	Scr_CompileShutdown();
	Scr_FreeEntityList();

	if ( !scrVarPub.timeArrayId )
	{
		return;
	}

	Scr_FreeGameVariable(bComplete);

	for ( id = FindNextSibling(scrVarPub.timeArrayId); id; id = FindNextSibling(id) )
	{
		VM_TerminateTime(FindObject(id));
	}

	while ( 1 )
	{
		id = FindNextSibling(scrVarPub.pauseArrayId);

		if ( !id )
		{
			break;
		}

		id = FindNextSibling(FindObject(id));
		assert(id);

		parentId = GetVariableValueAddress(id)->pointerValue;

		AddRefToObject(parentId);
		Scr_CancelNotifyList(parentId);
		RemoveRefToObject(parentId);
	}

	assert(scrVarPub.levelId);
	ClearObject(scrVarPub.levelId);
	RemoveRefToEmptyObject(scrVarPub.levelId);
	scrVarPub.levelId = 0;

	assert(scrVarPub.animId);
	ClearObject(scrVarPub.animId);
	RemoveRefToEmptyObject(scrVarPub.animId);
	scrVarPub.animId = 0;

	assert(scrVarPub.timeArrayId);
	ClearObject(scrVarPub.timeArrayId);
	RemoveRefToEmptyObject(scrVarPub.timeArrayId);
	scrVarPub.timeArrayId = 0;

	assert(scrVarPub.pauseArrayId);
	RemoveRefToEmptyObject(scrVarPub.pauseArrayId);
	scrVarPub.pauseArrayId = 0;

	assert(!scrVarPub.freeEntList);
	Scr_FreeObjects();
}

/*
==============
Scr_SetDynamicEntityField
==============
*/
void Scr_SetDynamicEntityField( int entnum, int classnum, unsigned int index )
{
	unsigned int entId;

	entId = Scr_GetEntityId(entnum, classnum);
	assert(GetObjectType( entId ) == VAR_ENTITY);

	Scr_SetStructField(entId, index);
}

/*
==============
Scr_MakeArray
==============
*/
void Scr_MakeArray()
{
	IncInParam();

	scrVmPub.top->type = VAR_POINTER;
	scrVmPub.top->u.pointerValue = Scr_AllocArray();
}

/*
==============
Scr_AddVector
==============
*/
void Scr_AddVector( const vec3_t value )
{
	IncInParam();

	scrVmPub.top->type = VAR_VECTOR;
	scrVmPub.top->u.vectorValue = Scr_AllocVector(value);
}

/*
==============
Scr_AddConstString
==============
*/
void Scr_AddConstString( unsigned int value )
{
	assert(value);
	IncInParam();

	scrVmPub.top->type = VAR_STRING;
	scrVmPub.top->u.stringValue = value;

	SL_AddRefToString(value);
}

/*
==============
Scr_AddIString
==============
*/
void Scr_AddIString( const char *value )
{
	assert(value);
	IncInParam();

	scrVmPub.top->type = VAR_ISTRING;
	scrVmPub.top->u.stringValue = SL_GetString(value, 0);
}

/*
==============
Scr_AddString
==============
*/
void Scr_AddString( const char *value )
{
	assert(value);
	IncInParam();

	scrVmPub.top->type = VAR_STRING;
	scrVmPub.top->u.stringValue = SL_GetString(value, 0);
}

/*
==============
Scr_AddObject
==============
*/
void Scr_AddObject( unsigned int id )
{
	assert(id);
	assert(GetObjectType( id ) != VAR_THREAD);
	assert(GetObjectType( id ) != VAR_NOTIFY_THREAD);
	assert(GetObjectType( id ) != VAR_TIME_THREAD);
	assert(GetObjectType( id ) != VAR_CHILD_THREAD);
	assert(GetObjectType( id ) != VAR_DEAD_THREAD);

	IncInParam();

	scrVmPub.top->type = VAR_POINTER;
	scrVmPub.top->u.pointerValue = id;

	AddRefToObject(id);
}

/*
==============
Scr_AddUndefined
==============
*/
void Scr_AddUndefined()
{
	IncInParam();
	scrVmPub.top->type = VAR_UNDEFINED;
}

/*
==============
Scr_AddAnim
==============
*/
void Scr_AddAnim( scr_anim_s value )
{
	IncInParam();

	scrVmPub.top->type = VAR_ANIMATION;
	scrVmPub.top->u.codePosValue = value.linkPointer;
}

/*
==============
Scr_AddFloat
==============
*/
void Scr_AddFloat( float value )
{
	IncInParam();

	scrVmPub.top->type = VAR_FLOAT;
	scrVmPub.top->u.floatValue = value;
}

/*
==============
Scr_AddInt
==============
*/
void Scr_AddInt( int value )
{
	IncInParam();

	scrVmPub.top->type = VAR_INTEGER;
	scrVmPub.top->u.intValue = value;
}

/*
==============
Scr_AddBool
==============
*/
void Scr_AddBool( bool value )
{
	assert(value == false || value == true);
	IncInParam();

	scrVmPub.top->type = VAR_INTEGER;
	scrVmPub.top->u.intValue = value;
}

/*
==============
Scr_AddStruct
==============
*/
void Scr_AddStruct()
{
	unsigned int id = AllocObject();

	Scr_AddObject(id);
	RemoveRefToObject(id);
}

/*
==============
Scr_AddEntityNum
==============
*/
void Scr_AddEntityNum( int entnum, int classnum )
{
	Scr_AddObject( Scr_GetEntityId( entnum, classnum ) );
}

/*
==============
GetEntityFieldValue
==============
*/
VariableValue GetEntityFieldValue( unsigned int classnum, int entnum, int offset )
{
	VariableValue value;

	assert(!scrVmPub.inparamcount);
	assert(!scrVmPub.outparamcount);

	scrVmPub.top = scrVmGlob.eval_stack - 1;
	scrVmGlob.eval_stack->type = VAR_UNDEFINED;

	Scr_GetObjectField(classnum, entnum, offset);

	assert(!scrVmPub.inparamcount || scrVmPub.inparamcount == 1);
	assert(!scrVmPub.outparamcount);
	assert(scrVmPub.top - scrVmPub.inparamcount == scrVmGlob.eval_stack - 1);

	scrVmPub.inparamcount = 0;

	value.u = scrVmGlob.eval_stack->u;
	value.type = scrVmGlob.eval_stack->type;

	return value;
}

/*
==============
Scr_NotifyNum
==============
*/
void Scr_NotifyNum( int entnum, int classnum, unsigned int stringValue, unsigned int paramcount )
{
	int type;
	VariableValue *startTop;
	unsigned int id;

	assert(scrVarPub.timeArrayId);
	assert(paramcount <= scrVmPub.inparamcount);

	Scr_ClearOutParams();

	startTop = &scrVmPub.top[-paramcount];
	paramcount = scrVmPub.inparamcount - paramcount;

	id = FindEntityId(entnum, classnum);

	if ( id )
	{
		type = startTop->type;

		startTop->type = VAR_PRECODEPOS;
		scrVmPub.inparamcount = 0;

		VM_Notify(id, stringValue, scrVmPub.top);

		startTop->type = type;
	}

	while ( scrVmPub.top != startTop )
	{
		RemoveRefToValue(scrVmPub.top);
		scrVmPub.top--;
	}

	assert(!scrVmPub.outparamcount);
	scrVmPub.inparamcount = paramcount;
}

/*
==============
Scr_Init
==============
*/
void Scr_Init()
{
	if ( scrVarPub.bInited )
	{
		return;
	}

	SL_Restart();

	Var_Init();
	Scr_VM_Init();

	scrCompilePub.script_loading = false;
	scrAnimPub.animtree_loading = false;

	scrCompilePub.scriptsPos = 0;
	scrCompilePub.loadedscripts = 0;

	scrAnimPub.animtrees = 0;

	scrCompilePub.builtinMeth = 0;
	scrCompilePub.builtinFunc = 0;

	scrVarPub.bInited = true;
}

/*
==============
Scr_ExecCode
==============
*/
void Scr_ExecCode( const char *pos, unsigned int localId )
{
	Scr_ResetTimeout();

	assert(scrVarPub.timeArrayId);
	assert(!scrVmPub.inparamcount);
	assert(!scrVmPub.outparamcount);
	assert(!scrVarPub.evaluate);
	assert(!scrVmPub.debugCode);

	scrVmPub.debugCode = true;

	if ( localId )
	{
		VM_Execute(localId, pos, 0);
	}
	else
	{
		AddRefToObject(scrVarPub.levelId);
		localId = AllocThread(scrVarPub.levelId);

		VM_Execute(localId, pos, 0);

		Scr_KillThread(localId);
		RemoveRefToObject(localId);
	}

	assert(scrVmPub.debugCode);

	scrVmPub.debugCode = false;

	assert(scrVmPub.inparamcount == 1);
	assert(!scrVmPub.outparamcount);

	if ( scrVmPub.function_count )
	{
		scrVmPub.function_count--;
		scrVmPub.function_frame--;
	}

	scrVmPub.top--;
	scrVmPub.inparamcount = 0;
}

/*
==============
Scr_AddExecEntThreadNum
==============
*/
void Scr_AddExecEntThreadNum( int entnum, int classnum, int handle, unsigned int paramcount )
{
	const char *pos;
	unsigned int threadId, id, selfId;

	pos = &scrVarPub.programBuffer[handle];

	if ( !scrVmPub.function_count )
	{
		assert(scrVmPub.localVars == scrVmGlob.localVarsStack - 1);
		Scr_ResetTimeout();
	}

	assert(scrVarPub.timeArrayId);
	assert(handle);
	assert(Scr_IsInScriptMemory( pos ));

	selfId = Scr_GetEntityId(entnum, classnum);
	AddRefToObject(selfId);

	threadId = AllocThread(selfId);
	id = VM_Execute(threadId, pos, paramcount);

	RemoveRefToObject(id);

	scrVmPub.outparamcount++;
	scrVmPub.inparamcount--;

	assert(scrVmPub.localVars == scrVmGlob.localVarsStack - 1);
}

/*
==============
Scr_AddExecThread
==============
*/
void Scr_AddExecThread( int handle, unsigned int paramcount )
{
	const char *pos;
	unsigned int id, threadId;

	pos = &scrVarPub.programBuffer[handle];

	if ( !scrVmPub.function_count )
	{
		assert(scrVmPub.localVars == scrVmGlob.localVarsStack - 1);
		Scr_ResetTimeout();
	}

	assert(scrVarPub.timeArrayId);
	assert(handle);
	assert(Scr_IsInScriptMemory( pos ));

	AddRefToObject(scrVarPub.levelId);

	threadId = AllocThread(scrVarPub.levelId);
	id = VM_Execute(threadId, pos, paramcount);

	RemoveRefToObject(id);

	scrVmPub.outparamcount++;
	scrVmPub.inparamcount--;

	assert(scrVmPub.localVars == scrVmGlob.localVarsStack - 1);
}

/*
==============
Scr_ExecEntThreadNum
==============
*/
unsigned short Scr_ExecEntThreadNum( int entnum, int classnum, int handle, unsigned int paramcount )
{
	const char *pos;
	unsigned int threadId, id, selfId;

	pos = &scrVarPub.programBuffer[handle];

	if ( !scrVmPub.function_count )
	{
		assert(scrVmPub.localVars == scrVmGlob.localVarsStack - 1);
		Scr_ResetTimeout();
	}

	assert(scrVarPub.timeArrayId);
	assert(handle);
	assert(Scr_IsInScriptMemory( pos ));

	selfId = Scr_GetEntityId(entnum, classnum);
	AddRefToObject(selfId);

	threadId = AllocThread(selfId);
	id = VM_Execute(threadId, pos, paramcount);

	RemoveRefToValue(scrVmPub.top);
	scrVmPub.top->type = VAR_UNDEFINED;

	scrVmPub.top--;
	scrVmPub.inparamcount--;

	//assert(scrVmPub.localVars == scrVmGlob.localVarsStack - 1);

	return id;
}

/*
==============
Scr_ExecThread
==============
*/
unsigned short Scr_ExecThread( int handle, unsigned int paramcount )
{
	const char *pos;
	unsigned int threadId, id;

	pos = &scrVarPub.programBuffer[handle];

	if ( !scrVmPub.function_count )
	{
		assert(scrVmPub.localVars == scrVmGlob.localVarsStack - 1);
		Scr_ResetTimeout();
	}

	assert(scrVarPub.timeArrayId);
	assert(handle);
	assert(Scr_IsInScriptMemory( pos ));

	AddRefToObject(scrVarPub.levelId);

	threadId = AllocThread(scrVarPub.levelId);
	id = VM_Execute(threadId, pos, paramcount);

	RemoveRefToValue(scrVmPub.top);
	scrVmPub.top->type = VAR_UNDEFINED;

	scrVmPub.top--;
	scrVmPub.inparamcount--;

	assert(scrVmPub.localVars == scrVmGlob.localVarsStack - 1);

	return id;
}

/*
==============
Scr_RunCurrentThreads
==============
*/
void Scr_RunCurrentThreads()
{
	assert(!scrVmPub.function_count);
	assert(!scrVarPub.error_message);
	assert(!scrVarPub.error_index);
	assert(!scrVmPub.outparamcount);
	assert(!scrVmPub.inparamcount);
	assert(scrVmPub.top == scrVmPub.stack);

	VM_SetTime();
}

/*
==============
Scr_IncTime
==============
*/
void Scr_IncTime()
{
	Scr_RunCurrentThreads();
	Scr_FreeEntityList();

	assert(!(scrVarPub.time & ~VAR_NAME_LOW_MASK));

	scrVarPub.time++;
	scrVarPub.time &= VAR_NAME_LOW_MASK;
}

/*
==============
Scr_ErrorInternal
==============
*/
void Scr_ErrorInternal()
{
	assert(scrVarPub.error_message);

	if ( !scrVarPub.evaluate && !scrCompilePub.script_loading )
	{
		if ( scrVarPub.developer && scrVmGlob.loading )
		{
			scrVmPub.terminal_error = true;
		}

		if ( scrVmPub.function_count || scrVmPub.debugCode )
		{
			assert(g_script_error_level >= 0 && g_script_error_level < MAX_VM_STACK_DEPTH + 1);
			longjmp(g_script_error[g_script_error_level], -1);
		}

		Com_Error(ERR_DROP, "%s", scrVarPub.error_message);
	}

	if ( scrVmPub.terminal_error )
	{
		Com_Error(ERR_DROP, "%s", scrVarPub.error_message);
	}
}

/*
==============
Scr_FindAllThreadsInternal
==============
*/
int Scr_FindAllThreadsInternal( unsigned int selfId, unsigned int threadId, int count, bool a4, unsigned int *threads )
{
	unsigned int id, stackId;

	id = FindObject(threadId);

	for ( stackId = FindNextSibling(id); stackId; stackId = FindNextSibling(stackId) )
	{
		if ( GetObjectType(stackId) != VAR_STACK )
		{
			continue;
		}

		for ( threadId = GetVariableValueAddress(stackId)->stackValue->localId; threadId; threadId = GetSafeParentLocalId(threadId) )
		{
			if ( a4 && selfId != Scr_GetSelf(threadId) )
			{
				continue;
			}

			if ( threads )
			{
				threads[count] = threadId;
			}

			count++;
			break;
		}
	}

	return count;
}

/*
==============
Scr_TerminateRunningThread
==============
*/
void Scr_TerminateRunningThread( unsigned int localId )
{
	int threadId, topThread, function_count;

	assert(scrVmPub.function_count);
	function_count = scrVmPub.function_count;
	topThread = scrVmPub.function_count;

	while ( 1 )
	{
		assert(function_count);
		threadId = scrVmPub.function_frame_start[function_count].fs.localId;

		if ( threadId == localId )
		{
			break;
		}

		function_count--;

		if ( !GetSafeParentLocalId(threadId) )
		{
			topThread = function_count;
		}
	}

	while ( topThread >= function_count )
	{
		scrVmPub.function_frame_start[topThread].fs.pos = &g_EndPos;
		topThread--;
	}
}

/*
==============
Scr_AddLocalVars
==============
*/
int Scr_AddLocalVars( unsigned int localId )
{
	unsigned int fieldIndex;
	int localVarCount = 0;

	for ( fieldIndex = FindPrevSibling(localId); fieldIndex; fieldIndex = FindPrevSibling(fieldIndex) )
	{
		*scrVmPub.localVars++;
		*scrVmPub.localVars = fieldIndex;

		localVarCount++;
	}

	return localVarCount;
}

/*
==============
Scr_GetWaittillThreadStackId
==============
*/
unsigned int Scr_GetWaittillThreadStackId( unsigned int localId, unsigned int startLocalId )
{
	unsigned short stringValue;
	unsigned int id, index, parentId, selfId;

	assert(scrVarPub.developer);
	stringValue = Scr_GetThreadNotifyName(startLocalId);

	if ( !stringValue )
	{
		return FindVariable(startLocalId, OBJECT_STACK);
	}

	selfId = Scr_GetSelf(startLocalId);

	id = FindObjectVariable(scrVarPub.pauseArrayId, selfId);
	parentId = FindObject(id);

	id = FindObjectVariable(parentId, startLocalId);

	index = FindVariable(GetVariableValueAddress(id)->pointerValue, OBJECT_NOTIFY_LIST);
	parentId = FindObject(index);

	index = FindVariable(parentId, stringValue);
	parentId = FindObject(index);

	return FindObjectVariable(parentId, startLocalId);
}

/*
==============
Scr_VM_Init
==============
*/
void Scr_VM_Init()
{
	scrVmPub.maxstack = &scrVmPub.stack[MAX_VM_OPERAND_STACK - 1];
	scrVmPub.top = scrVmPub.stack;

	scrVmPub.function_count = 0;
	scrVmPub.function_frame = scrVmPub.function_frame_start;

	scrVmPub.localVars = scrVmGlob.localVarsStack - 1;

	scrVarPub.evaluate = false;
	scrVmPub.debugCode = false;

	Scr_ClearErrorMessage();

	scrVmPub.terminal_error = false;

	scrVmPub.outparamcount = 0;
	scrVmPub.inparamcount = 0;

	scrVarPub.tempVariable = AllocValue();

	scrVarPub.timeArrayId = 0;
	scrVarPub.pauseArrayId = 0;

	scrVarPub.levelId = 0;
	scrVarPub.gameId = 0;
	scrVarPub.animId = 0;

	scrVarPub.freeEntList = 0;

	scrVmPub.stack->type = VAR_CODEPOS;
	scrVmGlob.loading = false;
}

/*
==============
VM_UnarchiveStack
==============
*/
void VM_UnarchiveStack( unsigned int startLocalId, function_stack_t *fs, VariableStackBuffer *stackValue )
{
	int function_count, size;
	unsigned int localId;
	VariableValue *top;
	char *buf;

	assert(!scrVmPub.function_count);
	assert(stackValue->pos);
	assert(fs->startTop == &scrVmPub.stack[0]);

	scrVmPub.function_frame->fs.pos = stackValue->pos;

	scrVmPub.function_count++;
	scrVmPub.function_frame++;

	size = stackValue->size;
	buf = stackValue->buf;

	top = fs->startTop;

	while ( size )
	{
		top++;
		size--;

		top->type = *(unsigned char *)buf;
		buf += sizeof(unsigned char);

		if ( top->type == VAR_CODEPOS )
		{
			assert(scrVmPub.function_count < MAX_VM_STACK_DEPTH);
			scrVmPub.function_frame->fs.pos = *(const char **)buf;

			scrVmPub.function_count++;
			scrVmPub.function_frame++;
		}
		else
		{
			top->u.codePosValue = *(const char **)buf;
		}

		buf += sizeof(VariableUnion);
	}

	fs->pos = stackValue->pos;
	fs->top = top;

	localId = stackValue->localId;
	fs->localId = localId;
	Scr_ClearWaitTime(startLocalId);

	assert(scrVmPub.function_count < MAX_VM_STACK_DEPTH);
	function_count = scrVmPub.function_count;

	while ( 1 )
	{
		scrVmPub.function_frame_start[function_count].fs.localId = localId;
		function_count--;

		if ( !function_count )
		{
			break;
		}

		localId = GetParentLocalId(localId);
	}

	while ( ++function_count != scrVmPub.function_count )
	{
		scrVmPub.function_frame_start[function_count].fs.localVarCount = Scr_AddLocalVars(scrVmPub.function_frame_start[function_count].fs.localId);
	}

	fs->localVarCount = Scr_AddLocalVars(fs->localId);

	if ( stackValue->time != (unsigned char)scrVarPub.time )
	{
		Scr_ResetTimeout();
	}

	MT_Free(stackValue, stackValue->bufLen);
	assert(scrVmPub.stack[0].type == VAR_CODEPOS);
}

/*
==============
VM_ArchiveStack
==============
*/
VariableStackBuffer* VM_ArchiveStack( int size, const char *pos, VariableValue *top, unsigned int localVarCount, unsigned int *localId )
{
	unsigned int id;
	int bufLen;
	char *buf;
	VariableStackBuffer *stackValue;

	assert(size == (unsigned short)size);
	bufLen = STACKBUF_BUFFER_SIZE * size + sizeof(*stackValue) - 1;
	assert(bufLen == (unsigned short)bufLen);

	stackValue = (VariableStackBuffer *)MT_Alloc(bufLen);
	id = *localId;

	stackValue->localId = *localId;
	stackValue->size = size;
	stackValue->bufLen = bufLen;
	stackValue->pos = pos;
	stackValue->time = scrVarPub.time;

	scrVmPub.localVars -= localVarCount;
	buf = &stackValue->buf[STACKBUF_BUFFER_SIZE * size];

	while ( size )
	{
		buf -= sizeof(VariableUnion);

		if ( top->type == VAR_CODEPOS )
		{
			--scrVmPub.function_count;
			--scrVmPub.function_frame;

			*(const char **)buf = scrVmPub.function_frame->fs.pos;
			scrVmPub.localVars -= scrVmPub.function_frame->fs.localVarCount;

			id = GetParentLocalId(id);
		}
		else
		{
			*(const char **)buf = top->u.codePosValue;
		}

		buf -= sizeof(unsigned char);
		assert(top->type >= 0 && top->type < (1 << 8));
		*(unsigned char *)buf = top->type;

		top--;
		size--;
	}

	scrVmPub.function_count--;
	scrVmPub.function_frame--;

	AddRefToObject(id);
	*localId = id;

	return stackValue;
}

/*
==============
VM_Shutdown
==============
*/
void VM_Shutdown()
{
	if ( !scrVarPub.gameId )
	{
		return;
	}

	FreeValue(scrVarPub.gameId);
	scrVarPub.gameId = 0;
}

/*
==============
GetDummyFieldValue
==============
*/
unsigned int GetDummyFieldValue( void )
{
	ClearVariableValue(scrVarPub.tempVariable);
	return scrVarPub.tempVariable;
}

/*
==============
GetDummyObject
==============
*/
unsigned int GetDummyObject( void )
{
	ClearVariableValue(scrVarPub.tempVariable);
	return GetObjectA(scrVarPub.tempVariable);
}

/*
==============
VM_CancelNotifyInternal
==============
*/
void VM_CancelNotifyInternal( unsigned int notifyListOwnerId, unsigned int startLocalId, unsigned int notifyListId, unsigned int notifyNameListId, unsigned int stringValue )
{
	assert(stringValue == Scr_GetThreadNotifyName( startLocalId ));
	assert(notifyListId == FindObject( FindVariable( notifyListOwnerId, OBJECT_NOTIFY_LIST ) ));
	assert(notifyNameListId == FindObject( FindVariable( notifyListId, stringValue ) ));

	Scr_RemoveThreadNotifyName(startLocalId);
	RemoveObjectVariable(notifyNameListId, startLocalId);

	if ( GetArraySize(notifyNameListId) )
	{
		return;
	}

	RemoveVariable(notifyListId, stringValue);

	if ( GetArraySize(notifyListId) )
	{
		return;
	}

	RemoveVariable(notifyListOwnerId, OBJECT_NOTIFY_LIST);
}

/*
==============
Scr_CancelWaittill
==============
*/
void Scr_CancelWaittill( unsigned int startLocalId )
{
	unsigned int id, selfNameId, selfId;

	selfId = Scr_GetSelf(startLocalId);
	id = FindObjectVariable(scrVarPub.pauseArrayId, selfId);

	selfNameId = FindObject(id);
	id = FindObjectVariable(selfNameId, startLocalId);

	VM_CancelNotify(GetVariableValueAddress(id)->pointerValue, startLocalId);
	RemoveObjectVariable(selfNameId, startLocalId);

	if ( GetArraySize(selfNameId) )
	{
		return;
	}

	RemoveObjectVariable(scrVarPub.pauseArrayId, selfId);
}

/*
==============
VM_TrimStack
==============
*/
void VM_TrimStack( unsigned int startLocalId, VariableStackBuffer *stackValue, bool fromEndon )
{
	VariableValue tempValue;
	int size;
	char *buf;
	VariableValue value;
	unsigned int parentLocalId, localId;

	assert(startLocalId);

	size = stackValue->size;
	localId = stackValue->localId;
	buf = &stackValue->buf[STACKBUF_BUFFER_SIZE * size];

	while ( size )
	{
		buf -= sizeof(VariableUnion);
		value.u.codePosValue = *(const char **)buf;

		buf -= sizeof(unsigned char);
		value.type = *(unsigned char *)buf;

		size--;

		if ( value.type != VAR_CODEPOS )
		{
			RemoveRefToValue(&value);
			continue;
		}

		if ( !FindObjectVariable(scrVarPub.pauseArrayId, localId) )
		{
			parentLocalId = GetParentLocalId(localId);
			Scr_KillThread(localId);

			RemoveRefToObject(localId);
			localId = parentLocalId;

			continue;
		}

		assert(startLocalId != localId);

		stackValue->localId = localId;
		stackValue->size = size + 1;

		Scr_StopThread(localId);

		if ( fromEndon )
		{
			continue;
		}

		Scr_SetThreadNotifyName(startLocalId, 0);
		stackValue->pos = 0;

		tempValue.type = VAR_STACK;
		tempValue.u.stackValue = stackValue;

		SetNewVariableValue(GetNewVariable(startLocalId, OBJECT_STACK), &tempValue);
		return;
	}

	assert(startLocalId == localId);

	if ( fromEndon )
	{
		RemoveVariable(startLocalId, OBJECT_STACK);
	}

	Scr_KillThread(startLocalId);
	RemoveRefToObject(startLocalId);

	MT_Free(stackValue, stackValue->bufLen);
}



















































































VariableValue* Scr_GetValue(unsigned int param)
{
	return &scrVmPub.top[int(-param)];
}














void IncInParam()
{
	Scr_ClearOutParams();

	if ( scrVmPub.top == scrVmPub.maxstack )
		Com_Error(ERR_DROP, "Internal script stack overflow");

	++scrVmPub.top;
	++scrVmPub.inparamcount;
}













































































void GetEntityFieldValue_Bad(VariableValue *pValue, unsigned int classnum, int entnum, int offset)
{
	scrVmPub.top = scrVmGlob.eval_stack - 1;
	scrVmGlob.eval_stack->type = VAR_UNDEFINED;
	Scr_GetObjectField(classnum, entnum, offset);
	scrVmPub.inparamcount = 0;
	pValue->u = scrVmGlob.eval_stack->u;
	pValue->type = scrVmGlob.eval_stack->type;
}









void VM_TerminateStack(unsigned int endLocalId, unsigned int startLocalId, VariableStackBuffer *stackValue)
{
	VariableValue tempValue;
	unsigned char type;
	int size;
	char *buf;
	VariableUnion u;
	unsigned int parentLocalId;
	unsigned int localId;

	size = stackValue->size;
	localId = stackValue->localId;
	buf = &stackValue->buf[5 * size];

	while ( size )
	{
		buf -= 4;
		u.codePosValue = *(const char **)buf--;
		type = *buf;
		--size;

		if ( type == VAR_CODEPOS )
		{
			parentLocalId = GetParentLocalId(localId);
			Scr_KillThread(localId);
			RemoveRefToObject(localId);

			if ( localId == endLocalId )
			{
				++size;
				*buf = 0;
				Scr_SetThreadWaitTime(startLocalId, scrVarPub.time);
				stackValue->pos = u.codePosValue;
				stackValue->localId = parentLocalId;
				stackValue->size = size;
				tempValue.type = VAR_STACK;
				tempValue.u.stackValue = stackValue;
				SetNewVariableValue(GetNewObjectVariable(GetArray(GetVariable(scrVarPub.timeArrayId, scrVarPub.time)), startLocalId), &tempValue);
				return;
			}

			localId = parentLocalId;
		}
		else
		{
			RemoveRefToValue(type, u);
		}
	}

	Scr_KillThread(localId);
	RemoveRefToObject(localId);
	MT_Free(stackValue, stackValue->bufLen);
}







void VM_TerminateTime(unsigned int timeId)
{
	VariableStackBuffer *stackValue;
	unsigned int startLocalId;
	unsigned int stackId;

	AddRefToObject(timeId);

	while ( 1 )
	{
		stackId = FindNextSibling(timeId);

		if ( !stackId )
			break;

		startLocalId = GetVariableKeyObject(stackId);
		stackValue = GetVariableValueAddress_Bad(stackId)->u.stackValue;
		RemoveObjectVariable(timeId, startLocalId);
		Scr_ClearWaitTime(startLocalId);
		VM_TerminateStack(startLocalId, startLocalId, stackValue);
	}

	RemoveRefToObject(timeId);
}

void Scr_TerminateWaittillThread(unsigned int localId, unsigned int startLocalId)
{
	unsigned short ThreadNotifyName;
	unsigned int object;
	unsigned int ObjectVariable;
	unsigned int variable;
	unsigned int var;
	VariableStackBuffer *stackValue;
	unsigned int stackId;
	unsigned int var2;
	unsigned int notifyListId;
	unsigned int notifyNameListId;
	unsigned int notifyListOwnerId;
	unsigned int stringValue;
	unsigned int selfNameId;
	unsigned int selfId;

	ThreadNotifyName = Scr_GetThreadNotifyName(startLocalId);
	stringValue = ThreadNotifyName;

	if ( ThreadNotifyName )
	{
		selfId = Scr_GetSelf(startLocalId);
		object = FindObjectVariable(scrVarPub.pauseArrayId, selfId);
		selfNameId = FindObject(object);
		ObjectVariable = FindObjectVariable(selfNameId, startLocalId);
		notifyListOwnerId = GetVariableValueAddress_Bad(ObjectVariable)->u.pointerValue;
		variable = FindVariable(notifyListOwnerId, 0x1FFFEu);
		notifyListId = FindObject(variable);
		var = FindVariable(notifyListId, stringValue);
		notifyNameListId = FindObject(var);
		stackId = FindObjectVariable(notifyNameListId, startLocalId);
		stackValue = GetVariableValueAddress_Bad(stackId)->u.stackValue;
		VM_CancelNotifyInternal(notifyListOwnerId, startLocalId, notifyListId, notifyNameListId, stringValue);
		RemoveObjectVariable(selfNameId, startLocalId);

		if ( !GetArraySize(selfNameId) )
			RemoveObjectVariable(scrVarPub.pauseArrayId, selfId);
	}
	else
	{
		var2 = FindVariable(startLocalId, 0x1FFFFu);
		stackValue = GetVariableValueAddress_Bad(var2)->u.stackValue;
		RemoveVariable(startLocalId, 0x1FFFFu);
	}

	VM_TerminateStack(localId, startLocalId, stackValue);
}

void Scr_TerminateWaitThread(unsigned int localId, unsigned int startLocalId)
{
	unsigned int variable;
	VariableStackBuffer *stackValue;
	unsigned int object;
	unsigned int time;
	unsigned int parentId;

	time = Scr_GetThreadWaitTime(startLocalId);
	Scr_ClearWaitTime(startLocalId);
	variable = FindVariable(scrVarPub.timeArrayId, time);
	parentId = FindObject(variable);
	object = FindObjectVariable(parentId, startLocalId);
	stackValue = GetVariableValueAddress_Bad(object)->u.stackValue;
	RemoveObjectVariable(parentId, startLocalId);

	if ( !GetArraySize(parentId) && time != scrVarPub.time )
		RemoveVariable(scrVarPub.timeArrayId, time);

	VM_TerminateStack(localId, startLocalId, stackValue);
}



void Scr_TerminateThread(unsigned int localId)
{
	int type;
	int startLocalId;

	startLocalId = GetStartLocalId(localId);
	type = GetObjectType(startLocalId);

	if ( type == VAR_NOTIFY_THREAD )
	{
		Scr_TerminateWaittillThread(localId, startLocalId);
	}
	else if ( type > VAR_NOTIFY_THREAD )
	{
		if ( type == VAR_TIME_THREAD )
			Scr_TerminateWaitThread(localId, startLocalId);
	}
	else if ( type == VAR_THREAD )
	{
		Scr_TerminateRunningThread(localId);
	}
}









void VM_Notify(unsigned int notifyListOwnerId, unsigned int stringValue, VariableValue *top)
{
	unsigned int localId;
	int type;
	unsigned int variable;
	unsigned int array;
	unsigned int newObject;
	VariableValue stackValue;
	VariableValue value;
	VariableValue value2;
	bool bNoStack;
	int notifyListEntry;
	unsigned int selfNameId;
	unsigned int selfId;
	int bufLen;
	size_t len;
	int newSize;
	int size;
	char *buf;
	VariableStackBuffer *stackBuf;
	VariableStackBuffer *newStackBuf;
	VariableValueInternal_u *stackId;
	VariableValue *vars;
	unsigned int startLocalId;
	unsigned int notifyListId;
	unsigned int notifyNameListId;

	notifyListId = FindVariable(notifyListOwnerId, 0x1FFFEu);

	if ( notifyListId )
	{
		notifyListId = FindObject(notifyListId);
		notifyNameListId = FindVariable(notifyListId, stringValue);

		if ( notifyNameListId )
		{
			notifyNameListId = FindObject(notifyNameListId);
			AddRefToObject(notifyNameListId);
			scrVarPub.evaluate = 1;
			notifyListEntry = notifyNameListId;
next:
			while ( 1 )
			{
				notifyListEntry = FindPrevSibling(notifyListEntry);

				if ( !notifyListEntry )
					break;

				startLocalId = GetVariableKeyObject(notifyListEntry);
				selfId = Scr_GetSelf(startLocalId);
				localId = FindObjectVariable(scrVarPub.pauseArrayId, selfId);
				selfNameId = FindObject(localId);

				if ( GetObjectType(notifyListEntry) )
				{
					stackId = GetVariableValueAddress_Bad(notifyListEntry);
					newStackBuf = stackId->u.stackValue;

					if ( *((byte *)newStackBuf->pos - 1) == OP_waittillmatch )
					{
						size = *newStackBuf->pos;
						buf = &newStackBuf->buf[5 * (newStackBuf->size - size)];

						for ( vars = top; size; --vars )
						{
							if ( vars->type == VAR_PRECODEPOS )
								goto next;

							--size;
							value.type = (unsigned char)*buf;

							if ( value.type == VAR_PRECODEPOS )
								break;

							value.u.intValue = *(int *)++buf;
							buf += 4;
							AddRefToValue(&value);
							type = vars->type;
							value2.u = vars->u;
							value2.type = type;
							AddRefToValue(&value2);
							Scr_EvalEquality(&value, &value2);

							if ( scrVarPub.error_message )
							{
								RuntimeError(
								    newStackBuf->pos,
								    *newStackBuf->pos - size + 3,
								    scrVarPub.error_message,
								    scrVmGlob.dialog_error_message);
								Scr_ClearErrorMessage();
								goto next;
							}

							if ( !value.u.intValue )
								goto next;
						}

						++newStackBuf->pos;
						bNoStack = 1;
					}
					else
					{
						bNoStack = top->type == VAR_PRECODEPOS;
					}

					stackValue.type = VAR_STACK;
					stackValue.u.stackValue = newStackBuf;
					variable = GetVariable(scrVarPub.timeArrayId, scrVarPub.time);
					array = GetArray(variable);
					newObject = GetNewObjectVariable(array, startLocalId);
					SetNewVariableValue(newObject, &stackValue);
					stackId = GetVariableValueAddress_Bad(newObject);
					VM_CancelNotifyInternal(notifyListOwnerId, startLocalId, notifyListId, notifyNameListId, stringValue);
					RemoveObjectVariable(selfNameId, startLocalId);

					if ( !GetArraySize(selfNameId) )
						RemoveObjectVariable(scrVarPub.pauseArrayId, selfId);

					Scr_SetThreadWaitTime(startLocalId, scrVarPub.time);

					if ( bNoStack )
					{
						notifyListEntry = notifyNameListId;
					}
					else
					{
						size = newStackBuf->size;
						newSize = size;
						vars = top;

						do
						{
							++newSize;
							--vars;
						}
						while ( vars->type != VAR_PRECODEPOS );

						len = 5 * size;
						bufLen = 5 * newSize + sizeof(VariableStackBuffer);

						if ( !MT_Realloc(newStackBuf->bufLen, bufLen) )
						{
							stackBuf = (VariableStackBuffer *)MT_Alloc(bufLen);
							stackBuf->bufLen = bufLen;
							stackBuf->pos = newStackBuf->pos;
							stackBuf->localId = newStackBuf->localId;
							memcpy(stackBuf->buf, newStackBuf->buf, len);
							MT_Free(newStackBuf, newStackBuf->bufLen);
							newStackBuf = stackBuf;
							stackId->u.stackValue = stackBuf;
						}

						newStackBuf->size = newSize;
						buf = &newStackBuf->buf[len];
						newSize -= size;

						do
						{
							AddRefToValue(++vars);
							*buf++ = vars->type;
							*(int *)buf = vars->u.intValue;
							buf += 4;
							--newSize;
						}

						while ( newSize );
						notifyListEntry = notifyNameListId;
					}
				}
				else
				{
					VM_CancelNotifyInternal(notifyListOwnerId, startLocalId, notifyListId, notifyNameListId, stringValue);
					Scr_KillEndonThread(startLocalId);
					RemoveObjectVariable(selfNameId, startLocalId);

					if ( !GetArraySize(selfNameId) )
						RemoveObjectVariable(scrVarPub.pauseArrayId, selfId);

					Scr_TerminateThread(selfId);
					notifyListEntry = notifyNameListId;
				}
			}

			RemoveRefToObject(notifyNameListId);
			scrVarPub.evaluate = 0;
		}
	}
}





















unsigned int VM_ExecuteInternal(const char *pos, unsigned int localId, unsigned int localVarCount, VariableValue *top, VariableValue *startTop)
{
	int gOpcode;
	int gParamCount;
	unsigned int parentLocalId;
	unsigned int builtinIndex;
	VariableValue stackValue;
	int jumpOffset;
	unsigned int waitTime;
	unsigned int stringValue;
	unsigned int id;
	unsigned int threadId;
	const char *tempCodePos;
	VariableValue tempValue;
	unsigned int outparamcount;
	unsigned int selfId;
	unsigned int objectId;
	unsigned int fieldValueId;
	int gCaseCount;
	unsigned int caseValue;
	unsigned int currentCaseValue;
	unsigned int classnum;
	int entnum;
	const char *currentCodePos;
	unsigned char removeCount;
	scr_entref_t entref;
	unsigned int stackId;

	gParamCount = 0;

	if ( setjmp(g_script_error[++g_script_error_level]) )
	{
		switch ( gOpcode )
		{
		case OP_EvalLocalArrayRefCached0:
		case OP_EvalLocalArrayRefCached:
		case OP_EvalArrayRef:
		case OP_ClearArray:
		case OP_EvalLocalVariableRef:
			if ( scrVarPub.error_index < 0 )
				scrVarPub.error_index = 1;
			break;

		case OP_EvalSelfFieldVariable:
		case OP_EvalFieldVariable:
		case OP_ClearFieldVariable:
		case OP_SetVariableField:
		case OP_SetSelfFieldVariableField:
		case OP_inc:
		case OP_dec:
			scrVarPub.error_index = 0;
			break;

		case OP_CallBuiltin0:
		case OP_CallBuiltin1:
		case OP_CallBuiltin2:
		case OP_CallBuiltin3:
		case OP_CallBuiltin4:
		case OP_CallBuiltin5:
		case OP_CallBuiltin:
			if ( scrVarPub.error_index > 0 )
				scrVarPub.error_index = scrVmPub.outparamcount - scrVarPub.error_index + 1;
			break;

		case OP_CallBuiltinMethod0:
		case OP_CallBuiltinMethod1:
		case OP_CallBuiltinMethod2:
		case OP_CallBuiltinMethod3:
		case OP_CallBuiltinMethod4:
		case OP_CallBuiltinMethod5:
		case OP_CallBuiltinMethod:
			if ( scrVarPub.error_index <= 0 )
			{
				if ( scrVarPub.error_index < 0 )
					scrVarPub.error_index = 1;
			}
			else
			{
				scrVarPub.error_index = scrVmPub.outparamcount - scrVarPub.error_index + 2;
			}
			break;

		default:
			break;
		}

		RuntimeError(pos, scrVarPub.error_index, scrVarPub.error_message, scrVmGlob.dialog_error_message);
		Scr_ClearErrorMessage();

		switch ( gOpcode )
		{
		case OP_EvalLocalArrayCached:
		case OP_EvalArray:
			RemoveRefToValue(top--);
			RemoveRefToValue(top);
			top->type = VAR_UNDEFINED;
			break;

		case OP_EvalLocalArrayRefCached0:
		case OP_EvalLocalArrayRefCached:
		case OP_EvalArrayRef:
		case OP_EvalLocalVariableRef:
			fieldValueId = GetDummyFieldValue();
			RemoveRefToValue(top);
			--top;
			break;

		case OP_ClearArray:
		case OP_wait:
			RemoveRefToValue(top);
			--top;
			break;

		case OP_GetSelfObject:
			objectId = GetDummyObject();
			break;

		case OP_EvalSelfFieldVariable:
		case OP_EvalFieldVariable:
			top->type = VAR_UNDEFINED;
			break;

		case OP_EvalSelfFieldVariableRef:
		case OP_EvalFieldVariableRef:
			fieldValueId = GetDummyFieldValue();
			break;

		case OP_ClearFieldVariable:
			if ( scrVmPub.outparamcount )
				scrVmPub.outparamcount = 0;
			break;

		case OP_checkclearparams:
			while ( top->type != VAR_PRECODEPOS )
				RemoveRefToValue(top--);
			top->type = VAR_CODEPOS;
			break;

		case OP_SetVariableField:
			if ( scrVmPub.outparamcount )
			{
				RemoveRefToValue(top);
				scrVmPub.outparamcount = 0;
				--top;
				break;
			}
			--top;
			break;

		case OP_SetSelfFieldVariableField:
			RemoveRefToValue(top);
			scrVmPub.outparamcount = 0;
			--top;
			break;

		case OP_CallBuiltin0:
		case OP_CallBuiltin1:
		case OP_CallBuiltin2:
		case OP_CallBuiltin3:
		case OP_CallBuiltin4:
		case OP_CallBuiltin5:
		case OP_CallBuiltin:
		case OP_CallBuiltinMethod0:
		case OP_CallBuiltinMethod1:
		case OP_CallBuiltinMethod2:
		case OP_CallBuiltinMethod3:
		case OP_CallBuiltinMethod4:
		case OP_CallBuiltinMethod5:
		case OP_CallBuiltinMethod:
			Scr_ClearOutParams();
			top = scrVmPub.top + 1;
			scrVmPub.top[1].type = VAR_UNDEFINED;
			break;

		case OP_ScriptFunctionCall2:
		case OP_ScriptFunctionCall:
		case OP_ScriptMethodCall:
			Scr_ReadCodePos(&pos);
			while ( top->type != VAR_PRECODEPOS )
				RemoveRefToValue(top--);
			top->type = VAR_UNDEFINED;
			break;

		case OP_ScriptFunctionCallPointer:
		case OP_ScriptMethodCallPointer:
			while ( top->type != VAR_PRECODEPOS )
				RemoveRefToValue(top--);
			top->type = VAR_UNDEFINED;
			break;

		case OP_ScriptThreadCall:
		case OP_ScriptMethodThreadCall:
			Scr_ReadCodePos(&pos);
			for ( outparamcount = Scr_ReadUnsigned(&pos); outparamcount; --outparamcount )
				RemoveRefToValue(top--);
			++top;
			top->type = VAR_UNDEFINED;
			break;

		case OP_ScriptThreadCallPointer:
		case OP_ScriptMethodThreadCallPointer:
			for ( outparamcount = Scr_ReadUnsigned(&pos); outparamcount; --outparamcount )
				RemoveRefToValue(top--);
			++top;
			top->type = VAR_UNDEFINED;
			break;

		case OP_CastFieldObject:
			objectId = GetDummyObject();
			--top;
			break;

		case OP_EvalLocalVariableObjectCached:
			++pos;
			objectId = GetDummyObject();
			break;

		case OP_JumpOnFalse:
		case OP_JumpOnTrue:
		case OP_JumpOnFalseExpr:
		case OP_JumpOnTrueExpr:
			Scr_ReadUnsignedShort(&pos);
			--top;
			break;

		case OP_jumpback:
			jumpOffset = Scr_ReadUnsignedShort(&pos);
			pos -= jumpOffset;
			break;

		case OP_bit_or:
		case OP_bit_ex_or:
		case OP_bit_and:
		case OP_equality:
		case OP_inequality:
		case OP_less:
		case OP_greater:
		case OP_less_equal:
		case OP_greater_equal:
		case OP_shift_left:
		case OP_shift_right:
		case OP_plus:
		case OP_minus:
		case OP_multiply:
		case OP_divide:
		case OP_mod:
			--top;
			break;

		case OP_waittillmatch:
			++pos;
			RemoveRefToValue(top--);
			RemoveRefToValue(top);
			--top;
			break;

		case OP_waittill:
		case OP_endon:
			RemoveRefToValue(top--);
			RemoveRefToValue(top);
			--top;
			break;

		case OP_notify:
			while ( top->type != VAR_PRECODEPOS )
				RemoveRefToValue(top--);
			RemoveRefToValue(top);
			--top;
			break;

		case OP_switch:
			while ( gCaseCount )
			{
				currentCaseValue = Scr_ReadUnsigned(&pos);
				currentCodePos = Scr_ReadCodePos(&pos);
				--gCaseCount;
			}
			if ( !currentCaseValue )
				pos = currentCodePos;
			RemoveRefToValue(top);
			--top;
			break;

		default:
			break;
		}
	}

	while ( 1 )
	{
		gOpcode = *(unsigned char *)pos++;

		switch ( gOpcode )
		{
		case OP_End:
			parentLocalId = GetSafeParentLocalId(localId);
			Scr_KillThread(localId);
			scrVmPub.localVars -= localVarCount;
			while ( top->type != VAR_CODEPOS )
				RemoveRefToValue(top--);
			--scrVmPub.function_count;
			--scrVmPub.function_frame;
			if ( !parentLocalId )
			{
				startTop[1].type = VAR_UNDEFINED;
				if ( gParamCount )
				{
					--gParamCount;
					RemoveRefToObject(localId);
					pos = scrVmPub.function_frame->fs.pos;
					localId = scrVmPub.function_frame->fs.localId;
					localVarCount = scrVmPub.function_frame->fs.localVarCount;
					top = scrVmPub.function_frame->fs.top;
					startTop = scrVmPub.function_frame->fs.startTop;
					top->type = scrVmPub.function_frame->topType;
					++top;
					continue;
				}
				--g_script_error_level;
				return localId;
			}
			top->type = VAR_UNDEFINED;
			RemoveRefToObject(localId);
			pos = scrVmPub.function_frame->fs.pos;
			localVarCount = scrVmPub.function_frame->fs.localVarCount;
			localId = parentLocalId;
			continue;

		case OP_Return:
			parentLocalId = GetSafeParentLocalId(localId);
			Scr_KillThread(localId);
			scrVmPub.localVars -= localVarCount;
			tempValue.u = top->u;
			tempValue.type = top->type;
			for ( --top; top->type != VAR_CODEPOS; --top )
				RemoveRefToValue(top);
			--scrVmPub.function_count;
			--scrVmPub.function_frame;
			if ( !parentLocalId )
			{
				top[1].u = tempValue.u;
				top[1].type = tempValue.type;
				if ( gParamCount )
				{
					--gParamCount;
					RemoveRefToObject(localId);
					pos = scrVmPub.function_frame->fs.pos;
					localId = scrVmPub.function_frame->fs.localId;
					localVarCount = scrVmPub.function_frame->fs.localVarCount;
					top = scrVmPub.function_frame->fs.top;
					startTop = scrVmPub.function_frame->fs.startTop;
					top->type = scrVmPub.function_frame->topType;
					++top;
					continue;
				}
				--g_script_error_level;
				return localId;
			}
			top->u = tempValue.u;
			top->type = tempValue.type;
			RemoveRefToObject(localId);
			pos = scrVmPub.function_frame->fs.pos;
			localVarCount = scrVmPub.function_frame->fs.localVarCount;
			localId = parentLocalId;
			continue;

		case OP_GetUndefined:
			++top;
			top->type = VAR_UNDEFINED;
			continue;

		case OP_GetZero:
			++top;
			top->type = VAR_INTEGER;
			top->u.intValue = 0;
			continue;

		case OP_GetByte:
			++top;
			top->type = VAR_INTEGER;
			top->u.intValue = *(unsigned char *)pos++;
			continue;

		case OP_GetNegByte:
			++top;
			top->type = VAR_INTEGER;
			top->u.intValue = -*(unsigned char *)pos++;
			continue;

		case OP_GetUnsignedShort:
			++top;
			top->type = VAR_INTEGER;
			top->u.intValue = Scr_ReadUnsignedShort(&pos);
			continue;

		case OP_GetNegUnsignedShort:
			++top;
			top->type = VAR_INTEGER;
			top->u.intValue = -Scr_ReadUnsignedShort(&pos);
			continue;

		case OP_GetInteger:
			++top;
			top->type = VAR_INTEGER;
			top->u.intValue = Scr_ReadUnsigned(&pos);
			continue;

		case OP_GetFloat:
			++top;
			top->type = VAR_FLOAT;
			top->u.floatValue = Scr_ReadFloat(&pos);
			continue;

		case OP_GetString:
			++top;
			top->type = VAR_STRING;
			top->u.stringValue = Scr_ReadUnsignedShort(&pos);
			SL_AddRefToString(top->u.stringValue);
			continue;

		case OP_GetIString:
			++top;
			top->type = VAR_ISTRING;
			top->u.stringValue = Scr_ReadUnsignedShort(&pos);
			SL_AddRefToString(top->u.stringValue);
			continue;

		case OP_GetVector:
			++top;
			top->type = VAR_VECTOR;
			top->u.vectorValue = Scr_ReadVector(&pos);
			continue;

		case OP_GetLevelObject:
			objectId = scrVarPub.levelId;
			continue;

		case OP_GetAnimObject:
			objectId = scrVarPub.animId;
			continue;

		case OP_GetSelf:
			++top;
			top->type = VAR_POINTER;
			top->u.pointerValue = Scr_GetSelf(localId);
			AddRefToObject(top->u.pointerValue);
			continue;

		case OP_GetLevel:
			++top;
			top->type = VAR_POINTER;
			top->u.pointerValue = scrVarPub.levelId;
			AddRefToObject(scrVarPub.levelId);
			continue;

		case OP_GetGame:
			++top;
			tempValue = Scr_EvalVariable(scrVarPub.gameId);
			top->u = tempValue.u;
			top->type = tempValue.type;
			continue;

		case OP_GetAnim:
			++top;
			top->type = VAR_POINTER;
			top->u.pointerValue = scrVarPub.animId;
			AddRefToObject(scrVarPub.animId);
			continue;

		case OP_GetAnimation:
			++top;
			top->type = VAR_ANIMATION;
			top->u.pointerValue = Scr_ReadUnsigned(&pos);
			continue;

		case OP_GetGameRef:
			fieldValueId = scrVarPub.gameId;
			continue;

		case OP_GetFunction:
			++top;
			top->type = VAR_FUNCTION;
			top->u.codePosValue = Scr_ReadCodePos(&pos);
			continue;

		case OP_CreateLocalVariable:
			++scrVmPub.localVars;
			++localVarCount;
			scrVmPub.localVars[0] = GetNewVariable(localId, Scr_ReadUnsignedShort(&pos));
			continue;

		case OP_RemoveLocalVariables:
			removeCount = *pos++;
			scrVmPub.localVars -= removeCount;
			localVarCount -= removeCount;
			while ( removeCount )
			{
				RemoveNextVariable(localId);
				--removeCount;
			}
			continue;

		case OP_EvalLocalVariableCached0:
			++top;
			tempValue = Scr_EvalVariable(scrVmPub.localVars[0]);
			top->u = tempValue.u;
			top->type = tempValue.type;
			continue;

		case OP_EvalLocalVariableCached1:
			++top;
			tempValue = Scr_EvalVariable(scrVmPub.localVars[-1]);
			top->u = tempValue.u;
			top->type = tempValue.type;
			continue;

		case OP_EvalLocalVariableCached2:
			++top;
			tempValue = Scr_EvalVariable(scrVmPub.localVars[-2]);
			top->u = tempValue.u;
			top->type = tempValue.type;
			continue;

		case OP_EvalLocalVariableCached3:
			++top;
			tempValue = Scr_EvalVariable(scrVmPub.localVars[-3]);
			top->u = tempValue.u;
			top->type = tempValue.type;
			continue;

		case OP_EvalLocalVariableCached4:
			++top;
			tempValue = Scr_EvalVariable(scrVmPub.localVars[-4]);
			top->u = tempValue.u;
			top->type = tempValue.type;
			continue;

		case OP_EvalLocalVariableCached5:
			++top;
			tempValue = Scr_EvalVariable(scrVmPub.localVars[-5]);
			top->u = tempValue.u;
			top->type = tempValue.type;
			continue;

		case OP_EvalLocalVariableCached:
			++top;
			tempValue = Scr_EvalVariable(Scr_GetLocalVar(pos));
			top->u = tempValue.u;
			top->type = tempValue.type;
			++pos;
			continue;

		case OP_EvalLocalArrayCached:
			++top;
			tempValue = Scr_EvalVariable(Scr_GetLocalVar(pos));
			top->u = tempValue.u;
			top->type = tempValue.type;
			++pos;
			Scr_EvalArray(top, top - 1);
			--top;
			continue;

		case OP_EvalArray:
			Scr_EvalArray(top, top - 1);
			--top;
			continue;

		case OP_EvalLocalArrayRefCached0:
			fieldValueId = scrVmPub.localVars[0];
			fieldValueId = Scr_EvalArrayIndex(Scr_EvalArrayRef(fieldValueId), top);
			--top;
			continue;

		case OP_EvalLocalArrayRefCached:
			fieldValueId = Scr_GetLocalVar(pos++);
			fieldValueId = Scr_EvalArrayIndex(Scr_EvalArrayRef(fieldValueId), top);
			--top;
			continue;

		case OP_EvalArrayRef:
			fieldValueId = Scr_EvalArrayIndex(Scr_EvalArrayRef(fieldValueId), top);
			--top;
			continue;

		case OP_ClearArray:
			ClearArray(fieldValueId, top);
			--top;
			continue;

		case OP_EmptyArray:
			++top;
			top->type = VAR_POINTER;
			top->u.pointerValue = Scr_AllocArray();
			continue;

		case OP_GetSelfObject:
			objectId = Scr_GetSelf(localId);
			if ( IsFieldObject(objectId) )
				continue;
			Scr_Error(va("%s is not an object", var_typename[GetObjectType(objectId)]));

		case OP_EvalLevelFieldVariable:
			objectId = scrVarPub.levelId;
			++top;
			tempValue = Scr_EvalVariable(FindVariable(objectId, Scr_ReadUnsignedShort(&pos)));
			top->u = tempValue.u;
			top->type = tempValue.type;
			continue;

		case OP_EvalAnimFieldVariable:
			objectId = scrVarPub.animId;
			++top;
			tempValue = Scr_EvalVariable(FindVariable(objectId, Scr_ReadUnsignedShort(&pos)));
			top->u = tempValue.u;
			top->type = tempValue.type;
			continue;

		case OP_EvalSelfFieldVariable:
			objectId = Scr_GetSelf(localId);
			if ( IsFieldObject(objectId) )
			{
				++top;
				tempValue = Scr_FindVariableField(objectId, Scr_ReadUnsignedShort(&pos));
				top->u = tempValue.u;
				top->type = tempValue.type;
				continue;
			}
			++top;
			Scr_ReadUnsignedShort(&pos);
			Scr_Error(va("%s is not an object", var_typename[GetObjectType(objectId)]));

		case OP_EvalFieldVariable:
			++top;
			tempValue = Scr_FindVariableField(objectId, Scr_ReadUnsignedShort(&pos));
			top->u = tempValue.u;
			top->type = tempValue.type;
			continue;

		case OP_EvalLevelFieldVariableRef:
			objectId = scrVarPub.levelId;
			fieldValueId = Scr_GetVariableField(objectId, Scr_ReadUnsignedShort(&pos));
			continue;

		case OP_EvalAnimFieldVariableRef:
			objectId = scrVarPub.animId;
			fieldValueId = Scr_GetVariableField(objectId, Scr_ReadUnsignedShort(&pos));
			continue;

		case OP_EvalSelfFieldVariableRef:
			objectId = Scr_GetSelf(localId);
			fieldValueId = Scr_GetVariableField(objectId, Scr_ReadUnsignedShort(&pos));
			continue;

		case OP_EvalFieldVariableRef:
			fieldValueId = Scr_GetVariableField(objectId, Scr_ReadUnsignedShort(&pos));
			continue;

		case OP_ClearFieldVariable:
			ClearVariableField(objectId, Scr_ReadUnsignedShort(&pos), top);
			continue;

		case OP_SafeCreateVariableFieldCached:
			++scrVmPub.localVars;
			++localVarCount;
			scrVmPub.localVars[0] = GetNewVariable(localId, Scr_ReadUnsignedShort(&pos));
			if ( top->type != VAR_PRECODEPOS )
			{
				SetVariableValue(scrVmPub.localVars[0], top);
				--top;
				continue;
			}
			continue;

		case OP_SafeSetVariableFieldCached0:
			if ( top->type != VAR_PRECODEPOS )
			{
				SetVariableValue(scrVmPub.localVars[0], top);
				--top;
				continue;
			}
			continue;

		case OP_SafeSetVariableFieldCached:
			if ( top->type != VAR_PRECODEPOS )
			{
				SetVariableValue(Scr_GetLocalVar(pos), top);
				++pos;
				--top;
				continue;
			}
			++pos;
			continue;

		case OP_SafeSetWaittillVariableFieldCached:
			if ( top->type != VAR_CODEPOS )
			{
				SetVariableValue(Scr_GetLocalVar(pos), top);
				++pos;
				--top;
				continue;
			}
			ClearVariableValue(Scr_GetLocalVar(pos));
			++pos;
			continue;

		case OP_clearparams:
			while ( top->type != VAR_CODEPOS )
				RemoveRefToValue(top--);
			continue;

		case OP_checkclearparams:
			if ( top->type == VAR_PRECODEPOS )
			{
				top->type = VAR_CODEPOS;
			}
			else
			{
				Scr_Error("function called with too many parameters");
			}
			continue;

		case OP_EvalLocalVariableRefCached0:
			fieldValueId = scrVmPub.localVars[0];
			continue;

		case OP_EvalLocalVariableRefCached:
			fieldValueId = Scr_GetLocalVar(pos++);
			continue;

		case OP_SetLevelFieldVariableField:
			SetVariableValue(GetVariable(scrVarPub.levelId, Scr_ReadUnsignedShort(&pos)), top);
			--top;
			continue;

		case OP_SetVariableField:
			SetVariableFieldValue(fieldValueId, top);
			--top;
			continue;

		case OP_SetAnimFieldVariableField:
			SetVariableValue(GetVariable(scrVarPub.animId, Scr_ReadUnsignedShort(&pos)), top);
			--top;
			continue;

		case OP_SetSelfFieldVariableField:
			objectId = Scr_GetSelf(localId);
			fieldValueId = Scr_GetVariableField(objectId, Scr_ReadUnsignedShort(&pos));
			SetVariableFieldValue(fieldValueId, top);
			--top;
			continue;

		case OP_SetLocalVariableFieldCached0:
			SetVariableValue(scrVmPub.localVars[0], top);
			--top;
			continue;

		case OP_SetLocalVariableFieldCached:
			SetVariableValue(Scr_GetLocalVar(pos), top);
			++pos;
			--top;
			continue;

		case OP_CallBuiltin0:
			scrVmPub.top = top;
			builtinIndex = Scr_ReadUnsignedShort(&pos);
			scrVmPub.function_frame->fs.pos = pos;
			((void (*)(void))scrCompilePub.func_table[builtinIndex])();
			top = scrVmPub.top;
			pos = scrVmPub.function_frame->fs.pos;
			if ( scrVmPub.outparamcount )
			{
				outparamcount = scrVmPub.outparamcount;
				scrVmPub.outparamcount = 0;
				scrVmPub.top -= outparamcount;
				do
				{
					RemoveRefToValue(top--);
					--outparamcount;
				}
				while ( outparamcount );
			}
			if ( scrVmPub.inparamcount )
			{
				scrVmPub.inparamcount = 0;
			}
			else
			{
				++top;
				top->type = VAR_UNDEFINED;
			}
			continue;

		case OP_CallBuiltin1:
			scrVmPub.outparamcount = 1;
			scrVmPub.top = top;
			builtinIndex = Scr_ReadUnsignedShort(&pos);
			scrVmPub.function_frame->fs.pos = pos;
			((void (*)(void))scrCompilePub.func_table[builtinIndex])();
			top = scrVmPub.top;
			pos = scrVmPub.function_frame->fs.pos;
			if ( scrVmPub.outparamcount )
			{
				outparamcount = scrVmPub.outparamcount;
				scrVmPub.outparamcount = 0;
				scrVmPub.top -= outparamcount;
				do
				{
					RemoveRefToValue(top--);
					--outparamcount;
				}
				while ( outparamcount );
			}
			if ( scrVmPub.inparamcount )
			{
				scrVmPub.inparamcount = 0;
			}
			else
			{
				++top;
				top->type = VAR_UNDEFINED;
			}
			continue;

		case OP_CallBuiltin2:
			scrVmPub.outparamcount = 2;
			scrVmPub.top = top;
			builtinIndex = Scr_ReadUnsignedShort(&pos);
			scrVmPub.function_frame->fs.pos = pos;
			((void (*)(void))scrCompilePub.func_table[builtinIndex])();
			top = scrVmPub.top;
			pos = scrVmPub.function_frame->fs.pos;
			if ( scrVmPub.outparamcount )
			{
				outparamcount = scrVmPub.outparamcount;
				scrVmPub.outparamcount = 0;
				scrVmPub.top -= outparamcount;
				do
				{
					RemoveRefToValue(top--);
					--outparamcount;
				}
				while ( outparamcount );
			}
			if ( scrVmPub.inparamcount )
			{
				scrVmPub.inparamcount = 0;
			}
			else
			{
				++top;
				top->type = VAR_UNDEFINED;
			}
			continue;

		case OP_CallBuiltin3:
			scrVmPub.outparamcount = 3;
			scrVmPub.top = top;
			builtinIndex = Scr_ReadUnsignedShort(&pos);
			scrVmPub.function_frame->fs.pos = pos;
			((void (*)(void))scrCompilePub.func_table[builtinIndex])();
			top = scrVmPub.top;
			pos = scrVmPub.function_frame->fs.pos;
			if ( scrVmPub.outparamcount )
			{
				outparamcount = scrVmPub.outparamcount;
				scrVmPub.outparamcount = 0;
				scrVmPub.top -= outparamcount;
				do
				{
					RemoveRefToValue(top--);
					--outparamcount;
				}
				while ( outparamcount );
			}
			if ( scrVmPub.inparamcount )
			{
				scrVmPub.inparamcount = 0;
			}
			else
			{
				++top;
				top->type = VAR_UNDEFINED;
			}
			continue;

		case OP_CallBuiltin4:
			scrVmPub.outparamcount = 4;
			scrVmPub.top = top;
			builtinIndex = Scr_ReadUnsignedShort(&pos);
			scrVmPub.function_frame->fs.pos = pos;
			((void (*)(void))scrCompilePub.func_table[builtinIndex])();
			top = scrVmPub.top;
			pos = scrVmPub.function_frame->fs.pos;
			if ( scrVmPub.outparamcount )
			{
				outparamcount = scrVmPub.outparamcount;
				scrVmPub.outparamcount = 0;
				scrVmPub.top -= outparamcount;
				do
				{
					RemoveRefToValue(top--);
					--outparamcount;
				}
				while ( outparamcount );
			}
			if ( scrVmPub.inparamcount )
			{
				scrVmPub.inparamcount = 0;
			}
			else
			{
				++top;
				top->type = VAR_UNDEFINED;
			}
			continue;

		case OP_CallBuiltin5:
			scrVmPub.outparamcount = 5;
			scrVmPub.top = top;
			builtinIndex = Scr_ReadUnsignedShort(&pos);
			scrVmPub.function_frame->fs.pos = pos;
			((void (*)(void))scrCompilePub.func_table[builtinIndex])();
			top = scrVmPub.top;
			pos = scrVmPub.function_frame->fs.pos;
			if ( scrVmPub.outparamcount )
			{
				outparamcount = scrVmPub.outparamcount;
				scrVmPub.outparamcount = 0;
				scrVmPub.top -= outparamcount;
				do
				{
					RemoveRefToValue(top--);
					--outparamcount;
				}
				while ( outparamcount );
			}
			if ( scrVmPub.inparamcount )
			{
				scrVmPub.inparamcount = 0;
			}
			else
			{
				++top;
				top->type = VAR_UNDEFINED;
			}
			continue;

		case OP_CallBuiltin:
			scrVmPub.outparamcount = *(unsigned char *)pos++;
			scrVmPub.top = top;
			builtinIndex = Scr_ReadUnsignedShort(&pos);
			scrVmPub.function_frame->fs.pos = pos;
			((void (*)(void))scrCompilePub.func_table[builtinIndex])();
			top = scrVmPub.top;
			pos = scrVmPub.function_frame->fs.pos;
			if ( scrVmPub.outparamcount )
			{
				outparamcount = scrVmPub.outparamcount;
				scrVmPub.outparamcount = 0;
				scrVmPub.top -= outparamcount;
				do
				{
					RemoveRefToValue(top--);
					--outparamcount;
				}
				while ( outparamcount );
			}
			if ( scrVmPub.inparamcount )
			{
				scrVmPub.inparamcount = 0;
			}
			else
			{
				++top;
				top->type = VAR_UNDEFINED;
			}
			continue;

		case OP_CallBuiltinMethod0:
			scrVmPub.top = top - 1;
			builtinIndex = Scr_ReadUnsignedShort(&pos);
			if ( top->type != VAR_POINTER )
			{
				RemoveRefToValue(top);
				scrVarPub.error_index = -1;
				Scr_Error(va("%s is not an entity", var_typename[top->type]));
			}
			objectId = top->u.pointerValue;
			if ( GetObjectType(objectId) == VAR_ENTITY )
			{
				entref = Scr_GetEntityIdRef(objectId);
				RemoveRefToObject(objectId);
				scrVmPub.function_frame->fs.pos = pos;
				((void (*)(scr_entref_t))scrCompilePub.func_table[builtinIndex])(entref);
				top = scrVmPub.top;
				pos = scrVmPub.function_frame->fs.pos;
				if ( scrVmPub.outparamcount )
				{
					outparamcount = scrVmPub.outparamcount;
					scrVmPub.outparamcount = 0;
					scrVmPub.top -= outparamcount;
					do
					{
						RemoveRefToValue(top--);
						--outparamcount;
					}
					while ( outparamcount );
				}
				if ( scrVmPub.inparamcount )
				{
					scrVmPub.inparamcount = 0;
				}
				else
				{
					++top;
					top->type = VAR_UNDEFINED;
				}
				continue;
			}
			RemoveRefToObject(objectId);
			scrVarPub.error_index = -1;
			Scr_Error(va("%s is not an entity", var_typename[GetObjectType(objectId)]));

		case OP_CallBuiltinMethod1:
			scrVmPub.outparamcount = 1;
			scrVmPub.top = top - 1;
			builtinIndex = Scr_ReadUnsignedShort(&pos);
			if ( top->type != VAR_POINTER )
			{
				RemoveRefToValue(top);
				scrVarPub.error_index = -1;
				Scr_Error(va("%s is not an entity", var_typename[top->type]));
			}
			objectId = top->u.pointerValue;
			if ( GetObjectType(objectId) == VAR_ENTITY )
			{
				entref = Scr_GetEntityIdRef(objectId);
				RemoveRefToObject(objectId);
				scrVmPub.function_frame->fs.pos = pos;
				((void (*)(scr_entref_t))scrCompilePub.func_table[builtinIndex])(entref);
				top = scrVmPub.top;
				pos = scrVmPub.function_frame->fs.pos;
				if ( scrVmPub.outparamcount )
				{
					outparamcount = scrVmPub.outparamcount;
					scrVmPub.outparamcount = 0;
					scrVmPub.top -= outparamcount;
					do
					{
						RemoveRefToValue(top--);
						--outparamcount;
					}
					while ( outparamcount );
				}
				if ( scrVmPub.inparamcount )
				{
					scrVmPub.inparamcount = 0;
				}
				else
				{
					++top;
					top->type = VAR_UNDEFINED;
				}
				continue;
			}
			RemoveRefToObject(objectId);
			scrVarPub.error_index = -1;
			Scr_Error(va("%s is not an entity", var_typename[GetObjectType(objectId)]));

		case OP_CallBuiltinMethod2:
			scrVmPub.outparamcount = 2;
			scrVmPub.top = top - 1;
			builtinIndex = Scr_ReadUnsignedShort(&pos);
			if ( top->type != VAR_POINTER )
			{
				RemoveRefToValue(top);
				scrVarPub.error_index = -1;
				Scr_Error(va("%s is not an entity", var_typename[top->type]));
			}
			objectId = top->u.pointerValue;
			if ( GetObjectType(objectId) == VAR_ENTITY )
			{
				entref = Scr_GetEntityIdRef(objectId);
				RemoveRefToObject(objectId);
				scrVmPub.function_frame->fs.pos = pos;
				((void (*)(scr_entref_t))scrCompilePub.func_table[builtinIndex])(entref);
				top = scrVmPub.top;
				pos = scrVmPub.function_frame->fs.pos;
				if ( scrVmPub.outparamcount )
				{
					outparamcount = scrVmPub.outparamcount;
					scrVmPub.outparamcount = 0;
					scrVmPub.top -= outparamcount;
					do
					{
						RemoveRefToValue(top--);
						--outparamcount;
					}
					while ( outparamcount );
				}
				if ( scrVmPub.inparamcount )
				{
					scrVmPub.inparamcount = 0;
				}
				else
				{
					++top;
					top->type = VAR_UNDEFINED;
				}
				continue;
			}
			RemoveRefToObject(objectId);
			scrVarPub.error_index = -1;
			Scr_Error(va("%s is not an entity", var_typename[GetObjectType(objectId)]));

		case OP_CallBuiltinMethod3:
			scrVmPub.outparamcount = 3;
			scrVmPub.top = top - 1;
			builtinIndex = Scr_ReadUnsignedShort(&pos);
			if ( top->type != VAR_POINTER )
			{
				RemoveRefToValue(top);
				scrVarPub.error_index = -1;
				Scr_Error(va("%s is not an entity", var_typename[top->type]));
			}
			objectId = top->u.pointerValue;
			if ( GetObjectType(objectId) == VAR_ENTITY )
			{
				entref = Scr_GetEntityIdRef(objectId);
				RemoveRefToObject(objectId);
				scrVmPub.function_frame->fs.pos = pos;
				((void (*)(scr_entref_t))scrCompilePub.func_table[builtinIndex])(entref);
				top = scrVmPub.top;
				pos = scrVmPub.function_frame->fs.pos;
				if ( scrVmPub.outparamcount )
				{
					outparamcount = scrVmPub.outparamcount;
					scrVmPub.outparamcount = 0;
					scrVmPub.top -= outparamcount;
					do
					{
						RemoveRefToValue(top--);
						--outparamcount;
					}
					while ( outparamcount );
				}
				if ( scrVmPub.inparamcount )
				{
					scrVmPub.inparamcount = 0;
				}
				else
				{
					++top;
					top->type = VAR_UNDEFINED;
				}
				continue;
			}
			RemoveRefToObject(objectId);
			scrVarPub.error_index = -1;
			Scr_Error(va("%s is not an entity", var_typename[GetObjectType(objectId)]));

		case OP_CallBuiltinMethod4:
			scrVmPub.outparamcount = 4;
			scrVmPub.top = top - 1;
			builtinIndex = Scr_ReadUnsignedShort(&pos);
			if ( top->type != VAR_POINTER )
			{
				RemoveRefToValue(top);
				scrVarPub.error_index = -1;
				Scr_Error(va("%s is not an entity", var_typename[top->type]));
			}
			objectId = top->u.pointerValue;
			if ( GetObjectType(objectId) == VAR_ENTITY )
			{
				entref = Scr_GetEntityIdRef(objectId);
				RemoveRefToObject(objectId);
				scrVmPub.function_frame->fs.pos = pos;
				((void (*)(scr_entref_t))scrCompilePub.func_table[builtinIndex])(entref);
				top = scrVmPub.top;
				pos = scrVmPub.function_frame->fs.pos;
				if ( scrVmPub.outparamcount )
				{
					outparamcount = scrVmPub.outparamcount;
					scrVmPub.outparamcount = 0;
					scrVmPub.top -= outparamcount;
					do
					{
						RemoveRefToValue(top--);
						--outparamcount;
					}
					while ( outparamcount );
				}
				if ( scrVmPub.inparamcount )
				{
					scrVmPub.inparamcount = 0;
				}
				else
				{
					++top;
					top->type = VAR_UNDEFINED;
				}
				continue;
			}
			RemoveRefToObject(objectId);
			scrVarPub.error_index = -1;
			Scr_Error(va("%s is not an entity", var_typename[GetObjectType(objectId)]));

		case OP_CallBuiltinMethod5:
			scrVmPub.outparamcount = 5;
			scrVmPub.top = top - 1;
			builtinIndex = Scr_ReadUnsignedShort(&pos);
			if ( top->type != VAR_POINTER )
			{
				RemoveRefToValue(top);
				scrVarPub.error_index = -1;
				Scr_Error(va("%s is not an entity", var_typename[top->type]));
			}
			objectId = top->u.pointerValue;
			if ( GetObjectType(objectId) == VAR_ENTITY )
			{
				entref = Scr_GetEntityIdRef(objectId);
				RemoveRefToObject(objectId);
				scrVmPub.function_frame->fs.pos = pos;
				((void (*)(scr_entref_t))scrCompilePub.func_table[builtinIndex])(entref);
				top = scrVmPub.top;
				pos = scrVmPub.function_frame->fs.pos;
				if ( scrVmPub.outparamcount )
				{
					outparamcount = scrVmPub.outparamcount;
					scrVmPub.outparamcount = 0;
					scrVmPub.top -= outparamcount;
					do
					{
						RemoveRefToValue(top--);
						--outparamcount;
					}
					while ( outparamcount );
				}
				if ( scrVmPub.inparamcount )
				{
					scrVmPub.inparamcount = 0;
				}
				else
				{
					++top;
					top->type = VAR_UNDEFINED;
				}
				continue;
			}
			RemoveRefToObject(objectId);
			scrVarPub.error_index = -1;
			Scr_Error(va("%s is not an entity", var_typename[GetObjectType(objectId)]));

		case OP_CallBuiltinMethod:
			scrVmPub.outparamcount = *(unsigned char *)pos++;
			scrVmPub.top = top - 1;
			builtinIndex = Scr_ReadUnsignedShort(&pos);
			if ( top->type != VAR_POINTER )
			{
				RemoveRefToValue(top);
				scrVarPub.error_index = -1;
				Scr_Error(va("%s is not an entity", var_typename[top->type]));
			}
			objectId = top->u.pointerValue;
			if ( GetObjectType(objectId) == VAR_ENTITY )
			{
				entref = Scr_GetEntityIdRef(objectId);
				RemoveRefToObject(objectId);
				scrVmPub.function_frame->fs.pos = pos;
				((void (*)(scr_entref_t))scrCompilePub.func_table[builtinIndex])(entref);
				top = scrVmPub.top;
				pos = scrVmPub.function_frame->fs.pos;
				if ( scrVmPub.outparamcount )
				{
					outparamcount = scrVmPub.outparamcount;
					scrVmPub.outparamcount = 0;
					scrVmPub.top -= outparamcount;
					do
					{
						RemoveRefToValue(top--);
						--outparamcount;
					}
					while ( outparamcount );
				}
				if ( scrVmPub.inparamcount )
				{
					scrVmPub.inparamcount = 0;
				}
				else
				{
					++top;
					top->type = VAR_UNDEFINED;
				}
				continue;
			}
			RemoveRefToObject(objectId);
			scrVarPub.error_index = -1;
			Scr_Error(va("%s is not an entity", var_typename[GetObjectType(objectId)]));

		case OP_wait: // VoroN: use sv_fps here??
			if ( top->type == VAR_FLOAT )
			{
				if ( top->u.floatValue < 0.0 )
					Scr_Error("negative wait is not allowed");
				waitTime = Q_rint(top->u.floatValue * 20.0);
				//waitTime = Q_rint(top->u.floatValue * float(sv_fps->current.integer));
				if ( !waitTime )
					waitTime = top->u.floatValue != 0.0;
			}
			else if ( top->type == VAR_INTEGER )
			{
				waitTime = 20 * top->u.intValue;
				//waitTime = sv_fps->current.integer * top->u.intValue;
			}
			else
			{
				scrVarPub.error_index = 2;
				Scr_Error(va("type %s is not a float", var_typename[top->type]));
			}
			// Com_Printf("%i\n", waitTime);
			if ( waitTime > 0xFFFFFE )
			{
				scrVarPub.error_index = 2;
				if ( (waitTime & 0x80000000) == 0 )
					Scr_Error("wait is too long");
				Scr_Error("negative wait is not allowed");
			}
			if ( waitTime )
				Scr_ResetTimeout();
			waitTime = (scrVarPub.time + waitTime) & 0xFFFFFF;
			--top;
			stackValue.type = VAR_STACK;
			stackValue.u.stackValue = VM_ArchiveStack(top - startTop, pos, top, localVarCount, &localId);
			id = GetArray(GetVariable(scrVarPub.timeArrayId, waitTime));
			stackId = GetNewObjectVariable(id, localId);
			SetNewVariableValue(stackId, &stackValue);
			Scr_SetThreadWaitTime(localId, waitTime);
			startTop[1].type = VAR_UNDEFINED;
			if ( gParamCount )
			{
				--gParamCount;
				RemoveRefToObject(localId);
				pos = scrVmPub.function_frame->fs.pos;
				localId = scrVmPub.function_frame->fs.localId;
				localVarCount = scrVmPub.function_frame->fs.localVarCount;
				top = scrVmPub.function_frame->fs.top;
				startTop = scrVmPub.function_frame->fs.startTop;
				top->type = scrVmPub.function_frame->topType;
				++top;
				continue;
			}
			--g_script_error_level;
			return localId;

		case OP_waittillFrameEnd:
			stackValue.type = VAR_STACK;
			stackValue.u.stackValue = VM_ArchiveStack(top - startTop, pos, top, localVarCount, &localId);
			id = GetArray(GetVariable(scrVarPub.timeArrayId, scrVarPub.time));
			stackId = GetNewObjectVariableReverse(id, localId);
			SetNewVariableValue(stackId, &stackValue);
			Scr_SetThreadWaitTime(localId, scrVarPub.time);
			startTop[1].type = VAR_UNDEFINED;
			if ( gParamCount )
			{
				--gParamCount;
				RemoveRefToObject(localId);
				pos = scrVmPub.function_frame->fs.pos;
				localId = scrVmPub.function_frame->fs.localId;
				localVarCount = scrVmPub.function_frame->fs.localVarCount;
				top = scrVmPub.function_frame->fs.top;
				startTop = scrVmPub.function_frame->fs.startTop;
				top->type = scrVmPub.function_frame->topType;
				++top;
				continue;
			}
			--g_script_error_level;
			return localId;

		case OP_PreScriptCall:
			++top;
			top->type = VAR_PRECODEPOS;
			continue;

		case OP_ScriptFunctionCall2:
			++top;
			top->type = VAR_PRECODEPOS;
			if ( scrVmPub.function_count <= 30 )
			{
				selfId = Scr_GetSelf(localId);
				AddRefToObject(selfId);
				localId = AllocChildThread(selfId, localId);
				scrVmPub.function_frame->fs.pos = pos;
				pos = Scr_ReadCodePos(&scrVmPub.function_frame->fs.pos);
				scrVmPub.function_frame->fs.localVarCount = localVarCount;
				localVarCount = 0;
				++scrVmPub.function_count;
				++scrVmPub.function_frame;
				scrVmPub.function_frame->fs.localId = localId;
				continue;
			}
			Scr_Error("script stack overflow (too many embedded function calls)");

		case OP_ScriptFunctionCall:
			if ( scrVmPub.function_count <= 30 )
			{
				selfId = Scr_GetSelf(localId);
				AddRefToObject(selfId);
				localId = AllocChildThread(selfId, localId);
				scrVmPub.function_frame->fs.pos = pos;
				pos = Scr_ReadCodePos(&scrVmPub.function_frame->fs.pos);
				scrVmPub.function_frame->fs.localVarCount = localVarCount;
				localVarCount = 0;
				++scrVmPub.function_count;
				++scrVmPub.function_frame;
				scrVmPub.function_frame->fs.localId = localId;
				continue;
			}
			Scr_Error("script stack overflow (too many embedded function calls)");

		case OP_ScriptFunctionCallPointer:
			if ( top->type != VAR_FUNCTION )
			{
				Scr_Error(va("%s is not a function pointer", var_typename[top->type]));
			}
			if ( scrVmPub.function_count <= 30 )
			{
				selfId = Scr_GetSelf(localId);
				AddRefToObject(selfId);
				localId = AllocChildThread(selfId, localId);
				scrVmPub.function_frame->fs.pos = pos;
				pos = top->u.codePosValue;
				--top;
				scrVmPub.function_frame->fs.localVarCount = localVarCount;
				localVarCount = 0;
				++scrVmPub.function_count;
				++scrVmPub.function_frame;
				scrVmPub.function_frame->fs.localId = localId;
				continue;
			}
			scrVarPub.error_index = 1;
			Scr_Error("script stack overflow (too many embedded function calls)");

		case OP_ScriptMethodCall:
			if ( top->type != VAR_POINTER )
			{
				scrVarPub.error_index = 1;
				Scr_Error(va("%s is not an object", var_typename[top->type]));
			}
			if ( scrVmPub.function_count <= 30 )
			{
				localId = AllocChildThread(top->u.stringValue, localId);
				--top;
				scrVmPub.function_frame->fs.pos = pos;
				pos = Scr_ReadCodePos(&scrVmPub.function_frame->fs.pos);
				scrVmPub.function_frame->fs.localVarCount = localVarCount;
				localVarCount = 0;
				++scrVmPub.function_count;
				++scrVmPub.function_frame;
				scrVmPub.function_frame->fs.localId = localId;
				continue;
			}
			Scr_Error("script stack overflow (too many embedded function calls)");

		case OP_ScriptMethodCallPointer:
			if ( top->type == VAR_FUNCTION )
			{
				tempCodePos = top->u.codePosValue;
				--top;
				if ( top->type != VAR_POINTER )
				{
					scrVarPub.error_index = 2;
					Scr_Error(va("%s is not an object", var_typename[top->type]));
				}
				if ( scrVmPub.function_count <= 30 )
				{
					localId = AllocChildThread(top->u.stringValue, localId);
					--top;
					scrVmPub.function_frame->fs.pos = pos;
					pos = tempCodePos;
					scrVmPub.function_frame->fs.localVarCount = localVarCount;
					localVarCount = 0;
					++scrVmPub.function_count;
					++scrVmPub.function_frame;
					scrVmPub.function_frame->fs.localId = localId;
					continue;
				}
				scrVarPub.error_index = 1;
				Scr_Error("script stack overflow (too many embedded function calls)");
			}
			RemoveRefToValue(top--);
			Scr_Error(va("%s is not a function pointer", var_typename[top[1].type]));

		case OP_ScriptThreadCall:
			if ( scrVmPub.function_count <= 30 )
			{
				selfId = Scr_GetSelf(localId);
				AddRefToObject(selfId);
				localId = AllocThread(selfId);
				scrVmPub.function_frame->fs.pos = pos;
				scrVmPub.function_frame->fs.startTop = startTop;
				pos = Scr_ReadCodePos(&scrVmPub.function_frame->fs.pos);
				startTop = &top[-Scr_ReadUnsigned(&scrVmPub.function_frame->fs.pos)];
				scrVmPub.function_frame->fs.top = startTop;
				scrVmPub.function_frame->topType = startTop->type;
				startTop->type = VAR_PRECODEPOS;
				++gParamCount;
				scrVmPub.function_frame->fs.localVarCount = localVarCount;
				localVarCount = 0;
				++scrVmPub.function_count;
				++scrVmPub.function_frame;
				scrVmPub.function_frame->fs.localId = localId;
				continue;
			}
			scrVarPub.error_index = 1;
			Scr_Error("script stack overflow (too many embedded function calls)");

		case OP_ScriptThreadCallPointer:
			if ( top->type == VAR_FUNCTION )
			{
				if ( scrVmPub.function_count <= 30 )
				{
					tempCodePos = top->u.codePosValue;
					--top;
					selfId = Scr_GetSelf(localId);
					AddRefToObject(selfId);
					localId = AllocThread(selfId);
					scrVmPub.function_frame->fs.pos = pos;
					scrVmPub.function_frame->fs.startTop = startTop;
					pos = tempCodePos;
					startTop = &top[-Scr_ReadUnsigned(&scrVmPub.function_frame->fs.pos)];
					scrVmPub.function_frame->fs.top = startTop;
					scrVmPub.function_frame->topType = startTop->type;
					startTop->type = VAR_PRECODEPOS;
					++gParamCount;
					scrVmPub.function_frame->fs.localVarCount = localVarCount;
					localVarCount = 0;
					++scrVmPub.function_count;
					++scrVmPub.function_frame;
					scrVmPub.function_frame->fs.localId = localId;
					continue;
				}
				scrVarPub.error_index = 1;
				Scr_Error("script stack overflow (too many embedded function calls)");
			}
			Scr_Error(va("%s is not a function pointer", var_typename[top->type]));

		case OP_ScriptMethodThreadCall:
			if ( top->type != VAR_POINTER )
			{
				scrVarPub.error_index = 2;
				Scr_Error(va("%s is not an object", var_typename[top->type]));
			}
			if ( scrVmPub.function_count > 30 )
			{
				scrVarPub.error_index = 1;
				Scr_Error("script stack overflow (too many embedded function calls)");
			}
			localId = AllocThread(top->u.stringValue);
			--top;
			scrVmPub.function_frame->fs.pos = pos;
			scrVmPub.function_frame->fs.startTop = startTop;
			pos = Scr_ReadCodePos(&scrVmPub.function_frame->fs.pos);
			startTop = &top[-Scr_ReadUnsigned(&scrVmPub.function_frame->fs.pos)];
			scrVmPub.function_frame->fs.top = startTop;
			scrVmPub.function_frame->topType = startTop->type;
			startTop->type = VAR_PRECODEPOS;
			++gParamCount;
			scrVmPub.function_frame->fs.localVarCount = localVarCount;
			localVarCount = 0;
			++scrVmPub.function_count;
			++scrVmPub.function_frame;
			scrVmPub.function_frame->fs.localId = localId;
			continue;

		case OP_ScriptMethodThreadCallPointer:
			if ( top->type != VAR_FUNCTION )
			{
				RemoveRefToValue(top--);
				Scr_Error(va("%s is not a function pointer", var_typename[top[1].type]));
			}
			tempCodePos = top->u.codePosValue;
			--top;
			if ( top->type != VAR_POINTER )
			{
				scrVarPub.error_index = 2;
				Scr_Error(va("%s is not an object", var_typename[top->type]));
			}
			if ( scrVmPub.function_count > 30 )
			{
				scrVarPub.error_index = 1;
				Scr_Error("script stack overflow (too many embedded function calls)");
			}
			localId = AllocThread(top->u.stringValue);
			--top;
			scrVmPub.function_frame->fs.pos = pos;
			scrVmPub.function_frame->fs.startTop = startTop;
			pos = tempCodePos;
			startTop = &top[-Scr_ReadUnsigned(&scrVmPub.function_frame->fs.pos)];
			scrVmPub.function_frame->fs.top = startTop;
			scrVmPub.function_frame->topType = startTop->type;
			startTop->type = VAR_PRECODEPOS;
			++gParamCount;
			scrVmPub.function_frame->fs.localVarCount = localVarCount;
			localVarCount = 0;
			++scrVmPub.function_count;
			++scrVmPub.function_frame;
			scrVmPub.function_frame->fs.localId = localId;
			continue;

		case OP_DecTop:
			RemoveRefToValue(top);
			--top;
			continue;

		case OP_CastFieldObject:
			objectId = Scr_EvalFieldObject(scrVarPub.tempVariable, top);
			--top;
			continue;

		case OP_EvalLocalVariableObjectCached:
			objectId = Scr_EvalVariableObject(Scr_GetLocalVar(pos));
			++pos;
			continue;

		case OP_CastBool:
			Scr_CastBool(top);
			continue;

		case OP_BoolNot:
			Scr_EvalBoolNot(top);
			continue;

		case OP_BoolComplement:
			Scr_EvalBoolComplement(top);
			continue;

		case OP_JumpOnFalse:
			Scr_CastBool(top);
			jumpOffset = Scr_ReadUnsignedShort(&pos);
			if ( !top->u.intValue )
				pos += jumpOffset;
			--top;
			continue;

		case OP_JumpOnTrue:
			Scr_CastBool(top);
			jumpOffset = Scr_ReadUnsignedShort(&pos);
			if ( top->u.intValue )
				pos += jumpOffset;
			--top;
			continue;

		case OP_JumpOnFalseExpr:
			Scr_CastBool(top);
			jumpOffset = Scr_ReadUnsignedShort(&pos);
			if ( top->u.intValue )
			{
				--top;
				continue;
			}
			pos += jumpOffset;
			continue;

		case OP_JumpOnTrueExpr:
			Scr_CastBool(top);
			jumpOffset = Scr_ReadUnsignedShort(&pos);
			if ( !top->u.intValue )
			{
				--top;
				continue;
			}
			pos += jumpOffset;
			continue;

		case OP_jump:
			jumpOffset = Scr_ReadUnsigned(&pos);
			pos += jumpOffset;
			continue;

#define INFINITE_LOOP_TIMEOUT 5000
		case OP_jumpback:
			if ( (unsigned int)(Sys_MilliSeconds() - scrVmGlob.starttime) >= INFINITE_LOOP_TIMEOUT )
			{
				if ( !scrVmGlob.loading )
				{
					if ( !scrVmPub.abort_on_error )
					{
						Com_Printf("script runtime error: potential infinite loop in script - killing thread.\n");
						Scr_PrintPrevCodePos(CON_CHANNEL_DONT_FILTER, pos, 0);
						Scr_ResetTimeout();
						while ( 1 )
						{
							parentLocalId = GetSafeParentLocalId(localId);
							Scr_KillThread(localId);
							scrVmPub.localVars -= localVarCount;
							while ( top->type != VAR_CODEPOS )
								RemoveRefToValue(top--);
							--scrVmPub.function_count;
							--scrVmPub.function_frame;
							if ( !parentLocalId )
								break;
							RemoveRefToObject(localId);
							pos = scrVmPub.function_frame->fs.pos;
							localVarCount = scrVmPub.function_frame->fs.localVarCount;
							localId = parentLocalId;
							--top;
						}
						startTop[1].type = VAR_UNDEFINED;
						if ( gParamCount )
						{
							--gParamCount;
							RemoveRefToObject(localId);
							pos = scrVmPub.function_frame->fs.pos;
							localId = scrVmPub.function_frame->fs.localId;
							localVarCount = scrVmPub.function_frame->fs.localVarCount;
							top = scrVmPub.function_frame->fs.top;
							startTop = scrVmPub.function_frame->fs.startTop;
							top->type = scrVmPub.function_frame->topType;
							++top;
							continue;
						}
						--g_script_error_level;
						return localId;
					}
					Scr_TerminalError("potential infinite loop in script");
				}
				Com_Printf("script runtime warning: potential infinite loop in script.\n");
				Scr_PrintPrevCodePos(CON_CHANNEL_DONT_FILTER, pos, 0);
				jumpOffset = Scr_ReadUnsignedShort(&pos);
				pos -= jumpOffset;
				Scr_ResetTimeout();
			}
			else
			{
				jumpOffset = Scr_ReadUnsignedShort(&pos);
				pos -= jumpOffset;
			}
			continue;

		case OP_inc:
			++top;
			tempValue = Scr_EvalVariableField(fieldValueId);
			top->u = tempValue.u;
			top->type = tempValue.type;
			if ( top->type == VAR_INTEGER )
			{
				++top->u.intValue;
				++pos;
			}
			else
			{
				Scr_Error(va("++ must be applied to an int (applied to %s)", var_typename[top->type]));
			}
			SetVariableFieldValue(fieldValueId, top);
			--top;
			continue;

		case OP_dec:
			++top;
			tempValue = Scr_EvalVariableField(fieldValueId);
			top->u = tempValue.u;
			top->type = tempValue.type;
			if ( top->type == VAR_INTEGER )
			{
				--top->u.intValue;
				++pos;
			}
			else
			{
				Scr_Error(va("-- must be applied to an int (applied to %s)", var_typename[top->type]));
			}
			SetVariableFieldValue(fieldValueId, top);
			--top;
			continue;

		case OP_bit_or:
			Scr_EvalOr(top - 1, top);
			--top;
			continue;

		case OP_bit_ex_or:
			Scr_EvalExOr(top - 1, top);
			--top;
			continue;

		case OP_bit_and:
			Scr_EvalAnd(top - 1, top);
			--top;
			continue;

		case OP_equality:
			Scr_EvalEquality(top - 1, top);
			--top;
			continue;

		case OP_inequality:
			Scr_EvalInequality(top - 1, top);
			--top;
			continue;

		case OP_less:
			Scr_EvalLess(top - 1, top);
			--top;
			continue;

		case OP_greater:
			Scr_EvalGreater(top - 1, top);
			--top;
			continue;

		case OP_less_equal:
			Scr_EvalLessEqual(top - 1, top);
			--top;
			continue;

		case OP_greater_equal:
			Scr_EvalGreaterEqual(top - 1, top);
			--top;
			continue;

		case OP_shift_left:
			Scr_EvalShiftLeft(top - 1, top);
			--top;
			continue;

		case OP_shift_right:
			Scr_EvalShiftRight(top - 1, top);
			--top;
			continue;

		case OP_plus:
			Scr_EvalPlus(top - 1, top);
			--top;
			continue;

		case OP_minus:
			Scr_EvalMinus(top - 1, top);
			--top;
			continue;

		case OP_multiply:
			Scr_EvalMultiply(top - 1, top);
			--top;
			continue;

		case OP_divide:
			Scr_EvalDivide(top - 1, top);
			--top;
			continue;

		case OP_mod:
			Scr_EvalMod(top - 1, top);
			--top;
			continue;

		case OP_size:
			Scr_EvalSizeValue(top);
			continue;

		case OP_waittillmatch:
		case OP_waittill:
			if ( top->type != VAR_POINTER )
			{
				scrVarPub.error_index = 2;
				Scr_Error(va("%s is not an object", var_typename[top->type]));
			}
			if ( !IsFieldObject(top->u.pointerValue) )
			{
				scrVarPub.error_index = 2;
				Scr_Error(va("%s is not an object", var_typename[GetObjectType(top->u.pointerValue)]));
			}
			tempValue.u = top->u;
			--top;
			if ( top->type == VAR_STRING )
			{
				stringValue = top->u.stringValue;
				--top;
				stackValue.type = VAR_STACK;
				stackValue.u.stackValue = VM_ArchiveStack(top - startTop, pos, top, localVarCount, &localId);
				id = GetArray(GetVariable(GetArray(GetVariable(tempValue.u.stringValue, 0x1FFFEu)), stringValue));
				stackId = GetNewObjectVariable(id, localId);
				SetNewVariableValue(stackId, &stackValue);
				tempValue.type = VAR_POINTER;
				SetNewVariableValue(GetNewObjectVariable(GetArray(GetObjectVariable(scrVarPub.pauseArrayId, Scr_GetSelf(localId))), localId), &tempValue);
				Scr_SetThreadNotifyName(localId, stringValue);
				startTop[1].type = VAR_UNDEFINED;
				if ( gParamCount )
				{
					--gParamCount;
					RemoveRefToObject(localId);
					pos = scrVmPub.function_frame->fs.pos;
					localId = scrVmPub.function_frame->fs.localId;
					localVarCount = scrVmPub.function_frame->fs.localVarCount;
					top = scrVmPub.function_frame->fs.top;
					startTop = scrVmPub.function_frame->fs.startTop;
					top->type = scrVmPub.function_frame->topType;
					++top;
					continue;
				}
				--g_script_error_level;
				return localId;
			}
			++top;
			scrVarPub.error_index = 3;
			Scr_Error("first parameter of waittill must evaluate to a string");

		case OP_notify:
			if ( top->type != VAR_POINTER )
			{
				scrVarPub.error_index = 2;
				Scr_Error(va("%s is not an object", var_typename[top->type]));
			}
			id = top->u.pointerValue;
			if ( !IsFieldObject(id) )
			{
				scrVarPub.error_index = 2;
				Scr_Error(va("%s is not an object", var_typename[GetObjectType(top->u.pointerValue)]));
			}
			--top;
			if ( top->type != VAR_STRING )
			{
				++top;
				scrVarPub.error_index = 1;
				Scr_Error("first parameter of notify must evaluate to a string");
			}
			stringValue = top->u.stringValue;
			--top;
			scrVmPub.function_frame->fs.pos = pos;
			VM_Notify(id, stringValue, top);
			pos = scrVmPub.function_frame->fs.pos;
			RemoveRefToObject(id);
			SL_RemoveRefToString(stringValue);
			while ( top->type != VAR_PRECODEPOS )
				RemoveRefToValue(top--);
			--top;
			continue;

		case OP_endon:
			if ( top->type != VAR_POINTER )
			{
				scrVarPub.error_index = 1;
				Scr_Error(va("%s is not an object", var_typename[top->type]));
			}
			if ( !IsFieldObject(top->u.pointerValue) )
			{
				scrVarPub.error_index = 1;
				Scr_Error(va("%s is not an object", var_typename[GetObjectType(top->u.pointerValue)]));
			}
			if ( top[-1].type == VAR_STRING )
			{
				stringValue = top[-1].u.stringValue;
				AddRefToObject(localId);
				threadId = AllocThread(localId);
				GetObjectVariable(GetArray(GetVariable(GetArray(GetVariable(top->u.stringValue, 0x1FFFEu)), stringValue)), threadId);
				RemoveRefToObject(threadId);
				tempValue.type = VAR_POINTER;
				tempValue.u = top->u;
				SetNewVariableValue(GetNewObjectVariable(GetArray(GetObjectVariable(scrVarPub.pauseArrayId, localId)), threadId), &tempValue);
				Scr_SetThreadNotifyName(threadId, stringValue);
				top -= 2;
				continue;
			}
			Scr_Error("first parameter of endon must evaluate to a string");

		case OP_voidCodepos:
			++top;
			top->type = VAR_PRECODEPOS;
			continue;

		case OP_switch:
			jumpOffset = Scr_ReadUnsigned(&pos);
			pos += jumpOffset;
			gCaseCount = Scr_ReadUnsignedShort(&pos);
			if ( top->type == VAR_STRING )
			{
				caseValue = top->u.stringValue;
				SL_RemoveRefToString(top->u.stringValue);
			}
			else if ( top->type == VAR_INTEGER )
			{
				if ( IsValidArrayIndex(top->u.pointerValue) )
				{
					caseValue = GetInternalVariableIndex(top->u.pointerValue);
				}
				else
				{
					Scr_Error(va("switch index %d out of range", top->u.intValue));
				}
			}
			else
			{
				Scr_Error(va("cannot switch on %s", var_typename[top->type]));
			}
			if ( !gCaseCount )
			{
loop_dec_top:
				--top;
				continue;
			}
			do
			{
				currentCaseValue = Scr_ReadUnsigned(&pos);
				currentCodePos = Scr_ReadCodePos(&pos);
				if ( currentCaseValue == caseValue )
				{
					pos = currentCodePos;
					goto loop_dec_top;
				}
				--gCaseCount;
			}
			while ( gCaseCount );
			if ( !currentCaseValue )
				pos = currentCodePos;
			--top;
			continue;

		case OP_endswitch:
			gCaseCount = Scr_ReadUnsignedShort(&pos);
			Scr_ReadIntArray(&pos, 2 * gCaseCount);
			continue;

		case OP_vector:
			top -= 2;
			Scr_CastVector(top);
			continue;

		case OP_NOP:
			continue;

		case OP_abort:
			--g_script_error_level;
			return 0;

		case OP_object:
			++top;
			classnum = Scr_ReadUnsigned(&pos);
			entnum = Scr_ReadUnsigned(&pos);
			top->u.pointerValue = FindEntityId(entnum, classnum);
			if ( !top->u.pointerValue )
			{
				top->type = VAR_UNDEFINED;
				Scr_Error("unknown object");
			}
			top->type = VAR_POINTER;
			AddRefToObject(top->u.pointerValue);
			continue;

		case OP_thread_object:
			++top;
			top->u.pointerValue = Scr_ReadUnsignedShort(&pos);
			top->type = VAR_POINTER;
			AddRefToObject(top->u.pointerValue);
			continue;

		case OP_EvalLocalVariable:
			++top;
			tempValue = Scr_EvalVariable(FindVariable(localId, Scr_ReadUnsignedShort(&pos)));
			top->u = tempValue.u;
			top->type = tempValue.type;
			continue;

		case OP_EvalLocalVariableRef:
			fieldValueId = FindVariable(localId, Scr_ReadUnsignedShort(&pos));
			if ( fieldValueId )
				continue;
			Scr_Error("cannot create a new local variable in the debugger");

		case OP_prof_begin:
			++pos;
			continue;

		case OP_prof_end:
			++pos;
			continue;

		case OP_breakpoint:
			if (scrVarPub.developer)
			{
				Com_PrintMessage(CON_CHANNEL_DONT_FILTER, "\nCode hit debug breakpoint at:\n");
				Scr_PrintPrevCodePos(CON_CHANNEL_DONT_FILTER, pos, 0);
			}
			continue;

		default:
			scrVmPub.terminal_error = 1;
			RuntimeErrorInternal(CON_CHANNEL_DONT_FILTER, pos, 0, va("CODE ERROR: unknown opcode %d", gOpcode));
			continue;
		}
	}
}

unsigned int VM_Execute(unsigned int localId, const char *pos, unsigned int paramcount)
{
	int varType;
	unsigned int parentId;
	VariableValue *startTop;
	unsigned int numparams;

	Scr_ClearOutParams();
	startTop = Scr_GetValue(paramcount);
	numparams = scrVmPub.inparamcount - paramcount;

	if ( scrVmPub.function_count > 29 )
	{
		Scr_KillThread(localId);
		scrVmPub.inparamcount = numparams + 1;

		while ( numparams )
		{
			RemoveRefToValue(scrVmPub.top);
			--scrVmPub.top;
			--numparams;
		}

		++scrVmPub.top;
		scrVmPub.top->type = VAR_UNDEFINED;
		RuntimeError(pos, 0, "script stack overflow (too many embedded function calls)", 0);

		return localId;
	}
	else
	{
		if ( scrVmPub.function_count )
		{
			++scrVmPub.function_count;
			++scrVmPub.function_frame;
			scrVmPub.function_frame->fs.localId = 0;
		}

		scrVmPub.function_frame->fs.pos = pos;
		++scrVmPub.function_count;
		++scrVmPub.function_frame;
		scrVmPub.function_frame->fs.localId = localId;
		varType = startTop->type;
		startTop->type = VAR_PRECODEPOS;
		scrVmPub.inparamcount = 0;
		parentId = VM_ExecuteInternal(pos, localId, 0, scrVmPub.top, startTop);
		startTop->type = varType;
		scrVmPub.top = startTop + 1;
		scrVmPub.inparamcount = numparams + 1;
		ClearVariableValue(scrVarPub.tempVariable);

		if ( scrVmPub.function_count )
		{
			--scrVmPub.function_count;
			--scrVmPub.function_frame;
		}

		return parentId;
	}
}

void VM_Resume(unsigned int timeId)
{
	unsigned int id;
	VariableStackBuffer *stackBuf;
	unsigned int parentId;
	unsigned int localId;
	function_stack_t stack;

	Scr_ResetTimeout();
	AddRefToObject(timeId);

	for ( stack.startTop = scrVmPub.stack; ; RemoveRefToValue(stack.startTop + 1) )
	{
		localId = FindNextSibling(timeId);

		if ( !localId )
			break;

		parentId = GetVariableKeyObject(localId);
		stackBuf = GetVariableValueAddress_Bad(localId)->u.stackValue;
		RemoveObjectVariable(timeId, parentId);
		VM_UnarchiveStack(parentId, &stack, stackBuf);
		id = VM_ExecuteInternal(stack.pos, stack.localId, stack.localVarCount, stack.top, stack.startTop);
		RemoveRefToObject(id);
	}

	RemoveRefToObject(timeId);
	ClearVariableValue(scrVarPub.tempVariable);
	scrVmPub.top = scrVmPub.stack;
}

void VM_SetTime()
{
	unsigned int localId;
	unsigned int id;

	if ( scrVarPub.timeArrayId )
	{
		id = FindVariable(scrVarPub.timeArrayId, scrVarPub.time);

		if ( id )
		{
			localId = FindObject(id);
			VM_Resume(localId);
			SafeRemoveVariable(scrVarPub.timeArrayId, scrVarPub.time);
		}
	}
}















void Scr_ClearStrings()
{
	scrCompilePub.canonicalStrings = 0;
	Hunk_ClearHigh(scrVarPub.canonicalStrMark);
}

void Scr_AllocStrings()
{
	scrVarPub.canonicalStrMark = Hunk_SetMark();
	scrCompilePub.canonicalStrings = (uint16_t *)Hunk_AllocInternal(0x20000u);
	scrVarPub.canonicalStrCount = 0;
}















