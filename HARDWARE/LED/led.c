#include "led.h"
#include "delay.h"

//绿灯PC0
//4G模块供电PC1
//4G模块复位PB1
void GPIOINIT(void)
{

    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD  | RCC_APB2Periph_GPIOE, ENABLE);	 //使能PA,PD端口时钟
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void RESET4G(void)
{
    GPIO_SetBits(GPIOC,GPIO_Pin_5);		//高电平复位
    delay_ms(500);
    GPIO_ResetBits(GPIOC,GPIO_Pin_5);           //低电平工作
}
