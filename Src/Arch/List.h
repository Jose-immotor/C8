

#ifndef __LIST_H__
#define __LIST_H__    

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"

typedef struct _List
{
	struct _List* next;
}List;

void List_Add(List* pListHead, List* newNode);

#ifdef __cplusplus
}
#endif

#endif 

