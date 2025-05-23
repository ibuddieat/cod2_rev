#include "../qcommon/qcommon.h"
#include "script_public.h"

#define MAX_VM_STACK_DEPTH 32

scrVmGlob_t scrVmGlob;
scrVmPub_t scrVmPub;

int g_script_error_level;
jmp_buf g_script_error[MAX_VM_STACK_DEPTH + 1];

void Scr_InitSystem()
{
	scrVarPub.timeArrayId = AllocObject();
	scrVarPub.pauseArrayId = Scr_AllocArray();
	scrVarPub.levelId = AllocObject();
	scrVarPub.animId = AllocObject();
	scrVarPub.time = 0;
	g_script_error_level = -1;
}

qboolean Scr_IsSystemActive()
{
	return scrVarPub.timeArrayId != 0;
}

VariableValue* Scr_GetValue(unsigned int param)
{
	return &scrVmPub.top[int(-param)];
}

void Scr_ErrorInternal()
{
	if ( !scrVarPub.evaluate && !scrCompilePub.script_loading )
	{
		if ( scrVarPub.developer && scrVmGlob.loading )
			scrVmPub.terminal_error = 1;

		if ( scrVmPub.function_count || scrVmPub.debugCode )
			longjmp(g_script_error[g_script_error_level], -1);

		Com_Error(ERR_DROP, "%s", scrVarPub.error_message);
	}

	if ( scrVmPub.terminal_error )
		Com_Error(ERR_DROP, "%s", scrVarPub.error_message);
}

void Scr_Error(const char *error)
{
	if ( !scrVarPub.error_message )
		scrVarPub.error_message = error;

	Scr_ErrorInternal();
}

void Scr_ObjectError(const char *error)
{
	scrVarPub.error_index = -1;
	Scr_Error(error);
}

void Scr_ParamError(int paramNum, const char *error)
{
	scrVarPub.error_index = paramNum + 1;
	Scr_Error(error);
}

int Scr_GetType(unsigned int param)
{
	if ( param < scrVmPub.outparamcount )
		return Scr_GetValue(param)->type;

	Scr_Error(va("parameter %d does not exist", param + 1));
	return 0;
}

unsigned int Scr_GetNumParam()
{
	return scrVmPub.outparamcount;
}

void Scr_ClearOutParams()
{
	while ( scrVmPub.outparamcount )
	{
		RemoveRefToValue(scrVmPub.top);
		--scrVmPub.top;
		--scrVmPub.outparamcount;
	}
}

void IncInParam()
{
	Scr_ClearOutParams();

	if ( scrVmPub.top == scrVmPub.maxstack )
		Com_Error(ERR_DROP, "Internal script stack overflow");

	++scrVmPub.top;
	++scrVmPub.inparamcount;
}

void Scr_AddUndefined()
{
	IncInParam();
	scrVmPub.top->type = VAR_UNDEFINED;
}

void Scr_AddBool(bool value)
{
	assert(value == false || value == true);
	IncInParam();
	scrVmPub.top->type = VAR_INTEGER;
	scrVmPub.top->u.intValue = value;
}

void Scr_AddInt(int value)
{
	IncInParam();
	scrVmPub.top->type = VAR_INTEGER;
	scrVmPub.top->u.intValue = value;
}

void Scr_AddFloat(float value)
{
	IncInParam();
	scrVmPub.top->type = VAR_FLOAT;
	scrVmPub.top->u.floatValue = value;
}

void Scr_AddAnim(scr_anim_t value)
{
	IncInParam();
	scrVmPub.top->type = VAR_ANIMATION;
	scrVmPub.top->u.codePosValue = value.linkPointer;
}

void Scr_AddObject(unsigned int id)
{
	IncInParam();
	scrVmPub.top->type = VAR_POINTER;
	scrVmPub.top->u.pointerValue = id;
	AddRefToObject(id);
}

void Scr_AddEntityNum(int entnum, unsigned int classnum)
{
	unsigned int entId;

	entId = Scr_GetEntityId(entnum, classnum);
	Scr_AddObject(entId);
}

void Scr_AddString(const char *value)
{
	IncInParam();
	scrVmPub.top->type = VAR_STRING;
	scrVmPub.top->u.stringValue = SL_GetString(value, 0);
}

void Scr_AddIString(const char *value)
{
	IncInParam();
	scrVmPub.top->type = VAR_ISTRING;
	scrVmPub.top->u.stringValue = SL_GetString(value, 0);
}

void Scr_AddConstString(unsigned int value)
{
	IncInParam();
	scrVmPub.top->type = VAR_STRING;
	scrVmPub.top->u.stringValue = value;
	SL_AddRefToString(value);
}

void Scr_AddVector(const float *value)
{
	IncInParam();
	scrVmPub.top->type = VAR_VECTOR;
	scrVmPub.top->u.vectorValue = Scr_AllocVector(value);
}

void Scr_MakeArray()
{
	IncInParam();
	scrVmPub.top->type = VAR_POINTER;
	scrVmPub.top->u.pointerValue = Scr_AllocArray();
}

