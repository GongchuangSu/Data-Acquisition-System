/************************************************************************
 Copyright(C):2015-2020. �人���̴�ѧ������ϢѧԺ508. All rights reserved.
 �ļ���������STM32�����ݲɼ�ϵͳ
 ���ߣ��չ���    ���ʱ�䣺2015.05.13
 ����������������ʵ�ֻ���STM32�����ݲɼ�ϵͳ����ƣ���ϵͳ�ܹ���1KHz�ɼ�
       ��ͨ���Ĺ������ݣ���������SPIFlash�У����һ�����ͨ��USART����ͨ��
			 �ϴ�����λ����ʾ�����ڵ������֣���ʹ�ö๦�ܰ���ʵ�ֵ�Դ�Ŀ��غͲ�
			 ������ͣ��
 ˵����1.TFTLCD��ʾ��
                 �人���̴�ѧ
						      ����STM32
							 ��·���ݲɼ�ϵͳ
				 ��W25Q64����⵽������ʾ
				          W25Q64 Ready!
				 �����ݲɼ���ʼ������ʾ 
				  Data collection began...
				 �����ݲɼ���ϣ�����ʾ 
				  Data collection complete!
				 �����ݿ�ʼ�ϴ�������ʾ
				  Data is being uploaded...
				 �������ϴ���ϣ�����ʾ
				  Data upload is completed.
				 �������ϴ�����ֹ������ʾ
				  Data upload is terminated!
				 ���ػ�������ʾ
				  Shutdown!
				2.�๦�ܰ��������ţ�����-->�ɼ�����-->�ɼ�ֹͣ-->�ػ�˳��
				  ��������������
					�ɼ�������д�룺����һ��
					�ɼ�ֹͣ���ϴ�������һ��
					�ػ�����������
				3.�ж����ȼ����ⲿ�ж�>TIM2�ж�>TIM3�ж�
				  �ⲿ�жϣ�����KEY_UP����ϵͳ���ֲ���
					TIM2�жϣ������������µ���Чʱ��
					TIM3�жϣ���һ����Ƶ�ʲɼ����ݲ�д��Flash			
        4.LED�ƣ��������̵���-->����һ�룬�����һ��Ƶ����˸
				  -->����һ�룬�����һ��Ƶ����˸-->�ػ��󣬺��̵�ͬʱ��һ�����
        5.Ӳ�����ӣ�
          ADC1ͨ��0-->PA0   ADC1ͨ��1-->PA1   ADC1ͨ��2-->PA2
          DS0-->PB5   DS1-->PE5	
          KEY_UP-->PA0			
        6.�ɼ����ݵĸ���=(time_collect-1)*6.��8λ�ֽ�Ϊ��λ���м���.					
************************************************************************/
#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"	 
#include "adc.h"
#include "word.h"
#include "timer.h"
#include "flash.h"	
#include "exti.h"

//�ɼ��������ݴ���TEXT_Buffer�У���д��Flash��
u8 TEXT_Buffer1[126][2];     
u8 TEXT_Buffer2[126][2];

u8 flag=0,flag1=0,flag2=0,flag3=0;
u16 adcx;
u32 time_collect=0;   //���ڼ������òɼ�ʱ��
u16 data_residue=0;   //���ڽ�����ɼ�ʱ�䲻��42ms�ı���ʱ�����ֲɼ�����δд��FLash������
u8 key;
u16 m=0,n=0,k=0;
u8 temp=0,temp1=0,temp2=0,temp3=0;
u8 flag_cycle=0,flag_cycle_1=0;
u16 counter=0;
u32 FLASH_SIZE=8*1024*1024;	//FLASH ��СΪ8M�ֽ�

