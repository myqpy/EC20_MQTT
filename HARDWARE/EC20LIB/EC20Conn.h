#ifndef __EC20_H__
#define __EC20_H__	 
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
#define SIM_OK 0
#define SIM_COMMUNTION_ERR 0xff
#define SIM_CPIN_ERR 0xfe
#define SIM_CREG_FAIL 0xfd
#define SIM_MAKE_CALL_ERR 0Xfc
#define SIM_ATA_ERR       0xfb

#define SIM_CMGF_ERR 0xfa
#define SIM_CSCS_ERR 0xf9
#define SIM_CSCA_ERR 0xf8
#define SIM_CSMP_ERR 0Xf7
#define SIM_CMGS_ERR       0xf6
#define SIM_CMGS_SEND_FAIL       0xf5

#define SIM_CNMI_ERR 0xf4


extern u8 Flag_Rec_Message;	//收到短信标示
extern char MQTT_conn_ok;	  			 //判断GPRS是否链接到服务器了
extern u8 SIM900_CSQ[3];
extern u8 GSM_Dect(void);
extern u8 EC20_CONNECT_SERVER_SEND_INFOR(u8 *ProductKey,u8 *DeviceName,u8 *DeviceSecret);
extern u8 EC20_GPRS_SEND_DATA(u8 *temp_data,u8 *Topic);
void sim_at_response(u8 mode);
void check_cmd(void);
u8 EC20_send_cmd(u8 *cmd,u8 *ack,u16 waittime);
u8 Send_SUB(char *TopicValue);
u8 HTTP_PostPkt(char *pkt,char *DEV_Humi1,char *DEV_Temp1,char *DEV_LED1,char *val1, char *val2,char *val3);
#endif





