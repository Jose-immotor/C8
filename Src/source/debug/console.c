
#include "common.h"
#include "console.h"
#include "drv_usart.h"

//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 0
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
void _sys_exit(int x) 
{ 
	x = x; 
}
//__use_no_semihosting was requested, but _ttywrch was 
void _ttywrch(int ch)
{
    ch = ch;
}
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
//	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
//    USART1->DR = (u8) ch;
//	usart0_put_byte(ch);
	while((USART_STAT0(USART0)&0X40)==0);//ѭ������,ֱ���������   
    usart_data_transmit(USART0, (uint8_t)ch);    
	return ch;
}
#endif 
/*ʹ��microLib�ķ���*/
int fputc(int ch, FILE *f)
{
//	USART_SendData(USART1, (uint8_t) ch);

//	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) {}	
	usart0_put_byte(ch);
	return ch;
}

