#ifndef __USART4_H
#define __USART4_H	 
#include "sys.h"


//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEKս��STM32������
//RS485���� ����	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/9/9
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////
	  		  	
extern u8 RS485_RX_BUF[512]; 		//���ջ���,���64���ֽ�
extern u16 RS485_RX_CNT;   			//���յ������ݳ���
extern u8 RS485_received;

//ģʽ����
//#define RS485_TX_EN		PCout(13)	//485ģʽ����.0,����;1,����.
//����봮���жϽ��գ��벻Ҫע�����º궨��
#define EN_USART4_RX 	1			//0,������;1,����.




void RS485_Init(u32 bound);
void RS485_Send_Data(u8 *buf,u8 len);
void RS485_Receive_Data(u8 *buf,u8 *len);


#endif	   
