#include "delay.h"
#include "sys.h"

#include "bsp_usart.h"

  

#include "LCD5110.h"
#include "EC20.h"
#include <stdio.h>


#include "service.h"

int errcont = 0;

u8 res=1;





//#define PRODUCTKEY "a1RWs8Cm26C"
//#define DEVICENAME   "LX4GCSIC"
//#define DEVICESECRET   "EOQp6uOKiHUMwl52FFBELoG2pKqJSTja"

#define PRODUCTKEY "client2"
#define DEVICENAME   "zxiat"
#define DEVICESECRET   "zxiat135246"

int main(void)
{
	BSP_init();
    
//    RS485_Send_Data(cx,8);//发送5个字节

	while(res)
	{
		errcont++;
		printf("connecting: %d times\r\n",errcont);
		res=EC20_INIT();
		delay_ms(1000);
		
		if(errcont > 5)
		{
			NVIC_SystemReset();	//超时重启

		}
	}
	errcont = 0;
	res=1;
	
	while(res)
	{
		res=EC20_CONNECT_SERVER_CFG_INFOR((u8 *)PRODUCTKEY,(u8 *)DEVICENAME,(u8 *)DEVICESECRET);   //接入阿里云
		errcont++;
		printf("Conncting: %d times\r\n",errcont);
		if(errcont > 5)
		{
			NVIC_SystemReset();	//超时重启
		}
	}
	printf("Platform Connected\r\n");
	
	while(1)
    {
		RS485_Process();
		MQTT_Process();
	}
	
//	while(1)
//    {
//        while(1)
//        {

//            while(1)
//            {

//                if(EC20_MQTT_SEND_AUTO((u8 *)PRODUCTKEY,(u8 *)DEVICENAME)==0)
//                {
//                    delay_ms(2000);
//                    check_cmd();    //不存储到串口数组清空数据
//                    memset(USART2_RX_BUF, 0, sizeof(USART2_RX_BUF));
//                    USART2_RX_STA=0;
//                    delay_ms(2000);
//                }


//				
//				RS485_Receive_Data(rs485buf,&rs485cnt);
////                if(USART_RX_STA&0X8000)		//接收到数据 接收到串口1的数据
////                {
////                    printf("串口1:收到JSON数据");
////                    printf((const char*)USART_RX_BUF,"\r\n");

////                    if(EC20_MQTT_SEND_DATA((u8 *)PRODUCTKEY,(u8 *)DEVICENAME,(u8 *)USART_RX_BUF)==0) printf("数据发送成功\r\n");;//TCP
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

