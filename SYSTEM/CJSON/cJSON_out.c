#include "cJSON_out.h"
#include <float.h>
#include <limits.h>
#include <ctype.h>
#include "cJSON.h"
//#include "board.h"
#include <string.h>
#include "client.h"

Updata_struct updata_data;

/*
*********************************************************************************************************
*	�� �� ��: JSON_out
*	����˵��: �������ص�JSON�ַ���
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void JSON_out(int k,char *s_repon)
{
    cJSON *root=cJSON_Parse(s_repon);
    cJSON *entity=NULL;
    cJSON * code=NULL;
    cJSON * url_list=NULL ;
	int i;
    char *ret;


    switch(k)
    {
    case 0 :
        memset (&updata_data,0,sizeof (updata_data));
        if(root==NULL)break;

        entity = cJSON_GetObjectItem(root,"entity");
        ret=strstr(s_repon, "time");
        if(ret==NULL)//δ���µ����
        {
            memcpy(updata_data .time,entity->valuestring,19);
        }
        else //���³�������
        {
            memcpy(updata_data .time,entity->valuestring,19);
            url_list=cJSON_GetObjectItem(entity,"urls");
            if(cJSON_GetArraySize(url_list)==0)
            {
                updata_data.flag =0;
            }
            else
            {

                for(i=0; i<cJSON_GetArraySize(url_list); i++)
                {
                    cJSON * item = cJSON_GetArrayItem(url_list, i);
                    memcpy(	&updata_data .urls [i],item ->valuestring,strlen(item ->valuestring));
                }
                updata_data.flag =1;
            }
        }
        /*��ȡ�����б�*/
        //  vehicleNums_list=cJSON_GetObjectItem(entity,"vehicleRfids");


        break;
    case 1 :
        if(root==NULL)break;
        code= cJSON_GetObjectItem(root,"code");
        if(code->valueint==200)
        {

        }
        break;


    }
    cJSON_Delete(root);
}
/*
*********************************************************************************************************
*	�� �� ��: send_ml
*	����˵��: ������������ӿ�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void send_ml(char *str, int flag)
{
    char *copy_buf;
    char* cp_str1;
    copy_buf = strpbrk((char *)str, "200");
    if(copy_buf)
    {
        /*��ȡJSCON�ַ�*/
        cp_str1=strstr(copy_buf,"{\"code\"");
        if(cp_str1)
        {
            JSON_out(flag,cp_str1);
        }
    }
}
/*
*********************************************************************************************************
*	�� �� ��: Send_is_ok
*	����˵��: ���������Ƿ�ɹ�
*	��    �Σ���
*	�� �� ֵ: �ɹ�����1�����ɹ�����0
*********************************************************************************************************
*/
#if 0
static int Send_is_ok(void)
{
    jilu_flag = 0;
    /*��������ʱ��*/
    sendbuf(1);
    /*��������*/
    if(strlen((char*)USART3_RX_BUF) >= 40)
    {
        send_ml((char*)USART3_RX_BUF, 1);
        OSTimeDlyHMSM(0, 0, 3, 0);
    }
    memset(USART3_RX_BUF, 0, sizeof(USART3_RX_BUF));
    /*�����Ƿ�ɹ�*/
    return jilu_flag;
}
#endif


