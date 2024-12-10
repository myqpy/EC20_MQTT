#include "sys.h"
#include "usart4.h"
#include "delay.h"
#include "bsp_usart.h"	
//////////////////////////////////////////////////////////////////////////////////
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板
//RS485驱动 代码
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/9
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved
//////////////////////////////////////////////////////////////////////////////////


#ifdef EN_USART4_RX   	//如果使能了接收


//接收缓存区
u8 RS485_RX_BUF[512];  	//接收缓冲,最大64个字节.
//接收到的数据长度
u16 RS485_RX_CNT=0;
u8 RS485_TX_EN = 0;

void UART4_IRQHandler(void)
{
    u8 res;

    if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET) //接收到数据
    {

        res =USART_ReceiveData(UART4); 	//读取接收到的数据
        if(RS485_RX_CNT<512)
        {
            RS485_RX_BUF[RS485_RX_CNT]=res;		//记录接收到的值
            RS485_RX_CNT++;						//接收数据增加1
//			printf("0x%02x ",RS485_RX_BUF[RS485_RX_CNT]);
        }
    }
}
#endif
//初始化IO 串口2
//pclk1:PCLK1时钟频率(Mhz)
//bound:波特率
void RS485_Init(u32 bound)
{
    //    //GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4|RCC_APB2Periph_GPIOC, ENABLE);	//使能USART4，GPIOC时钟
    USART_DeInit(UART4);  //复位串口4
    //USART4_TX   PC.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PC.10
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
    GPIO_Init(GPIOC, &GPIO_InitStructure); //初始化PA9

    //USART4_RX	  PC.11
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//PC.11
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOC, &GPIO_InitStructure);  //初始化PA10



//	//以上是串口
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;   //485控制
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
//	GPIO_Init(GPIOC, &GPIO_InitStructure);
//	GPIO_ResetBits(GPIOC,GPIO_Pin_13);



#ifdef EN_USART4_RX		  	//如果使能了接收
    //USART4 NVIC 配置

    NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器

    //USART 初始化设置

    USART_InitStructure.USART_BaudRate = bound;//一般设置为9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
    USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

    USART_Init(UART4, &USART_InitStructure); //初始化串口
    USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);//开启中断
    USART_Cmd(UART4, ENABLE);                    //使能串口
#endif
	RS485_TX_EN=0;			//默认为接收模式
}

//RS485发送len个字节.
//buf:发送区首地址
//len:发送的字节数(为了和本代码的接收匹配,这里建议不要超过64个字节)
void RS485_Send_Data(u8 *buf,u8 len)
{
    u8 t;
	RS485_TX_EN=1;			//设置为发送模式
    for(t=0; t<len; t++)		//循环发送数据
    {
        while(USART_GetFlagStatus(UART4, USART_FLAG_TC) == RESET);
        USART_SendData(UART4,buf[t]);
    }

    while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
    RS485_RX_CNT=0;
	RS485_TX_EN=0;				//设置为接收模式
}
//RS485查询接收到的数据
//buf:接收缓存首地址
//len:读到的数据长度
void RS485_Receive_Data(u8 *buf,u8 *len)
{
    u8 rxlen=RS485_RX_CNT;
    u8 i=0;
    *len=0;				//默认为0
    delay_ms(10);		//等待10ms,连续超过10ms没有接收到一个数据,则认为接收结束
    if(rxlen==RS485_RX_CNT&&rxlen)//接收到了数据,且接收完成了
    {
        for(i=0; i<rxlen; i++)
        {
            buf[i]=RS485_RX_BUF[i];
//			Usart_SendByte(USART1, buf[i]);	
        }
		RS485_received = 1;
//		printf("%d ",RS485_RX_CNT);

        *len=RS485_RX_CNT;	//记录本次数据长度
        RS485_RX_CNT=0;		//清零
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
