#include "exti.h"
#include "led.h"
#include "lcd.h"
#include "key.h"
#include "delay.h"
#include "usart.h"

 extern u8 flag,flag1,flag2,flag3;
 extern u8 temp1;
 
//�ⲿ�ж�0�������
void EXTIX_Init(void)
{
 
 	  EXTI_InitTypeDef EXTI_InitStructure;
 	  NVIC_InitTypeDef NVIC_InitStructure;

    KEY_Init();	 //	�����˿ڳ�ʼ��

  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	//ʹ�ܸ��ù���ʱ��

  //GPIOA.0 �ж����Լ��жϳ�ʼ������   �½��ش���
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource0);

  	EXTI_InitStructure.EXTI_Line=EXTI_Line0;	                //KEY_UP
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;   //���������غ��½����жϴ���
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);	 	                 //����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ���


  	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;			//ʹ�ܰ���WK_UP���ڵ��ⲿ�ж�ͨ��
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	//��ռ���ȼ�0�� 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;					//�����ȼ�0
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//ʹ���ⲿ�ж�ͨ��
  	NVIC_Init(&NVIC_InitStructure); 
 
}


//�ⲿ�ж�0������� 
void EXTI0_IRQHandler(void)
{
	if(KEY3==1)
	{
		delay_us(1000);
		if(KEY3==1)
		TIM_Cmd(TIM2, ENABLE);  //������ʹ��TIMx
  }
	else
	{
		delay_us(1000);
		if(KEY3!=1)
		TIM_Cmd(TIM2, DISABLE); //�½���ʧ��TIMx
		
		if(flag<=15&&flag>=3) 
		{
			if(flag2==1)      //Ҫ�ȿ������̰���������
      {
				flag1=!flag1;			
			  flag=0;
      }
			else
				flag=0;
    }
		
	  if(flag>=30)    //����3��
		{
			flag2=!flag2;
			flag=0;
    }
		
		if(flag>=16&&flag<=29)
			flag=0;
  }
		
	EXTI_ClearITPendingBit(EXTI_Line0); //���LINE0�ϵ��жϱ�־λ  
}
 
