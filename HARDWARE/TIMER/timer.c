#include "sys.h"
#include "timer.h"
#include "key.h"
#include "led.h"
#include "adc.h"
#include "word.h"
#include "usart.h"
#include "flash.h"

extern u8 TEXT_Buffer1[126][2],TEXT_Buffer2[126][2]; 
extern u16 adcx;
extern u32 time_collect;
extern u8 key;
extern u8 flag_cycle,flag_cycle_1;
extern u8 flag,flag1,temp1;
extern u16 m;
extern u16 counter;
extern u32 FLASH_SIZE;
#define SIZE sizeof(TEXT_Buffer1)

u16 i,j;
u16 t=0;

//ͨ�ö�ʱ��2�жϳ�ʼ��
//����ʱ��ѡ��ΪAPB1��2������APB1Ϊ36M
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//Tout=((arr+1)*(psc+1))/Tclk  ���У�Tclk��TIM������ʱ��Ƶ�ʣ���λΪMhz�� Tout��TIM���ʱ�䣨��λΪus��
//����ʹ�õ��Ƕ�ʱ��2!
	
void TIM2_Int_Init(u16 arr,u16 psc)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); //ʱ��ʹ��
	
	//��ʱ��TIM3��ʼ��
	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
 
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE ); //ʹ��ָ����TIM3�ж�,��������ж�

	//�ж����ȼ�NVIC����
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  //TIM3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  //��ռ���ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //�����ȼ�3��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //��ʼ��NVIC�Ĵ���


  //TIM_Cmd(TIM2, ENABLE);  //ʹ��TIMx					 
}




void TIM3_Int_Init(u16 arr,u16 psc)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //ʱ��ʹ��
	
	//��ʱ��TIM3��ʼ��
	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
 
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //ʹ��ָ����TIM3�ж�,��������ж�

	//�ж����ȼ�NVIC����
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //��ռ���ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  //�����ȼ�3��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //��ʼ��NVIC�Ĵ���


	//TIM_Cmd(TIM3, ENABLE);  //ʹ��TIMx					 
}



//��ʱ��2�жϷ������
void TIM2_IRQHandler(void)   //TIM3�ж�
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)  //���TIM3�����жϷ������
		{
			TIM_ClearITPendingBit(TIM2, TIM_IT_Update  );  //���TIMx�����жϱ�־
			
			flag++;     	
			
		}
}

//��ʱ��3�жϷ������
void TIM3_IRQHandler(void)   //TIM3�ж�
{    	
	//LED0=0;
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  //���TIM3�����жϷ������
		{
			TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //���TIMx�����жϱ�־
			
			time_collect++;
			
			t++;

			
			if(t>=2)
			{	
				if(flag_cycle==0)
				{
					//ÿ���жϲɼ�AD��Լ��ʱΪ420us					
					adcx=Get_Adc_Average(ADC_Channel_1,4);
					TEXT_Buffer1[m][0]=adcx>>8;
					TEXT_Buffer1[m][1]=adcx&0x00ff;
					
					m++;
					
					adcx=Get_Adc_Average(ADC_Channel_2,4);
					TEXT_Buffer1[m][0]=adcx>>8;
					TEXT_Buffer1[m][1]=adcx&0x00ff;
					
					m++;
					
					adcx=Get_Adc_Average(ADC_Channel_3,4);
					TEXT_Buffer1[m][0]=adcx>>8;
					TEXT_Buffer1[m][1]=adcx&0x00ff;
					
					m++;
        }
				
				if(flag_cycle==1)
				{
					//ÿ���жϲɼ�AD��Լ��ʱΪ420us					
					adcx=Get_Adc_Average(ADC_Channel_1,4);
					TEXT_Buffer2[m][0]=adcx>>8;
					TEXT_Buffer2[m][1]=adcx&0x00ff;
					
					m++;
					
					adcx=Get_Adc_Average(ADC_Channel_2,4);
					TEXT_Buffer2[m][0]=adcx>>8;
					TEXT_Buffer2[m][1]=adcx&0x00ff;
					
					m++;
					
					adcx=Get_Adc_Average(ADC_Channel_3,4);
					TEXT_Buffer2[m][0]=adcx>>8;
					TEXT_Buffer2[m][1]=adcx&0x00ff;
					
					m++;
        }
				

		  if(m==126)   //���鴫��252���ֽڴ�Լ��ʱ372us
			{
 				m=0;
				flag_cycle=!flag_cycle;
				flag_cycle_1=0;
				LED0=!LED0;	
			}					
				t=2;        			
      }
		}					

}





