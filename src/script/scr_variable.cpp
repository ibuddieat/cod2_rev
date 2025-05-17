#include "../qcommon/qcommon.h"
#include "script_public.h"

scrVarGlob_t scrVarGlob;
scrVarPub_t scrVarPub;

/*
==============
Scr_GetClassnumForCharId
==============
*/
int Scr_GetClassnumForCharId( char charId )
{
	for ( int i = 0; i < CLASS_NUM_COUNT; i++ )
	{
		if ( scrClassMap[i].charId == charId )
		{
			return i;
		}
	}

	return -1;
}

/*
==============
Scr_GetChecksum
==============
*/
void Scr_GetChecksum( int *checksum )
{
	UNIMPLEMENTED(__FUNCTION__);
}

/*
==============
Scr_FindField
==============
*/
unsigned int Scr_FindField( const char *name, int *type )
{
	const char *pos;
	int len;
	unsigned int index;

	assert(scrVarPub.fieldBuffer);

	for ( pos = scrVarPub.fieldBuffer; *pos; pos += len + 3 )
	{
		len = strlen(pos) + 1;

		if ( !I_stricmp(name, pos) )
		{
			pos = &pos[len];
			index = *(unsigned short *)&pos[0];
			*type = pos[2];

			return index;
		}
	}

	return 0;
}

/*
==============
Scr_GetEntityIdRef
==============
*/
scr_entref_t Scr_GetEntityIdRef( unsigned int entId )
{
	scr_entref_t entref;
	VariableValueInternal *entValue;

	entValue = &scrVarGlob.variableList[entId];

	assert((entValue->w.type & VAR_MASK) == VAR_ENTITY);
	assert((entValue->w.name >> VAR_NAME_BITS) < CLASS_NUM_COUNT);

	entref.entnum = entValue->u.o.u.entnum;
	entref.classnum = entValue->w.classnum >> VAR_NAME_BITS;

	return entref;
}

/*
==============
GetVarType
==============
*/
int GetVarType( unsigned int id )
{
	UNIMPLEMENTED(__FUNCTION__);
	return 0;
}

/*
==============
IsVarFree
==============
*/
bool IsVarFree( unsigned int id )
{
	UNIMPLEMENTED(__FUNCTION__);
	return true;
}

/*
==============
GetObjectType
==============
*/
unsigned int GetObjectType( unsigned int id )
{
	assert((scrVarGlob.variableList[id].w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	return scrVarGlob.variableList[id].w.status & VAR_MASK;
}

/*
==============
Scr_GetEntNum
==============
*/
int Scr_GetEntNum( unsigned int id )
{
	assert(GetObjectType( id ) == VAR_ENTITY);
	return scrVarGlob.variableList[id].u.o.u.entnum;
}

/*
==============
Scr_GetEntClassId
==============
*/
char Scr_GetEntClassId( unsigned int id )
{
	assert(GetObjectType( id ) == VAR_ENTITY);
	return scrClassMap[scrVarGlob.variableList[id].w.name >> VAR_NAME_BITS].charId;
}

/*
==============
Scr_IsThreadAlive
==============
*/
int Scr_IsThreadAlive( unsigned int thread )
{
	VariableValueInternal *entryValue;

	assert(scrVarPub.timeArrayId);
	entryValue = &scrVarGlob.variableList[thread];

	assert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	assert(((entryValue->w.type & VAR_MASK) >= VAR_THREAD && (entryValue->w.type & VAR_MASK) <= VAR_CHILD_THREAD) || (entryValue->w.type & VAR_MASK) == VAR_DEAD_THREAD);

	return (entryValue->w.type & VAR_MASK) != VAR_DEAD_THREAD;
}

/*
==============
IsFieldObject
==============
*/
bool IsFieldObject( unsigned int id )
{
	VariableValueInternal *entryValue;

	assert(id);
	entryValue = &scrVarGlob.variableList[id];

	assert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	assert(IsObject( entryValue ));

	return (entryValue->w.type & VAR_MASK) < VAR_ARRAY;
}

/*
==============
FindObject
==============
*/
unsigned int FindObject( unsigned int id )
{
	VariableValueInternal *entryValue;

	assert(id);
	entryValue = &scrVarGlob.variableList[id];

	assert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	assert((entryValue->w.type & VAR_MASK) == VAR_POINTER);

	return entryValue->u.u.pointerValue;
}

/*
==============
GetVariableName
==============
*/
unsigned int GetVariableName( unsigned int id )
{
	VariableValueInternal *entryValue;

	entryValue = &scrVarGlob.variableList[id];

	assert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	assert(!IsObject( entryValue ));

	return entryValue->w.name >> VAR_NAME_BITS;
}

/*
==============
FindPrevSibling
==============
*/
unsigned int FindPrevSibling( unsigned int id )
{
	VariableValueInternal *list, *entryValue;
	unsigned int nextSibling, childId, prevSibling;

	assert(id);
	list = scrVarGlob.variableList;

	entryValue = &list[id];
	assert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);

	nextSibling = entryValue->nextSibling;

	childId = list[nextSibling].hash.u.prev;
	prevSibling = list[childId].hash.u.prevSibling;

	entryValue = &list[prevSibling];
	childId = entryValue->hash.id;

	entryValue = &list[childId];
	assert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);

	if ( IsObject( entryValue ) )
	{
		return 0;
	}

	return childId;
}

/*
==============
FindNextSibling
==============
*/
unsigned int FindNextSibling( unsigned int id )
{
	VariableValueInternal *list, *entryValue;
	unsigned int nextSibling, childId;

	assert(id);
	list = scrVarGlob.variableList;

	entryValue = &list[id];
	assert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);

	nextSibling = entryValue->nextSibling;

	entryValue = &list[nextSibling];
	childId = entryValue->hash.id;

	entryValue = &list[childId];
	assert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);

	if ( IsObject( entryValue ) )
	{
		return 0;
	}

	return childId;
}

/*
==============
GetArraySize
==============
*/
unsigned int GetArraySize( unsigned int id )
{
	VariableValueInternal *entryValue;

	assert(id);
	entryValue = &scrVarGlob.variableList[id];
	assert((entryValue->w.type & VAR_MASK) == VAR_ARRAY);

	return entryValue->u.o.u.size;
}

/*
==============
GetVariableValueAddress
==============
*/
union VariableUnion* GetVariableValueAddress( unsigned int id )
{
	VariableValueInternal *entryValue;

	assert(id);
	entryValue = &scrVarGlob.variableList[id];

	assert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	assert((entryValue->w.type & VAR_MASK) != VAR_UNDEFINED);
	assert(!IsObject( entryValue ));

	return &entryValue->u.u;
}

/*
==============
SetNewVariableValue
==============
*/
void SetNewVariableValue( unsigned int id, VariableValue *value )
{
	VariableValueInternal *entryValue;

	assert((value->type & VAR_MASK) < VAR_THREAD);
	entryValue = &scrVarGlob.variableList[id];

	assert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	assert(!IsObject( entryValue ));
	assert(value->type >= 0 && value->type < VAR_COUNT);
	assert((entryValue->w.type & VAR_MASK) == VAR_UNDEFINED);
	assert((value->type != VAR_POINTER) || ((entryValue->w.type & VAR_MASK) < FIRST_DEAD_OBJECT));
	assert(!(entryValue->w.type & VAR_MASK));

	entryValue->w.type |= value->type;
	entryValue->u.u = value->u;
}

/*
==============
SetNewVariableValue
==============
*/
VariableValue Scr_GetArrayIndexValue( unsigned int name )
{
	VariableValue value;

	assert(name);

	if ( name < SL_MAX_STRING_INDEX )
	{
		value.type = VAR_STRING;
		value.u.stringValue = (unsigned short)name;
	}
	else if ( name < 0x1FFFE )
	{
		value.type = VAR_POINTER;
		value.u.pointerValue = name - SL_MAX_STRING_INDEX;
	}
	else
	{
		value.type = VAR_INTEGER;
		value.u.intValue = name - MAX_ARRAYINDEX;
	}

	return value;
}

/*
==============
GetInternalVariableIndex
==============
*/
unsigned int GetInternalVariableIndex( unsigned int unsignedValue )
{
	assert(IsValidArrayIndex( unsignedValue ));
	return ( unsignedValue + MAX_ARRAYINDEX ) & VAR_NAME_LOW_MASK;
}

/*
==============
IsValidArrayIndex
==============
*/
bool IsValidArrayIndex( unsigned int unsignedValue )
{
	return unsignedValue < MAX_ARRAYINDEX;
}

/*
==============
AddRefToVector
==============
*/
void AddRefToVector( const vec3_t vectorValue )
{
	RefVector *refVec = (RefVector *)(( byte *)vectorValue - REFSTRING_STRING_OFFSET );

	if ( refVec->byteLen )
	{
		return;
	}

	refVec->refCount++;
	assert(refVec->refCount);
}

