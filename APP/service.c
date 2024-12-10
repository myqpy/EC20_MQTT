#include "service.h"
#include "delay.h"
#include "bsp_usart.h"
#include "cJSON.h"
#include <string.h>
#include <stdlib.h>
#include "usart2.h" 
#include "EC20.h"
#include "usart4.h"	
#include "key.h"
#include "led.h"
#include "MODBUS.h"
#include "util.h"
#include "iwdg.h"
#define item_controller_id 2

u8 json_start, json_end, json_parse[256],json_send[512];
u8 RS485SEND[56], RS485RECV[56],RS485_received = 0, RS485SENDFLAG=0, RS485RECVFLAG=0,rs485cnt, RS485SendLength=0;
u8 MQTTSendFlag=0, MQTTSend[512];
int MQTTSendCount;
Platform_Command p_command;
RS485_Command rs_command;
char print[512];
char json_raw_data[512];
char num[2];
u8 time_1s=0, lost_connection=0;

void bufferSendPushByte(unsigned char byte)
{
    RS485SEND[RS485SendLength] = byte;
    ++RS485SendLength;
}

int bufferSendPushBytes(const unsigned char *bytes, unsigned int size)
{
    unsigned int i;
    if (bytes == NULL)
    {
        return 0;
    }
    for (i = 0; i < size; ++i)
    {
        bufferSendPushByte(*(bytes + i));
    }

    return 1;
}

int copyU16ToU8ArrayToBufferSend(const unsigned char *u8array)
{
    return bufferSendPushBytes(u8array, 2);
}

//使能定时器2,使能中断.
void Timer2_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); //TIM2时钟使能 
 
	//TIM4初始化设置
 	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 计数到5000为500ms
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值  10Khz的计数频率  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(TIM2, TIM_IT_Update|TIM_IT_Trigger, ENABLE);//TIM2 允许更新，触发中断

	//TIM4中断分组配置
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  //TIM2中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级03级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器

	TIM_Cmd(TIM2, DISABLE);							 
}


void Tim3_Int_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);

    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_Period = arr;
    TIM_TimeBaseInitStruct.TIM_Prescaler = psc;

    TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStruct);

    TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);

    NVIC_InitStruct.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3;
    NVIC_Init(&NVIC_InitStruct);
	TIM_Cmd(TIM3, DISABLE);
    TIM_Cmd(TIM3,ENABLE);
}

void BSP_init(void)
{
	NVIC_Configuration(); 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
    GPIOINIT();		 //初始化与LED连接的硬件接口
    DEBUG_USART_Config(115200);	 	 //串口初始化为115200
    USART2_Init(115200);         //与4G模块通信
    RS485_Init(9600);          //与485模块通信 波特率看你传感器而定义
	Tim3_Int_Init(9999,7199);
	Timer2_Init(2999,7199);
    printf("\r\n############("__DATE__ " - " __TIME__ ")############ \r\n");
    delay_init();	    	 //延时函数初始化
    RESET4G();
	RS485RECVFLAG=0;
	
}

void platform_connection(const char *PRODUCTKEY, const char *DEVICENAME, const char *DEVICESECRET)
{
	
	u8 errcont = 0,res=1;
	
	while(res)
	{
		errcont++;
		printf("connecting: %d times\r\n",errcont);
		res=EC20_INIT();
		delay_ms(1000);
		
		if(errcont > 5) NVIC_SystemReset();	//超时重启
	}
	errcont = 0;
	res=1;
	
	while(res)
	{
		res=EC20_CONNECT_SERVER_CFG_INFOR((u8 *)PRODUCTKEY,(u8 *)DEVICENAME,(u8 *)DEVICESECRET);   //接入阿里云
		errcont++;
		printf("Conncting: %d times\r\n",errcont);
		if(errcont > 5) NVIC_SystemReset();	//超时重启
		
	}
	printf("Platform Connected\r\n");
	IWDG_Init(IWDG_Prescaler_64,1250); //2s看门狗
}



