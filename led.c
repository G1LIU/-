#include "stm32f10x.h"                  // Device header

void LED_Init (void)
{
     RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA, ENABLE) ;
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 |GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed  = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	GPIO_ResetBits(GPIOA,GPIO_Pin_1|GPIO_Pin_2);

}
void LED1_ON (void)
{
	GPIO_ResetBits(GPIOA,GPIO_Pin_1);
	
}
void LED1_OFF (void)
{
	GPIO_SetBits(GPIOA,GPIO_Pin_1);
	
}
void LED1_Turn (void)//对LED1高低电平进行取反
{
	if(GPIO_ReadOutputDataBit(GPIOA,GPIO_Pin_1)==0)
	{
		GPIO_SetBits(GPIOA,GPIO_Pin_1);//当前是低电平拉高
	}
	else
	{
		GPIO_ResetBits(GPIOA,GPIO_Pin_1);//当前是高电平拉低
	}
	// GPIOA->ODR ^= GPIO_Pin_1;   // 直接翻转输出寄存器对应位

}
void LED2_ON (void )
{
	GPIO_ResetBits(GPIOA,GPIO_Pin_2);
	
}
void LED2_OFF (void)
{
	GPIO_SetBits(GPIOA,GPIO_Pin_2);
}
void LED2_Turn(void)////对LED2高低电平进行取反
{
	if(GPIO_ReadOutputDataBit(GPIOA,GPIO_Pin_2)==0)
	{
		GPIO_SetBits(GPIOA,GPIO_Pin_2);//当前是低电平拉高
	}
	else
	{
		GPIO_ResetBits(GPIOA,GPIO_Pin_2);//当前是高电平拉低
	}
	// GPIOA->ODR ^= GPIO_Pin_2;   // 直接翻转输出寄存器对应位
}
	