/*
==============
Scr_GetRefCountToObject
==============
*/
int Scr_GetRefCountToObject( unsigned int id )
{
	VariableValueInternal *entryValue = &scrVarGlob.variableList[id];

	assert(((entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL));
	assert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	assert(IsObject( entryValue ));

	return entryValue->u.o.refCount;
}

/*
==============
AddRefToObject
==============
*/
void AddRefToObject( unsigned int id )
{
	assert(id);
	VariableValueInternal *entryValue = &scrVarGlob.variableList[id];
	entryValue->u.o.refCount++;

	assert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	assert(IsObject( entryValue ));
	assert(entryValue->u.o.refCount);
}

/*
==============
Scr_GetSelf
==============
*/
unsigned int Scr_GetSelf( unsigned int threadId )
{
	VariableValueInternal *entryValue = &scrVarGlob.variableList[threadId];

	assert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	assert(((entryValue->w.type & VAR_MASK) >= VAR_THREAD) && ((entryValue->w.type & VAR_MASK) <= VAR_CHILD_THREAD));

	return entryValue->u.o.u.self;
}

/*
==============
GetStartLocalId
==============
*/
unsigned int GetStartLocalId( unsigned int threadId )
{
	assert((scrVarGlob.variableList[threadId].w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL);
	assert((scrVarGlob.variableList[threadId].w.type & VAR_MASK) >= VAR_THREAD && (scrVarGlob.variableList[threadId].w.type & VAR_MASK) <= VAR_CHILD_THREAD);

	while ( (scrVarGlob.variableList[threadId].w.type & VAR_MASK) == VAR_CHILD_THREAD )
	{
		threadId = scrVarGlob.variableList[threadId].w.parentLocalId >> VAR_NAME_BITS;
	}

	assert((scrVarGlob.variableList[threadId].w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL);
	assert((scrVarGlob.variableList[threadId].w.type & VAR_MASK) >= VAR_THREAD && (scrVarGlob.variableList[threadId].w.type & VAR_MASK) <= VAR_TIME_THREAD);

	return threadId;
}

/*
==============
GetSafeParentLocalId
==============
*/
unsigned int GetSafeParentLocalId( unsigned int threadId )
{
	VariableValueInternal *entryValue = &scrVarGlob.variableList[threadId];

	assert((entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL);
	assert((entryValue->w.type & VAR_MASK) >= VAR_THREAD && (entryValue->w.type & VAR_MASK) <= VAR_CHILD_THREAD);

	if ( (entryValue->w.type & VAR_MASK) != VAR_CHILD_THREAD )
	{
		return 0;
	}

	return entryValue->w.parentLocalId >> VAR_NAME_BITS;
}

/*
==============
GetParentLocalId
==============
*/
unsigned int GetParentLocalId( unsigned int threadId )
{
	VariableValueInternal *entryValue = &scrVarGlob.variableList[threadId];

	assert((entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL);
	assert((entryValue->w.type & VAR_MASK) == VAR_CHILD_THREAD);

	return entryValue->w.parentLocalId >> VAR_NAME_BITS;
}

/*
==============
Scr_GetThreadWaitTime
==============
*/
unsigned int Scr_GetThreadWaitTime( unsigned int startLocalId )
{
	VariableValueInternal *entryValue = &scrVarGlob.variableList[startLocalId];

	assert((entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL);
	assert((entryValue->w.type & VAR_MASK) == VAR_TIME_THREAD);

	return entryValue->w.waitTime >> VAR_NAME_BITS;
}

/*
==============
Scr_ClearWaitTime
==============
*/
void Scr_ClearWaitTime( unsigned int startLocalId )
{
	VariableValueInternal *entryValue = &scrVarGlob.variableList[startLocalId];

	assert(((entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL));
	assert((entryValue->w.type & VAR_MASK) == VAR_TIME_THREAD);

	entryValue->w.type &= ~VAR_MASK;
	entryValue->w.type |= VAR_THREAD;
}

/*
==============
Scr_SetThreadWaitTime
==============
*/
void Scr_SetThreadWaitTime( unsigned int startLocalId, unsigned int waitTime )
{
	VariableValueInternal *entryValue = &scrVarGlob.variableList[startLocalId];

	assert(((entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL));
	assert(((entryValue->w.type & VAR_MASK) == VAR_THREAD) || !Scr_GetThreadNotifyName(startLocalId));

	entryValue->w.type &= ~VAR_MASK;
	entryValue->w.type = (unsigned char)entryValue->w.type;
	entryValue->w.type |= VAR_TIME_THREAD;

	entryValue->w.waitTime |= waitTime << VAR_NAME_BITS;
}

/*
==============
Scr_GetThreadNotifyName
==============
*/
unsigned short Scr_GetThreadNotifyName( unsigned int startLocalId )
{
	VariableValueInternal *entryValue = &scrVarGlob.variableList[startLocalId];

	assert((entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL);
	assert((entryValue->w.type & VAR_MASK) == VAR_NOTIFY_THREAD);
	assert((entryValue->w.notifyName >> VAR_NAME_BITS) < ( 1 << 16 ));

	return entryValue->w.notifyName >> VAR_NAME_BITS;
}

/*
==============
Scr_RemoveThreadEmptyNotifyName
==============
*/
void Scr_RemoveThreadEmptyNotifyName( unsigned int startLocalId )
{
	UNIMPLEMENTED(__FUNCTION__);
}

/*
==============
Scr_SetThreadNotifyName
==============
*/
void Scr_SetThreadNotifyName( unsigned int startLocalId, unsigned int notifyName )
{
	VariableValueInternal *entryValue = &scrVarGlob.variableList[startLocalId];

	assert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	assert(((entryValue->w.type & VAR_MASK) == VAR_THREAD));

	entryValue->w.type &= ~VAR_MASK;
	entryValue->w.type = (unsigned char)entryValue->w.type;
	entryValue->w.type |= VAR_NOTIFY_THREAD;

	entryValue->w.notifyName |= notifyName << VAR_NAME_BITS;
}

/*
==============
GetVariableKeyObject
==============
*/
unsigned int GetVariableKeyObject( unsigned int id )
{
	VariableValueInternal *entryValue = &scrVarGlob.variableList[id];

	assert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	assert(!IsObject(entryValue));

	return (entryValue->w.name >> VAR_NAME_BITS) - SL_MAX_STRING_INDEX;
}

/*
==============
Scr_GetNumScriptVars
==============
*/
unsigned int Scr_GetNumScriptVars()
{
	UNIMPLEMENTED(__FUNCTION__);
	return 0;
}

/*
==============
Scr_DumpScriptVariables
==============
*/
void Scr_DumpScriptVariables( bool spreadsheet, bool summary, bool total, bool functionSummary, bool lineSort, const char *fileName, const char *functionName, int minCount )
{
	UNIMPLEMENTED(__FUNCTION__);
}

/*
==============
Scr_DumpScriptVariablesDefault
==============
*/
void Scr_DumpScriptVariablesDefault()
{
	Scr_DumpScriptVariables(0, 0, 0, 0, 0, 0, 0, 0);
}

/*
==============
IsObjectId
==============
*/
bool IsObjectId( unsigned int id )
{
	return GetValueType( id ) >= VAR_THREAD;
}

/*
==============
Scr_EvalVariableObject
==============
*/
unsigned int Scr_EvalVariableObject( unsigned int id )
{
	VariableValueInternal *entryValue;
	int type;

	entryValue = &scrVarGlob.variableList[id];
	assert(((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE) || !id);

	type = entryValue->w.type & VAR_MASK;

	if ( type == VAR_POINTER )
	{
		type = scrVarGlob.variableList[entryValue->u.u.pointerValue].w.type & VAR_MASK;

		if ( type < VAR_ARRAY )
		{
			assert(type >= FIRST_OBJECT);
			return entryValue->u.u.pointerValue;
		}
	}

	Scr_Error(va("%s is not a field object", var_typename[type]));
	return 0;
}

/*
==============
RemoveRefToEmptyObject
==============
*/
void RemoveRefToEmptyObject( unsigned int id )
{
	VariableValueInternal *entryValue = &scrVarGlob.variableList[id];

	assert(((entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL));
	assert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	assert(IsObject(entryValue));
	//assert(!entryValue->nextSibling);

	if ( !entryValue->u.o.refCount )
	{
		FreeVariable(id);
		return;
	}

	assert(id);
	entryValue->u.o.refCount--;
}

/*
==============
Var_Init
==============
*/
void Var_Init()
{
	Var_ResetAll();
	Var_InitClassMap();
}

/*
==============
FindObjectVariable
==============
*/
unsigned int FindObjectVariable( unsigned int parentId, unsigned int id )
{
	return scrVarGlob.variableList[FindVariableIndexInternal(parentId, id + SL_MAX_STRING_INDEX)].hash.id;
}

/*
==============
FindVariable
==============
*/
unsigned int FindVariable( unsigned int parentId, unsigned int unsignedValue )
{
	return scrVarGlob.variableList[FindVariableIndexInternal(parentId, unsignedValue)].hash.id;
}

/*
==============
AddRefToValue
==============
*/
void AddRefToValue( int type, VariableUnion u )
{
	switch ( type )
	{
	case VAR_POINTER:
		AddRefToObject(u.pointerValue);
		break;

	case VAR_STRING:
	case VAR_ISTRING:
		SL_AddRefToString(u.stringValue);
		break;

	case VAR_VECTOR:
		assert(type - 1 == VAR_VECTOR - VAR_BEGIN_REF);
		AddRefToVector(u.vectorValue);
		break;
	}
}

/*
==============
Scr_GetOffset
==============
*/
int Scr_GetOffset( unsigned int classnum, const char *name )
{
	unsigned int fieldId = FindVariable( scrClassMap[classnum].id, SL_ConvertFromString( name ) );

	if ( !fieldId )
	{
		return -1;
	}

	return scrVarGlob.variableList[fieldId].u.u.entityOffset;
}

/*
==============
Scr_EvalVariable
==============
*/
VariableValue Scr_EvalVariable( unsigned int id )
{
	VariableValueInternal *entryValue;
	VariableValue value;

	entryValue = &scrVarGlob.variableList[id];
	assert(((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE) || !id);

	value.type = entryValue->w.type & VAR_MASK;
	value.u = entryValue->u.u;

	assert(!IsObject(entryValue));
	AddRefToValue(value.type, value.u);

	return value;
}

/*
==============
Scr_FindAllVariableField
==============
*/
unsigned int Scr_FindAllVariableField( unsigned int parentId, unsigned int *names ) // untested
{
	VariableValueInternal *parentValue;
	unsigned int classnum, name, id, count;

	parentValue = &scrVarGlob.variableList[parentId];
	assert((parentValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	assert((parentValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	assert(IsObject(parentValue));
	count = 0;

	switch ( parentValue->w.type & VAR_MASK )
	{
	case VAR_THREAD:
	case VAR_NOTIFY_THREAD:
	case VAR_TIME_THREAD:
	case VAR_CHILD_THREAD:
	case VAR_OBJECT:
	case VAR_DEAD_ENTITY:
		for ( id = scrVarGlob.variableList[parentValue->nextSibling].hash.id; id != parentId; id = scrVarGlob.variableList[scrVarGlob.variableList[id].nextSibling].hash.id )
		{
			name = scrVarGlob.variableList[id].w.name >> VAR_NAME_BITS;
			assert(name);

			if ( name == 0x1FFFE || name == 0x1FFFF )
			{
				continue;
			}

			assert(name <= scrVarPub.canonicalStrCount);

			if ( names )
			{
				names[count] = name;
			}

			count++;
		}
		break;

	case VAR_ENTITY:
		classnum = scrClassMap[parentValue->w.name >> VAR_NAME_BITS].id;
		assert(classnum < CLASS_NUM_COUNT);
		for ( id = scrVarGlob.variableList[scrVarGlob.variableList[classnum].nextSibling].hash.id; id != classnum; id = scrVarGlob.variableList[scrVarGlob.variableList[id].nextSibling].hash.id )
		{
			name = (scrVarGlob.variableList[id].w.name >> VAR_NAME_BITS) - MAX_ARRAYINDEX;

			if ( name > scrVarPub.canonicalStrCount )
			{
				continue;
			}

			if ( FindVariable( parentId, name ) )
			{
				continue;
			}

			if ( names )
			{
				names[count] = name;
			}

			count++;
		}
		for ( id = scrVarGlob.variableList[parentValue->nextSibling].hash.id; id != parentId; id = scrVarGlob.variableList[scrVarGlob.variableList[id].nextSibling].hash.id )
		{
			name = scrVarGlob.variableList[id].w.name >> VAR_NAME_BITS;
			assert(name);

			if ( name == 0x1FFFE || name == 0x1FFFF )
			{
				continue;
			}

			assert(name <= scrVarPub.canonicalStrCount);

			if ( names )
			{
				names[count] = name;
			}

			count++;
		}
		break;

	case VAR_ARRAY:
		for ( id = scrVarGlob.variableList[parentValue->nextSibling].hash.id; id != parentId; id = scrVarGlob.variableList[scrVarGlob.variableList[id].nextSibling].hash.id )
		{
			name = scrVarGlob.variableList[id].w.name >> VAR_NAME_BITS;
			assert(name);

			if ( names )
			{
				names[count] = name;
			}

			count++;
		}
		break;
	}

	return count;
}

/*
==============
FindArrayVariable
==============
*/
unsigned int FindArrayVariable( unsigned int parentId, unsigned int index )
{
	return scrVarGlob.variableList[ FindArrayVariableIndex( parentId, index ) ].hash.id;
}

/*
==============
FindEntityId
==============
*/
unsigned int FindEntityId( int entnum, int classnum )
{
	unsigned int entArrayId, id;
	VariableValueInternal *entryValue;

	assert((unsigned)entnum < (1 << 16));
	entArrayId = scrClassMap[classnum].entArrayId;

	assert(entArrayId);
	id = FindArrayVariable(entArrayId, entnum);

	if ( !id )
	{
		return 0;
	}

	entryValue = &scrVarGlob.variableList[id];

	assert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	assert((entryValue->w.type & VAR_MASK) == VAR_POINTER);
	assert(entryValue->u.u.pointerValue);

	return entryValue->u.u.pointerValue;
}

/*
==============
Scr_DumpScriptThreads
==============
*/
void Scr_DumpScriptThreads()
{
	ThreadDebugInfo *infoArray, *pInfo;
	ThreadDebugInfo info;
	int num, size, i, j, count, classnum, id;
	VariableValueInternal *entryValue;
	VariableStackBuffer *stackBuf;
	const char *pos, *buf, *currentPos;
	unsigned char type;

	infoArray = (ThreadDebugInfo *)Z_Malloc( sizeof( *infoArray ) * VARIABLELIST_CHILD_SIZE );

	if ( !infoArray )
	{
		Com_Printf("Cannot dump script threads: out of memory\n");
		return;
	}

	num = 0;

	for ( id = 1; id < VARIABLELIST_CHILD_SIZE; id++ )
	{
		entryValue = &scrVarGlob.variableList[id];

		if ( (entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_FREE )
		{
			continue;
		}

		if ( (entryValue->w.type & VAR_MASK) != VAR_STACK )
		{
			continue;
		}

		pInfo = &infoArray[num];

		num++;
		info.posSize = 0;

		stackBuf = entryValue->u.u.stackValue;

		size = stackBuf->size;
		pos = stackBuf->pos;
		buf = stackBuf->buf;

		while ( size )
		{
			size--;
			type = *buf;
			buf += 1;
			currentPos = *(const char **)buf;
			buf += 4;

			if ( type == VAR_CODEPOS )
			{
				info.pos[info.posSize] = currentPos;
				info.posSize++;
			}
		}

		info.pos[info.posSize] = pos;
		info.posSize++;

		pInfo->varUsage = Scr_GetThreadUsage(stackBuf, &pInfo->endonUsage);
		pInfo->posSize = info.posSize;

		info.posSize--;

		for ( j = 0; j < pInfo->posSize; j++ )
		{
			pInfo->pos[j] = info.pos[info.posSize - j];
		}
	}

	qsort(infoArray, num, sizeof(*infoArray), ThreadInfoCompare);
	Com_Printf("********************************\n");

	i = 0;

	while ( i < num )
	{
		pInfo = &infoArray[i];

		count = 0;
		info.varUsage = 0;
		info.endonUsage = 0;

		do
		{
			count++;

			info.varUsage = info.varUsage + infoArray[i].varUsage;
			info.endonUsage = info.endonUsage + infoArray[i].endonUsage;

			i++;
		}
		while ( i < num && !ThreadInfoCompare(pInfo, &infoArray[i]) );

		Com_Printf("count: %d, var usage: %d, endon usage: %d\n", count, (int)info.varUsage, (int)info.endonUsage);
		Scr_PrintPrevCodePos(CON_CHANNEL_DONT_FILTER, pInfo->pos[0], 0);

		for ( j = 1; j < pInfo->posSize; j++ )
		{
			Com_Printf("called from:\n");
			Scr_PrintPrevCodePos(CON_CHANNEL_DONT_FILTER, pInfo->pos[j], 0);
		}
	}

	Z_Free(infoArray);
	Com_Printf("********************************\n");

	for ( classnum = 0; classnum < CLASS_NUM_COUNT; classnum++ )
	{
		if ( !scrClassMap[classnum].entArrayId )
		{
			continue;
		}

		info.varUsage = 0;
		count = 0;

		for ( id = FindNextSibling(scrClassMap[classnum].entArrayId); id; id = FindNextSibling(id) )
		{
			count++;

			if ( (scrVarGlob.variableList[id].w.type & VAR_MASK) == VAR_POINTER )
			{
				info.varUsage = Scr_GetObjectUsage(scrVarGlob.variableList[id].u.u.pointerValue) + info.varUsage;
			}
		}

		Com_Printf("ent type '%s'... count: %d, var usage: %d\n", scrClassMap[classnum].name, count, (int)info.varUsage);
	}

	Com_Printf("********************************\n");
}

/*
==============
AllocChildThread
==============
*/
unsigned int AllocChildThread( unsigned int self, unsigned int parentLocalId )
{
	unsigned short id;
	VariableValueInternal *entryValue;

	id = AllocVariable();
	entryValue = &scrVarGlob.variableList[id];
	entryValue->w.status = VAR_STAT_EXTERNAL;

	assert(!(entryValue->w.type & VAR_MASK));
	entryValue->w.type |= VAR_CHILD_THREAD;

	assert(!(entryValue->w.parentLocalId & VAR_NAME_HIGH_MASK));
	entryValue->w.parentLocalId |= parentLocalId << VAR_NAME_BITS;

	entryValue->u.o.refCount = 0;
	entryValue->u.o.u.self = self;

	return id;
}

/*
==============
AllocThread
==============
*/
unsigned int AllocThread( unsigned int self )
{
	unsigned short id;
	VariableValueInternal *entryValue;

	id = AllocVariable();
	entryValue = &scrVarGlob.variableList[id];
	entryValue->w.status = VAR_STAT_EXTERNAL;

	assert(!(entryValue->w.type & VAR_MASK));
	entryValue->w.type |= VAR_THREAD;

	entryValue->u.o.refCount = 0;
	entryValue->u.o.u.self = self;

	return id;
}

/*
==============
Scr_AllocArray
==============
*/
unsigned int Scr_AllocArray()
{
	unsigned short id;
	VariableValueInternal *entryValue;

	id = AllocVariable();
	entryValue = &scrVarGlob.variableList[id];
	entryValue->w.status = VAR_STAT_EXTERNAL;

	assert(!(entryValue->w.type & VAR_MASK));
	entryValue->w.type |= VAR_ARRAY;

	entryValue->u.o.refCount = 0;
	entryValue->u.o.u.size = 0;

	return id;
}

/*
==============
AllocObject
==============
*/
unsigned int AllocObject()
{
	unsigned short id;
	VariableValueInternal *entryValue;

	id = AllocVariable();
	entryValue = &scrVarGlob.variableList[id];
	entryValue->w.status = VAR_STAT_EXTERNAL;

	assert(!(entryValue->w.type & VAR_MASK));
	entryValue->w.type |= VAR_OBJECT;

	entryValue->u.o.refCount = 0;

	return id;
}

/*
==============
AllocValue
==============
*/
unsigned int AllocValue()
{
	unsigned short id;
	VariableValueInternal *entryValue;

	id = AllocVariable();
	entryValue = &scrVarGlob.variableList[id];
	entryValue->w.status = VAR_STAT_EXTERNAL;

	return id;
}

/*
==============
Scr_InitStringSet
==============
*/
unsigned int Scr_InitStringSet( void )
{
	UNIMPLEMENTED(__FUNCTION__);
	return 0;
}

/*
==============
Scr_SetClassMap
==============
*/
void Scr_SetClassMap( int classnum )
{
	assert(!scrClassMap[classnum].entArrayId);
	assert(!scrClassMap[classnum].id);

	scrClassMap[classnum].entArrayId = Scr_AllocArray();
	scrClassMap[classnum].id = Scr_AllocArray();
}

/*
==============
GetArray
==============
*/
unsigned int GetArray( unsigned int id )
{
	assert(id);

	VariableValueInternal *entryValue = &scrVarGlob.variableList[id];

	assert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	assert((entryValue->w.type & VAR_MASK) == VAR_UNDEFINED || (entryValue->w.type & VAR_MASK) == VAR_POINTER);

	if ( (entryValue->w.type & VAR_MASK) == VAR_UNDEFINED )
	{
		entryValue->w.type |= VAR_POINTER;
		entryValue->u.u.pointerValue = Scr_AllocArray();
	}

	assert((entryValue->w.type & VAR_MASK) == VAR_POINTER);

	return entryValue->u.u.pointerValue;
}

/*
==============
GetObject
==============
*/
unsigned int GetObject_( unsigned int id )
{
	assert(id);

	VariableValueInternal *entryValue = &scrVarGlob.variableList[id];

	assert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	assert((entryValue->w.type & VAR_MASK) == VAR_UNDEFINED || (entryValue->w.type & VAR_MASK) == VAR_POINTER);

	if ( (entryValue->w.type & VAR_MASK) == VAR_UNDEFINED )
	{
		entryValue->w.type |= VAR_POINTER;
		entryValue->u.u.pointerValue = AllocObject();
	}

	assert((entryValue->w.type & VAR_MASK) == VAR_POINTER);

	return entryValue->u.u.pointerValue;
}

/*
==============
GetNewObjectVariableReverse
==============
*/
unsigned int GetNewObjectVariableReverse( unsigned int parentId, unsigned int id )
{
	assert((scrVarGlob.variableList[parentId].w.type & VAR_MASK) == VAR_ARRAY);
	return scrVarGlob.variableList[ GetNewVariableIndexReverseInternal( parentId, id + SL_MAX_STRING_INDEX ) ].hash.id;
}

/*
==============
GetNewObjectVariable
==============
*/
unsigned int GetNewObjectVariable( unsigned int parentId, unsigned int id )
{
	assert((scrVarGlob.variableList[parentId].w.type & VAR_MASK) == VAR_ARRAY);
	return scrVarGlob.variableList[ GetNewVariableIndexInternal( parentId, id + SL_MAX_STRING_INDEX ) ].hash.id;
}

/*
==============
GetObjectVariable
==============
*/
unsigned int GetObjectVariable( unsigned int parentId, unsigned int id )
{
	assert((scrVarGlob.variableList[parentId].w.type & VAR_MASK) == VAR_ARRAY);
	return scrVarGlob.variableList[ GetVariableIndexInternal( parentId, id + SL_MAX_STRING_INDEX ) ].hash.id;
}

/*
==============
GetNewVariable
==============
*/
unsigned int GetNewVariable( unsigned int parentId, unsigned int name )
{
	return scrVarGlob.variableList[ GetNewVariableIndexInternal( parentId, name ) ].hash.id;
}

/*
==============
GetVariable
==============
*/
unsigned int GetVariable( unsigned int parentId, unsigned int name )
{
	return scrVarGlob.variableList[ GetVariableIndexInternal( parentId, name ) ].hash.id;
}

/*
==============
GetVariable
==============
*/
unsigned int Scr_GetVariableField( unsigned int parentId, unsigned int name )
{
	VariableValueInternal *entryValue;
	unsigned int index, type;

	assert(parentId);
	entryValue = &scrVarGlob.variableList[parentId];

	assert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	assert(IsObject(entryValue));

	type = entryValue->w.type & VAR_MASK;

	if ( type <= VAR_OBJECT )
	{
		return GetVariable(parentId, name);
	}

	if ( type != VAR_ENTITY )
	{
		Scr_Error(va("cannot set field of %s", var_typename[type]));
		return 0;
	}

	index = FindVariable(parentId, name);

	if ( !index )
	{
		scrVarPub.entId = parentId;
		scrVarPub.entFieldName = name;
		return VARIABLELIST_CHILD_SIZE;
	}

	return index;
}

/*
==============
GetNewArrayVariable
==============
*/
unsigned int GetNewArrayVariable( unsigned int parentId, unsigned int name )
{
	return scrVarGlob.variableList[ GetNewArrayVariableIndex( parentId, name ) ].hash.id;
}

/*
==============
GetArrayVariable
==============
*/
unsigned int GetArrayVariable( unsigned int parentId, unsigned int name )
{
	return scrVarGlob.variableList[ GetArrayVariableIndex( parentId, name ) ].hash.id;
}

/*
==============
RemoveRefToVector
==============
*/
void RemoveRefToVector( const vec3_t vectorValue )
{
	RefVector *refVec = (RefVector *)( (byte *)vectorValue - REFSTRING_STRING_OFFSET );

	if ( refVec->byteLen )
	{
		return;
	}

	if ( refVec->refCount )
	{
		refVec->refCount--;
		return;
	}

	MT_Free( refVec, sizeof( *refVec ) );
}

/*
==============
Scr_EvalArrayIndex
==============
*/
unsigned int Scr_EvalArrayIndex( unsigned int parentId, VariableValue *index )
{
	unsigned int stringValue;

	switch ( index->type )
	{
	case VAR_INTEGER:
		if ( IsValidArrayIndex(index->u.pointerValue) )
			return GetArrayVariable(parentId, index->u.pointerValue);
		Scr_Error(va("array index %d out of range", index->u.pointerValue));
		return 0;

	case VAR_STRING:
		stringValue = GetVariable(parentId, index->u.stringValue);
		SL_RemoveRefToString(index->u.stringValue);
		return stringValue;

	default:
		Scr_Error(va("%s is not an array index", var_typename[index->type]));
		return 0;
	}
}

/*
==============
Scr_GetEntityId
==============
*/
unsigned int Scr_GetEntityId( int entnum, int classnum )
{
	VariableValueInternal *entryValue;
	unsigned int entId, id, entArrayId;

	assert((unsigned)entnum < (1 << 16));

	entArrayId = scrClassMap[classnum].entArrayId;
	assert(entArrayId);

	id = GetArrayVariable(entArrayId, entnum);
	assert(id);

	entryValue = &scrVarGlob.variableList[id];
	assert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);

	if ( (entryValue->w.status & VAR_MASK) )
	{
		assert((entryValue->w.type & VAR_MASK) == VAR_POINTER);
		return entryValue->u.u.pointerValue;
	}

	entId = AllocEntity(classnum, entnum);
	assert(!(entryValue->w.type & VAR_MASK));

	entryValue->w.type |= VAR_POINTER;
	entryValue->u.u.pointerValue = entId;

	return entId;
}

/*
==============
Scr_AddClassField
==============
*/
void Scr_AddClassField( int classnum, const char *name, unsigned int offset )
{
	unsigned int str, classId, fieldId;
	VariableValueInternal *entryValue;
	const char *namePos;

	assert(offset < (1 << 16));
	for ( namePos = name; *namePos; namePos++ )
	{
		assert((*namePos < 'A' || *namePos > 'Z'));
	}

	classId = scrClassMap[classnum].id;
	str = SL_GetCanonicalString(name);
	assert(!FindArrayVariable(classId, (unsigned)str));

	entryValue = &scrVarGlob.variableList[GetNewArrayVariable(classId, str)];

	entryValue->w.type &= ~VAR_MASK;
	entryValue->w.type |= VAR_INTEGER;
	entryValue->u.u.intValue = (unsigned short)offset;

	str = SL_GetString_(name, 0);
	assert(!FindVariable(classId, str));

	fieldId = GetNewVariable(classId, str);
	SL_RemoveRefToString(str);

	entryValue = &scrVarGlob.variableList[fieldId];

	entryValue->w.type &= ~VAR_MASK;
	entryValue->w.type |= VAR_INTEGER;
	entryValue->u.u.intValue = (unsigned short)offset;
}

/*
==============
RemoveRefToValue
==============
*/
void RemoveRefToValue( int type, VariableUnion u )
{
	switch ( type )
	{
	case VAR_POINTER:
		RemoveRefToObject(u.pointerValue);
		break;

	case VAR_STRING:
	case VAR_ISTRING:
		SL_RemoveRefToString(u.stringValue);
		break;

	case VAR_VECTOR:
		assert(type - 1 == VAR_VECTOR - VAR_BEGIN_REF);
		RemoveRefToVector(u.vectorValue);
		break;
	}
}

/*
==============
Scr_AllocVector
==============
*/
float* Scr_AllocVector( const vec3_t v )
{
	float* av = Scr_AllocVectorInternal();
	VectorCopy(v, av);

	return av;
}

/*
==============
FreeValue
==============
*/
void FreeValue( unsigned int id )
{
	VariableValueInternal *entryValue = &scrVarGlob.variableList[id];

	assert(((entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL));
	assert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	assert(!IsObject( entryValue ));
	assert(scrVarGlob.variableList[entryValue->v.index].hash.id == id);

	RemoveRefToValue(entryValue->w.type & VAR_MASK, entryValue->u.u);
	FreeVariable(id);
}

/*
==============
Scr_RemoveThreadNotifyName
==============
*/
void Scr_RemoveThreadNotifyName( unsigned int startLocalId )
{
	unsigned short stringValue;
	VariableValueInternal *entryValue;

	entryValue = &scrVarGlob.variableList[startLocalId];

	assert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	assert((entryValue->w.type & VAR_MASK) == VAR_NOTIFY_THREAD);

	stringValue = Scr_GetThreadNotifyName(startLocalId);
	assert(stringValue);

	SL_RemoveRefToString(stringValue);

	entryValue->w.type &= ~VAR_MASK;
	entryValue->w.type |= VAR_THREAD;
}

/*
==============
Var_Shutdown
==============
*/
void Var_Shutdown()
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
Scr_CopyEntityNum
==============
*/
void Scr_CopyEntityNum( int fromEntnum, int toEntnum, int classnum )
{
	unsigned int fromEntId = FindEntityId( fromEntnum, classnum );

	if ( !fromEntId )
	{
		return;
	}

	if ( !FindNextSibling(fromEntId) )
	{
		return;
	}

	assert( !FindEntityId( toEntnum, classnum ) );
	CopyEntity( fromEntId, Scr_GetEntityId( toEntnum, classnum ) );
}

/*
==============
Scr_ClearVector
==============
*/
void Scr_ClearVector( VariableValue *value )
{
	for ( int i = 2; i >= 0; i-- )
	{
		RemoveRefToValue( &value[i] );
	}

	value->type = VAR_UNDEFINED;
}

/*
==============
Scr_CastString
==============
*/
bool Scr_CastString( VariableValue *value )
{
	const float *constTempVector;

	switch ( value->type )
	{
	case VAR_STRING:
		return true;

	case VAR_INTEGER:
		value->type = VAR_STRING;
		value->u.stringValue = SL_GetStringForInt(value->u.intValue);
		return true;

	case VAR_FLOAT:
		value->type = VAR_STRING;
		value->u.stringValue = SL_GetStringForFloat(value->u.floatValue);
		return true;

	case VAR_VECTOR:
		value->type = VAR_STRING;
		constTempVector = value->u.vectorValue;
		value->u.stringValue = SL_GetStringForVector(value->u.vectorValue);
		RemoveRefToVector(constTempVector);
		return true;

	default:
		scrVarPub.error_message = va("cannot cast %s to string", var_typename[value->type]);
		RemoveRefToValue(value);
		value->type = VAR_UNDEFINED;
		return false;
	}
}

/*
==============
Scr_CastBool
==============
*/
void Scr_CastBool( VariableValue *value )
{
	int type;

	switch ( value->type )
	{
	case VAR_INTEGER:
		value->u.intValue = value->u.intValue != 0;
		break;

	case VAR_FLOAT:
		value->type = VAR_INTEGER;
		value->u.intValue = value->u.floatValue != 0;
		break;

	default:
		type = value->type;
		RemoveRefToValue(value);
		value->type = VAR_UNDEFINED;
		Scr_Error(va("cannot cast %s to bool", var_typename[type]));
		break;
	}
}

/*
==============
Scr_EvalBoolComplement
==============
*/
void Scr_EvalBoolComplement( VariableValue *value )
{
	int type;

	if ( value->type == VAR_INTEGER )
	{
		value->u.intValue = ~value->u.intValue;
		return;
	}

	type = value->type;
	RemoveRefToValue(value);
	value->type = VAR_UNDEFINED;
	Scr_Error(va("~ cannot be applied to \"%s\"", var_typename[type]));
}

/*
==============
Scr_EvalBoolNot
==============
*/
void Scr_EvalBoolNot( VariableValue *value )
{
	Scr_CastBool(value);

	if ( value->type == VAR_INTEGER )
	{
		value->u.intValue = value->u.intValue == 0;
	}
}

/*
==============
ClearVariableValue
==============
*/
void ClearVariableValue( unsigned int id )
{
	VariableValueInternal *entryValue;

	assert(id);
	entryValue = &scrVarGlob.variableList[id];

	assert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	assert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	assert(!IsObject( entryValue ));
	assert((entryValue->w.type & VAR_MASK) != VAR_STACK);

	RemoveRefToValue(entryValue->w.status & VAR_MASK, entryValue->u.u);

	entryValue->w.type &= ~VAR_MASK;
	assert((entryValue->w.type & VAR_MASK) == VAR_UNDEFINED);
}

/*
==============
SetVariableValue
==============
*/
void SetVariableValue( unsigned int id, VariableValue *value )
{
	VariableValueInternal *entryValue;

	assert(id);
	assert(!IsObjectVal( value ));
	assert(value->type >= 0 && value->type < VAR_COUNT);
	assert(value->type != VAR_STACK);

	entryValue = &scrVarGlob.variableList[id];

	assert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	assert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	assert(!IsObject( entryValue ));
	assert((entryValue->w.type & VAR_MASK) != VAR_STACK);

	RemoveRefToValue(entryValue->w.type & VAR_MASK, entryValue->u.u);

	entryValue->w.type &= ~VAR_MASK;
	entryValue->w.type |= value->type;
	entryValue->u.u = value->u;
}

/*
==============
SetEmptyArray
==============
*/
void SetEmptyArray( unsigned int parentId )
{
	VariableValue tempValue;

	tempValue.type = VAR_POINTER;
	tempValue.u.pointerValue = Scr_AllocArray();

	SetVariableValue(parentId, &tempValue);
}

/*
==============
Scr_EvalFieldObject
==============
*/
unsigned int Scr_EvalFieldObject( unsigned int tempVariable, VariableValue *value )
{
	int type;
	VariableValue tempValue;

	type = value->type;

	if ( type == VAR_POINTER )
	{
		type = scrVarGlob.variableList[value->u.pointerValue].w.type & VAR_MASK;

		if ( type < VAR_ARRAY )
		{
			assert(type >= FIRST_OBJECT);

			tempValue.type = VAR_POINTER;
			tempValue.u = value->u;

			SetVariableValue(tempVariable, &tempValue);
			return tempValue.u.pointerValue;
		}
	}

	RemoveRefToValue(value->type, value->u);
	Scr_Error(va("%s is not a field object", var_typename[type]));
	return 0;
}

/*
==============
Scr_CastVector
==============
*/
void Scr_CastVector( VariableValue *value )
{
	int type, i;
	vec3_t vec;

	for ( i = 2; i >= 0; i-- )
	{
		type = value[i].type;

		switch ( type )
		{
		case VAR_FLOAT:
			vec[2 - i] = value[i].u.floatValue;
			break;

		case VAR_INTEGER:
			vec[2 - i] = (float)value[i].u.intValue;
			break;

		default:
			scrVarPub.error_index = i + 1;
			Scr_ClearVector(value);
			Scr_Error(va("type %s is not a float", var_typename[type]));
			return;
		}
	}

	value->type = VAR_VECTOR;
	value->u.vectorValue = Scr_AllocVector(vec);
}

/*
==============
Scr_CastDebugString
==============
*/
void Scr_CastDebugString( VariableValue *value )
{
	unsigned int stringValue;

	switch ( value->type )
	{
	case VAR_POINTER:
		stringValue = SL_GetString_( var_typename[ GetValueType( value->u.pointerValue) ], 0 );
		break;

	case VAR_STRING:
	case VAR_VECTOR:
	case VAR_FLOAT:
	case VAR_INTEGER:
		Scr_CastString(value);
		return;

	case VAR_ISTRING:
		value->type = VAR_STRING;
		return;

	case VAR_ANIMATION:
		stringValue = SL_GetString_( XAnimGetAnimDebugName( Scr_GetAnims( value->u.pointerValue >> 16 ), (unsigned short)value->u.pointerValue ), 0 );
		break;

	default:
		stringValue = SL_GetString_( var_typename[ value->type ], 0 );
		break;
	}

	RemoveRefToValue(value);

	value->type = VAR_STRING;
	value->u.stringValue = stringValue;
}

/*
==============
SafeRemoveVariable
==============
*/
void SafeRemoveVariable( unsigned int parentId, unsigned int name )
{
	VariableValueInternal *entryValue;
	unsigned int index, id;

	index = FindVariableIndexInternal(parentId, name);

	if ( !index )
	{
		return;
	}

	id = scrVarGlob.variableList[index].hash.id;
	entryValue = &scrVarGlob.variableList[id];

	assert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	assert(!IsObject( entryValue ));

	MakeVariableExternal(&scrVarGlob.variableList[index], &scrVarGlob.variableList[parentId]);
	FreeValue(id);
}

/*
==============
RemoveNextVariable
==============
*/
void RemoveNextVariable( unsigned int parentId )
{
	VariableValueInternal *entryValue;
	unsigned short id;

	assert((scrVarGlob.variableList[parentId].w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	entryValue = &scrVarGlob.variableList[scrVarGlob.variableList[parentId].nextSibling];

	id = entryValue->hash.id;
	assert(id);

	MakeVariableExternal(entryValue, &scrVarGlob.variableList[parentId]);
	FreeValue(id);
}



























































unsigned int GetValueType(int varIndex)
{
	return scrVarGlob.variableList[varIndex].w.type & VAR_MASK;
}







unsigned int FindVariableIndexUnknown(unsigned int name, unsigned short index)
{
	unsigned int newIndex;
	VariableValueInternal *newEntry;
	VariableValueInternal *entryValue;
	VariableValueInternal *entry;

	entry = &scrVarGlob.variableList[scrVarGlob.variableList[index].hash.id];

	if ( (entry->w.name & VAR_STAT_MASK) != VAR_STAT_HEAD )
		return 0;

	if ( entry->w.name >> VAR_NAME_BITS != name )
	{
		newIndex = entry->v.next;

		for ( entryValue = &scrVarGlob.variableList[newIndex];
		        entryValue != &scrVarGlob.variableList[index];
		        entryValue = &scrVarGlob.variableList[newIndex] )
		{
			newEntry = &scrVarGlob.variableList[entryValue->hash.id];

			if ( newEntry->w.name >> VAR_NAME_BITS == name )
				return newIndex;

			newIndex = newEntry->v.next;
		}

		return 0;
	}

	return index;
}



unsigned int FindVariableIndexInternal(unsigned int parentId, unsigned int name)
{
	return FindVariableIndexUnknown(name, (parentId + name) % (VARIABLELIST_CHILD_SIZE - 1) + 1);
}










void MakeVariableExternal(VariableValueInternal *value, VariableValueInternal *parentValue)
{
	VariableValue indexValue;
	unsigned int nextSibling;
	unsigned int nextSiblingIndex;
	unsigned int prevSiblingIndex;
	Variable hash;
	VariableValueInternal *oldEntryValue;
	VariableValueInternal *entryValue;
	unsigned int index;
	VariableValueInternal *nextEntry;
	VariableValueInternal *oldEntry;
	unsigned int oldIndex;

	index = value - scrVarGlob.variableList;
	entryValue = &scrVarGlob.variableList[value->hash.id];

	if ( VAR_TYPE(parentValue) == VAR_ARRAY )
	{
		--parentValue->u.o.u.size;
		indexValue = Scr_GetArrayIndexValue(entryValue->w.classnum >> VAR_NAME_BITS);
		RemoveRefToValue(&indexValue);
	}

	if ( (entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_HEAD )
	{
		oldIndex = entryValue->v.next;
		oldEntry = &scrVarGlob.variableList[oldIndex];
		oldEntryValue = &scrVarGlob.variableList[oldEntry->hash.id];

		if ( oldEntry != value )
		{
			oldEntryValue->w.status &= ~VAR_STAT_MASK;
			oldEntryValue->w.status |= VAR_STAT_HEAD;
			prevSiblingIndex = value->hash.u.prevSibling;
			nextSiblingIndex = entryValue->nextSibling;
			nextSibling = oldEntryValue->nextSibling;
			scrVarGlob.variableList[nextSibling].hash.u.prev = index;
			scrVarGlob.variableList[scrVarGlob.variableList[oldEntry->hash.u.prev].hash.id].nextSibling = index;
			scrVarGlob.variableList[nextSiblingIndex].hash.u.prev = oldIndex;
			scrVarGlob.variableList[scrVarGlob.variableList[prevSiblingIndex].hash.id].nextSibling = oldIndex;
			hash = value->hash;
			value->hash = oldEntry->hash;
			oldEntry->hash = hash;
			index = oldIndex;
		}
	}
	else
	{
		oldEntry = value;
		oldEntryValue = entryValue;

		do
		{
			nextEntry = oldEntry;
			oldIndex = oldEntryValue->v.next;
			oldEntry = &scrVarGlob.variableList[oldIndex];
			oldEntryValue = &scrVarGlob.variableList[oldEntry->hash.id];
		}
		while ( oldEntry != value );

		scrVarGlob.variableList[nextEntry->hash.id].v.next = entryValue->v.next;
	}

	entryValue->w.status &= ~VAR_STAT_MASK;
	entryValue->w.status |= VAR_STAT_HEAD;
	entryValue->w.status |= VAR_STAT_EXTERNAL;
	entryValue->v.next = index;
}

unsigned int GetNewVariableIndexInternal3(unsigned int parentId, unsigned int name, unsigned int index)
{
	VariableValue value;
	unsigned int type;
	VariableValueInternal *parentValue;
	VariableValueInternal *newEntry;
	uint16_t next;
	uint16_t id;
	VariableValueInternal *newEntryValue;
	uint16_t i;
	uint16_t nextId;
	VariableValueInternal_u siblingValue;
	VariableValueInternal *entry;
	VariableValueInternal *entryValue;

	entry = &scrVarGlob.variableList[index];
	entryValue = &scrVarGlob.variableList[entry->hash.id];
	type = entryValue->w.status & VAR_STAT_MASK;

	if ( type )
	{
		if ( type == VAR_STAT_HEAD )
		{
			if ( entry->w.status & VAR_STAT_MASK )
			{
				index = scrVarGlob.variableList->u.next;

				if ( !scrVarGlob.variableList->u.next )
					Scr_TerminalError("exceeded maximum number of script variables");

				entry = &scrVarGlob.variableList[index];
				newEntry = &scrVarGlob.variableList[entry->hash.id];
				nextId = newEntry->u.next;
				scrVarGlob.variableList->u.next = nextId;
				scrVarGlob.variableList[nextId].hash.u.prev = 0;
				newEntry->w.status = VAR_STAT_MOVABLE;
				newEntry->v.next = entryValue->v.next;
				entryValue->v.next = index;
			}
			else
			{
				next = entry->v.next;
				newEntryValue = &scrVarGlob.variableList[next];
				newEntry = entry;
				siblingValue.next = newEntryValue->hash.u.prev;
				nextId = entry->u.next;
				scrVarGlob.variableList[scrVarGlob.variableList[siblingValue.next].hash.id].u.next = nextId;
				scrVarGlob.variableList[nextId].hash.u.prevSibling = siblingValue.next;
				newEntryValue->hash.id = entry->hash.id;
				entry->hash.id = index;
				newEntryValue->hash.u.prev = entry->hash.u.prev;
				scrVarGlob.variableList[scrVarGlob.variableList[newEntryValue->hash.u.prev].hash.id].nextSibling = next;
				scrVarGlob.variableList[entryValue->nextSibling].hash.u.prev = next;
				entryValue->w.status &= ~VAR_STAT_MASK;
				entryValue->w.status |= VAR_STAT_MOVABLE;
				newEntry->w.status = VAR_STAT_HEAD;
			}
		}
		else
		{
			if ( entry->w.status & VAR_STAT_MASK )
			{
				next = scrVarGlob.variableList->u.next;

				if ( !scrVarGlob.variableList->u.next )
					Scr_TerminalError("exceeded maximum number of script variables");

				newEntryValue = &scrVarGlob.variableList[next];
				newEntry = &scrVarGlob.variableList[newEntryValue->hash.id];
				nextId = newEntry->u.next;
				scrVarGlob.variableList->u.next = nextId;
				scrVarGlob.variableList[nextId].hash.u.prev = 0;
			}
			else
			{
				next = entry->v.next;
				newEntryValue = &scrVarGlob.variableList[next];
				newEntry = entry;
				siblingValue.next = newEntryValue->hash.u.prev;
				nextId = entry->u.next;
				scrVarGlob.variableList[scrVarGlob.variableList[siblingValue.next].hash.id].u.next = nextId;
				scrVarGlob.variableList[nextId].hash.u.prevSibling = siblingValue.next;
			}

			siblingValue.o.u.self = entryValue->nextSibling;
			scrVarGlob.variableList[scrVarGlob.variableList[entry->hash.u.prev].hash.id].nextSibling = next;
			scrVarGlob.variableList[siblingValue.o.u.self].hash.u.prev = next;

			if ( type == VAR_STAT_MOVABLE )
			{
				siblingValue.o.u.self = entryValue->v.next;

				for ( i = scrVarGlob.variableList[siblingValue.o.u.self].hash.id;
				        scrVarGlob.variableList[i].v.next != index;
				        i = scrVarGlob.variableList[scrVarGlob.variableList[i].v.next].hash.id )
				{
					;
				}

				scrVarGlob.variableList[i].v.next = next;
			}
			else
			{
				entryValue->v.next = next;
			}

			newEntryValue->hash.u.prev = entry->hash.u.prev;
			id = newEntryValue->hash.id;
			newEntryValue->hash.id = entry->hash.id;
			entry->hash.id = id;
			newEntry->w.status = VAR_STAT_HEAD;
			newEntry->v.next = index;
		}
	}
	else
	{
		next = entry->v.next;
		nextId = entryValue->u.next;

		if ( next == entry->hash.id || entry->w.status & VAR_STAT_MASK )
		{
			newEntry = entryValue;
		}
		else
		{
			scrVarGlob.variableList[next].hash.id = entry->hash.id;
			entry->hash.id = index;
			entryValue->v.next = next;
			entryValue->u.next = entry->u.next;
			newEntry = entry;
		}

		siblingValue.next = entry->hash.u.prev;
		scrVarGlob.variableList[scrVarGlob.variableList[siblingValue.next].hash.id].u.next = nextId;
		scrVarGlob.variableList[nextId].hash.u.prevSibling = siblingValue.next;
		newEntry->w.status = VAR_STAT_HEAD;
		newEntry->v.next = index;
	}

	newEntry->w.status = newEntry->w.status;
	newEntry->w.name |= name << VAR_NAME_BITS;
	parentValue = &scrVarGlob.variableList[parentId];

	if ( VAR_TYPE(parentValue) == VAR_ARRAY )
	{
		++parentValue->u.o.u.size;
		value = Scr_GetArrayIndexValue(name);
		AddRefToValue(&value);
	}

	return index;
}

unsigned int GetNewVariableIndexInternal2(unsigned int parentId, unsigned int name, unsigned int index)
{
	unsigned int siblingId;
	VariableValueInternal *entry;
	uint16_t siblingIndex;

	siblingId = GetNewVariableIndexInternal3(parentId, name, index);
	entry = &scrVarGlob.variableList[parentId];
	siblingIndex = entry->nextSibling;
	scrVarGlob.variableList[scrVarGlob.variableList[siblingId].hash.id].nextSibling = siblingIndex;
	scrVarGlob.variableList[siblingIndex].hash.u.prev = siblingId;
	scrVarGlob.variableList[siblingId].hash.u.prev = entry->v.next;
	entry->nextSibling = siblingId;

	return siblingId;
}



unsigned int GetVariableIndexInternal(unsigned int parentId, unsigned int name)
{
	unsigned int index;

	index = FindVariableIndexUnknown(name, (parentId + name) % (VARIABLELIST_CHILD_SIZE - 1) + 1);

	if ( !index )
		return GetNewVariableIndexInternal2(parentId, name, (parentId + name) % (VARIABLELIST_CHILD_SIZE - 1) + 1);

	return index;
}

unsigned int GetArrayVariableIndex(unsigned int parentId, unsigned int index)
{
	return GetVariableIndexInternal(parentId, (index + MAX_ARRAYINDEX) & VAR_NAME_LOW_MASK);
}



unsigned int GetNewVariableIndexInternal(unsigned int parentId, unsigned int name)
{
	return GetNewVariableIndexInternal2(parentId, name, (parentId + name) % (VARIABLELIST_CHILD_SIZE - 1) + 1);
}

unsigned int GetNewVariableIndexReverseInternal2(unsigned int parentId, unsigned int name, unsigned int index)
{
	unsigned int siblingId;
	VariableValueInternal *parent;
	VariableValueInternal *sibling;
	VariableValueInternal_u siblingValue;

	siblingId = GetNewVariableIndexInternal3(parentId, name, index);
	parent = &scrVarGlob.variableList[scrVarGlob.variableList[scrVarGlob.variableList[parentId].nextSibling].hash.u.prev];
	siblingValue.next = parent->hash.u.prev;
	sibling = &scrVarGlob.variableList[scrVarGlob.variableList[siblingValue.next].hash.id];
	scrVarGlob.variableList[scrVarGlob.variableList[siblingId].hash.id].nextSibling = scrVarGlob.variableList[parentId].v.next;
	parent->hash.u.prev = siblingId;
	scrVarGlob.variableList[siblingId].hash.u.prev = siblingValue.next;
	sibling->nextSibling = siblingId;

	return siblingId;
}

unsigned int GetNewVariableIndexReverseInternal(unsigned int parentId, unsigned int name)
{
	return GetNewVariableIndexReverseInternal2(parentId, name, (parentId + name) % (VARIABLELIST_CHILD_SIZE - 1) + 1);
}













unsigned int GetNewArrayVariableIndex(unsigned int parentId, unsigned int index)
{
	return GetNewVariableIndexInternal(parentId, (index + MAX_ARRAYINDEX) & VAR_NAME_LOW_MASK);
}







void SafeRemoveArrayVariable(unsigned int parentId, unsigned int name)
{
	SafeRemoveVariable(parentId, (name + MAX_ARRAYINDEX) & VAR_NAME_LOW_MASK);
}



union VariableValueInternal_u* GetVariableValueAddress_Bad(unsigned int id)
{
	return &scrVarGlob.variableList[id].u;
}









void FreeVariable(unsigned int id)
{
	VariableValueInternal_v value;
	VariableValueInternal *entryValue;
	unsigned short next;
	unsigned short prev;

	entryValue = &scrVarGlob.variableList[id];
	value.next = entryValue->v.next;
	prev = scrVarGlob.variableList[value.next].hash.u.prev;
	next = entryValue->nextSibling;
	scrVarGlob.variableList[next].hash.u.prev = prev;
	scrVarGlob.variableList[scrVarGlob.variableList[prev].hash.id].nextSibling = next;
	entryValue->w.status = 0;
	entryValue->u.next = scrVarGlob.variableList->u.next;
	scrVarGlob.variableList[value.next].hash.u.prev = 0;
	scrVarGlob.variableList[scrVarGlob.variableList->u.next].hash.u.prev = value.next;
	scrVarGlob.variableList->u.next = value.next;
}














void ClearObjectInternal(unsigned int parentId)
{
	unsigned int prevId;
	unsigned int i;
	unsigned int id;
	VariableValueInternal *parentValue;
	VariableValueInternal *value;

	parentValue = &scrVarGlob.variableList[parentId];
	value = &scrVarGlob.variableList[parentValue->nextSibling];

	for ( i = value->hash.id; i != parentId; i = value->hash.id )
	{
		MakeVariableExternal(value, parentValue);
		value = &scrVarGlob.variableList[scrVarGlob.variableList[i].nextSibling];
	}

	id = scrVarGlob.variableList[parentValue->nextSibling].hash.id;

	while ( id != parentId )
	{
		prevId = id;
		id = scrVarGlob.variableList[scrVarGlob.variableList[id].nextSibling].hash.id;
		FreeValue(prevId);
	}
}

void ClearObject(unsigned int parentId)
{
	AddRefToObject(parentId);
	ClearObjectInternal(parentId);
	RemoveRefToEmptyObject(parentId);
}

void RemoveVariable(unsigned int parentId, unsigned int index)
{
	unsigned int id;
	VariableValueInternal *value;

	value = &scrVarGlob.variableList[FindVariableIndexInternal(parentId, index)];
	id = value->hash.id;
	MakeVariableExternal(value, &scrVarGlob.variableList[parentId]);
	FreeValue(id);
}



void RemoveObjectVariable(unsigned int parentId, unsigned int id)
{
	RemoveVariable(parentId, id + SL_MAX_STRING_INDEX);
}

void RemoveArrayVariable(unsigned int parentId, unsigned int unsignedValue)
{
	RemoveVariable(parentId, (unsignedValue + MAX_ARRAYINDEX) & VAR_NAME_LOW_MASK);
}

void RemoveRefToObject(unsigned int id)
{
	VariableValueInternal *entryValue;

	entryValue = &scrVarGlob.variableList[id];

	if ( entryValue->u.o.refCount )
	{
		if ( !--entryValue->u.o.refCount
		        && VAR_TYPE(entryValue) == VAR_ENTITY
		        && scrVarGlob.variableList[entryValue->nextSibling].hash.id == id )
		{
			entryValue->w.type &= ~VAR_MASK;
			entryValue->w.type |= VAR_DEAD_ENTITY;
			RemoveArrayVariable(scrClassMap[entryValue->w.classnum >> VAR_NAME_BITS].entArrayId, entryValue->u.o.u.entnum);
		}
	}
	else
	{
		if ( scrVarGlob.variableList[entryValue->nextSibling].hash.id != id )
			ClearObject(id);

		FreeVariable(id);
	}
}
















unsigned int AllocVariable()
{
	uint16_t newIndex;
	uint16_t next;
	VariableValueInternal *entryValue;
	VariableValueInternal *entry;
	uint16_t index;

	index = scrVarGlob.variableList->u.next;

	if ( !scrVarGlob.variableList->u.next )
		Scr_TerminalError("exceeded maximum number of script variables");

	entry = &scrVarGlob.variableList[index];
	entryValue = &scrVarGlob.variableList[entry->hash.id];
	next = entryValue->u.next;

	if ( entry != entryValue && (entry->w.status & VAR_STAT_MASK) == 0 )
	{
		newIndex = entry->v.next;
		scrVarGlob.variableList[newIndex].hash.id = entry->hash.id;
		entry->hash.id = index;
		entryValue->v.next = newIndex;
		entryValue->u.next = entry->u.next;
		entryValue = &scrVarGlob.variableList[index];
	}

	scrVarGlob.variableList->u.next = next;
	scrVarGlob.variableList[next].hash.u.prev = 0;
	entryValue->v.next = index;
	entryValue->nextSibling = index;
	entry->hash.u.prev = index;

	return entry->hash.id;
}

unsigned int AllocEntity(unsigned int classnum, unsigned short entnum)
{
	unsigned int id;
	VariableValueInternal *entryValue;

	id = AllocVariable();

	entryValue = &scrVarGlob.variableList[id];
	entryValue->w.status = VAR_STAT_EXTERNAL;
	entryValue->w.type |= VAR_ENTITY;
	entryValue->w.classnum  |= classnum << VAR_NAME_BITS;
	entryValue->u.o.refCount = 0;
	entryValue->u.o.u.entnum = entnum;

	return id;
}





float* Scr_AllocVectorInternal()
{
	RefVector *refVec;

	refVec = (RefVector *)MT_Alloc(sizeof(RefVector));
	refVec->head = 0;

	return refVec->vec;
}







void Scr_AllocGameVariable()
{
	if ( !scrVarPub.gameId )
	{
		scrVarPub.gameId = AllocValue();
		SetEmptyArray(scrVarPub.gameId);
	}
}

int Scr_MakeValuePrimitive(unsigned int parentId)
{
	unsigned int name;
	unsigned int i;

	if ( (scrVarGlob.variableList[parentId].w.type & VAR_MASK) != VAR_ARRAY )
		return 0;

find_next_variable:
	for ( i = FindNextSibling(parentId); i; i = FindNextSibling(i) )
	{
		name = scrVarGlob.variableList[i].w.name >> VAR_NAME_BITS;

		switch ( scrVarGlob.variableList[i].w.type & VAR_MASK )
		{
		case VAR_POINTER:
			if ( !Scr_MakeValuePrimitive(scrVarGlob.variableList[i].u.u.pointerValue) )
				goto remove_variable;
			break;

		case VAR_CODEPOS:
		case VAR_PRECODEPOS:
		case VAR_FUNCTION:
		case VAR_STACK:
		case VAR_ANIMATION:
remove_variable:
			RemoveVariable(parentId, name);
			goto find_next_variable;

		default:
			continue;
		}
	}

	return 1;
}

void Scr_FreeGameVariable(int bComplete)
{
	if ( bComplete )
	{
		FreeValue(scrVarPub.gameId);
		scrVarPub.gameId = 0;
	}
	else
	{
		Scr_MakeValuePrimitive(scrVarGlob.variableList[scrVarPub.gameId].u.u.pointerValue);
	}
}

void Scr_FreeObjects()
{
	VariableValueInternal *entryValue;
	unsigned int id;

	for ( id = 1; id < VARIABLELIST_CHILD_SIZE; ++id )
	{
		entryValue = &scrVarGlob.variableList[id];

		if ( entryValue->w.status & VAR_STAT_MASK )
		{
			if ( VAR_TYPE(entryValue) == VAR_OBJECT || VAR_TYPE(entryValue) == VAR_DEAD_ENTITY )
			{
				Scr_CancelNotifyList(id);
				ClearObject(id);
			}
		}
	}
}



void Scr_FreeEntityNum(int entnum, unsigned int classnum)
{
	VariableValueInternal *entryValue;
	unsigned int entId;
	unsigned int entnumId;
	unsigned int entArrayId;

	if ( scrVarPub.bInited )
	{
		entArrayId = scrClassMap[classnum].entArrayId;
		entnumId = FindArrayVariable(entArrayId, entnum);

		if ( entnumId )
		{
			entId = FindObject(entnumId);
			entryValue = &scrVarGlob.variableList[entId];
			entryValue->w.type &= ~VAR_MASK;
			entryValue->w.type |= VAR_DEAD_ENTITY;
			AddRefToObject(entId);
			entryValue->u.o.u.nextEntId = scrVarPub.freeEntList;
			scrVarPub.freeEntList = entId;
			RemoveArrayVariable(entArrayId, entnum);
		}
	}
}





void Scr_ClearThread(unsigned int parentId)
{
	if ( scrVarGlob.variableList[scrVarGlob.variableList[parentId].nextSibling].hash.id != parentId )
		ClearObjectInternal(parentId);

	RemoveRefToObject(scrVarGlob.variableList[parentId].u.o.u.self);
}

void Scr_KillEndonThread(unsigned int threadId)
{
	VariableValueInternal *entryValue;

	entryValue = &scrVarGlob.variableList[threadId];
	RemoveRefToObject(entryValue->u.o.u.self);
	entryValue->w.type &= ~VAR_MASK;
	entryValue->w.type |= VAR_DEAD_THREAD;
}

void Scr_KillThread(unsigned int parentId)
{
	union VariableValueInternal_u *threadvar;
	unsigned int startLocalId;
	unsigned int notifyListEntry;
	unsigned int selfNameId;
	unsigned int id;
	VariableValueInternal *parentValue;

	parentValue = &scrVarGlob.variableList[parentId];
	Scr_ClearThread(parentId);
	id = FindObjectVariable(scrVarPub.pauseArrayId, parentId);

	if ( id )
	{
		for ( selfNameId = FindObject(id); ; RemoveObjectVariable(selfNameId, startLocalId) )
		{
			notifyListEntry = FindNextSibling(selfNameId);

			if ( !notifyListEntry )
				break;

			startLocalId = (unsigned short)(scrVarGlob.variableList[notifyListEntry].w.type >> VAR_NAME_BITS);
			threadvar = GetVariableValueAddress_Bad(FindObjectVariable(selfNameId, startLocalId));
			VM_CancelNotify(threadvar->u.pointerValue, startLocalId);
			Scr_KillEndonThread(startLocalId);
		}

		RemoveObjectVariable(scrVarPub.pauseArrayId, parentId);
	}

	parentValue->w.type &= ~VAR_MASK;
	parentValue->w.type |= VAR_DEAD_THREAD;
}









unsigned int FindArrayVariableIndex(unsigned int parentId, unsigned int index)
{
	return FindVariableIndexInternal(parentId, (index + MAX_ARRAYINDEX) & VAR_NAME_LOW_MASK);
}



bool IsObjectFree(unsigned int id)
{
	return (scrVarGlob.variableList[id].w.status & VAR_STAT_MASK) == 0;
}



void Scr_RemoveClassMap(unsigned int classnum)
{
	if ( scrVarPub.bInited )
	{
		RemoveRefToObject(scrClassMap[classnum].entArrayId);
		scrClassMap[classnum].entArrayId = 0;
		RemoveRefToObject(scrClassMap[classnum].id);
		scrClassMap[classnum].id = 0;
	}
}

unsigned int Scr_FindArrayIndex(unsigned int parentId, VariableValue *index)
{
	unsigned int varIndex;

	if ( index->type == VAR_INTEGER )
	{
		if ( IsValidArrayIndex(index->u.intValue) )
		{
			return FindArrayVariable(parentId, index->u.intValue);
		}
		else
		{
			Scr_Error(va("array index %d out of range", index->u.intValue));
			AddRefToObject(parentId);
			return 0;
		}
	}
	else if ( index->type == VAR_STRING )
	{
		varIndex = FindVariable(parentId, index->u.stringValue);
		SL_RemoveRefToString(index->u.stringValue);
		return varIndex;
	}
	else
	{
		Scr_Error(va("%s is not an array index", var_typename[index->type]));
		AddRefToObject(parentId);
		return 0;
	}
}

void Scr_EvalArray(VariableValue *value, VariableValue *index)
{
	unsigned int ArrayIndex;
	int type;
	VariableValue val;
	char str[2];
	const char *string;
	VariableValueInternal *entryValue;

	type = value->type;

	if ( type == VAR_STRING )
	{
		if ( index->type == VAR_INTEGER )
		{
			if ( index->u.intValue < 0
			        || (string = SL_ConvertToString(value->u.intValue), index->u.intValue >= strlen(string)) )
			{
				Scr_Error(va("string index %d out of range", index->u.intValue));
			}
			else
			{
				index->type = VAR_STRING;
				str[0] = string[index->u.intValue];
				str[1] = 0;
				index->u.stringValue = SL_GetStringOfLen(str, 0, sizeof(str));
				SL_RemoveRefToString(value->u.intValue);
			}
		}
		else
		{
			Scr_Error(va("%s is not a string index", var_typename[index->type]));
		}
	}
	else if ( type > VAR_STRING )
	{
		if ( type != VAR_VECTOR )
		{
			scrVarPub.error_index = 1;
			Scr_Error(va("%s is not an array, string, or vector", var_typename[value->type]));
			return;
		}

		if ( index->type == VAR_INTEGER )
		{
			if ( index->u.intValue > 2 )
			{
				Scr_Error(va("vector index %d out of range", index->u.intValue));
			}
			else
			{
				index->type = VAR_FLOAT;
				index->u.floatValue = value->u.vectorValue[index->u.intValue];
				RemoveRefToVector(value->u.vectorValue);
			}
		}
		else
		{
			Scr_Error(va("%s is not a vector index", var_typename[index->type]));
		}
	}
	else
	{
		if ( type != VAR_POINTER )
		{
			scrVarPub.error_index = 1;
			Scr_Error(va("%s is not an array, string, or vector", var_typename[value->type]));
			return;
		}

		entryValue = &scrVarGlob.variableList[value->u.intValue];

		if ( VAR_TYPE(entryValue) == VAR_ARRAY )
		{
			ArrayIndex = Scr_FindArrayIndex(value->u.pointerValue, index);
			val = Scr_EvalVariable(ArrayIndex);
			index->u = val.u;
			index->type = val.type;
			RemoveRefToObject(value->u.pointerValue);
		}
		else
		{
			scrVarPub.error_index = 1;
			Scr_Error(va("%s is not an array", var_typename[VAR_TYPE(entryValue)]));
		}
	}
}





void CopyArray(unsigned int parentId, unsigned int newParentId)
{
	int i;
	VariableValueInternal *newEntryValue;
	VariableValueInternal *entryValue;
	int type;

	for ( i = scrVarGlob.variableList[scrVarGlob.variableList[parentId].nextSibling].hash.id;
	        i != parentId;
	        i = scrVarGlob.variableList[scrVarGlob.variableList[i].nextSibling].hash.id )
	{
		entryValue = &scrVarGlob.variableList[i];
		type = VAR_TYPE(entryValue);

		newEntryValue = &scrVarGlob.variableList[scrVarGlob.variableList[GetVariableIndexInternal(
		                    newParentId,
		                    entryValue->w.classnum >> VAR_NAME_BITS)].hash.id];

		newEntryValue->w.type |= type;

		if ( type == VAR_POINTER )
		{
			if ( (scrVarGlob.variableList[entryValue->u.u.pointerValue].w.type & VAR_MASK) == VAR_ARRAY )
			{
				newEntryValue->u.u.pointerValue = Scr_AllocArray();
				CopyArray(entryValue->u.u.pointerValue, newEntryValue->u.u.pointerValue);
			}
			else
			{
				newEntryValue->u.u = entryValue->u.u;
				AddRefToObject(entryValue->u.u.pointerValue);
			}
		}
		else
		{
			newEntryValue->u.u = entryValue->u.u;
			AddRefToValue(type, entryValue->u.u);
		}
	}
}

void Scr_EvalVariableEntityField(VariableValue *pValue, unsigned int entId, unsigned int name)
{
	VariableValue val;
	unsigned int id;
	unsigned int fieldId;
	VariableValueInternal *entryValue;
	VariableValueInternal *objectValue;

	entryValue = &scrVarGlob.variableList[entId];
	fieldId = FindArrayVariable(scrClassMap[(entryValue->w.classnum >> VAR_NAME_BITS)].id, name);

	if ( fieldId )
	{
		GetEntityFieldValue(
		    &val,
		    entryValue->w.classnum >> VAR_NAME_BITS,
		    entryValue->u.o.u.entnum,
		    scrVarGlob.variableList[fieldId].u.u.entityOffset);

		pValue->u = val.u;
		pValue->type = val.type;

		if ( pValue->type == VAR_POINTER )
		{
			objectValue = &scrVarGlob.variableList[pValue->u.pointerValue];

			if ( VAR_TYPE(objectValue) == VAR_ARRAY )
			{
				if ( objectValue->u.o.refCount )
				{
					id = pValue->u.pointerValue;
					RemoveRefToObject(id);
					pValue->u.pointerValue = Scr_AllocArray();
					CopyArray(id, pValue->u.pointerValue);
				}
			}
		}
	}
	else
	{
		pValue->u.intValue = 0;
		pValue->type = VAR_UNDEFINED;
	}
}

void Scr_FindVariableFieldInternal(VariableValue *pValue, unsigned int parentId, unsigned int name)
{
	unsigned int index;

	index = FindVariable(parentId, name);

	if ( index )
	{
		*pValue = Scr_EvalVariable(index);
	}
	else if ( (scrVarGlob.variableList[parentId].w.type & VAR_MASK) == VAR_ENTITY )
	{
		Scr_EvalVariableEntityField(pValue, parentId, name);
	}
	else
	{
		pValue->u.intValue = 0;
		pValue->type = VAR_UNDEFINED;
	}
}

VariableValue Scr_FindVariableField(unsigned int parentId, unsigned int name)
{
	VariableValue value;

	Scr_FindVariableFieldInternal(&value, parentId, name);
	return value;
}

void Scr_EvalVariableFieldInternal(VariableValue *pValue, unsigned int id)
{
	VariableValue value;

	if ( id == VARIABLELIST_CHILD_SIZE )
	{
		Scr_EvalVariableEntityField(&value, scrVarPub.entId, scrVarPub.entFieldName);
	}
	else
	{
		value = Scr_EvalVariable(id);
	}

	*pValue = value;
}



void Scr_UnmatchingTypesError(VariableValue *value1, VariableValue *value2)
{
	int type1;
	int type2;
	const char *error_message;

	if ( scrVarPub.error_message )
	{
		error_message = NULL;
	}
	else
	{
		type1 = value1->type;
		type2 = value2->type;
		Scr_CastDebugString(value1);
		Scr_CastDebugString(value2);

		error_message = va("pair '%s' and '%s' has unmatching types '%s' and '%s'", SL_ConvertToString(value1->u.stringValue), SL_ConvertToString(value2->u.stringValue), var_typename[type1], var_typename[type2]);
	}

	RemoveRefToValue(value1);
	value1->type = VAR_UNDEFINED;
	RemoveRefToValue(value2);
	value2->type = VAR_UNDEFINED;
	Scr_Error(error_message);
}

VariableValue Scr_EvalVariableField(unsigned int id)
{
	VariableValue value;

	Scr_EvalVariableFieldInternal(&value, id);
	return value;
}

void Scr_EvalOr(VariableValue *value1, VariableValue *value2)
{
	if ( value1->type == VAR_INTEGER && value2->type == VAR_INTEGER )
		value1->u.intValue |= value2->u.intValue;
	else
		Scr_UnmatchingTypesError(value1, value2);
}

void Scr_EvalExOr(VariableValue *value1, VariableValue *value2)
{
	if ( value1->type == VAR_INTEGER && value2->type == VAR_INTEGER )
		value1->u.intValue ^= value2->u.intValue;
	else
		Scr_UnmatchingTypesError(value1, value2);
}

void Scr_EvalAnd(VariableValue *value1, VariableValue *value2)
{
	if ( value1->type == VAR_INTEGER && value2->type == VAR_INTEGER )
		value1->u.intValue &= value2->u.intValue;
	else
		Scr_UnmatchingTypesError(value1, value2);
}

void Scr_CastWeakerPair(VariableValue *value1, VariableValue *value2)
{
	int type1;
	int type2;

	type1 = value1->type;
	type2 = value2->type;

	if ( type1 != type2 )
	{
		if ( type1 == VAR_FLOAT && type2 == VAR_INTEGER )
		{
			value2->type = VAR_FLOAT;
			value2->u.floatValue = (float)value2->u.intValue;
		}
		else if ( type1 == VAR_INTEGER && type2 == VAR_FLOAT )
		{
			value1->type = VAR_FLOAT;
			value1->u.floatValue = (float)value1->u.intValue;
		}
		else
		{
			Scr_UnmatchingTypesError(value1, value2);
		}
	}
}

void Scr_EvalEquality(VariableValue *value1, VariableValue *value2)
{
	int tempInt;
	int equal;

	Scr_CastWeakerPair(value1, value2);

	switch ( value1->type )
	{
	case VAR_UNDEFINED:
		value1->type = VAR_INTEGER;
		value1->u.intValue = 1;
		break;

	case VAR_STRING:
	case VAR_ISTRING:
		value1->type = VAR_INTEGER;
		tempInt = value1->u.intValue == value2->u.intValue;
		SL_RemoveRefToString(value1->u.intValue);
		SL_RemoveRefToString(value2->u.intValue);
		value1->u.intValue = tempInt;
		break;

	case VAR_VECTOR:
		value1->type = VAR_INTEGER;
		equal = VectorCompare(value1->u.vectorValue, value2->u.vectorValue);
		RemoveRefToVector(value1->u.vectorValue);
		RemoveRefToVector(value2->u.vectorValue);
		value1->u.intValue = equal;
		break;

	case VAR_FLOAT:
		value1->type = VAR_INTEGER;
		value1->u.intValue = I_fabs(value1->u.floatValue - value2->u.floatValue) < 0.000001;
		break;

	case VAR_INTEGER:
		value1->u.intValue = value1->u.intValue == value2->u.intValue;
		break;

	case VAR_FUNCTION:
		value1->type = VAR_INTEGER;
		value1->u.intValue = value1->u.intValue == value2->u.intValue;
		break;

	case VAR_ANIMATION:
		value1->type = VAR_INTEGER;
		value1->u.intValue = value1->u.intValue == value2->u.intValue;
		break;

	case VAR_POINTER:
		if ( !(((scrVarGlob.variableList[value1->u.intValue].w.type & VAR_MASK) == VAR_ARRAY ||
		        (scrVarGlob.variableList[value2->u.intValue].w.type & VAR_MASK) == VAR_ARRAY)
		        && !scrVarPub.evaluate ))
		{
			value1->type = VAR_INTEGER;
			equal = value1->u.intValue == value2->u.intValue;
			RemoveRefToObject(value1->u.intValue);
			RemoveRefToObject(value2->u.intValue);
			value1->u.intValue = equal;
			break;
		}

	default:
		Scr_UnmatchingTypesError(value1, value2);
		break;
	}
}

void Scr_EvalInequality(VariableValue *value1, VariableValue *value2)
{
	Scr_EvalEquality(value1, value2);
	value1->u.intValue = value1->u.intValue == 0;
}

void Scr_EvalLess(VariableValue *value1, VariableValue *value2)
{
	int type;

	Scr_CastWeakerPair(value1, value2);
	type = value1->type;

	if ( type == VAR_FLOAT )
	{
		value1->type = VAR_INTEGER;
		value1->u.intValue = value2->u.floatValue > (float)value1->u.floatValue;
	}
	else if ( type == VAR_INTEGER )
	{
		value1->u.intValue = value1->u.intValue < value2->u.intValue;
	}
	else
	{
		Scr_UnmatchingTypesError(value1, value2);
	}
}

void Scr_EvalGreater(VariableValue *value1, VariableValue *value2)
{
	int type;

	Scr_CastWeakerPair(value1, value2);
	type = value1->type;

	if ( type == VAR_FLOAT )
	{
		value1->type = VAR_INTEGER;
		value1->u.intValue = value1->u.floatValue > (float)value2->u.floatValue;
	}
	else if ( type == VAR_INTEGER )
	{
		value1->u.intValue = value1->u.intValue > value2->u.intValue;
	}
	else
	{
		Scr_UnmatchingTypesError(value1, value2);
	}
}

void Scr_EvalLessEqual(VariableValue *value1, VariableValue *value2)
{
	Scr_EvalGreater(value1, value2);
	value1->u.intValue = value1->u.intValue == 0;
}

void Scr_EvalGreaterEqual(VariableValue *value1, VariableValue *value2)
{
	Scr_EvalLess(value1, value2);
	value1->u.intValue = value1->u.intValue == 0;
}

void Scr_EvalShiftLeft(VariableValue *value1, VariableValue *value2)
{
	if ( value1->type == VAR_INTEGER && value2->type == VAR_INTEGER )
		value1->u.intValue <<= value2->u.intValue;
	else
		Scr_UnmatchingTypesError(value1, value2);
}

void Scr_EvalShiftRight(VariableValue *value1, VariableValue *value2)
{
	if ( value1->type == VAR_INTEGER && value2->type == VAR_INTEGER )
		value1->u.intValue >>= value2->u.intValue;
	else
		Scr_UnmatchingTypesError(value1, value2);
}

void Scr_CastWeakerStringPair(VariableValue *value1, VariableValue *value2)
{
	const float *constTempVector;
	int type1;
	int type2;

	type1 = value1->type;
	type2 = value2->type;

	if ( type1 != type2 )
	{
		if ( type1 < type2 )
		{
			if ( type1 == VAR_STRING )
			{
				switch ( type2 )
				{
				case VAR_VECTOR:
					value2->type = VAR_STRING;
					constTempVector = value2->u.vectorValue;
					value2->u.intValue = SL_GetStringForVector(value2->u.vectorValue);
					RemoveRefToVector(constTempVector);
					return;

				case VAR_FLOAT:
					value2->type = VAR_STRING;
					value2->u.intValue = SL_GetStringForFloat(value2->u.floatValue);
					return;

				case VAR_INTEGER:
					value2->type = VAR_STRING;
					value2->u.intValue = SL_GetStringForInt(value2->u.intValue);
					return;
				}
			}
			else if ( type1 != VAR_FLOAT )
			{
				Scr_UnmatchingTypesError(value1, value2);
				return;
			}

			if ( type2 == VAR_INTEGER )
			{
				value2->type = VAR_FLOAT;
				value2->u.floatValue = (float)value2->u.intValue;
				return;
			}

			Scr_UnmatchingTypesError(value1, value2);
			return;
		}

		if ( type2 == VAR_STRING )
		{
			switch ( type1 )
			{
			case VAR_VECTOR:
				value1->type = VAR_STRING;
				constTempVector = value1->u.vectorValue;
				value1->u.intValue = SL_GetStringForVector(value1->u.vectorValue);
				RemoveRefToVector(constTempVector);
				return;

			case VAR_FLOAT:
				value1->type = VAR_STRING;
				value1->u.intValue = SL_GetStringForFloat(value1->u.floatValue);
				return;

			case VAR_INTEGER:
				value1->type = VAR_STRING;
				value1->u.intValue = SL_GetStringForInt(value1->u.intValue);
				return;
			}
		}
		else if ( type2 != VAR_FLOAT )
		{
			Scr_UnmatchingTypesError(value1, value2);
			return;
		}

		if ( type1 == VAR_INTEGER )
		{
			value1->type = VAR_FLOAT;
			value1->u.floatValue = (float)value1->u.intValue;
			return;
		}

		Scr_UnmatchingTypesError(value1, value2);
		return;
	}
}

void Scr_EvalPlus(VariableValue *value1, VariableValue *value2)
{
	unsigned int s;
	float *vec;
	int s1len;
	const char *s1;
	const char *s2;
	char str[8192];
	unsigned int len;

	Scr_CastWeakerStringPair(value1, value2);

	switch ( value1->type )
	{
	case VAR_STRING:
		s1 = SL_ConvertToString(value1->u.stringValue);
		s2 = SL_ConvertToString(value2->u.stringValue);
		s1len = SL_GetStringLen(value1->u.stringValue);
		len = s1len + SL_GetStringLen(value2->u.stringValue) + 1;

		if (len >= sizeof(str))
		{
			SL_RemoveRefToString(value1->u.stringValue);
			SL_RemoveRefToString(value2->u.stringValue);
			value1->type = VAR_UNDEFINED;
			value2->type = VAR_UNDEFINED;
			Scr_Error(va("cannot concat \"%s\" and \"%s\" - max string length exceeded", s1, s2));
			return;
		}

		strcpy(str, s1);
		strcpy(str + s1len, s2);

		s = SL_GetStringOfLen(str, 0, len);

		SL_RemoveRefToString(value1->u.stringValue);
		SL_RemoveRefToString(value2->u.stringValue);
		value1->u.stringValue = s;
		break;

	case VAR_VECTOR:
		vec = Scr_AllocVectorInternal();
		VectorAdd(value1->u.vectorValue, value2->u.vectorValue, vec);
		RemoveRefToVector(value1->u.vectorValue);
		RemoveRefToVector(value2->u.vectorValue);
		value1->u.vectorValue = vec;
		break;

	case VAR_FLOAT:
		value1->u.floatValue = value1->u.floatValue + value2->u.floatValue;
		break;

	case VAR_INTEGER:
		value1->u.intValue += value2->u.intValue;
		break;

	default:
		Scr_UnmatchingTypesError(value1, value2);
		break;
	}
}

void Scr_EvalMinus(VariableValue *value1, VariableValue *value2)
{
	float *tempVector;

	Scr_CastWeakerPair(value1, value2);

	switch ( value1->type )
	{
	case VAR_VECTOR:
		tempVector = Scr_AllocVectorInternal();
		VectorSubtract(value1->u.vectorValue, value2->u.vectorValue, tempVector);
		RemoveRefToVector(value1->u.vectorValue);
		RemoveRefToVector(value2->u.vectorValue);
		value1->u.vectorValue = tempVector;
		break;

	case VAR_FLOAT:
		value1->u.floatValue = value1->u.floatValue - value2->u.floatValue;
		break;

	case VAR_INTEGER:
		value1->u.intValue -= value2->u.intValue;
		break;

	default:
		Scr_UnmatchingTypesError(value1, value2);
		break;
	}
}

void Scr_EvalMultiply(VariableValue *value1, VariableValue *value2)
{
	int type;

	Scr_CastWeakerPair(value1, value2);
	type = value1->type;

	if ( type == VAR_FLOAT )
	{
		value1->u.floatValue = value1->u.floatValue * value2->u.floatValue;
	}
	else if ( type == VAR_INTEGER )
	{
		value1->u.intValue *= value2->u.intValue;
	}
	else
	{
		Scr_UnmatchingTypesError(value1, value2);
	}
}

void Scr_EvalDivide(VariableValue *value1, VariableValue *value2)
{
	int type;

	Scr_CastWeakerPair(value1, value2);
	type = value1->type;

	if ( type == VAR_FLOAT )
	{
		if ( value2->u.floatValue == 0.0 )
		{
			value1->u.intValue = 0;
			Scr_Error("divide by 0");
			return;
		}

		value1->u.floatValue = value1->u.floatValue / value2->u.floatValue;
	}
	else
	{
		if ( type == VAR_INTEGER )
		{
			value1->type = VAR_FLOAT;

			if ( value2->u.intValue )
			{
				value1->u.floatValue = (float)value1->u.intValue / (float)value2->u.intValue;
				return;
			}

			value1->u.intValue = 0;
			Scr_Error("divide by 0");
			return;
		}

		Scr_UnmatchingTypesError(value1, value2);
	}
}

void Scr_EvalMod(VariableValue *value1, VariableValue *value2)
{
	if ( value1->type == VAR_INTEGER && value2->type == VAR_INTEGER )
	{
		if ( value2->u.intValue )
		{
			value1->u.intValue %= value2->u.intValue;
		}
		else
		{
			value1->u.intValue = 0;
			Scr_Error("divide by 0");
		}
	}
	else
	{
		Scr_UnmatchingTypesError(value1, value2);
	}
}

void Scr_EvalSizeValue(VariableValue *value)
{
	unsigned int stringValue;
	char *error_message;
	VariableValueInternal *entryValue;
	unsigned int id;

	if ( value->type == VAR_POINTER )
	{
		id = value->u.intValue;
		entryValue = &scrVarGlob.variableList[value->u.intValue];
		value->type = VAR_INTEGER;

		if ( VAR_TYPE(entryValue) == VAR_ARRAY )
		{
			value->u.intValue = entryValue->u.o.u.size;
		}
		else
		{
			value->u.intValue = 1;
		}

		RemoveRefToObject(id);
	}
	else if ( value->type == VAR_STRING )
	{
		value->type = VAR_INTEGER;
		stringValue = value->u.stringValue;
		value->u.intValue = strlen(SL_ConvertToString(value->u.intValue));
		SL_RemoveRefToString(stringValue);
	}
	else
	{
		error_message = va("size cannot be applied to %s", var_typename[value->type]);
		RemoveRefToValue(value);
		value->type = VAR_UNDEFINED;
		Scr_Error(error_message);
	}
}




void Scr_EvalBinaryOperator(int op, VariableValue *value1, VariableValue *value2)
{
	switch ( op )
	{
	case OP_bit_or:
		Scr_EvalOr(value1, value2);
		break;

	case OP_bit_ex_or:
		Scr_EvalExOr(value1, value2);
		break;

	case OP_bit_and:
		Scr_EvalAnd(value1, value2);
		break;

	case OP_equality:
		Scr_EvalEquality(value1, value2);
		break;

	case OP_inequality:
		Scr_EvalInequality(value1, value2);
		break;

	case OP_less:
		Scr_EvalLess(value1, value2);
		break;

	case OP_greater:
		Scr_EvalGreater(value1, value2);
		break;

	case OP_less_equal:
		Scr_EvalLessEqual(value1, value2);
		break;

	case OP_greater_equal:
		Scr_EvalGreaterEqual(value1, value2);
		break;

	case OP_shift_left:
		Scr_EvalShiftLeft(value1, value2);
		break;

	case OP_shift_right:
		Scr_EvalShiftRight(value1, value2);
		break;

	case OP_plus:
		Scr_EvalPlus(value1, value2);
		break;

	case OP_minus:
		Scr_EvalMinus(value1, value2);
		break;

	case OP_multiply:
		Scr_EvalMultiply(value1, value2);
		break;

	case OP_divide:
		Scr_EvalDivide(value1, value2);
		break;

	case OP_mod:
		Scr_EvalMod(value1, value2);
		break;

	default:
		return;
	}
}

unsigned int GetObjectA(unsigned int id)
{
	VariableValueInternal *entryValue;

	entryValue = &scrVarGlob.variableList[id];

	if ( VAR_TYPE(entryValue) == VAR_UNDEFINED )
	{
		entryValue->w.type |= VAR_POINTER;
		entryValue->u.u.pointerValue = AllocObject();
	}

	return entryValue->u.u.pointerValue;
}

unsigned int FreeTempVariableObject()
{
	ClearVariableValue(scrVarPub.tempVariable);
	return GetObjectA(scrVarPub.tempVariable);
}

unsigned int FreeTempVariable()
{
	ClearVariableValue(scrVarPub.tempVariable);
	return scrVarPub.tempVariable;
}



void Scr_FreeValue(unsigned int id)
{
	RemoveRefToObject(id);
}

void Scr_StopThread(unsigned int threadId)
{
	Scr_ClearThread(threadId);
	scrVarGlob.variableList[threadId].u.o.u.self = scrVarPub.levelId;
	AddRefToObject(scrVarPub.levelId);
}

void Scr_FreeEntityList()
{
	unsigned int id;
	VariableValueInternal *entryValue;

	while ( scrVarPub.freeEntList )
	{
		id = scrVarPub.freeEntList;
		entryValue = &scrVarGlob.variableList[scrVarPub.freeEntList];
		scrVarPub.freeEntList = entryValue->u.o.u.entnum;
		entryValue->u.o.u.entnum = 0;

		Scr_CancelNotifyList(id);

		if ( scrVarGlob.variableList[entryValue->nextSibling].hash.id != id )
			ClearObjectInternal(id);

		RemoveRefToObject(id);
	}
}



void ClearVariableField(unsigned int parentId, unsigned int name, VariableValue *value)
{
	unsigned int classnum;
	VariableValueInternal *entryValue;
	unsigned int fieldId;
	VariableValue *nextValue;

	if ( FindVariableIndexInternal(parentId, name) )
	{
		RemoveVariable(parentId, name);
	}
	else
	{
		entryValue = &scrVarGlob.variableList[parentId];

		if ( VAR_TYPE(entryValue) == VAR_ENTITY )
		{
			classnum = entryValue->w.classnum >> VAR_NAME_BITS;
			fieldId = FindArrayVariable(scrClassMap[classnum].id, name);

			if ( fieldId )
			{
				nextValue = value + 1;
				nextValue->type = VAR_UNDEFINED;
				SetEntityFieldValue(classnum, entryValue->u.o.u.entnum, scrVarGlob.variableList[fieldId].u.u.entityOffset, nextValue);
			}
		}
	}
}



void ClearArray(unsigned int parentId, VariableValue *value)
{
	VariableValue entityVal;
	unsigned int fieldId;
	VariableValue varValue;
	unsigned int id;
	VariableValueInternal *newEntryValue;
	VariableValueInternal *parentValue;
	VariableValueInternal *entryValue;

	if ( parentId == VARIABLELIST_CHILD_SIZE )
	{
		newEntryValue = &scrVarGlob.variableList[scrVarPub.entId];
		fieldId = FindArrayVariable(
		              scrClassMap[(newEntryValue->w.classnum >> VAR_NAME_BITS)].id,
		              scrVarPub.entFieldName);

		if ( !fieldId
		        || (GetEntityFieldValue(
		                &entityVal,
		                newEntryValue->w.classnum >> VAR_NAME_BITS,
		                newEntryValue->u.o.u.entnum,
		                scrVarGlob.variableList[fieldId].u.u.entityOffset),
		            varValue = entityVal,
		            entityVal.type == VAR_UNDEFINED) )
		{
			varValue.type = VAR_UNDEFINED;
			scrVarPub.error_index = 1;
			Scr_Error(va("%s is not an array", var_typename[varValue.type]));
			return;
		}

		if ( varValue.type == VAR_POINTER && !scrVarGlob.variableList[varValue.u.pointerValue].u.o.refCount )
		{
			RemoveRefToValue(&varValue);
			scrVarPub.error_index = 1;
			Scr_Error("read-only array cannot be changed");
			return;
		}

		RemoveRefToValue(&varValue);
		entryValue = 0;
	}
	else
	{
		entryValue = &scrVarGlob.variableList[parentId];
		varValue.type = VAR_TYPE(entryValue);
		varValue.u = entryValue->u.u;
	}

	if ( varValue.type != VAR_POINTER )
	{
		scrVarPub.error_index = 1;
		Scr_Error(va("%s is not an array", var_typename[varValue.type]));
		return;
	}

	parentValue = &scrVarGlob.variableList[varValue.u.pointerValue];

	if ( VAR_TYPE(parentValue) == VAR_ARRAY )
	{
		if ( parentValue->u.o.refCount )
		{
			id = varValue.u.pointerValue;
			RemoveRefToObject(varValue.u.pointerValue);
			varValue.u.pointerValue = Scr_AllocArray();
			CopyArray(id, varValue.u.pointerValue);
			entryValue->u.u = varValue.u;
		}

		if ( value->type == VAR_INTEGER )
		{
			if ( IsValidArrayIndex(value->u.pointerValue) )
			{
				SafeRemoveArrayVariable(varValue.u.pointerValue, value->u.stringValue);
			}
			else
			{
				Scr_Error(va("array index %d out of range", value->u.pointerValue));
			}
		}
		else if ( value->type == VAR_STRING )
		{
			SL_RemoveRefToString(value->u.stringValue);
			SafeRemoveVariable(varValue.u.pointerValue, value->u.stringValue);
		}
		else
		{
			Scr_Error(va("%s is not an array index", var_typename[value->type]));
		}
	}
	else
	{
		scrVarPub.error_index = 1;
		Scr_Error(va("%s is not an array", var_typename[VAR_TYPE(parentValue)]));
	}
}

unsigned int Scr_EvalArrayRef(unsigned int parentId)
{
	VariableValue varValue;
	unsigned int fieldId;
	VariableValue value;
	unsigned int id;
	VariableValueInternal *entValue;
	VariableValueInternal *entryValue;
	VariableValueInternal *parentValue;

	if ( parentId != VARIABLELIST_CHILD_SIZE )
	{
		parentValue = &scrVarGlob.variableList[parentId];
		value.type = VAR_TYPE(parentValue);

		if ( value.type )
		{
			value.u = parentValue->u.u;
			goto add_array;
		}

		parentValue->w.status |= VAR_POINTER;
		parentValue->u.u.pointerValue = Scr_AllocArray();
		return parentValue->u.u.pointerValue;
	}

	entValue = &scrVarGlob.variableList[scrVarPub.entId];
	fieldId = FindArrayVariable(scrClassMap[(entValue->w.classnum >> VAR_NAME_BITS)].id, scrVarPub.entFieldName);

	if ( !fieldId
	        || (GetEntityFieldValue(
	                &varValue,
	                entValue->w.classnum >> VAR_NAME_BITS,
	                entValue->u.o.u.entnum,
	                scrVarGlob.variableList[fieldId].u.u.entityOffset),
	            value = varValue,
	            varValue.type == VAR_UNDEFINED) )
	{
		parentValue = &scrVarGlob.variableList[GetNewVariable(scrVarPub.entId, scrVarPub.entFieldName)];
		parentValue->w.status |= VAR_POINTER;
		parentValue->u.u.pointerValue = Scr_AllocArray();
		return parentValue->u.u.pointerValue;
	}

	if ( value.type == VAR_POINTER && !scrVarGlob.variableList[value.u.pointerValue].u.o.refCount )
	{
		RemoveRefToValue(&value);
		scrVarPub.error_index = 1;
		Scr_Error("read-only array cannot be changed");
		return 0;
	}

	RemoveRefToValue(&value);
	parentValue = 0;

add_array:
	if ( value.type == VAR_POINTER )
	{
		entryValue = &scrVarGlob.variableList[value.u.pointerValue];

		if ( VAR_TYPE(entryValue) == VAR_ARRAY )
		{
			if ( entryValue->u.o.refCount )
			{
				id = value.u.pointerValue;
				RemoveRefToObject(value.u.pointerValue);
				value.u.pointerValue = Scr_AllocArray();
				CopyArray(id, value.u.pointerValue);
				parentValue->u.u = value.u;
			}

			return value.u.pointerValue;
		}
		else
		{
			scrVarPub.error_index = 1;
			Scr_Error(va("%s is not an array", var_typename[VAR_TYPE(entryValue)]));
			return 0;
		}
	}
	else
	{
		scrVarPub.error_index = 1;

		if ( value.type == VAR_STRING )
		{
			Scr_Error("string characters cannot be individually changed");
			return 0;
		}
		else
		{
			if ( value.type == VAR_VECTOR )
			{
				Scr_Error("vector components cannot be individually changed");
			}
			else
			{
				Scr_Error(va("%s is not an array", var_typename[value.type]));
			}

			return 0;
		}
	}
}

void SetVariableEntityFieldValue(unsigned int entId, unsigned int fieldName, VariableValue *value)
{
	unsigned int entryValue;
	unsigned int fieldId;

	fieldId = FindArrayVariable(scrClassMap[scrVarGlob.variableList[entId].w.classnum >> VAR_NAME_BITS].id, fieldName);

	if ( !fieldId
	        || !SetEntityFieldValue(
	            scrVarGlob.variableList[entId].w.classnum >> VAR_NAME_BITS,
	            scrVarGlob.variableList[entId].u.o.u.entnum,
	            scrVarGlob.variableList[fieldId].u.u.entityOffset,
	            value) )
	{
		entryValue = GetNewVariable(entId, fieldName);
		scrVarGlob.variableList[entryValue].w.type |= value->type;
		scrVarGlob.variableList[entryValue].u.u = value->u;
	}
}



void SetVariableFieldValue(unsigned int id, VariableValue *value)
{
	if ( id == VARIABLELIST_CHILD_SIZE )
		SetVariableEntityFieldValue(scrVarPub.entId, scrVarPub.entFieldName, value);
	else
		SetVariableValue(id, value);
}

















float Scr_GetObjectEntryUsage(VariableValueInternal *entryValue)
{
	return Scr_GetEntryUsage(VAR_TYPE(entryValue), entryValue->u.u) + 1.0;
}

float Scr_GetEntryUsage(unsigned int type, VariableUnion u)
{
	if ( type == VAR_POINTER && (scrVarGlob.variableList[u.intValue].w.type & VAR_MASK) == VAR_ARRAY )
		return Scr_GetObjectUsage(u.pointerValue) / ((float)scrVarGlob.variableList[u.intValue].u.o.refCount + 1.0);
	else
		return 0.0;
}

float Scr_GetObjectUsage(unsigned int parentId)
{
	float usage;
	unsigned int i;

	usage = 1.0;

	for ( i = FindNextSibling(parentId); i; i = FindNextSibling(i) )
		usage = Scr_GetObjectEntryUsage(&scrVarGlob.variableList[i]) + usage;

	return usage;
}

float Scr_GetEndonUsage(unsigned int parentId)
{
	unsigned int id;
	unsigned int localId;

	localId = FindObjectVariable(scrVarPub.pauseArrayId, parentId);

	if ( localId )
	{
		id = FindObject(localId);
		return Scr_GetObjectUsage(id);
	}
	else
	{
		return 0.0;
	}
}

float Scr_GetThreadUsage(VariableStackBuffer *stackBuf, float *endonUsage)
{
	unsigned int localId;
	float usage;
	int size;
	char *buf;
	char *pos;
	VariableUnion u;

	size = stackBuf->size;
	buf = &stackBuf->buf[5 * size];
	usage = Scr_GetObjectUsage(stackBuf->localId);
	*endonUsage = Scr_GetEndonUsage(stackBuf->localId);
	localId = stackBuf->localId;

	while ( size )
	{
		pos = buf - 4;
		u.intValue = *(int *)pos;
		buf = pos - 1;
		--size;

		if ( *buf == VAR_CODEPOS )
		{
			localId = GetParentLocalId(localId);
			usage = Scr_GetObjectUsage(localId) + usage;
			*endonUsage = Scr_GetEndonUsage(localId) + *endonUsage;
		}
		else
		{
			usage = Scr_GetEntryUsage((uint8_t)*buf, u) + usage;
		}
	}

	return usage;
}

int ThreadInfoCompare(const void *ainfo1, const void *ainfo2)
{
	const char *pos1;
	int i;
	const char *pos2;
	ThreadDebugInfo *info1, *info2;
	info1 = (ThreadDebugInfo *)ainfo1;
	info2 = (ThreadDebugInfo *)ainfo2;

	for ( i = 0; ; ++i )
	{
		if ( i >= info1->posSize || i >= info2->posSize )
		{
			return info1->posSize - info2->posSize;
		}

		pos1 = info1->pos[i];
		pos2 = info2->pos[i];

		if ( pos1 != pos2 )
		{
			break;
		}
	}

	return pos1 - pos2;
}




void Scr_AddFieldsForFile(const char *filename)
{
	size_t strsize;
	char *lwr;
	fileHandle_t f;
	int dataType;
	int i;
	int size;
	unsigned int index;
	int type;
	char *targetPos;
	char *token;
	const char *sourcePos;
	char *buffer;
	int len;

	len = FS_FOpenFileByMode(filename, &f, FS_READ);

	if ( len < 0 )
	{
		Com_Error(ERR_DROP, va("cannot find '%s'", filename));
	}

	buffer = (char *)Hunk_AllocateTempMemoryHighInternal(len + 1);
	FS_Read(buffer, len, f);
	buffer[len] = 0;
	FS_FCloseFile(f);
	sourcePos = buffer;

	Com_BeginParseSession("Scr_AddFields");

	while ( 1 )
	{
		token = Com_Parse(&sourcePos);

		if ( !sourcePos )
			break;

		if ( !strcmp(token, "float") )
		{
			type = VAR_FLOAT;
		}
		else if ( !strcmp(token, "int") )
		{
			type = VAR_INTEGER;
		}
		else if ( !strcmp(token, "string") )
		{
			type = VAR_STRING;
		}
		else
		{
			if ( strcmp(token, "vector") )
			{
				Com_Error(ERR_DROP, va("unknown type '%s' in '%s'", token, filename));
			}
			type = VAR_VECTOR;
		}

		token = Com_Parse(&sourcePos);

		if ( !sourcePos )
		{
			Com_Error(ERR_DROP, va("missing field name in '%s'", filename));
		}

		strsize = strlen(token);
		len = strsize + 1;

		for ( i = strsize; i >= 0; --i )
		{
			lwr = &token[i];
			*lwr = tolower(token[i]);
		}

		index = SL_GetCanonicalString(token);

		if ( Scr_FindField(token, &dataType) )
			Com_Error(ERR_DROP, "duplicate key '%s' in '%s'", token, filename);

		size = len + 3;
		targetPos = (char *)TempMalloc(len + 3);
		strcpy(targetPos, token);
		targetPos += len;
		*(uint16_t *)targetPos = index;
		targetPos += 2;
		*targetPos++ = type;
		*targetPos = 0;
	}

	Com_EndParseSession();
	Hunk_ClearTempMemoryHighInternal();
}

void Scr_AddFields(const char *path, const char *extension)
{
	char s[MAX_QPATH];
	int i;
	char **fields;
	int numfields;

	fields = FS_ListFiles(path, extension, FS_LIST_PURE_ONLY, &numfields);
	TempMemoryReset();
	scrVarPub.fieldBuffer = (const char *)Hunk_AllocLowInternal(0);
	*(char *)scrVarPub.fieldBuffer = 0;

	for ( i = 0; i < numfields; ++i )
	{
		snprintf(s, sizeof(s), "%s/%s", path, fields[i]);
		Scr_AddFieldsForFile(s);
	}

	if ( fields )
		FS_FreeFileList(fields);

	*(char *)TempMalloc(1) = 0;
	Hunk_ConvertTempToPermLowInternal();
}

void Var_ResetAll()
{
	union Variable_u hash;
	VariableValueInternal *entryValue;
	unsigned int i;

	hash.prev = 0;

	for ( i = 1; i < VARIABLELIST_CHILD_SIZE; ++i )
	{
		entryValue = &scrVarGlob.variableList[i];
		entryValue->w.status = 0;
		entryValue->v.next = i;
		entryValue->hash.id = i;
		entryValue->hash.u = hash;
		scrVarGlob.variableList[hash.prev].u.next = i;
		hash.prev = i;
	}

	scrVarGlob.variableList->w.status = 0;
	scrVarGlob.variableList->w.type = 0;
	scrVarGlob.variableList->v.next = 0;
	scrVarGlob.variableList->hash.id = 0;
	scrVarGlob.variableList->hash.u = hash;
	scrVarGlob.variableList[hash.prev].u.next = 0;
}

void Var_FreeTempVariables()
{
	if ( scrVarPub.tempVariable )
	{
		FreeValue(scrVarPub.tempVariable);
		scrVarPub.tempVariable = 0;
	}
}



void Var_InitClassMap()
{
	unsigned int classnum;

	for ( classnum = 0; classnum < CLASS_NUM_COUNT; ++classnum )
	{
		scrClassMap[classnum].entArrayId = 0;
		scrClassMap[classnum].id = 0;
	}
}



void CopyEntity(unsigned int parentId, unsigned int newParentId)
{
	int type;
	VariableValueInternal *newEntryValue;
	VariableValueInternal *entryValue;
	unsigned int index;
	unsigned int id;

	for ( id = FindNextSibling(parentId); id; id = FindNextSibling(id) )
	{
		entryValue = &scrVarGlob.variableList[id];
		index = entryValue->w.status >> 8;
		if ( index != 131070 )
		{
			newEntryValue = &scrVarGlob.variableList[GetVariable(newParentId, index)];
			type = entryValue->w.status & 0x1F;
			newEntryValue->w.status |= type;
			newEntryValue->u.u.intValue = entryValue->u.u.intValue;
			AddRefToValue(type, newEntryValue->u.u);
		}
	}
}

