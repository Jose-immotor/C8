
#ifndef __BLE_USER_H_
#define __BLE_USER_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"

#define MD5_KEY_LEN 16

	//����������ݽ�ɫ����
	typedef enum _BLE_USER_ROLE
	{
		BLE_USER_INVALID = 0,	//��Ч�û�
		BLE_USER_ADMIN = BIT_0,	//����Ա�û�
		//BLE_USER_GUEST = BIT_1	//��ͨ�û����ݲ�֧�֣�����չ
	}BLE_USER_ROLE;

	typedef struct _BleUser
	{
		BLE_USER_ROLE role;
		Bool isLogin;
		
		char admin[MD5_KEY_LEN + 1];
		//char guest[MD5_KEY_LEN + 1];
	}BleUser;

	void BleUser_init(BleUser* bleUser, const uint8* mac);
	void BleUser_logout(BleUser* bleUser);
	BLE_USER_ROLE BleUser_login(BleUser* bleUser, const char* userID);
	BLE_USER_ROLE BleUser_getRole(const BleUser* bleUser);

#ifdef __cplusplus
}
#endif

#endif
