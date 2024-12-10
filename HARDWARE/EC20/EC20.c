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
//��
//////////////////////////////////////////////////////////////////////////////////
u8 EC20CSQ[BUFLEN];
char AtStrBuf[BUFLEN];   								//��ӡ������
int MQTTVAL=0;
u8 Flag_Rec_Message=0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//EC20�������(���Ų��ԡ����Ų��ԡ�GPRS����)���ô���
//EC20���������,�����յ���Ӧ��
//str:�ڴ���Ӧ����
//����ֵ:0,û�еõ��ڴ���Ӧ����
//    ����,�ڴ�Ӧ������λ��(str��λ��)
u8* EC20_check_cmd(u8 *str)
{
    char *strx=0;
    if(USART2_RX_STA&0X8000)		//���յ�һ��������
    {
        USART2_RX_BUF[USART2_RX_STA&0X7FFF]=0;//��ӽ�����
        strx=strstr((const char*)USART2_RX_BUF,(const char*)str);
        printf("ATCMD<-\r\n %s\r\n",USART2_RX_BUF);//��������
    }
    return (u8*)strx;
}

//void check_cmd(void)
//{
//    if(USART2_RX_STA&0X8000)		//���յ�һ��������
//    {
//        USART2_RX_BUF[USART2_RX_STA&0X7FFF]=0;//��ӽ�����
//    }
//}

