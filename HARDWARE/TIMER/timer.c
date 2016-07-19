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

//通用定时器2中断初始化
//这里时钟选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数
//Tout=((arr+1)*(psc+1))/Tclk  其中，Tclk：TIM的输入时钟频率（单位为Mhz） Tout：TIM溢出时间（单位为us）
//这里使用的是定时器2!
	
void TIM2_Int_Init(u16 arr,u16 psc)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); //时钟使能
	
	//定时器TIM3初始化
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE ); //使能指定的TIM3中断,允许更新中断

	//中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器


  //TIM_Cmd(TIM2, ENABLE);  //使能TIMx					 
}




void TIM3_Int_Init(u16 arr,u16 psc)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //时钟使能
	
	//定时器TIM3初始化
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //使能指定的TIM3中断,允许更新中断

	//中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器


	//TIM_Cmd(TIM3, ENABLE);  //使能TIMx					 
}



//定时器2中断服务程序
void TIM2_IRQHandler(void)   //TIM3中断
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)  //检查TIM3更新中断发生与否
		{
			TIM_ClearITPendingBit(TIM2, TIM_IT_Update  );  //清除TIMx更新中断标志
			
			flag++;     	
			
		}
}

//定时器3中断服务程序
void TIM3_IRQHandler(void)   //TIM3中断
{    	
	//LED0=0;
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  //检查TIM3更新中断发生与否
		{
			TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //清除TIMx更新中断标志
			
			time_collect++;
			
			t++;

			
			if(t>=2)
			{	
				if(flag_cycle==0)
				{
					//每次中断采集AD大约耗时为420us					
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
					//每次中断采集AD大约耗时为420us					
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
				

		  if(m==126)   //数组传递252个字节大约耗时372us
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





