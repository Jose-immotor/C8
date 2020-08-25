
#include "Common.h"
#include "NfcCardReader.h"
#include "fm175xx.h"
#include "type_a.h"

#define ERROR		1
#define	OK			0

static NfcCardReader_FsmFn NfcCardReader_findStatusProcFun(NfcCardReader* pReader, NfcCardReaderStatus status);

static void NfcCardReader_switchStatus(NfcCardReader* pReader, NfcCardReaderStatus status)
{
	if (status == pReader->status) return;

	if (status == NfcCardReaderStatus_sleep)
	{
		//����͹���
		FM175XX_SoftPowerdown();
	}
	else if (status == NfcCardReaderStatus_trans)
	{
	}

	pReader->status = status;

	//�޸�״̬������ָ��
	pReader->Fsm = NfcCardReader_findStatusProcFun(pReader, status);
}

//�ѿ�
static Bool NfcCardReader_searchPort(NfcCardReader* pReader, int port)
{
	//��¼����ʱ��ʹ���
	pReader->searchTicks = (pReader->port != port) ? 0 : GET_TICKS();
	pReader->searchCounter = (pReader->port != port) ? 0 : (pReader->searchCounter + 1);

	pReader->port = port;

	//���������߱�־
	FM175XX_switchPort(port);
	FM175XX_SoftReset();
	if (pReader->port)
	{
		Set_Rf(1);
	}
	else
	{
		Set_Rf(2);
	}
	Pcd_ConfigISOType(0);

	pReader->latestErr = TypeA_CardActivate(PICC_ATQA, PICC_UID, PICC_SAK);

	if (pReader->latestErr == OK)
	{
		pReader->latestErr = TypeA_RATS(0x20, pReader->picc_ats);
		pReader->Event(pReader->cbObj, (pReader->latestErr == OK) ? CARD_EVENT_SEARCH_SUCCESS : CARD_EVENT_SEARCH_FAILED);

		FM17522_Delayms(10);//��Ϊ��Ƭ�ĳ����м����ӳ٣�����Ҳ��Ӧ�ӳ�һ��
		//�л���NfcCardReaderStatus_trans״̬
		NfcCardReader_switchStatus(pReader, NfcCardReaderStatus_trans);
	}
	else
	{
		//������͵͹���
		FM175XX_SoftPowerdown();
	}

	return (pReader->latestErr == OK);
}

static void NfcCardReader_fsm_trans(NfcCardReader* pReader, uint8 msgId, uint32_t param)
{
	if (msgId == CARD_READER_MSG_RUN)
	{
		if (pReader->txLen)
		{
			//�趨��ʱ�¼�
			Pcd_SetTimer(300);
			//��������
			pReader->latestErr = Pcd_Comm(Transceive, pReader->txBuf, pReader->txLen, pReader->rxBuf, &pReader->rxLen);

			if (pReader->latestErr != OK)
			{
				pReader->Event(pReader->cbObj, CARD_EVENT_TX_DATA_FAILED);
				NfcCardReader_switchStatus(pReader, NfcCardReaderStatus_sleep);
			}
			else
			{
				pReader->Event(pReader->cbObj, CARD_EVENT_RX_DATA_SUCCESS);
			}
			pReader->rxLen = 0;
			pReader->txLen = 0;
		}
	}
	else if (msgId == CARD_READER_MSG_SEARCH_PORT)
	{
		if (pReader->port != (uint8_t)param)
		{
			NfcCardReader_searchPort(pReader, (uint8_t)param);
		}
	}
}

static void NfcCardReader_fsm_sleep(NfcCardReader* pReader, uint8 msgId, uint32_t param)
{
	if (msgId == CARD_READER_MSG_RUN)
	{
		if (pReader->txLen )
		{
			//ͬһ�˿�2���ѿ�����С�����10MS
			if (SwTimer_isTimerOutEx(pReader->searchTicks, 10))
			{
				//�ѿ�
				if (!NfcCardReader_searchPort(pReader, pReader->port) && pReader->searchCounter > 2)
				{
					//���ʧ�ܣ��÷���ʧ��
					pReader->Event(pReader->cbObj, CARD_EVENT_TX_DATA_FAILED);
					pReader->Event(pReader->cbObj, CARD_EVENT_RX_DATA_FAILED);
					pReader->rxLen = 0;
					pReader->txLen = 0;
				}
			}
		}
	}
	else if (msgId == CARD_READER_MSG_WAKEUP)
	{
		NfcCardReader_searchPort(pReader, pReader->port);
	}
	else if (msgId == CARD_READER_MSG_SEARCH_PORT)
	{
		NfcCardReader_searchPort(pReader, (uint8_t)param);
	}
}

//����״̬��Ӧ�Ĵ�����
static NfcCardReader_FsmFn NfcCardReader_findStatusProcFun(NfcCardReader* pReader, NfcCardReaderStatus status)
{
	struct
	{
		NfcCardReaderStatus opStatus;
		NfcCardReader_FsmFn Fsm;
	}
	const static g_dispatch[] =
	{
		{NfcCardReaderStatus_sleep	, NfcCardReader_fsm_sleep},
		{NfcCardReaderStatus_trans	, NfcCardReader_fsm_trans},
	};

	for (int i = 0; i < GET_ELEMENT_COUNT(g_dispatch); i++)
	{
		if (g_dispatch[i].opStatus == pReader->status)
		{
			return g_dispatch[i].Fsm;
		}
	}
	return Null;
}

static void NfcCardReader_fsm(NfcCardReader* pReader, uint8 msgId, uint32_t param)
{
	//All status do...

	if (pReader->Fsm)
	{
		pReader->Fsm(pReader, msgId, param);
	}
}

Bool NfcCardReader_isIdle(NfcCardReader* pReader)
{
	return pReader->txLen == 0;
}

Bool NfcCardReader_Send(NfcCardReader* pReader, uint8_t port, const void* data, int len)
{
	//��������û�з������
	if (pReader->txLen) return False;
	//���鷢�ͳ���
	if (len > sizeof(pReader->txBuf)) return False;

	memcpy(pReader->txBuf, data, len);
	pReader->txLen = len;

	NfcCardReader_fsm(pReader, CARD_READER_MSG_SEARCH_PORT, port);

	return True;
}

static void NfcCardReader_run(NfcCardReader* pReader)
{
	NfcCardReader_fsm(pReader, CARD_READER_MSG_RUN, 0);
}

void NfcCardReader_start(NfcCardReader* pReader)
{
	NfcCardReader_switchStatus(pReader, NfcCardReaderStatus_sleep);
}

void NfcCardReader_init(NfcCardReader* pReader, NfcCardReader_EventFn Event, void* cbObj)
{
	memset(pReader, 0, sizeof(NfcCardReader));

	pReader->status = NfcCardReaderStatus_unknown;
	pReader->Event = Event;
	pReader->cbObj = cbObj;
}
