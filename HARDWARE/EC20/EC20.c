#include "EC20.h"
#include "bsp_usart.h"
#include "delay.h"
#include "string.h"
#include "key.h"
#include "usart2.h"
#include "oled.h"
#include "math.h"
#include "stdio.h"

//********************************************************************************
//无
//////////////////////////////////////////////////////////////////////////////////
u8 EC20CSQ[BUFLEN];
char AtStrBuf[BUFLEN];   								//打印缓存器
int MQTTVAL=0;
u8 Flag_Rec_Message=0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//EC20各项测试(拨号测试、短信测试、GPRS测试)共用代码
//EC20发送命令后,检测接收到的应答
//str:期待的应答结果
//返回值:0,没有得到期待的应答结果
//    其他,期待应答结果的位置(str的位置)
u8* EC20_check_cmd(u8 *str)
{
    char *strx=0;
    if(USART2_RX_STA&0X8000)		//接收到一次数据了
    {
        USART2_RX_BUF[USART2_RX_STA&0X7FFF]=0;//添加结束符
        strx=strstr((const char*)USART2_RX_BUF,(const char*)str);
        printf("ATCMD<-\r\n %s\r\n",USART2_RX_BUF);//发送命令
    }
    return (u8*)strx;
}

//void check_cmd(void)
//{
//    if(USART2_RX_STA&0X8000)		//接收到一次数据了
//    {
//        USART2_RX_BUF[USART2_RX_STA&0X7FFF]=0;//添加结束符
//    }
//}

//向EC20发送命令
//cmd:发送的命令字符串(不需要添加回车了),当cmd<0XFF的时候,发送数字(比如发送0X1A),大于的时候发送字符串.
//ack:期待的应答结果,如果为空,则表示不需要等待应答
//waittime:等待时间(单位:10ms)
//返回值:0,发送成功(得到了期待的应答结果)
//       1,发送失败
u8 EC20_send_cmd(u8 *cmd,u8 *ack,u16 waittime)
{
    u8 res=0;
    USART2_RX_STA=0;
    if((u32)cmd<=0XFF)
    {
        while(DMA1_Channel7->CNDTR!=0);	//等待通道7传输完成
        USART2->DR=(u32)cmd;
    }else u2_printf("%s\r\n",cmd);//发送命令
    printf("ATCMD->\r\n %s\r\n",cmd);//发送命令
    if(ack&&waittime)		//需要等待应答
    {
        while(--waittime)	//等待倒计时
        {
            delay_ms(10);
            if(USART2_RX_STA&0X8000)//接收到期待的应答结果
            {
				USART2_RX_BUF[USART2_RX_STA&0X7FFF]=0;//添加结束符
                if(EC20_check_cmd(ack))break;//得到有效数据
                USART2_RX_STA=0;
            }
        }
        if(waittime==0)res=1;
    }
    USART2_RX_STA=0;
    return res;
}