void Scr_AddArray()
{
	unsigned int ArraySize;
	unsigned int id;

	--scrVmPub.top;
	--scrVmPub.inparamcount;

	ArraySize = GetArraySize(scrVmPub.top->u.pointerValue);
	id = GetNewArrayVariable(scrVmPub.top->u.pointerValue, ArraySize);
	SetNewVariableValue(id, scrVmPub.top + 1);
}

void Scr_AddStruct()
{
	unsigned int id;

	id = AllocObject();
	Scr_AddObject(id);
	RemoveRefToObject(id);
}

void Scr_AddArrayStringIndexed(unsigned int stringValue)
{
	unsigned int id;

	--scrVmPub.top;
	--scrVmPub.inparamcount;

	id = GetNewVariable(scrVmPub.top->u.pointerValue, stringValue);
	SetNewVariableValue(id, scrVmPub.top + 1);
}

int Scr_GetPointerType(unsigned int index)
{
	if ( index >= scrVmPub.outparamcount )
	{
		Scr_Error(va("parameter %d does not exist", index + 1));
		return 0;
	}

	if ( Scr_GetValue(index)->type != VAR_POINTER )
	{
		Scr_Error(va("type %s is not a pointer", var_typename[Scr_GetValue(index)->type]));
	}

	return GetValueType(Scr_GetValue(index)->u.pointerValue);
}

void Scr_GetEntityRef(scr_entref_t *entRef, unsigned int index)
{
	VariableValue *entryValue;
	int entId;

	if ( index < scrVmPub.outparamcount )
	{
		entryValue = Scr_GetValue(index);

		if ( entryValue->type == VAR_POINTER )
		{
			entId = entryValue->u.pointerValue;

			if ( GetValueType(entryValue->u.pointerValue) == VAR_ENTITY )
			{
				Scr_GetEntityIdRef(entRef, entId);
				return;
			}

			scrVarPub.error_index = index + 1;
			Scr_Error(va("type %s is not an entity", var_typename[GetValueType(entId)]));
		}

		scrVarPub.error_index = index + 1;
		Scr_Error(va("type %s is not an entity", var_typename[entryValue->type]));
	}

	Scr_Error(va("parameter %d does not exist", index + 1));
	entRef = 0;
}

int Scr_GetInt(unsigned int index)
{
	VariableValue *entryValue;

	if ( index >= scrVmPub.outparamcount )
	{
		Scr_Error(va("parameter %d does not exist", index + 1));
		return 0;
	}

	entryValue = Scr_GetValue(index);

	if ( entryValue->type != VAR_INTEGER )
	{
		scrVarPub.error_index = index + 1;
		Scr_Error(va("type %s is not an int", var_typename[entryValue->type]));
	}

	return entryValue->u.intValue;
}

float Scr_GetFloat(unsigned int index)
{
	VariableValue *entryValue;

	if ( index >= scrVmPub.outparamcount )
	{
		Scr_Error(va("parameter %d does not exist", index + 1));
		return 0;
	}

	entryValue = Scr_GetValue(index);

	if ( entryValue->type != VAR_FLOAT )
	{
		if ( entryValue->type == VAR_INTEGER )
			return (float)entryValue->u.intValue;

		scrVarPub.error_index = index + 1;
		Scr_Error(va("type %s is not a float", var_typename[entryValue->type]));
	}

	return entryValue->u.floatValue;
}

unsigned int Scr_GetConstString(unsigned int index)
{
	VariableValue *entryValue;

	if ( index >= scrVmPub.outparamcount )
	{
		Scr_Error(va("parameter %d does not exist", index + 1));
		return 0;
	}

	entryValue = Scr_GetValue(index);

	if ( !Scr_CastString(entryValue) )
	{
		scrVarPub.error_index = index + 1;
		Scr_ErrorInternal();
	}

	return entryValue->u.stringValue;
}

unsigned int Scr_GetConstStringIncludeNull(unsigned int index)
{
	if ( index >= scrVmPub.outparamcount || Scr_GetValue(index)->type )
		return Scr_GetConstString(index);
	else
		return 0;
}

const char* Scr_GetString(unsigned int index)
{
	unsigned int stringValue;

	stringValue = Scr_GetConstString(index);
	return SL_ConvertToString(stringValue);
}

unsigned int Scr_GetConstIString(unsigned int index)
{
	VariableValue *entryValue;

	if ( index >= scrVmPub.outparamcount )
	{
		Scr_Error(va("parameter %d does not exist", index + 1));
		return 0;
	}

	entryValue = Scr_GetValue(index);

	if ( entryValue->type != VAR_ISTRING )
	{
		scrVarPub.error_index = index + 1;
		Scr_Error(va("type %s is not a localized string", var_typename[entryValue->type]));
	}

	return entryValue->u.stringValue;
}

const char* Scr_GetIString(unsigned int index)
{
	unsigned int stringValue;

	stringValue = Scr_GetConstIString(index);
	return SL_ConvertToString(stringValue);
}

void Scr_GetVector(unsigned int index, float *vector)
{
	VariableValue *entryValue;

	if ( index < scrVmPub.outparamcount )
	{
		entryValue = Scr_GetValue(index);

		if ( entryValue->type == VAR_VECTOR )
		{
			VectorCopy(entryValue->u.vectorValue, vector);
			return;
		}

		scrVarPub.error_index = index + 1;
		Scr_Error(va("type %s is not a vector", var_typename[entryValue->type]));
	}

	Scr_Error(va("parameter %d does not exist", index + 1));
}