//��EC20��������
//cmd:���͵������ַ���(����Ҫ��ӻس���),��cmd<0XFF��ʱ��,��������(���緢��0X1A),���ڵ�ʱ�����ַ���.
//ack:�ڴ���Ӧ����,���Ϊ��,���ʾ����Ҫ�ȴ�Ӧ��
//waittime:�ȴ�ʱ��(��λ:10ms)
//����ֵ:0,���ͳɹ�(�õ����ڴ���Ӧ����)
//       1,����ʧ��
u8 EC20_send_cmd(u8 *cmd,u8 *ack,u16 waittime)
{
    u8 res=0;
    USART2_RX_STA=0;
    if((u32)cmd<=0XFF)
    {
        while(DMA1_Channel7->CNDTR!=0);	//�ȴ�ͨ��7�������
        USART2->DR=(u32)cmd;
    }else u2_printf("%s\r\n",cmd);//��������
    printf("ATCMD->\r\n %s\r\n",cmd);//��������
    if(ack&&waittime)		//��Ҫ�ȴ�Ӧ��
    {
        while(--waittime)	//�ȴ�����ʱ
        {
            delay_ms(10);
            if(USART2_RX_STA&0X8000)//���յ��ڴ���Ӧ����
            {
				USART2_RX_BUF[USART2_RX_STA&0X7FFF]=0;//��ӽ�����
                if(EC20_check_cmd(ack))break;//�õ���Ч����
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
        if(EC20_send_cmd((u8 *)"ATE1",(u8 *)"OK",100))return SIM_COMMUNTION_ERR;	//ͨ�Ų���
    }
    if(EC20_send_cmd((u8 *)"AT+CPIN?",(u8 *)"READY",400))return SIM_CPIN_ERR;	//û��SIM��
    if(EC20_send_cmd((u8 *)"AT+CREG?",(u8 *)"0,1",400))	//���0��1 ���� 0��5
    {
        if(strstr((const char*)USART2_RX_BUF,"0,5")==NULL)
        {
            if(!EC20_send_cmd((u8 *)"AT+CSQ",(u8 *)"OK",200))
            {
                memcpy(EC20CSQ,USART2_RX_BUF+15,2);
            }
            return SIM_CREG_FAIL;	//�ȴ����ŵ�����
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
        printf("4Gģ���Լ�ɹ�\r\n");
        break;
    case SIM_COMMUNTION_ERR:
        printf("�������ӵ�4Gģ��,���Ե�...\r\n");
        break;
    case SIM_CPIN_ERR:
        printf("���ڼ�⵽SIM��,���Ե�..\r\n");
        break;
    case SIM_CREG_FAIL:
        printf("ע��������...\r\n");
        printf("��ǰ�ź�ֵ��%s",EC20CSQ);
        break;
    default:
        break;
    }
    return res;
}
u8 EC20_CONNECT_MQTT_SERVER(u8 *PRODUCTKEY,u8 *DEVICENAME,u8 *DEVICESECRET)
{
    if(EC20_send_cmd((u8 *)"AT+CGATT?",(u8 *)": 1",100))	 return 1;  //��⼤��PDP
    if(EC20_send_cmd((u8 *)"AT+QIACT?",(u8 *)"OK",100))          return 2;  //��⼤��ACT
    if(EC20_send_cmd((u8 *)"AT+QIDEACT=1",(u8 *)"OK",100))	 return 3;  //�رյ�ǰ����
    if(EC20_send_cmd((u8 *)"AT+QMTCLOSE=0",NULL,0))	 return 4;  //�ر�MQTT�ͻ���
    if(EC20_send_cmd((u8 *)"AT+QMTDISC=0",NULL,0))	 return 5;  //�رպ�MQTT����������������

    //���ý��밢����
    memset(AtStrBuf,0,BUFLEN);
//    sprintf(AtStrBuf,"AT+QMTCFG=\"ALIAUTH\",0,\"%s\",\"%s\",\"%s\"\r\n",PRODUCTKEY,DEVICENAME,DEVICESECRET);
//	sprintf(AtStrBuf,"AT+QMTCFG=0,\"%s\",\"%s\",\"%s\"\r\n",PRODUCTKEY,DEVICENAME,DEVICESECRET);
//    if(EC20_send_cmd((u8 *)AtStrBuf,(u8 *)"OK",1000))	 return 6;
	
    //�򿪰����Ƶ����ӣ���Ҫ�ȽϾõ�ʱ��
//    if(EC20_send_cmd((u8 *)"AT+QMTOPEN=0,\"113.45.254.107\",1883",(u8 *)"+QMTOPEN: 0,0",3000)) return 7;
	if(EC20_send_cmd((u8 *)"AT+QMTOPEN=1,\"113.45.254.107\",1883",(u8 *)"+QMTOPEN: 1,0",3000)) return 7;

//    //���ӵ��������豸
    memset(AtStrBuf,0,BUFLEN);
//	sprintf(AtStrBuf,"AT+QMTCONN=0,\"%s\",\"%s\",\"%s\"\r\n",PRODUCTKEY,DEVICENAME,DEVICESECRET);
	sprintf(AtStrBuf,"AT+QMTCONN=1,\"%s\",\"%s\",\"%s\"\r\n",PRODUCTKEY,DEVICENAME,DEVICESECRET);
//    sprintf(AtStrBuf,"AT+QMTCONN=0,\"%s\"\r\n",DEVICENAME);
//    if(EC20_send_cmd((u8 *)AtStrBuf,(u8 *)"+QMTCONN: 0,0,0",1000))	 return 8;
	if(EC20_send_cmd((u8 *)AtStrBuf,(u8 *)"+QMTCONN: 1,0,0",1000))	 return 8;

//    //���ĵ�������
    memset(AtStrBuf,0,BUFLEN);
//    sprintf(AtStrBuf,"AT+QMTSUB=0,1,\"testtopic/#\",0 \r\n");
//	sprintf(AtStrBuf,"AT+QMTSUB=1,1,\"testtopic/#\",0 \r\n");
	sprintf(AtStrBuf,"AT+QMTSUB=1,1,\"serverMsg\",0 \r\n");
//    sprintf(AtStrBuf,"AT+QMTSUB=0,1,\"/%s/%s/user/get\",0 \r\n",PRODUCTKEY,DEVICENAME);
//    if(EC20_send_cmd((u8 *)AtStrBuf,(u8 *)"+QMTSUB: 0,1,0,1",1000))	 return 9;
//	if(EC20_send_cmd((u8 *)AtStrBuf,(u8 *)"+QMTSUB: 0,1,0,0",1000))	 return 9;
	if(EC20_send_cmd((u8 *)AtStrBuf,(u8 *)"+QMTSUB: 1,1,0,0",1000))	 return 9;
//    printf("�豸�Ѿ����ӵ�������,׼���������� [..]\r\n");
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
    memset(AtStrBuf,0,BUFLEN); //������������
    //AT+QMTPUB=0,0,0,0,"/sys/a18dtRetCT0/BC26TEST/thing/event/property/post"
    sprintf(AtStrBuf,"AT+QMTPUB=0,0,0,0,\"/sys/%s/%s/thing/event/property/post\"\r\n",PRODUCTKEY,DEVICENAME);
    if(EC20_send_cmd((u8 *)AtStrBuf,">",100))                 return 1;

    MQTTVAL++;
		if(MQTTVAL > 65000)
			MQTTVAL = 0;
    memset(AtStrBuf,0,BUFLEN); //������������
    sprintf(AtStrBuf,"{params:{IndoorTemperature:%d.0}}",MQTTVAL);
    if(EC20_send_cmd((u8 *)AtStrBuf,NULL,0))                  return 2;
    if(EC20_send_cmd((u8 *)0x1A,(u8 *)"OK",1500))	return 3;
    printf("ϵͳ���ݷ��ͳɹ�  [OK]\r\n");
    return 0;
}


u8 EC20_MQTT_SEND_DATA(u8 *PRODUCTKEY,u8 *DEVICENAME,u8 *DATA)
{
    memset(AtStrBuf,0,BUFLEN); //������������
    //AT+QMTPUB=0,0,0,0,"/sys/a18dtRetCT0/BC26TEST/thing/event/property/post"
    sprintf(AtStrBuf,"AT+QMTPUB=1,0,0,0,\"/sys/%s/%s/thing/event/property/post\"\r\n",PRODUCTKEY,DEVICENAME);
//    sprintf(AtStrBuf,"AT+QMTPUBEX=1,0,0,0,\"testtopic/1\"");
	if(EC20_send_cmd((u8 *)AtStrBuf,">",100))             return 1;
    if(EC20_send_cmd(DATA,NULL,0))                  return 2;
    if(EC20_send_cmd((u8 *)0x1A,(u8 *)"OK",1500))   return 3;
    printf("�û����ݷ��ͳɹ�  [OK]\r\n");
    return 0;
}

u8 EC20_MQTT_SEND_DATA_qqq(u8 *DATA)
{
    memset(AtStrBuf,0,BUFLEN); //������������
    //AT+QMTPUB=0,0,0,0,"/sys/a18dtRetCT0/BC26TEST/thing/event/property/post"
//    sprintf(AtStrBuf,"AT+QMTPUB=1,0,0,0,\"/sys/%s/%s/thing/event/property/post\"\r\n",PRODUCTKEY,DEVICENAME);
//    sprintf(AtStrBuf,"AT+QMTPUBEX=1,0,0,0,\"testtopic/1\"");
	sprintf(AtStrBuf,"AT+QMTPUBEX=1,0,0,0,\"boxMsg\"");
	if(EC20_send_cmd((u8 *)AtStrBuf,">",100))             return 1;
    if(EC20_send_cmd(DATA,NULL,0))                  return 2;
    if(EC20_send_cmd((u8 *)0x1A,(u8 *)"OK",1500))   return 3;
    printf("�û����ݷ��ͳɹ�  [OK]\r\n");
    return 0;
}
