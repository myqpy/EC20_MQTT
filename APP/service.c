#include "service.h"
#include "delay.h"
#include "bsp_usart.h"
#include "cJSON.h"
#include <string.h>
#include <stdbool.h> 
#include <stdlib.h>
#include "usart2.h" 
#include "EC20.h"
#include "usart4.h"	
#include "key.h"
#include "led.h"
#include "MODBUS.h"
#include <cstdlib>


#pragma pack(push)
#pragma pack(1) // �ṹ��1�ֽڶ���
typedef struct Struct_Platform_Command
{
	uint8_t  parse_controller_id;
	uint8_t  parse_circuit_id;
	uint8_t  parse_start_component_id;
	uint16_t parse_end_component_id;
	uint16_t parse_crc;
} Platform_Command;
#pragma pack() // �ָ�Ĭ���ֽڶ���

#pragma pack(push)
#pragma pack(1) // �ṹ��1�ֽڶ���
typedef struct Struct_RS485_Command
{
	uint8_t  controller_id;
	uint8_t  total;
	uint16_t crc;
	struct{
		char*  parse_controller_id;
		char*  parse_circuit_id;
		char*  parse_start_component_id;
		char*  parse_end_component_id;
	}send;
} RS485_Command;
#pragma pack() // �ָ�Ĭ���ֽڶ���

// �޷���16λ����ת�޷����ֽ�����.
union U16ToU8Array
{
  unsigned short u16val;
  unsigned char u8array[2];
};

u8 json_start, json_end, json_parse[256];
u8 RS485SEND[256], RS485RECV[256],RS485_received = 0, RS485SENDFLAG=0, rs485cnt, RS485SendLength=0;
u8 MQTTSendFlag=0, MQTTSend[256];
Platform_Command p_command;
RS485_Command rs_command;
char print[256];

void BSP_init(void)
{
	NVIC_Configuration(); 	 //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
    GPIOINIT();		 //��ʼ����LED���ӵ�Ӳ���ӿ�
    DEBUG_USART_Config(115200);	 	 //���ڳ�ʼ��Ϊ115200
    USART2_Init(115200);         //��4Gģ��ͨ��
    RS485_Init(9600);          //��485ģ��ͨ�� �����ʿ��㴫����������
    printf("\r\n############("__DATE__ " - " __TIME__ ")############ \r\n");
    delay_init();	    	 //��ʱ������ʼ��
    RESET4G();
}

	
//�����ַ����������Ӵ���ĸ���ĵ�һ���ַ���λ��
int strStr(const char * haystack,  const char * needle)
{
	int len1=strlen(haystack);
	int len2=strlen(needle);
	int i=0,j=0,k=0;
	bool jieguo=true;
	if(len1<len2)
	{
		jieguo=false;
	}
	else
	{
		for(i=0;i<=len1-len2;i++)
	{
	for(j=i,k=0;j<i+len2;j++,k++)
	{
		if(haystack[j]!=needle[k])
		{
			jieguo=false;
			break;
		}
		if(haystack[j]==needle[k])
		{
			jieguo=true;
		}
	}
	if(jieguo==true)
	{
		break;
	}
	}
	}
	if(jieguo==false)
	{
		return -1;
	}
	else
	{
		return i;
	}
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

	memset(RS485SEND,0,256);
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
    pSub = cJSON_GetObjectItem(pJson, "end_component_id");
    if(NULL == pSub)
    {
        // get sub object faild
    }
//	printf("end_component_id : %s\r\n", pSub->valuestring);
	p_command.parse_end_component_id = atoi(pSub->valuestring);
	

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
//	printf((const char*)USART2_RX_BUF,"\r\n");
	check_cmd();
	memset(USART2_RX_BUF, 0, sizeof(USART2_RX_BUF));
	USART2_RX_STA=0;
}

//char * makeJson()
//{
//    cJSON * root;
//	char *out;
//    sJSON *arry;

//    root=cJSON_CreateObject();                     // ���������ݶ���
//    cJSON_AddStringToObject(root,"name","danxia");  // ��Ӽ�ֵ��
//    cJSON_AddStringToObject(root,"sex","woman");     // ��Ӽ�ֵ��
//    cJSON_AddNumberToObject(root,"age",18);        // ��Ӽ�ֵ��

//    out = cJSON_Print(root);   // ��json��ʽת�����ַ���
//    printf("%s\n",out);

//    // �ͷ��ڴ�  
//    cJSON_Delete(root);  
//    free(out);      

//	return out;
//}


