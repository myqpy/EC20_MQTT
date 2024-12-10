#include "sys.h"
#include "usart4.h"
#include "delay.h"
#include "bsp_usart.h"	
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


#ifdef EN_USART4_RX   	//���ʹ���˽���


//���ջ�����
u8 RS485_RX_BUF[512];  	//���ջ���,���64���ֽ�.
//���յ������ݳ���
u16 RS485_RX_CNT=0;
u8 RS485_TX_EN = 0;

void UART4_IRQHandler(void)
{
    u8 res;

    if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET) //���յ�����
    {

        res =USART_ReceiveData(UART4); 	//��ȡ���յ�������
        if(RS485_RX_CNT<512)
        {
            RS485_RX_BUF[RS485_RX_CNT]=res;		//��¼���յ���ֵ
            RS485_RX_CNT++;						//������������1
//			printf("0x%02x ",RS485_RX_BUF[RS485_RX_CNT]);
        }
    }
}
#endif
//��ʼ��IO ����2
//pclk1:PCLK1ʱ��Ƶ��(Mhz)
//bound:������
void RS485_Init(u32 bound)
{
    //    //GPIO�˿�����
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4|RCC_APB2Periph_GPIOC, ENABLE);	//ʹ��USART4��GPIOCʱ��
    USART_DeInit(UART4);  //��λ����4
    //USART4_TX   PC.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PC.10
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
    GPIO_Init(GPIOC, &GPIO_InitStructure); //��ʼ��PA9

    //USART4_RX	  PC.11
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//PC.11
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
    GPIO_Init(GPIOC, &GPIO_InitStructure);  //��ʼ��PA10



//	//�����Ǵ���
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;   //485����
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
//	GPIO_Init(GPIOC, &GPIO_InitStructure);
//	GPIO_ResetBits(GPIOC,GPIO_Pin_13);



#ifdef EN_USART4_RX		  	//���ʹ���˽���
    //USART4 NVIC ����

    NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
    NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���

    //USART ��ʼ������

    USART_InitStructure.USART_BaudRate = bound;//һ������Ϊ9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
    USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
    USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

    USART_Init(UART4, &USART_InitStructure); //��ʼ������
    USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);//�����ж�
    USART_Cmd(UART4, ENABLE);                    //ʹ�ܴ���
#endif
	RS485_TX_EN=0;			//Ĭ��Ϊ����ģʽ
}

//RS485����len���ֽ�.
//buf:�������׵�ַ
//len:���͵��ֽ���(Ϊ�˺ͱ�����Ľ���ƥ��,���ｨ�鲻Ҫ����64���ֽ�)
void RS485_Send_Data(u8 *buf,u8 len)
{
    u8 t;
	RS485_TX_EN=1;			//����Ϊ����ģʽ
    for(t=0; t<len; t++)		//ѭ����������
    {
        while(USART_GetFlagStatus(UART4, USART_FLAG_TC) == RESET);
        USART_SendData(UART4,buf[t]);
    }

    while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
    RS485_RX_CNT=0;
	RS485_TX_EN=0;				//����Ϊ����ģʽ
}
//RS485��ѯ���յ�������
//buf:���ջ����׵�ַ
//len:���������ݳ���
void RS485_Receive_Data(u8 *buf,u8 *len)
{
    u8 rxlen=RS485_RX_CNT;
    u8 i=0;
    *len=0;				//Ĭ��Ϊ0
    delay_ms(10);		//�ȴ�10ms,��������10msû�н��յ�һ������,����Ϊ���ս���
    if(rxlen==RS485_RX_CNT&&rxlen)//���յ�������,�ҽ��������
    {
        for(i=0; i<rxlen; i++)
        {
            buf[i]=RS485_RX_BUF[i];
//			Usart_SendByte(USART1, buf[i]);	
        }
		RS485_received = 1;
//		printf("%d ",RS485_RX_CNT);

        *len=RS485_RX_CNT;	//��¼�������ݳ���
        RS485_RX_CNT=0;		//����
    }
}
void ClearRAM(u8* ram,u32 n)
{
    u32 i;
    for (i = 0; i < n; i++)
    {
        ram[i] = 0x00;
    }
}
