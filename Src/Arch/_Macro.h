#ifndef __MACRO_H_
#define __MACRO_H_

#ifdef __cplusplus
extern "C"{
#endif

#define _FUNC_ __FUNCTION__
#define _FILE_ __FILE__ 
#define _LINE_ __LINE__ 

#define MIN(v1, v2) ((v1) > (v2) ? (v2) : (v1))
#define MAX(v1, v2) ((v1) < (v2) ? (v2) : (v1))
#define GET_ELEMENT_COUNT(array) (sizeof(array)/sizeof(array[1]))

#ifdef __cplusplus
}
#endif

#endif