u8 EC20_work_test(void)
{
    memset(EC20CSQ,0,BUFLEN);
    if(EC20_send_cmd((u8 *)"ATE1",(u8 *)"OK",100))
    {
        if(EC20_send_cmd((u8 *)"ATE1",(u8 *)"OK",100))return SIM_COMMUNTION_ERR;	//通信不上
    }
    if(EC20_send_cmd((u8 *)"AT+CPIN?",(u8 *)"READY",400))return SIM_CPIN_ERR;	//没有SIM卡
    if(EC20_send_cmd((u8 *)"AT+CREG?",(u8 *)"0,1",400))	//检测0，1 或者 0，5
    {
        if(strstr((const char*)USART2_RX_BUF,"0,5")==NULL)
        {
            if(!EC20_send_cmd((u8 *)"AT+CSQ",(u8 *)"OK",200))
            {
                memcpy(EC20CSQ,USART2_RX_BUF+15,2);
            }
            return SIM_CREG_FAIL;	//等待附着到网络
        }
    }
    return SIM_OK;
}
u8 EC20_INIT(void)
{
    u8 res;
    res=EC20_work_test();
    switch(res)
    {
    case SIM_OK:
        printf("4G模块自检成功\r\n");
        break;
    case SIM_COMMUNTION_ERR:
        printf("正在连接到4G模块,请稍等...\r\n");
        break;
    case SIM_CPIN_ERR:
        printf("正在检测到SIM卡,请稍等..\r\n");
        break;
    case SIM_CREG_FAIL:
        printf("注册网络中...\r\n");
        printf("当前信号值：%s",EC20CSQ);
        break;
    default:
        break;
    }
    return res;
}
u8 EC20_CONNECT_MQTT_SERVER(u8 *PRODUCTKEY,u8 *DEVICENAME,u8 *DEVICESECRET)
{
    if(EC20_send_cmd((u8 *)"AT+CGATT?",(u8 *)": 1",100))	 return 1;  //检测激活PDP
    if(EC20_send_cmd((u8 *)"AT+QIACT?",(u8 *)"OK",100))          return 2;  //检测激活ACT
    if(EC20_send_cmd((u8 *)"AT+QIDEACT=1",(u8 *)"OK",100))	 return 3;  //关闭当前连接
    if(EC20_send_cmd((u8 *)"AT+QMTCLOSE=0",NULL,0))	 return 4;  //关闭MQTT客户端
    if(EC20_send_cmd((u8 *)"AT+QMTDISC=0",NULL,0))	 return 5;  //关闭和MQTT服务器的所有连接

    //配置进入阿里云
    memset(AtStrBuf,0,BUFLEN);
//    sprintf(AtStrBuf,"AT+QMTCFG=\"ALIAUTH\",0,\"%s\",\"%s\",\"%s\"\r\n",PRODUCTKEY,DEVICENAME,DEVICESECRET);
//	sprintf(AtStrBuf,"AT+QMTCFG=0,\"%s\",\"%s\",\"%s\"\r\n",PRODUCTKEY,DEVICENAME,DEVICESECRET);
//    if(EC20_send_cmd((u8 *)AtStrBuf,(u8 *)"OK",1000))	 return 6;
	
    //打开阿里云的连接，需要比较久的时间
//    if(EC20_send_cmd((u8 *)"AT+QMTOPEN=0,\"113.45.254.107\",1883",(u8 *)"+QMTOPEN: 0,0",3000)) return 7;
	if(EC20_send_cmd((u8 *)"AT+QMTOPEN=1,\"113.45.254.107\",1883",(u8 *)"+QMTOPEN: 1,0",3000)) return 7;

//    //连接到阿里云设备
    memset(AtStrBuf,0,BUFLEN);
//	sprintf(AtStrBuf,"AT+QMTCONN=0,\"%s\",\"%s\",\"%s\"\r\n",PRODUCTKEY,DEVICENAME,DEVICESECRET);
	sprintf(AtStrBuf,"AT+QMTCONN=1,\"%s\",\"%s\",\"%s\"\r\n",PRODUCTKEY,DEVICENAME,DEVICESECRET);
//    sprintf(AtStrBuf,"AT+QMTCONN=0,\"%s\"\r\n",DEVICENAME);
//    if(EC20_send_cmd((u8 *)AtStrBuf,(u8 *)"+QMTCONN: 0,0,0",1000))	 return 8;
	if(EC20_send_cmd((u8 *)AtStrBuf,(u8 *)"+QMTCONN: 1,0,0",1000))	 return 8;

//    //订阅到阿里云
    memset(AtStrBuf,0,BUFLEN);
//    sprintf(AtStrBuf,"AT+QMTSUB=0,1,\"testtopic/#\",0 \r\n");
//	sprintf(AtStrBuf,"AT+QMTSUB=1,1,\"testtopic/#\",0 \r\n");
	sprintf(AtStrBuf,"AT+QMTSUB=1,1,\"serverMsg\",0 \r\n");
//    sprintf(AtStrBuf,"AT+QMTSUB=0,1,\"/%s/%s/user/get\",0 \r\n",PRODUCTKEY,DEVICENAME);
//    if(EC20_send_cmd((u8 *)AtStrBuf,(u8 *)"+QMTSUB: 0,1,0,1",1000))	 return 9;
//	if(EC20_send_cmd((u8 *)AtStrBuf,(u8 *)"+QMTSUB: 0,1,0,0",1000))	 return 9;
	if(EC20_send_cmd((u8 *)AtStrBuf,(u8 *)"+QMTSUB: 1,1,0,0",1000))	 return 9;
//    printf("设备已经连接到阿里云,准备发送数据 [..]\r\n");
    return 0;
}