char * makeJson()
{
    char* str = NULL;

    /* ����һ��JSON���ݶ���(����ͷ���) */
    cJSON* cjson_root = cJSON_CreateObject();

//	sprintf(print,"%x",p_command.parse_controller_id); 
//	itoa(p_command.parse_controller_id,str,16);
    /* ���һ���ַ������͵�JSON����(���һ������ڵ�) */
//    cJSON_AddStringToObject(cjson_root, "controller_id", rs_command.send.parse_controller_id);
//	cJSON_AddStringToObject(cjson_root, "circuit_id", rs_command.send.parse_circuit_id);
	
	cJSON_AddStringToObject(cjson_root, "controller_id", "1");
//	cJSON_AddStringToObject(cjson_root, "circuit_id", "2");
	
//	sprintf(print,"%x",p_command.parse_circuit_id); 
//    /* ���һ���ַ������͵�JSON����(���һ������ڵ�) */
//    cJSON_AddStringToObject(cjson_root, "circuit_id", print);
//	
//	sprintf(print,"%x",p_command.parse_start_component_id); 
//    /* ���һ���ַ������͵�JSON����(���һ������ڵ�) */
//    cJSON_AddStringToObject(cjson_root, "start_component_id", print);
//	
//	sprintf(print,"%x",p_command.parse_end_component_id); 
//    /* ���һ���ַ������͵�JSON����(���һ������ڵ�) */
//    cJSON_AddStringToObject(cjson_root, "parse_end_component_id", print);
//	
//	cJSON_AddStringToObject(cjson_root, "raw_data", MQTTSend);
//    /* ���һ���������͵�JSON����(���һ������ڵ�) */
//    cJSON_AddNumberToObject(cjson_root, "age", 22);

//    /* ���һ���������͵�JSON����(���һ������ڵ�) */
//    cJSON_AddNumberToObject(cjson_root, "weight", 56.4);

//    /* ����һ��JSON���ݶ��󣬲���ӵ�cjson_root */
//    cJSON* cjson_info = cJSON_CreateObject();
//    cJSON_AddItemToObject(cjson_info, "school", cJSON_CreateString("��̴�ѧ"));
//    cJSON_AddItemToObject(cjson_info, "grade", cJSON_CreateNumber(4));
//	cJSON_AddItemToObject(cjson_info, "רҵ", cJSON_CreateString("�������"));
	
//    cJSON_AddItemToObject(cjson_root, "information", cjson_info);

//    /* ���һ���������͵�JSON����(���һ������ڵ�) */
//    cJSON* cjson_gfAge = cJSON_CreateArray();
//    cJSON_AddItemToArray(cjson_gfAge, cJSON_CreateNumber(18));
//	cJSON_AddItemToArray(cjson_gfAge, cJSON_CreateNumber(20));
//	cJSON_AddItemToArray(cjson_gfAge, cJSON_CreateNumber(23));
//	cJSON_AddItemToArray(cjson_gfAge, cJSON_CreateNumber(25));
//	//cJSON_AddItemToArray(cjson_gfAge, 25); //������ҲҪ����JSON���ݣ���������д�Ǵ����

//    cJSON_AddItemToObject(cjson_root, "gf_age", cjson_gfAge);
//	
//	/* ���һ��ture����JSON����(���һ������ڵ�) */
//	cJSON_AddFalseToObject(cjson_root, "isBoy");

//    /* ���һ��ֵΪ False �Ĳ������͵�JSON����(���һ������ڵ�) */
//    cJSON_AddNullToObject(cjson_root, "studentPoint");

    /* ��ӡJSON����(��������)���������� */
    str = cJSON_Print(cjson_root);
    printf("%s\n", str);

	cJSON_Delete(cjson_root);
	
    return str;
}

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

// ˫�ֽڴ�С�λ���
unsigned short EndianSwap16(unsigned short u16val)
{
  return (((u16val & 0x00FF) << 8) +
          ((u16val & 0xFF00) >> 8));
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
	u16converter.u16val = EndianSwap16(p_command.parse_end_component_id);
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
	RS485SENDFLAG = 0;
	RS485SendLength = 0;
	
}

void RS485_RecvAndParse(u8 *buf)
{
	union U16ToU8Array u16converter;
	int i;
	char pos = 0;
	rs_command.controller_id = buf[pos];
	pos+=2;
	rs_command.total = buf[pos];
	pos+=rs_command.total+1;
	rs_command.crc = (buf[pos] << 8) + buf[pos + 1];
	u16converter.u16val = crc16(buf,rs_command.total+3);
	if(rs_command.crc == u16converter.u16val)	MQTTSendFlag=1;

//	printf("RS485RECV: ");
//	for (i = 3; i < rs_command.total; ++i)
//    {
//		printf("%02x ",buf[i]);
//    }
//	printf("\r\n");
	for (i = 3; i < rs_command.total; ++i) MQTTSend[i]=buf[i-3];
	
	
    
//	rs_command.end_component_id = (buf[pos+1] << 8) + buf[pos+2];
//	rs_command.end_component_id=EndianSwap16(rs_command.end_component_id);
//	printf(" \r\n %02x %02x %04x \r\n ",rs_command.controller_id, rs_command.total,rs_command.crc);
//	printf("%04x",u16converter.u16val);	
}

void RS485_Process()
{
	RS485_Receive_Data(RS485RECV,&rs485cnt);
	
	if(RS485_received == 1)
	{
		RS485_RecvAndParse(RS485RECV);
		
		RS485_received = 0;
	}
	if(RS485SENDFLAG == 1)
	{
		RS485_PackandSend();	
	}
}

void MQTT_Process()
{
	if(USART2_RX_STA&0X8000)		//���յ�����
	{
		getPlatformJson();

		memset(USART2_RX_BUF,0,sizeof(USART2_RX_BUF));
	}
	
	if(MQTTSendFlag==1)
	{
		if(EC20_MQTT_SEND_DATA_qqq((u8 *)makeJson())==0) printf("���ݷ��ͳɹ�\r\n");;//TCP
		printf("RS485 received \r\n");
		MQTTSendFlag = 0;
	}
}