//����Ĵ�С��252���ֽڣ�����6�ı���
#define SIZE sizeof(TEXT_Buffer1)   

 int main(void)
 {
		u8 Data_PC[SIZE];    //��Flash�ж�ȡ���ݵ�Data_PC�����ϴ�����λ��
	  u16 a,b=0;

		delay_init();	    	 //��ʱ������ʼ��	  
		NVIC_Configuration();//����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
		uart_init(9600);	   //���ڳ�ʼ��Ϊ9600
		LED_Init();			     //LED�˿ڳ�ʼ��
		LCD_Init();
	  KEY_Init(); 
		WORD_Init();	
	  TIM2_Int_Init(999,7199);	  
	  TIM3_Int_Init(9,7199);
	  EXTIX_Init();		 	   //�ⲿ�жϳ�ʼ��
		Adc_Init();		  		 //ADC��ʼ��
	 	SPI_Flash_Init();  	 //SPI FLASH ��ʼ�� 
		
	while(1)
	{
//���ܣ�����״̬��ʾ
//˵����1.�������̵�����DS1��
//      2.ʹ��temp��Ϊ�˱����ظ�ִ�и�if���
		if(flag2==1&&temp==0)     
		{
		  while(SPI_Flash_ReadID()!=W25Q64)							//��ⲻ��W25Q64
			{
				LCD_ShowString(40,150,200,16,16,"W25Q64 Check Failed!");
				delay_ms(500);
				LCD_ShowString(40,150,200,16,16,"Please Check!      ");
				delay_ms(500);
				LED0=!LED0;//DS0��˸
			}
	    LCD_ShowString(60,130,200,16,16,"W25Q64 Ready!");
		  LED1=0;
			temp=1;
    }
		
//���ܣ����ݲɼ���������Flash��	
//˵����1.ʹ��temp1��Ϊ�˱����ظ�ʹ��TIM3	
//      2.ʹ��temp2��Ϊ�˱����������ϴ�����λ���Ĺ����У�����ֹ�ϴ��������ͻ
//      3.flag1=1���������£�����ʼ�ɼ�	
//      4.flag2=1��������flag2=0����δ����	
//      5.�ɼ������У���ƣ�DS0����һ��Ƶ����˸		
		if(flag2==1&&temp2==0&&flag1==1)
		{
      if(temp1==0)
			{
				LCD_ShowString(20,150,200,16,16,"Data collection began...");
			  TIM_Cmd(TIM3, ENABLE);   //ʹ��TIM3�ж�
        LED0=0;				
      }
			temp1=1;
			if(flag_cycle==0&&flag_cycle_1==0)
			{
				SPI_Flash_Write((u8*)TEXT_Buffer2,FLASH_SIZE+counter*SIZE,SIZE);
				counter++;
				flag_cycle_1=1;
      }
			if(flag_cycle==1&&flag_cycle_1==0)
			{
				SPI_Flash_Write((u8*)TEXT_Buffer1,FLASH_SIZE+counter*SIZE,SIZE);
				counter++;
				flag_cycle_1=1;
      }						
		}

//���ܣ���Flash�вɼ����������ϴ�����λ��	
//˵����1.ʹ��temp1��Ϊ�˱�֤�ϴ����ڲɼ�֮�����
//      2.ʹ��temp3��Ϊ�˱������ڰ��������ʹ�øó����ظ���ִ��
//      3.flag1=0�������ٴα����£��ϴ���ʼ	�����ٴα����£����ϴ�����ֹ��֮���ٰ���û��������	
//      4.�ϴ������У���ƣ�DS0����һ��Ƶ����˸
		if(temp1==1&&temp3==0&&flag1==0)
		{
			TIM_Cmd(TIM3, DISABLE);    //ʧ��TIM3�ж�
			
			//���ڽ�����ɼ�ʱ�䲻��42ms�ı���ʱ�����ֲɼ�����δд��FLash������
			if((time_collect-1)%42!=0)
			{
				data_residue=(time_collect-1)%42*6;
				if(flag_cycle==0)
				{
					SPI_Flash_Write((u8*)TEXT_Buffer2,FLASH_SIZE+(counter+1)*SIZE,SIZE);
        }
				if(flag_cycle==1)
				{
					SPI_Flash_Write((u8*)TEXT_Buffer1,FLASH_SIZE+(counter+1)*SIZE,SIZE);					
        }
      }
				
			LCD_ShowString(20,170,200,16,16,"Data collection complete!");
			LCD_ShowString(20,190,200,16,16,"Data is being uploaded...");
			temp2=1;

			for(a=1;a<counter;a++)
			{
				LED0=!LED0;
				SPI_Flash_Read(Data_PC,FLASH_SIZE+a*SIZE,SIZE);
				for(b=0;b<SIZE;b++)     
				{
				 if(flag1==1)      //�ϴ���;ʹ�ð���ֹͣ����
				 {
					break;
				 }
				 USART_GetFlagStatus(USART1, USART_FLAG_TC);
				 USART_SendData(USART1,Data_PC[b]);			  
				 while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET);				
				}
				if(flag1==1)      //�ϴ���;ʹ�ð���ֹͣ����
				{
				 LCD_ShowString(20,210,200,16,16,"Data upload is terminated!");	
				 break;
				}
			}
			
			//���ڽ�����ɼ�ʱ�䲻��42ms�ı���ʱ�����ֲɼ�����δд��FLash������
			if(flag1!=1&&(time_collect-1)%42!=0)
			{
				LED0=!LED0;
				SPI_Flash_Read(Data_PC,FLASH_SIZE+(a+1)*SIZE,SIZE);
				for(b=0;b<data_residue;b++)     
				{
				 if(flag1==1)      //�ϴ���;ʹ�ð���ֹͣ����
				 {
					break;
				 }
				 USART_GetFlagStatus(USART1, USART_FLAG_TC);
				 USART_SendData(USART1,Data_PC[b]);			  
				 while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET);				
				}
				if(flag1==1)      //�ϴ���;ʹ�ð���ֹͣ����
				{
				 LCD_ShowString(20,210,200,16,16,"Data upload is terminated!");	
				 break;
				}
      }
			
			if(flag1!=1)   //Ҫô��ʾ�����ϴ���ɣ�Ҫô��ʾ�����ϴ����
			{
				LCD_ShowString(20,210,200,16,16,"Data upload is completed!");
			}
			temp3=1;		
     }

//���ܣ��ػ�������Flash	
//˵����1.flag2=0����ػ�������temp1,temp2��Ҫ��֤ǰ����ѱ�ִ��
//      2.�ػ������в���Flash�����֮����̵�ͬʱ��һ�����		 
		if(temp1==1&&temp2==1&&flag2==0)   //�ػ���״̬��ʾ
		{
			flag1=0;
			flag2=0;
      temp1=0;
      temp2=0;			
			LED0=1;
			LED1=1;
			SPI_Flash_Erase_Chip();
		  LED0=0;
			LED1=0;
			delay_ms(1000);
			LED0=1;
			LED1=1;
			LCD_ShowString(80,230,200,16,16,"Shutdown!");
    }
  }	
}