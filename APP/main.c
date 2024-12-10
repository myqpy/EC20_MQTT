#include "delay.h"
#include "sys.h"
#include "led.h"
#include "key.h"
#include "bsp_usart.h"
#include "MODBUS.h"
#include "string.h"    
#include "usart2.h" 
#include "usart4.h"	
#include "LCD5110.h"
#include "EC20.h"
#include <stdio.h>
#include <stdlib.h>


int errcont = 0;
u8 cx[8]={0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08};
u8 rs485buf[512]={0x01 ,0x03 ,0x04 ,0x01 ,0x8D ,0x01 ,0x36 ,0xEA ,0x62};
u8 rs485cnt=0,cnt=0;
u8 res=1;
u8 RS485_received = 0;

#define PRODUCTKEY "a1RWs8Cm26C"
#define DEVICENAME   "LX4GCSIC"
#define DEVICESECRET   "EOQp6uOKiHUMwl52FFBELoG2pKqJSTja"

int main(void)
{
    NVIC_Configuration(); 	 //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
    GPIOINIT();		 //��ʼ����LED���ӵ�Ӳ���ӿ�
    DEBUG_USART_Config(115200);	 	 //���ڳ�ʼ��Ϊ115200
    USART2_Init(115200);         //��4Gģ��ͨ��
    RS485_Init(9600);          //��485ģ��ͨ�� �����ʿ��㴫����������
    printf("\r\n############("__DATE__ " - " __TIME__ ")############ \r\n");
    delay_init();	    	 //��ʱ������ʼ��
    RESET4G();
//    RS485_Send_Data(cx,8);//����5���ֽ�

	while(res)
	{
		errcont++;
		printf("���ڳ�������: %d��\r\n",errcont);
		res=EC20_INIT();
		delay_ms(1000);
		
		if(errcont > 5)
		{
			NVIC_SystemReset();	//��ʱ����

		}
	}
	errcont = 0;
	res=1;
	
	while(1)
    {
		RS485_Receive_Data(rs485buf,&rs485cnt);
    }
	
//	while(1)
//    {
//        while(res)
//        {
//            res=EC20_INIT();
//            delay_ms(1000);
//            errcont++;
//            printf("���ڳ�������: %d��\r\n",errcont);
//            if(errcont > 5)
//            {
//                NVIC_SystemReset();	//��ʱ����

//            }
//        }
//        errcont = 0;
//        res=1;
//        while(1)
//        {
//            while(res)
//            {
//                res=EC20_CONNECT_SERVER_CFG_INFOR((u8 *)PRODUCTKEY,(u8 *)DEVICENAME,(u8 *)DEVICESECRET);   //���밢����
//                errcont++;
//                printf("���ڳ�������: %d��\r\n",errcont);
//                if(errcont > 5)
//                {
//                    NVIC_SystemReset();	//��ʱ����
//                }
//            }
//            printf("���������ƽӷ�����\r\n");
//            while(1)
//            {

//                if(EC20_MQTT_SEND_AUTO((u8 *)PRODUCTKEY,(u8 *)DEVICENAME)==0)
//                {
//                    delay_ms(2000);
//                    check_cmd();    //���洢�����������������
//                    memset(USART2_RX_BUF, 0, sizeof(USART2_RX_BUF));
//                    USART2_RX_STA=0;
//                    delay_ms(2000);
//                }

//                if(USART2_RX_STA&0X8000)		//���յ�����
//                {
//                    printf("����2:�յ��������·�����");
//                    printf((const char*)USART2_RX_BUF,"\r\n");
//                    if(strstr((const char*)USART2_RX_BUF,(const char*)"ON"))
//                    {
//                        GPIO_ResetBits(GPIOB,GPIO_Pin_12);   //�͵�ƽ�رռ̵���
//                        printf("LED���Ѵ�\r\n");
//                    }
//                    else if(strstr((const char*)USART2_RX_BUF,(const char*)"OFF"))
//                    {
//                        GPIO_SetBits(GPIOB,GPIO_Pin_12);     //�ߵ�ƽ�رռ̵���
//                        printf("LED���ѹر�\r\n");
//                    }
//                    check_cmd();
//                    memset(USART2_RX_BUF, 0, sizeof(USART2_RX_BUF));
//                    USART2_RX_STA=0;

//                }
//				
//				RS485_Receive_Data(rs485buf,&rs485cnt);
////                if(USART_RX_STA&0X8000)		//���յ����� ���յ�����1������
////                {
////                    printf("����1:�յ�JSON����");
////                    printf((const char*)USART_RX_BUF,"\r\n");

////                    if(EC20_MQTT_SEND_DATA((u8 *)PRODUCTKEY,(u8 *)DEVICENAME,(u8 *)USART_RX_BUF)==0) printf("���ݷ��ͳɹ�\r\n");;//TCP
////                    memset(USART_RX_BUF, 0, sizeof(USART_RX_BUF));
////                    USART_RX_STA=0;
////                    delay_ms(2000);
////                }
//				
//				
//            }
//        }
//    }
}

