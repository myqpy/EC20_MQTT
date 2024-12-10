#ifndef __SERVICE_H
#define	__SERVICE_H

#include "sys.h"    




void platform_connection(const char *PRODUCTKEY, const char *DEVICENAME, const char *DEVICESECRET);
void BSP_init(void);
int strStr(const char * haystack,  const char * needle);
void parsePlatformJson(char * pMsg);
void RS485_Process(void);
void RS485_PackandSend(void);
u8* RS485_RecvAndParse(u8 *buf);
char * makeJson(u8 *buf,u8 pos);
void Check_Mqtt_Connection(void);
void MQTT_Process(void);


#endif /* __BSP_USART_H */
