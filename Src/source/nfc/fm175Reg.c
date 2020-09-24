
#include "ArchDef.h"
#include "fm175Reg.h"
#include "drv_i2c.h"

Bool IICReg_readBytes(IICReg* reg, uint8 reg_addr, uint8* buf, int bufSize)
{
	reg->latestErr = gd32_i2c_read(NFC_I2C, reg->dev_addr, buf, reg_addr, bufSize);
	return reg->latestErr == 0;
}

Bool IICReg_writeBytes(IICReg* reg, uint8 reg_addr, const uint8* buf, int bufSize)
{
	if (bufSize > 255) return False;

	reg->latestErr = gd32_i2c_write(NFC_I2C, reg->dev_addr, (uint8*)buf, reg_addr, bufSize);
	return reg->latestErr == 0;
}

Bool IICReg_readByte(IICReg* reg, uint8 reg_addr, uint8* value)
{
	return IICReg_readBytes(reg, reg_addr, value, 1);
}

Bool IICReg_writeByte(IICReg* reg, uint8 reg_addr, uint8 value)
{
	return IICReg_writeBytes(reg, reg_addr, &value, 1);
}

Bool IICReg_readFifo(IICReg* reg, uint8* buf, int bufSize)
{
	return IICReg_readBytes(reg, FIFODataReg, buf, bufSize);
}

Bool IICReg_writeFifo(IICReg* reg, const uint8* buf, int bufSize)
{
	return IICReg_writeBytes(reg, FIFODataReg, buf, bufSize);
}

/*************************************************************/
/*��������	    Set_BitMask									 */
/*���ܣ�	    ��λ�Ĵ�������								 */
/*���������	reg_add���Ĵ�����ַ��mask���Ĵ�����λ		 */
/*����ֵ��	    OK											 */
/*				ERROR										 */
/*************************************************************/

Bool IICReg_SetBitMask(IICReg* reg, unsigned char reg_addr, unsigned char mask)
{
	uint8 val;
	IIC_REG_ERR_RETURN_FALSE(IICReg_readByte(reg, reg_addr, &val));
	IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(reg, reg_addr, val | mask));

	return True;
}

/*********************************************/
/*��������	    Clear_BitMask   */
/*���ܣ�	    ���λ�Ĵ�������    */

/*���������	reg_add���Ĵ�����ַ��mask���Ĵ������λ */
/*����ֵ��	    OK
				ERROR   */
				/*********************************************/
unsigned char IICReg_clearBitMask(IICReg* reg, unsigned char reg_addr, unsigned char mask)
{
	uint8 val;
	IIC_REG_ERR_RETURN_FALSE(IICReg_readByte(reg, reg_addr, &val));
	IIC_REG_ERR_RETURN_FALSE(IICReg_writeByte(reg, reg_addr, val & (~mask)));
	return True;
}

Bool IICReg_clearFIFO(IICReg* reg)
{
	uint8 val;
	IIC_REG_ERR_RETURN_FALSE(IICReg_SetBitMask(reg, FIFOLevelReg, 0x80));
	return IICReg_readByte(reg, FIFOLevelReg,  &val);
}

