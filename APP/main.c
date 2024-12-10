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
    NVIC_Configuration(); 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
    GPIOINIT();		 //初始化与LED连接的硬件接口
    DEBUG_USART_Config(115200);	 	 //串口初始化为115200
    USART2_Init(115200);         //与4G模块通信
    RS485_Init(9600);          //与485模块通信 波特率看你传感器而定义
    printf("\r\n############("__DATE__ " - " __TIME__ ")############ \r\n");
    delay_init();	    	 //延时函数初始化
    RESET4G();
//    RS485_Send_Data(cx,8);//发送5个字节

	while(res)
	{
		errcont++;
		printf("正在尝试连接: %d次\r\n",errcont);
		res=EC20_INIT();
		delay_ms(1000);
		
		if(errcont > 5)
		{
			NVIC_SystemReset();	//超时重启

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
//            printf("正在尝试连接: %d次\r\n",errcont);
//            if(errcont > 5)
//            {
//                NVIC_SystemReset();	//超时重启

//            }
//        }
//        errcont = 0;
//        res=1;
//        while(1)
//        {
//            while(res)
//            {
//                res=EC20_CONNECT_SERVER_CFG_INFOR((u8 *)PRODUCTKEY,(u8 *)DEVICENAME,(u8 *)DEVICESECRET);   //接入阿里云
//                errcont++;
//                printf("正在尝试连接: %d次\r\n",errcont);
//                if(errcont > 5)
//                {
//                    NVIC_SystemReset();	//超时重启
//                }
//            }
//            printf("已连阿里云接服务器\r\n");
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

//                if(USART2_RX_STA&0X8000)		//接收到数据
//                {
//                    printf("串口2:收到服务器下发数据");
//                    printf((const char*)USART2_RX_BUF,"\r\n");
//                    if(strstr((const char*)USART2_RX_BUF,(const char*)"ON"))
//                    {
//                        GPIO_ResetBits(GPIOB,GPIO_Pin_12);   //低电平关闭继电器
//                        printf("LED灯已打开\r\n");
//                    }
//                    else if(strstr((const char*)USART2_RX_BUF,(const char*)"OFF"))
//                    {
//                        GPIO_SetBits(GPIOB,GPIO_Pin_12);     //高电平关闭继电器
//                        printf("LED灯已关闭\r\n");
//                    }
//                    check_cmd();
//                    memset(USART2_RX_BUF, 0, sizeof(USART2_RX_BUF));
//                    USART2_RX_STA=0;

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

