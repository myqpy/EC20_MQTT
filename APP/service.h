#ifndef __SERVICE_H
#define	__SERVICE_H

#include "sys.h"    

void BSP_init(void);
int strStr(const char * haystack,  const char * needle);
void parsePlatformJson(char * pMsg);
void RS485_Process(void);
void RS485_PackandSend(void);
void MQTT_Process(void);


#endif /* __BSP_USART_H */