const char* Scr_GetDebugString(unsigned int index)
{
	VariableValue *value;

	if ( index >= scrVmPub.outparamcount )
	{
		Scr_Error(va("parameter %d does not exist", index + 1));
		return 0;
	}
	else
	{
		value = Scr_GetValue(index);
		Scr_CastDebugString(value);
		return SL_ConvertToString(value->u.stringValue);
	}
}

void Scr_GetAnim(scr_anim_s *pAnim, unsigned int index, XAnimTree_s *tree)
{
	XAnim_s *xanim;
	const char *animTreeName;
	const char *entityTreeName;
	const char *animName;
	VariableValue *pValue;

	if ( index >= scrVmPub.outparamcount )
		goto paramerror;

	pValue = Scr_GetValue(index);

	if ( pValue->type != VAR_ANIMATION )
	{
		scrVarPub.error_message = va("type %s is not an anim", var_typename[pValue->type]);
		goto error;
	}

	pAnim->linkPointer = pValue->u.codePosValue;

	if ( tree )
	{
		xanim = Scr_GetAnims(pAnim->tree);

		if ( xanim != XAnimGetAnims(tree) )
		{
			xanim = XAnimGetAnims(tree);
			animTreeName = XAnimGetAnimTreeDebugName(xanim);
			xanim = Scr_GetAnims(pAnim->tree);
			entityTreeName = XAnimGetAnimTreeDebugName(xanim);
			xanim = Scr_GetAnims(pAnim->tree);
			animName = XAnimGetAnimDebugName(xanim, pAnim->index);
			scrVarPub.error_message = va(
			                              "anim '%s' in animtree '%s' does not belong to the entity's animtree '%s'",
			                              animName,
			                              entityTreeName,
			                              animTreeName);
error:
			RemoveRefToValue(pValue);
			pValue->type = VAR_UNDEFINED;
			scrVarPub.error_index = index + 1;
			Scr_ErrorInternal();
paramerror:
			Scr_Error(va("parameter %d does not exist", index + 1));
			pAnim->index = 0;
			pAnim->tree = 0;
		}
	}
}

const char* Scr_GetTypeName(unsigned int index)
{
	if ( index < scrVmPub.outparamcount )
		return var_typename[Scr_GetValue(index)->type];

	Scr_Error(va("parameter %d does not exist", index + 1));
	return 0;
}

unsigned int Scr_GetConstLowercaseString(unsigned int index)
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

	value = Scr_GetValue(index);

	if ( !Scr_CastString(value) )
	{
		scrVarPub.error_index = index + 1;
		Scr_ErrorInternal();
		Scr_Error(va("parameter %d does not exist", index + 1));
		return 0;
	}

	stringValue = value->u.stringValue;
	string = SL_ConvertToString(value->u.stringValue);

	for ( i = 0; ; ++i )
	{
		tempString[i] = tolower(string[i]);

		if ( !string[i] )
			break;
	}

	value->u.stringValue = SL_GetString(tempString, 0);
	SL_RemoveRefToString(stringValue);

	return value->u.stringValue;
}

unsigned int Scr_GetObject(unsigned int paramnum)
{
	VariableValue *var;

	if (paramnum >= scrVmPub.outparamcount)
	{
		Scr_Error(va("parameter %d does not exist", paramnum + 1));
		return 0;
	}

	var = Scr_GetValue(paramnum);

	if (var->type == VAR_POINTER)
	{
		return var->u.pointerValue;
	}

	scrVarPub.error_index = paramnum + 1;
	Scr_Error(va("type %s is not an object", var_typename[var->type]));
	return 0;
}

void Scr_SetStructField(unsigned int structId, unsigned int index)
{
	unsigned int id;

	id = Scr_GetVariableField(structId, index);
	scrVmPub.inparamcount = 0;
	SetVariableFieldValue(id, scrVmPub.top);
	--scrVmPub.top;
}

void Scr_SetLoading(int bLoading)
{
	scrVmGlob.loading = bLoading;
}

void Scr_SetDynamicEntityField(int entnum, unsigned int classnum, unsigned int index)
{
	unsigned int entId;

	entId = Scr_GetEntityId(entnum, classnum);
	Scr_SetStructField(entId, index);
}

unsigned short Scr_ExecEntThreadNum(int entnum, unsigned int classnum, int handle, unsigned int paramcount)
{
	unsigned int threadId;
	const char *pos;
	unsigned int selfId;
	unsigned short id;

	pos = &scrVarPub.programBuffer[handle];

	if ( !scrVmPub.function_count )
		Scr_ResetTimeout();

	selfId = Scr_GetEntityId(entnum, classnum);
	AddRefToObject(selfId);
	threadId = AllocThread(selfId);
	id = VM_Execute(threadId, pos, paramcount);
	RemoveRefToValue(scrVmPub.top);
	scrVmPub.top->type = VAR_UNDEFINED;
	--scrVmPub.top;
	--scrVmPub.inparamcount;

	return id;
}

