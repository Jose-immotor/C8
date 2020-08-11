#ifndef __OBJ_H_
#define __OBJ_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "typedef.h"

	struct _Obj;
	typedef void (*ObjFn)();
	typedef struct _Obj
	{
		const char* name;
		ObjFn Start;
		ObjFn Stop;
		ObjFn Run;
	}Obj;

	void ObjList_Add(const Obj* pObj);

	void ObjList_Start();
	void ObjList_Stop();
	void ObjList_Run();

#ifdef __cplusplus
}
#endif

#endif

