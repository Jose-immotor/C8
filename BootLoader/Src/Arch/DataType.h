/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */

#ifndef __DATA_TYPE_H_
#define __DATA_TYPE_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"

	//�������Ͷ���
	typedef enum _DType
	{
		DT_BYTES = 0,	//�ֽ�����
		DT_UINT8 = 1,   //8λ�޷������� 
		DT_UINT16,  //16λ�޷������� 
		DT_UINT32,  //32λ�޷������� 

		DT_INT8,	  //8λ�з������� 
		DT_INT16,   //16λ�з������� 
		DT_INT32,   //32λ�з������� 

		DT_FLOAT32,	//32λ������
		DT_FLOAT64,	//64λ������

		DT_STRING,	//�ַ���
		DT_STRUCT,	//�ṹ��
	}DType;

	typedef uint32 (*DtConvertFn)(const void* val, DType dt);
	uint32 Dt_convertToU32(const void* val, DType dt);
	uint16 Dt_convertToU16(const void* val, DType dt);
	uint8  Dt_convertToU8(const void* val, DType dt);

	//��С��λ��ת��
	void* Dt_swap(uint8* val, DType dt);

#ifdef __cplusplus
}
#endif

#endif
