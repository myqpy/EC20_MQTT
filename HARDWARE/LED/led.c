#include "led.h"
#include "delay.h"

//�̵�PC0
//4Gģ�鹩��PC1
//4Gģ�鸴λPB1
void GPIOINIT(void)
{

    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD  | RCC_APB2Periph_GPIOE, ENABLE);	 //ʹ��PA,PD�˿�ʱ��
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void RESET4G(void)
{
    GPIO_SetBits(GPIOC,GPIO_Pin_5);		//�ߵ�ƽ��λ
    delay_ms(500);
    GPIO_ResetBits(GPIOC,GPIO_Pin_5);           //�͵�ƽ����
}
