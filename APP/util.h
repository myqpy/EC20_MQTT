#ifndef __UTIL_H
#define	__UTIL_H

#include "sys.h"    

#pragma pack(push)
#pragma pack(1) // �ṹ��1�ֽڶ���
typedef struct Struct_Platform_Command
{
	uint8_t  parse_controller_id;
	uint8_t  parse_circuit_id;
	uint8_t  parse_start_component_id;
	uint16_t parse_count;
	uint16_t parse_crc;
	uint8_t  connection_status;
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



char *myitoa(int value, char *string, int radix);
int strStr(const char * haystack,  const char * needle);
unsigned short EndianSwap16(unsigned short u16val);



#endif /* __UTIL_H */
