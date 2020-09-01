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

	void ObjList_add(const Obj* pObj);

	void ObjList_start();
	void ObjList_stop();
	void ObjList_run();

#ifdef __cplusplus
}
#endif

#endif

