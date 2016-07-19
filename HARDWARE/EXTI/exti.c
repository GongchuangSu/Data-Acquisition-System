#include "exti.h"
#include "led.h"
#include "lcd.h"
#include "key.h"
#include "delay.h"
#include "usart.h"

 extern u8 flag,flag1,flag2,flag3;
 extern u8 temp1;
 
//外部中断0服务程序
void EXTIX_Init(void)
{
 
 	  EXTI_InitTypeDef EXTI_InitStructure;
 	  NVIC_InitTypeDef NVIC_InitStructure;

    KEY_Init();	 //	按键端口初始化

  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	//使能复用功能时钟

  //GPIOA.0 中断线以及中断初始化配置   下降沿触发
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource0);

  	EXTI_InitStructure.EXTI_Line=EXTI_Line0;	                //KEY_UP
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;   //设置上升沿和下降沿中断触发
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);	 	                 //根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器


  	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;			//使能按键WK_UP所在的外部中断通道
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	//抢占优先级0， 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;					//子优先级0
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
  	NVIC_Init(&NVIC_InitStructure); 
 
}


//外部中断0服务程序 
void EXTI0_IRQHandler(void)
{
	if(KEY3==1)
	{
		delay_us(1000);
		if(KEY3==1)
		TIM_Cmd(TIM2, ENABLE);  //上升沿使能TIMx
  }
	else
	{
		delay_us(1000);
		if(KEY3!=1)
		TIM_Cmd(TIM2, DISABLE); //下降沿失能TIMx
		
		if(flag<=15&&flag>=3) 
		{
			if(flag2==1)      //要先开机，短按才起作用
      {
				flag1=!flag1;			
			  flag=0;
      }
			else
				flag=0;
    }
		
	  if(flag>=30)    //长按3秒
		{
			flag2=!flag2;
			flag=0;
    }
		
		if(flag>=16&&flag<=29)
			flag=0;
  }
		
	EXTI_ClearITPendingBit(EXTI_Line0); //清除LINE0上的中断标志位  
}
 
