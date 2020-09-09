
/*
 * Copyright (c) 2020-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 * 
 * Ble对象实现文件
 */

#include "Common.h"
#include "BleUser.h"
#include "md5.h"

static void BleUser_getMd5( const uint8* mac, BLE_USER_ROLE role, char* pOut)
{
	int i = 0;
	char temp[30] = { 0 };

	//mac convert to string
	char macStr[13] = { 0 };
	memset(macStr, 0, sizeof(macStr));
	for (i = 0; i < 6; i++, mac++)
	{
		sprintf(&macStr[i * 2], "%02X", *mac);
	}

	sprintf(temp, "%simmotor%d", macStr, role);
	MD5_Encrypt(pOut, temp);
}

//本地MD5密钥是否匹配
BLE_USER_ROLE BleUser_getRole(const BleUser* bleUser)
{
	return bleUser->isLogin ? bleUser->role : BLE_USER_INVALID;
}

void BleUser_logout(BleUser* bleUser)
{
	bleUser->isLogin = False;
	bleUser->role = BLE_USER_INVALID;
}

BLE_USER_ROLE BleUser_login(BleUser* bleUser, const char* userID)
{
	bleUser->isLogin = True;

	if (memcmp(userID, bleUser->admin, MD5_KEY_LEN) == 0)
	{
		bleUser->role = BLE_USER_ADMIN;
	}
	/*
	else if (memcmp(userID, bleUser->guest, MD5_KEY_LEN) == 0)
	{
		bleUser->role = BLE_USER_GUEST;
	}
	*/
	bleUser->role = BLE_USER_INVALID;

	return bleUser->role;
}

void BleUser_init(BleUser* bleUser, const uint8* mac)
{
	memset(bleUser, 0, sizeof(BleUser));

	BleUser_getMd5(mac, BLE_USER_ADMIN, bleUser->admin);
	//BleUser_GetMd5(bleUser, mac, BLE_USER_GUEST, bleUser->guest);
}