void parsePlatformJson(char * pMsg)
{
	cJSON * pJson;
	cJSON * pSub;
	
    if(NULL == pMsg)
    {
        return;
    }
    pJson = cJSON_Parse(pMsg);

    if(NULL == pJson)                                                                                         
    {
        // parse faild, return
      return ;
    }
	
    // get string from json
    pSub = cJSON_GetObjectItem(pJson, "controller_id");
    if(NULL == pSub)
    {
        //get object named "hello" faild
    }
//    printf("controller_id : %d\r\n", atoi(pSub->valuestring));
	rs_command.send.parse_controller_id = pSub->valuestring;
	p_command.parse_controller_id = atoi(pSub->valuestring);

    // get number from json
    pSub = cJSON_GetObjectItem(pJson, "circuit_id");
    if(NULL == pSub)
    {
        //get number from json faild
    }
//    printf("circuit_id : %s\r\n", pSub->valuestring);
	rs_command.send.parse_circuit_id = pSub->valuestring;
	p_command.parse_circuit_id = atoi(pSub->valuestring);

	
    // get bool from json
    pSub = cJSON_GetObjectItem(pJson, "start_component_id");
    if(NULL == pSub)
    {
        // get bool from json faild
    }                                                                                                         
//    printf("start_component_id : %s\r\n", pSub->valuestring);
	p_command.parse_start_component_id = atoi(pSub->valuestring);

 // get sub object
    pSub = cJSON_GetObjectItem(pJson, "count");
    if(NULL == pSub)
    {
        // get sub object faild
    }
//	printf("end_component_id : %s\r\n", pSub->valuestring);
	p_command.parse_count = atoi(pSub->valuestring);
	

    cJSON_Delete(pJson);
	cJSON_Delete(pSub);
}

void getPlatformJson(void)
{
	if((strstr((const char*)USART2_RX_BUF,"{"))!=NULL)
	{
		if((strstr((const char*)USART2_RX_BUF,"}"))!=NULL)
		{
			json_start = strStr((const char*)USART2_RX_BUF, "{");
			json_end = strStr((const char*)USART2_RX_BUF, "}");
			
			memcpy(json_parse,USART2_RX_BUF+json_start,json_end-json_start+1);
			RS485SENDFLAG=1;
//			printf((const char*)json_parse,"\r\n");
//			printf("\r\n");
		}
	}
	
	parsePlatformJson(json_parse);
}




char * makeJson(u8 *buf,u8 pos)
{
    char* str = NULL;
	int i;
    /* 创建一个JSON数据对象(链表头结点) */
    cJSON* cjson_root = cJSON_CreateObject();

	memset(json_raw_data,0,512);
	

	
	if(p_command.parse_controller_id!=item_controller_id)
	{
		cJSON_AddStringToObject(cjson_root, "raw_data", "wrong_controller");
	}
	else{
		if(pos!=0)
		{
			sprintf(print,"%d",rs_command.controller_id); 
			/* 添加一条字符串类型的JSON数据(添加一个链表节点) */
			cJSON_AddStringToObject(cjson_root, "controller_id", print);

			sprintf(print,"%d",p_command.parse_circuit_id); 
			/* 添加一条字符串类型的JSON数据(添加一个链表节点) */
			cJSON_AddStringToObject(cjson_root, "circuit_id", print);

			sprintf(print,"%d",p_command.parse_start_component_id); 
			/* 添加一条字符串类型的JSON数据(添加一个链表节点) */
			cJSON_AddStringToObject(cjson_root, "start_component_id", print);

			sprintf(print,"%d",(rs_command.total/2)-1+p_command.parse_start_component_id); 
			/* 添加一条字符串类型的JSON数据(添加一个链表节点) */
			cJSON_AddStringToObject(cjson_root, "end_component_id", print);
	//		printf("RS485RECV: ");
			for (i = 3; i < pos; ++i)
			{
				memset(num,0,2);
				sprintf(num,"%02x",buf[i]);
				strncat(json_raw_data,num,2);
			}
	//		printf("\r\n");
			cJSON_AddStringToObject(cjson_root, "raw_data", json_raw_data);
		}
		else 
		{
			cJSON_AddStringToObject(cjson_root, "raw_data", "card_no_data");
		}
	}
	
//	printf("json_raw_data: ");
//	printf("%s",json_raw_data);
//	printf("\r\n");
	memset(MQTTSend,0,sizeof(MQTTSend));
	

    /* 打印JSON对象(整条链表)的所有数据 */
    str = cJSON_Print(cjson_root);
	
	if(EC20_MQTT_SEND_DATA_qqq((u8 *)str)==0) printf("数据发送成功\r\n");;//TCP

	cJSON_Delete(cjson_root);
	cJSON_free(str);
	free(str);
	
    return str;
}


