#ifndef __UTP_DEF_H_
#define __UTP_DEF_H_

#ifdef __cplusplus
extern "C"{
#endif

#define UTP_PROTOCOL_VER	1

#define UTP_NO_RESULT_BYTE_IN_RSP		0xFF	//��Ч����������λ

#define UTP_PURE_DATA_SIZE 128	//UTP�����ݴ�С
#define UTP_REQ_BUF_SIZE 	UTP_PURE_DATA_SIZE
#define UTP_RSP_BUF_SIZE 	UTP_PURE_DATA_SIZE

#define UTP_FRAME_HEAD 		0x7E	//0x7D 0x02
#define UTP_FRAME_TAIL 		0x7E	//0x7D 0x02

#define UTP_FRAME_ESCAPE 	0x7D	//0x7D 0x01
#define UTP_FRAME_ESCAPE_HREAD   0x02
#define UTP_FRAME_ESCAPE_TAIL    0x02
#define UTP_FRAME_ESCAPE_ESCAPE  0x01

#define UTP_WAIT_RSP_TIME_MS	1000	//��������ȴ���Ӧʱ��
#define UTP_RETX_REQ_COUNT		3		//�����ط�����

//UTP �����붨��
typedef enum _OP_CODE
{
	 OP_SUCCESS 		= 0		
	,OP_PARAM_INVALID	= 1 //�ò�����Ч
	,OP_UNSUPPORTED		= 2 //�����֧��
	,OP_CRC_ERROR		= 3	//У�����
	,OP_NOT_READY		= 4	//�豸û׼����
	,OP_USERID_ERROR	= 5	//USER_ID����
	,OP_HW_ERROR		= 6	//ִ��ʧ��
	,OP_18650BAT_V_OVER_LOW	= 7	//18650��ѹ̫��
	,OP_NOT_ALLOW_DISCHARGE	= 8	//��֧�ַŵ�
	
	,OP_PENDING			= 0xF0	//ִ�еȴ�
	,OP_NO_RSP			= 0xF1	//No response
	,OP_FAILED			= 0xFF	//ִ��ʧ��
}OP_CODE;

/***************
�����¼�����
***************/
typedef enum
{
	UTP_TX_START,		//�������ʼ, ���ڸ��¼�������ͨ�Ų�����m_waitRspMs, m_maxTxCount

	UTP_CHANGED_BEFORE,	//pStorageֵ�ı�֮ǰ
	UTP_CHANGED_AFTER,	//pStorageֵ�ı�֮��
	UTP_GET_RSP,		//���յ�����,��ȡ��Ӧ����Ӧ������transferData, transferLen��

	UTP_REQ_FAILED,		//���������ʧ�ܣ�ԭ�򣺿��ܴ��䳬ʱ��������Ӧ����ʧ���롣
	UTP_REQ_SUCCESS,	//����������ɹ�������ɹ�������Ӧ���سɹ���
}UTP_TXF_EVENT;

typedef enum
{
	UTP_EVENT_RC_SUCCESS,	//�¼�����ɹ�
	UTP_EVENT_RC_FAILED,	//�¼�����ʧ��
	UTP_EVENT_RC_DO_NOTHING,	//�¼�û�д���
}UTP_EVENT_RC;

//UTP ����״̬
typedef enum
{
	  UTP_FSM_INIT = 0	//��ʼ��
	, UTP_FSM_RX_REQ	//���յ�����
	, UTP_FSM_WAIT_RSP	//�ȴ���Ӧ
}UTP_FSM;

typedef enum
{
	UTP_READ = 0	//���������ͣ���ҪӦ��
	, UTP_WRITE		//д�������ͣ���ҪӦ��
	, UTP_EVENT		//�¼�����  ����ҪӦ��
	, UTP_NOTIFY	//֪ͨ����  ������ҪӦ��
}UTP_CMD_TYPE;

typedef enum _UTP_RCV_RSP_RC
{
	RSP_SUCCESS = 0
	, RSP_TIMEOUT
	, RSP_FAILED
	, RSP_CANCEL		//ȡ������
}UTP_RCV_RSP_RC;

struct _UtpCmd;
typedef UTP_EVENT_RC(*UtpCmdFn)(void* pOwnerObj, const struct _UtpCmd* pCmd, UTP_TXF_EVENT ev);

typedef struct _UtpCmdEx
{
	/*
	�����EVENT��
		��UTP_GET_RSP�¼�֮ǰtransferDataָ����յ�������
		��UTP_GET_RSP�¼���transferDataĬ��ָ��UtpCmd->pData��Ҳ����
			���¼��������޸ģ�ָ��ҪӦ�������ָ�룬���ϲ�������
	�����READ/WRITE,ָ��Ӧ������
	*/
	uint8_t* transferData;	//��������ָ��
	uint8_t transferLen;		//�������ݳ���

	/*****************
	��������������Ӧʱ��ʱ���Ticks.
	����UTP_READ/UTP_WRITE���ͣ����������Ӧʱ��Ticks��
	����UTP_EVENT���ͣ���Ч
	����UTP_NOTIFY���ͣ���������ʱ��Ticks��
	******************/
	uint32_t rxRspTicks;	

	/*****************
	����������ʱʱ�䣬����������������Ч��
		0����ʾ�����ͣ�
		����ֵ����ʾ��ʱָ����ʱ��MS����
	******************/
	uint32_t sendDelayMs;

	UTP_RCV_RSP_RC rcvRspErr;	//������Ӧ������
}UtpCmdEx;

typedef struct _UtpCmd
{
	UtpCmdEx* pExt;	//UtpCmd��չ
	
	UTP_CMD_TYPE type;	//������д����ο�UTP_CMD_TYPE����
	uint8_t cmd;			//������
	const char* cmdName;//��������

	//�����READ�����������������
	//�����WRITE������Ҫ���͵�д�������
	//�����EVENT������Event������������
	//���ֵΪNull,��ʾ��ֵ������
	uint8_t* pStorage;
	int storageLen;	

	//�����READ��pDataָ��Ҫ���͵Ķ��������
	//�����WRITE��pDataָ���Ѿ����ͳɹ������ݣ����ں�pStorage�Ƚ��Ƿ��б仯��ȷ���Ƿ���Ҫ�����µ�д����
	//�����EVENT��pDataָ���¼���Ӧ��������ָ��
	//���ֵΪNull,��ʾ��ֵ������
	uint8_t* pData;	
	int dataLen;	

	UtpCmdFn Event;	//�¼��ص�����
}UtpCmd;

typedef OP_CODE(*UtpDispatchFn)(const uint8_t* pData, uint8_t dataLen, uint8_t* rspData, uint8_t* pRspLen);
typedef struct _RxReqDispatch
{
	uint8_t cmd;
	UtpDispatchFn Proc;
}RxReqDispatch;

#ifdef __cplusplus
}
#endif

#endif