void Scr_FreeThread(unsigned short handle)
{
	RemoveRefToObject(handle);
}

void Scr_IncTime()
{
	Scr_RunCurrentThreads();
	Scr_FreeEntityList();
	++scrVarPub.time;
	scrVarPub.time &= VAR_NAME_LOW_MASK;
}

void Scr_DecTime()
{
	--scrVarPub.time;
	scrVarPub.time &= VAR_NAME_LOW_MASK;
}

bool SetEntityFieldValue(unsigned int classnum, int entnum, int offset, VariableValue *value)
{
	scrVmPub.outparamcount = 1;
	scrVmPub.top = value;

	if ( Scr_SetObjectField(classnum, entnum, offset) )
	{
		if ( scrVmPub.outparamcount )
		{
			RemoveRefToValue(scrVmPub.top);
			--scrVmPub.top;
			scrVmPub.outparamcount = 0;
		}

		return true;
	}
	else
	{
		scrVmPub.outparamcount = 0;
		return false;
	}
}

void GetEntityFieldValue(VariableValue *pValue, unsigned int classnum, int entnum, int offset)
{
	scrVmPub.top = scrVmGlob.eval_stack - 1;
	scrVmGlob.eval_stack->type = VAR_UNDEFINED;
	Scr_GetObjectField(classnum, entnum, offset);
	scrVmPub.inparamcount = 0;
	pValue->u = scrVmGlob.eval_stack->u;
	pValue->type = scrVmGlob.eval_stack->type;
}

void VM_CancelNotifyInternal(unsigned int notifyListOwnerId, unsigned int startLocalId, unsigned int notifyListId, unsigned int notifyNameListId, unsigned int stringValue)
{
	Scr_RemoveThreadNotifyName(startLocalId);
	RemoveObjectVariable(notifyNameListId, startLocalId);

	if ( !GetArraySize(notifyNameListId) )
	{
		RemoveVariable(notifyListId, stringValue);

		if ( !GetArraySize(notifyListId) )
			RemoveVariable(notifyListOwnerId, 0x1FFFEu);
	}
}

void VM_CancelNotify(unsigned int notifyListOwnerId, unsigned int startLocalId)
{
	unsigned int varIndex;
	unsigned int index;
	unsigned int notifyNameListId;
	unsigned int notifyListId;
	unsigned int name;

	varIndex = FindVariable(notifyListOwnerId, 0x1FFFEu);
	notifyListId = FindObject(varIndex);
	name = Scr_GetThreadNotifyName(startLocalId);
	index = FindVariable(notifyListId, name);
	notifyNameListId = FindObject(index);
	VM_CancelNotifyInternal(notifyListOwnerId, startLocalId, notifyListId, notifyNameListId, name);
}

VariableStackBuffer* VM_ArchiveStack(int size, const char *codePos, VariableValue *top, unsigned int localVarCount, unsigned int *localId)
{
	unsigned int id;
	char *buf;
	char *pos;
	VariableStackBuffer *stackBuf;

	stackBuf = (VariableStackBuffer *)MT_Alloc(5 * size + sizeof(VariableStackBuffer));
	id = *localId;
	stackBuf->localId = *localId;
	stackBuf->size = size;
	stackBuf->bufLen = 5 * size + sizeof(VariableStackBuffer);
	stackBuf->pos = codePos;
	stackBuf->time = scrVarPub.time;
	scrVmPub.localVars -= localVarCount;
	buf = &stackBuf->buf[5 * size];

	while ( size )
	{
		pos = buf - 4;

		if ( top->type == VAR_CODEPOS )
		{
			--scrVmPub.function_count;
			--scrVmPub.function_frame;
			*(intptr_t *)pos = (intptr_t)scrVmPub.function_frame->fs.pos;
			scrVmPub.localVars -= scrVmPub.function_frame->fs.localVarCount;
			id = GetParentLocalId(id);
		}
		else
		{
			*(intptr_t *)pos = (intptr_t)top->u.codePosValue;
		}

		buf = pos - 1;
		*buf = top->type;
		--top;
		--size;
	}

	--scrVmPub.function_count;
	--scrVmPub.function_frame;
	AddRefToObject(id);
	*localId = id;

	return stackBuf;
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
			RemoveRefToValueInternal(type, u);
		}
	}

	Scr_KillThread(localId);
	RemoveRefToObject(localId);
	MT_Free(stackValue, stackValue->bufLen);
}

void VM_TrimStack(unsigned int startLocalId, VariableStackBuffer *stackValue, bool fromEndon)
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
			if ( FindObjectVariable(scrVarPub.pauseArrayId, localId) )
			{
				++size;
				stackValue->localId = localId;
				stackValue->size = size;
				Scr_StopThread(localId);

				if ( !fromEndon )
				{
					Scr_SetThreadNotifyName(startLocalId, 0);
					stackValue->pos = 0;
					tempValue.type = VAR_STACK;
					tempValue.u.stackValue = stackValue;
					SetNewVariableValue(GetNewVariable(startLocalId, 0x1FFFFu), &tempValue);
				}

				return;
			}

			parentLocalId = GetParentLocalId(localId);
			Scr_KillThread(localId);
			RemoveRefToObject(localId);
			localId = parentLocalId;
		}
		else
		{
			RemoveRefToValueInternal(type, u);
		}
	}

	if ( fromEndon )
		RemoveVariable(startLocalId, 0x1FFFFu);

	Scr_KillThread(startLocalId);
	RemoveRefToObject(startLocalId);
	MT_Free(stackValue, stackValue->bufLen);
}