void RS485_PackandSend()
{
//	char pos = -1;
	unsigned int i;
	union U16ToU8Array u16converter;
	
	memset(RS485SEND,0,256);
//	sprintf(print,"%02x",p_command.parse_controller_id);
//	p_command.parse_controller_id = strtol(p_command.parse_controller_id, NULL, 16 ); 
	
	bufferSendPushByte(p_command.parse_controller_id);
	bufferSendPushByte(0x03);
	bufferSendPushByte(p_command.parse_circuit_id);
	bufferSendPushByte(p_command.parse_start_component_id);
	u16converter.u16val = EndianSwap16(p_command.parse_count);
	copyU16ToU8ArrayToBufferSend(u16converter.u8array);
	u16converter.u16val = crc16(RS485SEND,RS485SendLength);
	u16converter.u16val = EndianSwap16(u16converter.u16val);
	copyU16ToU8ArrayToBufferSend(u16converter.u8array);
	
	printf("RS485SEND: ");
	for (i = 0; i < RS485SendLength; ++i)
    {
		printf("%02x ",RS485SEND[i]);
    }
	printf("\r\n");
	RS485_Send_Data(RS485SEND,RS485SendLength);
}

u8* RS485_RecvAndParse(u8 *buf)
{
	union U16ToU8Array u16converter;
//	int i;
	char pos = 0;
	rs_command.controller_id = buf[pos];
	pos+=2;
	rs_command.total = buf[pos];
	pos+=rs_command.total+1;
	rs_command.crc = (buf[pos] << 8) + buf[pos + 1];
	pos+=2;
	u16converter.u16val = crc16(buf,rs_command.total+3);
	if(rs_command.crc == u16converter.u16val)	

	makeJson(buf,pos-2);
//	MQTTSendFlag=1;

	return buf;
}

void RS485_Process()
{
	RS485_Receive_Data(RS485RECV,&rs485cnt);
	
	if(RS485_received == 1)
	{
		RS485_RecvAndParse(RS485RECV);
		RS485RECVFLAG=0;
		TIM_Cmd(TIM2, DISABLE);
		RS485_received = 0;
	}
	else if(RS485SENDFLAG == 1)
	{
		RS485_PackandSend();
		TIM_Cmd(TIM2, ENABLE);
		RS485SENDFLAG = 0;
		RS485SendLength = 0;
	}
	else if(RS485RECVFLAG==1)
	{
//		printf("RS485 no data");
		
//		MQTTSendFlag=1;
		makeJson(RS485RECV,0);
		TIM_Cmd(TIM2, DISABLE);
		RS485RECVFLAG=0;
	}
}

void Check_Mqtt_Connection()
{
	/*喂狗*/
	IWDG_Feed();
	
	if(time_1s>4)
	{
//		if(EC20_send_cmd(((u8 *))==0,NULL,0))) printf("check connection\r\n");//TCP
		EC20_send_cmd((u8 *)"AT+QMTCONN?",NULL,0);
		time_1s=0;
		if(lost_connection==1) NVIC_SystemReset();
	}	
}
void MQTT_Process()
{
	
	if(USART2_RX_STA&0X8000)		//接收到数据
	{
		USART2_RX_BUF[USART2_RX_STA&0X7FFF]=0;//添加结束符
		printf((const char*)USART2_RX_BUF,"\r\n");
		
		if((strstr((const char*)USART2_RX_BUF,"+QMTRECV:"))!=NULL)
		{
			getPlatformJson();
		}
		
		if((strstr((const char*)USART2_RX_BUF,"+QMTCONN?"))!=NULL)
		{
//			printf("lost_connection: %d\r\n",lost_connection);
			if((strstr((const char*)USART2_RX_BUF,"+QMTCONN:"))!=NULL)
			{
				
				if(USART2_RX_BUF[strStr((const char*)USART2_RX_BUF, "+QMTCONN:")+12] != 0x33) lost_connection=1;
			}
			else lost_connection=1;
		}

		
		USART2_RX_STA=0;
		memset(USART2_RX_BUF,0,sizeof(USART2_RX_BUF));
	}
	
	if(MQTTSendFlag==1)
	{
//		if(EC20_MQTT_SEND_DATA_qqq((u8 *)makeJson())==0) printf("数据发送成功\r\n");;//TCP
//		if(EC20_MQTT_SEND_DATA_qqq((u8 *)json_send)==0) printf("数据发送成功\r\n");;//TCP

//		printf("MQTT send \r\n");
		MQTTSendFlag = 0;
	}
}


void TIM3_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM3,TIM_IT_Update) == 1)
    {
        time_1s += 1;
        TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
    }

}

void TIM2_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update); 
		RS485RECVFLAG=1;
//        TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
    }

}
