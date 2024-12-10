#ifndef __MODBUS_H
#define __MODBUS_H
typedef unsigned char	uint8;
typedef unsigned int	uint16;
typedef unsigned long	uint32;


uint16 crc16(uint8 *puchMsg, uint16 usDataLen) ;
#endif	
