
#ifndef __BLE_TPU_H_
#define __BLE_TPU_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"

#define BLE_MAC_SIZE 6
	//Utp֡��������
	typedef struct _BleTpuFrameCfg
	{
		uint8_t cmdByteInd;		//�����ֽ�ƫ��
		uint8_t dataByteInd;	//�����ֽ�ƫ��

		int      txBufLen;	//txBuf�ĳ���
		uint8_t* txBuf;		//��ŷ��ͳ�ȥ���ݣ�ת��ǰ
		int      rxBufLen;	//rxBuf�ĳ���
		uint8_t* rxBuf;		//��Ž��յ���RAW���ݣ�ת��ǰ
		int		 transcodeBufLen;//transcodeBuf�ĳ���
		uint8_t* transcodeBuf;	 //���rxBufת����֡����

		uint8_t result_SUCCESS;		//�����붨�壺�ɹ�
		uint8_t result_UNSUPPORTED;	//�����붨�壺���յ���֧�ֵ�����

		uint32_t waitRspMsDefault;	//�����Ĭ�ϵĵȴ���Ӧʱ�䣬�������Ҫ�޸�Ϊ��Ĭ��ֵ��������������¼�����UTP_TX_START���޸�pUtp->waitRspMs
		uint32_t rxIntervalMs;		//�������ݼ��
		uint32_t sendCmdIntervalMs;	//����2������֮��ļ��ʱ��
	}BleTpuFrameCfg;

	typedef struct _BleTpu
	{
		UTP_FSM state;
		FRAME_STATE FrameState;

		uint16_t txBufLen;		//����֡����
		uint16_t transcodeBufLen;	//ת����֡����
		Queue rxBufQueue;		//����rxBuf��Queue��

		//��¼��������֡�ĵ�ǰTicks�����ڼ������һ������֡��2������֮���Ƿ�ʱ
		uint32_t rxDataTicks;

		/*****************************************************************/
		uint8_t reTxCount;	//�ط�����
		uint8_t maxTxCount;	//����ط�����

		uint32_t rxRspTicks;			//������Ӧ��Ticks�����ڼ��㷢��֡���

		uint32_t waitRspMs;			//�ȴ���Ӧʱ��
		SwTimer waitRspTimer;		//�ȴ���Ӧ��ʱ��

		const UtpCmd* pWaitRspCmd;	//��ǰ��ִ�е��������ڵȴ�RSP�����������READ/WRITE��Null��ʾû��

		/*****************************************************************/
		const UtpCfg* cfg;
		const BleTpuFrameCfg* frameCfg;
	}BleTpu;

	void BleTpu_Init(BleTpu* pBleTpu, const UtpCfg* cfg, const BleTpuFrameCfg* frameCfg);
	void BleTpu_Run(BleTpu* pBleTpu);
	uint8* BleTpu_ReqProc(BleTpu* pBleTpu, const uint8_t* pReq, int frameLen, uint8* rspLen);

#ifdef __cplusplus
}
#endif

#endif