u8 EC20_CONNECT_SERVER_CFG_INFOR(u8 *PRODUCTKEY,u8 *DEVICENAME,u8 *DEVICESECRET)
{
    u8 res;
    res=EC20_CONNECT_MQTT_SERVER(PRODUCTKEY,DEVICENAME,DEVICESECRET);
    return res;
}
u8 EC20_MQTT_SEND_AUTO(u8 *PRODUCTKEY,u8 *DEVICENAME)
{
    memset(AtStrBuf,0,BUFLEN); //发送数据命令
    //AT+QMTPUB=0,0,0,0,"/sys/a18dtRetCT0/BC26TEST/thing/event/property/post"
    sprintf(AtStrBuf,"AT+QMTPUB=0,0,0,0,\"/sys/%s/%s/thing/event/property/post\"\r\n",PRODUCTKEY,DEVICENAME);
    if(EC20_send_cmd((u8 *)AtStrBuf,">",100))                 return 1;

    MQTTVAL++;
		if(MQTTVAL > 65000)
			MQTTVAL = 0;
    memset(AtStrBuf,0,BUFLEN); //发送数据命令
    sprintf(AtStrBuf,"{params:{IndoorTemperature:%d.0}}",MQTTVAL);
    if(EC20_send_cmd((u8 *)AtStrBuf,NULL,0))                  return 2;
    if(EC20_send_cmd((u8 *)0x1A,(u8 *)"OK",1500))	return 3;
    printf("系统数据发送成功  [OK]\r\n");
    return 0;
}


u8 EC20_MQTT_SEND_DATA(u8 *PRODUCTKEY,u8 *DEVICENAME,u8 *DATA)
{
    memset(AtStrBuf,0,BUFLEN); //发送数据命令
    //AT+QMTPUB=0,0,0,0,"/sys/a18dtRetCT0/BC26TEST/thing/event/property/post"
    sprintf(AtStrBuf,"AT+QMTPUB=1,0,0,0,\"/sys/%s/%s/thing/event/property/post\"\r\n",PRODUCTKEY,DEVICENAME);
//    sprintf(AtStrBuf,"AT+QMTPUBEX=1,0,0,0,\"testtopic/1\"");
	if(EC20_send_cmd((u8 *)AtStrBuf,">",100))             return 1;
    if(EC20_send_cmd(DATA,NULL,0))                  return 2;
    if(EC20_send_cmd((u8 *)0x1A,(u8 *)"OK",1500))   return 3;
    printf("用户数据发送成功  [OK]\r\n");
    return 0;
}

u8 EC20_MQTT_SEND_DATA_qqq(u8 *DATA)
{
    memset(AtStrBuf,0,BUFLEN); //发送数据命令
    //AT+QMTPUB=0,0,0,0,"/sys/a18dtRetCT0/BC26TEST/thing/event/property/post"
//    sprintf(AtStrBuf,"AT+QMTPUB=1,0,0,0,\"/sys/%s/%s/thing/event/property/post\"\r\n",PRODUCTKEY,DEVICENAME);
//    sprintf(AtStrBuf,"AT+QMTPUBEX=1,0,0,0,\"testtopic/1\"");
	sprintf(AtStrBuf,"AT+QMTPUBEX=1,0,0,0,\"boxMsg\"");
	if(EC20_send_cmd((u8 *)AtStrBuf,">",100))             return 1;
    if(EC20_send_cmd(DATA,NULL,0))                  return 2;
    if(EC20_send_cmd((u8 *)0x1A,(u8 *)"OK",1500))   return 3;
    printf("用户数据发送成功  [OK]\r\n");
    return 0;
}
