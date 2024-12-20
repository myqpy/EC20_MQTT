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
#define BUFLEN 255

extern u8 Flag_Rec_Message;	//收到短信标示

extern u8 EC20_INIT(void);
u8 EC20_CONNECT_SERVER_CFG_INFOR(u8 *PRODUCTKEY,u8 *DEVICENAME,u8 *DEVICESECRET);
extern u8 EC20_MQTT_SEND_DATA(u8 *PRODUCTKEY,u8 *DEVICENAME,u8 *DATA);
extern u8 EC20_MQTT_SEND_DATA_qqq(u8 *DATA);
extern u8 EC20_MQTT_SEND_AUTO(u8 *PRODUCTKEY,u8 *DEVICENAME);
void check_cmd(void);
u8 EC20_send_cmd(u8 *cmd,u8 *ack,u16 waittime);
#endif





