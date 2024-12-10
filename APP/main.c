//#include "bsp_usart.h"

#include "service.h"


#define PRODUCTKEY "client2"
#define DEVICENAME   "zxiat"
#define DEVICESECRET   "zxiat135246"


int main(void)
{
	BSP_init();

	platform_connection(PRODUCTKEY, DEVICENAME, DEVICESECRET);
	
	while(1)
    {
		MQTT_Process();
		RS485_Process();
		Check_Mqtt_Connection();
	}

}