void Scr_CancelWaittill(unsigned int startLocalId)
{
	unsigned int localId;
	unsigned int parentId;
	VariableValueInternal_u *adr;
	unsigned int selfNameId;
	unsigned int selfId;

	selfId = Scr_GetSelf(startLocalId);
	localId = FindObjectVariable(scrVarPub.pauseArrayId, selfId);
	selfNameId = FindObject(localId);
	parentId = FindObjectVariable(selfNameId, startLocalId);
	adr = GetVariableValueAddress_Bad(parentId);
	VM_CancelNotify(adr->u.stringValue, startLocalId);
	RemoveObjectVariable(selfNameId, startLocalId);

	if ( !GetArraySize(selfNameId) )
		RemoveObjectVariable(scrVarPub.pauseArrayId, selfId);
}

void Scr_CancelNotifyList(unsigned int notifyListOwnerId)
{
	VariableStackBuffer *stackBuf;
	unsigned int localId;
	unsigned int selfId;
	VariableStackBuffer *stackValue;
	unsigned int startLocalId;
	unsigned int id;
	unsigned int parentId;
	unsigned int notifyNameListId;
	unsigned int arrayId;
	int stackId;
	int parentLocalId;

	while ( 1 )
	{
		id = FindVariable(notifyListOwnerId, 0x1FFFEu);

		if ( !id )
			break;

		parentId = FindObject(id);
		notifyNameListId = FindNextSibling(parentId);

		if ( !notifyNameListId )
			break;

		arrayId = FindObject(notifyNameListId);
		stackId = FindNextSibling(arrayId);

		if ( !stackId )
			break;

		startLocalId = GetVariableKeyObject(stackId);

		if ( GetValueType(stackId) == VAR_STACK )
		{
			stackValue = GetVariableValueAddress_Bad(stackId)->u.stackValue;
			Scr_CancelWaittill(startLocalId);
			VM_TrimStack(startLocalId, stackValue, 0);
		}
		else
		{
			AddRefToObject(startLocalId);
			Scr_CancelWaittill(startLocalId);
			selfId = Scr_GetSelf(startLocalId);
			localId = GetStartLocalId(selfId);
			parentLocalId = FindVariable(localId, 0x1FFFFu);

			if ( parentLocalId )
			{
				stackBuf = GetVariableValueAddress_Bad(parentLocalId)->u.stackValue;
				VM_TrimStack(localId, stackBuf, 1);
			}

			Scr_KillEndonThread(startLocalId);
			RemoveRefToEmptyObject(startLocalId);
		}
	}
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

void Scr_TerminateRunningThread(unsigned int localId)
{
	unsigned int id;
	int count;
	int current;

	current = scrVmPub.function_count;
	count = scrVmPub.function_count;

	while ( 1 )
	{
		id = scrVmPub.function_frame_start[current].fs.localId;

		if ( id == localId )
			break;

		--current;

		if ( !GetSafeParentLocalId(id) )
			count = current;
	}

	while ( count >= current )
		scrVmPub.function_frame_start[count--].fs.pos = &g_EndPos;
}

void Scr_TerminateThread(unsigned int localId)
{
	int type;
	int startLocalId;

	startLocalId = GetStartLocalId(localId);
	type = GetValueType(startLocalId);

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

unsigned short Scr_ExecThread(int callbackHook, unsigned int numArgs)
{
	unsigned int selfId;
	const char *codePos;
	unsigned short callback;

	codePos = &scrVarPub.programBuffer[callbackHook];

	if ( !scrVmPub.function_count )
		Scr_ResetTimeout();

	Scr_IsInScriptMemory(codePos);
	AddRefToObject(scrVarPub.levelId);
	selfId = AllocThread(scrVarPub.levelId);
	callback = VM_Execute(selfId, codePos, numArgs);
	RemoveRefToValue(scrVmPub.top);
	scrVmPub.top->type = VAR_UNDEFINED;
	--scrVmPub.top;
	--scrVmPub.inparamcount;

	return callback;
}

void Scr_AddExecThread(int handle, unsigned int paramcount)
{
	unsigned int selfId;
	unsigned int callback;
	const char *codePos;

	codePos = &scrVarPub.programBuffer[handle];

	if ( !scrVmPub.function_count )
		Scr_ResetTimeout();

	AddRefToObject(scrVarPub.levelId);
	selfId = AllocThread(scrVarPub.levelId);
	callback = VM_Execute(selfId, codePos, paramcount);
	RemoveRefToObject(callback);
	++scrVmPub.outparamcount;
	--scrVmPub.inparamcount;
}

int Scr_AddLocalVars(unsigned int localId)
{
	int count;
	unsigned int fieldIndex;

	count = 0;

	for ( fieldIndex = FindLastSibling(localId); fieldIndex; fieldIndex = FindLastSibling(fieldIndex) )
	{
		*++scrVmPub.localVars = fieldIndex;
		++count;
	}

	return count;
}

void VM_UnarchiveStack(unsigned int startLocalId, function_stack_t *stack, VariableStackBuffer *stackValue)
{
	int function_count;
	unsigned int localId;
	VariableValue *startTop;
	int size;
	const char *buf;
	const char *pos;

	scrVmPub.function_frame->fs.pos = stackValue->pos;
	++scrVmPub.function_count;
	++scrVmPub.function_frame;
	size = stackValue->size;
	buf = stackValue->buf;
	startTop = stack->startTop;

	while ( size )
	{
		++startTop;
		--size;
		startTop->type = *(unsigned char *)buf;
		pos = buf + 1;

		if ( startTop->type == VAR_CODEPOS )
		{
			scrVmPub.function_frame->fs.pos = *(const char **)pos;
			++scrVmPub.function_count;
			++scrVmPub.function_frame;
		}
		else
		{
			startTop->u.intValue = *(int *)pos;
		}

		buf = pos + 4;
	}

	stack->pos = stackValue->pos;
	stack->top = startTop;
	localId = stackValue->localId;
	stack->localId = localId;
	Scr_ClearWaitTime(startLocalId);
	function_count = scrVmPub.function_count;

	while ( 1 )
	{
		scrVmPub.function_frame_start[function_count--].fs.localId = localId;

		if ( !function_count )
			break;

		localId = GetParentLocalId(localId);
	}

	while ( ++function_count != scrVmPub.function_count )
	{
		scrVmPub.function_frame_start[function_count].fs.localVarCount = Scr_AddLocalVars(scrVmPub.function_frame_start[function_count].fs.localId);
	}

	stack->localVarCount = Scr_AddLocalVars(stack->localId);

	if ( stackValue->time != LOBYTE(scrVarPub.time) )
		Scr_ResetTimeout();

	MT_Free(stackValue, stackValue->bufLen);
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
				notifyListEntry = FindLastSibling(notifyListEntry);

				if ( !notifyListEntry )
					break;

				startLocalId = GetVariableKeyObject(notifyListEntry);
				selfId = Scr_GetSelf(startLocalId);
				localId = FindObjectVariable(scrVarPub.pauseArrayId, selfId);
				selfNameId = FindObject(localId);

				if ( GetValueType(notifyListEntry) )
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

void Scr_NotifyNum(int entnum, unsigned int classnum, unsigned int stringValue, unsigned int paramcount)
{
	int type;
	VariableValue *entryValue;
	unsigned int id;
	unsigned int params;

	Scr_ClearOutParams();
	entryValue = Scr_GetValue(paramcount);
	params = scrVmPub.inparamcount - paramcount;
	id = FindEntityId(entnum, classnum);

	if ( id )
	{
		type = entryValue->type;
		entryValue->type = VAR_PRECODEPOS;
		scrVmPub.inparamcount = 0;
		VM_Notify(id, stringValue, scrVmPub.top);
		entryValue->type = type;
	}

	while ( scrVmPub.top != entryValue )
	{
		RemoveRefToValue(scrVmPub.top);
		--scrVmPub.top;
	}

	scrVmPub.inparamcount = params;
}

void Scr_ResetTimeout()
{
	scrVmGlob.starttime = Sys_MilliSeconds();
}





const char* Scr_ReadCodePos(const char **pos)
{
	const char* value = *(reinterpret_cast<const char**>(const_cast<char *>(*pos)));
	*pos += sizeof(const char *);
	return value;
}

uintptr_t Scr_ReadUnsigned(const char **pos)
{
	uintptr_t value = *(reinterpret_cast<const uintptr_t*>(*pos));
	*pos += sizeof(uintptr_t);
	return value;
}

unsigned short Scr_ReadUnsignedShort(const char **pos)
{
	unsigned short value = *(reinterpret_cast<const unsigned short*>(*pos));
	*pos += sizeof(unsigned short);
	return value;
}

const unsigned int* Scr_ReadIntArray(const char **pos, int count)
{
	const unsigned int *value;

	value = reinterpret_cast<const unsigned int *>(*pos);
	*pos += sizeof(unsigned int) * count;
	return value;
}

float Scr_ReadFloat(const char **pos)
{
	float value = *(reinterpret_cast<const float*>(*pos));
	*pos += sizeof(float);
	return value;
}

const float* Scr_ReadVector(const char **pos)
{
	const float *value = reinterpret_cast<const float*>(*pos);
	*pos += sizeof(vec3_t);
	return value;
}

unsigned int Scr_GetLocalVar(const char *pos)
{
	return scrVmPub.localVars[-*(pos)];
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
			fieldValueId = FreeTempVariable();
			RemoveRefToValue(top);
			--top;
			break;

		case OP_ClearArray:
		case OP_wait:
			RemoveRefToValue(top);
			--top;
			break;

		case OP_GetSelfObject:
			objectId = FreeTempVariableObject();
			break;

		case OP_EvalSelfFieldVariable:
		case OP_EvalFieldVariable:
			top->type = VAR_UNDEFINED;
			break;

		case OP_EvalSelfFieldVariableRef:
		case OP_EvalFieldVariableRef:
			fieldValueId = FreeTempVariable();
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
			objectId = FreeTempVariableObject();
			--top;
			break;

		case OP_EvalLocalVariableObjectCached:
			++pos;
			objectId = FreeTempVariableObject();
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
			Scr_EvalVariable(&tempValue, scrVarPub.gameId);
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
			Scr_EvalVariable(&tempValue, scrVmPub.localVars[0]);
			top->u = tempValue.u;
			top->type = tempValue.type;
			continue;

		case OP_EvalLocalVariableCached1:
			++top;
			Scr_EvalVariable(&tempValue, scrVmPub.localVars[-1]);
			top->u = tempValue.u;
			top->type = tempValue.type;
			continue;

		case OP_EvalLocalVariableCached2:
			++top;
			Scr_EvalVariable(&tempValue, scrVmPub.localVars[-2]);
			top->u = tempValue.u;
			top->type = tempValue.type;
			continue;

		case OP_EvalLocalVariableCached3:
			++top;
			Scr_EvalVariable(&tempValue, scrVmPub.localVars[-3]);
			top->u = tempValue.u;
			top->type = tempValue.type;
			continue;

		case OP_EvalLocalVariableCached4:
			++top;
			Scr_EvalVariable(&tempValue, scrVmPub.localVars[-4]);
			top->u = tempValue.u;
			top->type = tempValue.type;
			continue;

		case OP_EvalLocalVariableCached5:
			++top;
			Scr_EvalVariable(&tempValue, scrVmPub.localVars[-5]);
			top->u = tempValue.u;
			top->type = tempValue.type;
			continue;

		case OP_EvalLocalVariableCached:
			++top;
			Scr_EvalVariable(&tempValue, Scr_GetLocalVar(pos));
			top->u = tempValue.u;
			top->type = tempValue.type;
			++pos;
			continue;

		case OP_EvalLocalArrayCached:
			++top;
			Scr_EvalVariable(&tempValue, Scr_GetLocalVar(pos));
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
			Scr_Error(va("%s is not an object", var_typename[GetValueType(objectId)]));

		case OP_EvalLevelFieldVariable:
			objectId = scrVarPub.levelId;
			++top;
			Scr_EvalVariable(&tempValue, FindVariable(objectId, Scr_ReadUnsignedShort(&pos)));
			top->u = tempValue.u;
			top->type = tempValue.type;
			continue;

		case OP_EvalAnimFieldVariable:
			objectId = scrVarPub.animId;
			++top;
			Scr_EvalVariable(&tempValue, FindVariable(objectId, Scr_ReadUnsignedShort(&pos)));
			top->u = tempValue.u;
			top->type = tempValue.type;
			continue;

		case OP_EvalSelfFieldVariable:
			objectId = Scr_GetSelf(localId);
			if ( IsFieldObject(objectId) )
			{
				++top;
				Scr_FindVariableFieldInternal(&tempValue, objectId, Scr_ReadUnsignedShort(&pos));
				top->u = tempValue.u;
				top->type = tempValue.type;
				continue;
			}
			++top;
			Scr_ReadUnsignedShort(&pos);
			Scr_Error(va("%s is not an object", var_typename[GetValueType(objectId)]));

		case OP_EvalFieldVariable:
			++top;
			Scr_FindVariableFieldInternal(&tempValue, objectId, Scr_ReadUnsignedShort(&pos));
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
			if ( GetValueType(objectId) == VAR_ENTITY )
			{
				Scr_GetEntityIdRef(&entref, objectId);
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
			Scr_Error(va("%s is not an entity", var_typename[GetValueType(objectId)]));

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
			if ( GetValueType(objectId) == VAR_ENTITY )
			{
				Scr_GetEntityIdRef(&entref, objectId);
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
			Scr_Error(va("%s is not an entity", var_typename[GetValueType(objectId)]));

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
			if ( GetValueType(objectId) == VAR_ENTITY )
			{
				Scr_GetEntityIdRef(&entref, objectId);
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
			Scr_Error(va("%s is not an entity", var_typename[GetValueType(objectId)]));

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
			if ( GetValueType(objectId) == VAR_ENTITY )
			{
				Scr_GetEntityIdRef(&entref, objectId);
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
			Scr_Error(va("%s is not an entity", var_typename[GetValueType(objectId)]));

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
			if ( GetValueType(objectId) == VAR_ENTITY )
			{
				Scr_GetEntityIdRef(&entref, objectId);
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
			Scr_Error(va("%s is not an entity", var_typename[GetValueType(objectId)]));

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
			if ( GetValueType(objectId) == VAR_ENTITY )
			{
				Scr_GetEntityIdRef(&entref, objectId);
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
			Scr_Error(va("%s is not an entity", var_typename[GetValueType(objectId)]));

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
			if ( GetValueType(objectId) == VAR_ENTITY )
			{
				Scr_GetEntityIdRef(&entref, objectId);
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
			Scr_Error(va("%s is not an entity", var_typename[GetValueType(objectId)]));

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
			Scr_EvalVariableFieldInternal(&tempValue, fieldValueId);
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
			Scr_EvalVariableFieldInternal(&tempValue, fieldValueId);
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
				Scr_Error(va("%s is not an object", var_typename[GetValueType(top->u.pointerValue)]));
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
				Scr_Error(va("%s is not an object", var_typename[GetValueType(top->u.pointerValue)]));
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
				Scr_Error(va("%s is not an object", var_typename[GetValueType(top->u.pointerValue)]));
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
			Scr_EvalVariable(&tempValue, FindVariable(localId, Scr_ReadUnsignedShort(&pos)));
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

void Scr_RunCurrentThreads()
{
	VM_SetTime();
}

void Scr_Settings(int developer, int developer_script, int abort_on_error)
{
	scrVarPub.developer = developer != 0;
	scrVarPub.developer_script = developer_script != 0;
	scrVmPub.abort_on_error = abort_on_error != 0;
}

void Scr_TerminalError(const char *error)
{
	Scr_DumpScriptThreads();
	Scr_DumpScriptVariables();
	scrVmPub.terminal_error = 1;
	Scr_Error(error);
}

void Scr_Abort()
{
	scrVarPub.timeArrayId = 0;
	scrVarPub.bInited = 0;
}

void Scr_ShutdownSystem(unsigned char sys, qboolean bComplete)
{
	unsigned int timeId;
	unsigned int parentId;
	VariableValueInternal_u notifyListOwnerId;
	unsigned int id;
	unsigned int nextId;
	unsigned int localId;

	Scr_CompileShutdown();
	Scr_FreeEntityList();

	if ( scrVarPub.timeArrayId )
	{
		Scr_FreeGameVariable(bComplete);

		for ( id = FindNextSibling(scrVarPub.timeArrayId); id; id = FindNextSibling(id) )
		{
			timeId = FindObject(id);
			VM_TerminateTime(timeId);
		}

		while ( 1 )
		{
			nextId = FindNextSibling(scrVarPub.pauseArrayId);

			if ( !nextId )
				break;

			parentId = FindObject(nextId);
			localId = FindNextSibling(parentId);
			notifyListOwnerId = *GetVariableValueAddress_Bad(localId);
			AddRefToObject(notifyListOwnerId.u.pointerValue);
			Scr_CancelNotifyList(notifyListOwnerId.u.pointerValue);
			RemoveRefToObject(notifyListOwnerId.u.pointerValue);
		}

		ClearObject(scrVarPub.levelId);
		RemoveRefToEmptyObject(scrVarPub.levelId);
		scrVarPub.levelId = 0;
		ClearObject(scrVarPub.animId);
		RemoveRefToEmptyObject(scrVarPub.animId);
		scrVarPub.animId = 0;
		ClearObject(scrVarPub.timeArrayId);
		RemoveRefToEmptyObject(scrVarPub.timeArrayId);
		scrVarPub.timeArrayId = 0;
		RemoveRefToEmptyObject(scrVarPub.pauseArrayId);
		scrVarPub.pauseArrayId = 0;
		Scr_FreeObjects();
	}
}

void Scr_Shutdown()
{
	if ( scrVarPub.bInited )
	{
		scrVarPub.bInited = 0;
		Var_FreeTempVariables();
		Var_Shutdown();
		SL_Shutdown();
	}
}

void Scr_ClearErrorMessage()
{
	scrVarPub.error_message = 0;
	scrVmGlob.dialog_error_message = 0;
	scrVarPub.error_index = 0;
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

void Scr_VM_Init()
{
	scrVmPub.maxstack = &scrVmPub.stack[2047];
	scrVmPub.top = scrVmPub.stack;
	scrVmPub.function_count = 0;
	scrVmPub.function_frame = scrVmPub.function_frame_start;
	scrVmPub.localVars = scrVmGlob.localVarsStack - 1;
	scrVarPub.evaluate = 0;
	scrVmPub.debugCode = 0;
	Scr_ClearErrorMessage();
	scrVmPub.terminal_error = 0;
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
	scrVmGlob.loading = 0;
}

void Scr_Init()
{
	if ( scrVarPub.bInited )
		return;

	SL_Restart();
	Var_Init();
	Scr_VM_Init();
	scrCompilePub.script_loading = 0;
	scrAnimPub.animtree_loading = 0;
	scrCompilePub.scriptsPos = 0;
	scrCompilePub.loadedscripts = 0;
	scrAnimPub.animtrees = 0;
	scrCompilePub.builtinMeth = 0;
	scrCompilePub.builtinFunc = 0;
	scrVarPub.bInited = 1;
}

void Scr_AddExecEntThreadNum(int entnum, unsigned int classnum, int handle, int paramcount)
{
	unsigned int threadId;
	unsigned int id;
	unsigned int self;
	const char *pos;

	pos = &scrVarPub.programBuffer[handle];
	if ( !scrVmPub.function_count )
		Scr_ResetTimeout();
	self = Scr_GetEntityId(entnum, classnum);
	AddRefToObject(self);
	threadId = AllocThread(self);
	id = VM_Execute(threadId, pos, paramcount);
	RemoveRefToObject(id);
	++scrVmPub.outparamcount;
	--scrVmPub.inparamcount;
}
