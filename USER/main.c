/************************************************************************
 Copyright(C):2015-2020. 武汉工程大学电气信息学院508. All rights reserved.
 文件名：基于STM32的数据采集系统
 作者：苏功闯    完成时间：2015.05.13
 描述：本程序用于实现基于STM32的数据采集系统的设计，该系统能够以1KHz采集
       三通道的过载数据，并保存在SPIFlash中，而且还可以通过USART串口通信
			 上传至上位机显示（串口调试助手）；使用多功能按键实现电源的开关和采
			 集的启停。
 说明：1.TFTLCD显示：
                 武汉工程大学
						      基于STM32
							 多路数据采集系统
				 若W25Q64被检测到，则显示
				          W25Q64 Ready!
				 若数据采集开始，则显示 
				  Data collection began...
				 若数据采集完毕，则显示 
				  Data collection complete!
				 若数据开始上传，则显示
				  Data is being uploaded...
				 若数据上传完毕，则显示
				  Data upload is completed.
				 若数据上传被终止，则显示
				  Data upload is terminated!
				 若关机，则显示
				  Shutdown!
				2.多功能按键：按着：开机-->采集开启-->采集停止-->关机顺序
				  开机：长按三秒
					采集开启并写入：长按一秒
					采集停止并上传：长按一秒
					关机：长按三秒
				3.中断优先级：外部中断>TIM2中断>TIM3中断
				  外部中断：按键KEY_UP控制系统各种操作
					TIM2中断：测量按键按下的有效时间
					TIM3中断：以一定的频率采集数据并写入Flash			
        4.LED灯：开机后，绿灯亮-->长按一秒，红灯以一定频率闪烁
				  -->长按一秒，红灯以一定频率闪烁-->关机后，红绿灯同时亮一秒后灭
        5.硬件连接：
          ADC1通道0-->PA0   ADC1通道1-->PA1   ADC1通道2-->PA2
          DS0-->PB5   DS1-->PE5	
          KEY_UP-->PA0			
        6.采集数据的个数=(time_collect-1)*6.以8位字节为单位进行计数.					
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

//采集到的数据存在TEXT_Buffer中，并写入Flash中
u8 TEXT_Buffer1[126][2];     
u8 TEXT_Buffer2[126][2];

u8 flag=0,flag1=0,flag2=0,flag3=0;
u16 adcx;
u32 time_collect=0;   //用于检测或设置采集时间
u16 data_residue=0;   //用于解决当采集时间不是42ms的倍数时，部分采集数据未写入FLash的问题
u8 key;
u16 m=0,n=0,k=0;
u8 temp=0,temp1=0,temp2=0,temp3=0;
u8 flag_cycle=0,flag_cycle_1=0;
u16 counter=0;
u32 FLASH_SIZE=8*1024*1024;	//FLASH 大小为8M字节

//数组的大小是252个字节，必须6的倍数
#define SIZE sizeof(TEXT_Buffer1)   

 int main(void)
 {
		u8 Data_PC[SIZE];    //从Flash中读取数据到Data_PC，并上传至上位机
	  u16 a,b=0;

		delay_init();	    	 //延时函数初始化	  
		NVIC_Configuration();//设置NVIC中断分组2:2位抢占优先级，2位响应优先级
		uart_init(9600);	   //串口初始化为9600
		LED_Init();			     //LED端口初始化
		LCD_Init();
	  KEY_Init(); 
		WORD_Init();	
	  TIM2_Int_Init(999,7199);	  
	  TIM3_Int_Init(9,7199);
	  EXTIX_Init();		 	   //外部中断初始化
		Adc_Init();		  		 //ADC初始化
	 	SPI_Flash_Init();  	 //SPI FLASH 初始化 
		
	while(1)
	{
//功能：开机状态显示
//说明：1.开机，绿灯亮（DS1）
//      2.使用temp是为了避免重复执行该if语句
		if(flag2==1&&temp==0)     
		{
		  while(SPI_Flash_ReadID()!=W25Q64)							//检测不到W25Q64
			{
				LCD_ShowString(40,150,200,16,16,"W25Q64 Check Failed!");
				delay_ms(500);
				LCD_ShowString(40,150,200,16,16,"Please Check!      ");
				delay_ms(500);
				LED0=!LED0;//DS0闪烁
			}
	    LCD_ShowString(60,130,200,16,16,"W25Q64 Ready!");
		  LED1=0;
			temp=1;
    }
		
//功能：数据采集并保存至Flash中	
//说明：1.使用temp1是为了避免重复使能TIM3	
//      2.使用temp2是为了避免在数据上传至上位机的过程中，与终止上传功能相冲突
//      3.flag1=1代表按键按下，并开始采集	
//      4.flag2=1代表开机，flag2=0代表未开机	
//      5.采集过程中，红灯（DS0）以一定频率闪烁		
		if(flag2==1&&temp2==0&&flag1==1)
		{
      if(temp1==0)
			{
				LCD_ShowString(20,150,200,16,16,"Data collection began...");
			  TIM_Cmd(TIM3, ENABLE);   //使能TIM3中断
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

//功能：将Flash中采集到的数据上传至上位机	
//说明：1.使用temp1是为了保证上传是在采集之后进行
//      2.使用temp3是为了避免由于按键误操作使得该程序重复被执行
//      3.flag1=0代表按键再次被按下，上传开始	；若再次被按下，则上传被终止；之后再按就没有作用了	
//      4.上传过程中，红灯（DS0）以一定频率闪烁
		if(temp1==1&&temp3==0&&flag1==0)
		{
			TIM_Cmd(TIM3, DISABLE);    //失能TIM3中断
			
			//用于解决当采集时间不是42ms的倍数时，部分采集数据未写入FLash的问题
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
				 if(flag1==1)      //上传中途使用按键停止传送
				 {
					break;
				 }
				 USART_GetFlagStatus(USART1, USART_FLAG_TC);
				 USART_SendData(USART1,Data_PC[b]);			  
				 while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET);				
				}
				if(flag1==1)      //上传中途使用按键停止传送
				{
				 LCD_ShowString(20,210,200,16,16,"Data upload is terminated!");	
				 break;
				}
			}
			
			//用于解决当采集时间不是42ms的倍数时，部分采集数据未写入FLash的问题
			if(flag1!=1&&(time_collect-1)%42!=0)
			{
				LED0=!LED0;
				SPI_Flash_Read(Data_PC,FLASH_SIZE+(a+1)*SIZE,SIZE);
				for(b=0;b<data_residue;b++)     
				{
				 if(flag1==1)      //上传中途使用按键停止传送
				 {
					break;
				 }
				 USART_GetFlagStatus(USART1, USART_FLAG_TC);
				 USART_SendData(USART1,Data_PC[b]);			  
				 while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET);				
				}
				if(flag1==1)      //上传中途使用按键停止传送
				{
				 LCD_ShowString(20,210,200,16,16,"Data upload is terminated!");	
				 break;
				}
      }
			
			if(flag1!=1)   //要么显示数据上传完成，要么显示数据上传完成
			{
				LCD_ShowString(20,210,200,16,16,"Data upload is completed!");
			}
			temp3=1;		
     }

//功能：关机并擦除Flash	
//说明：1.flag2=0代表关机，与上temp1,temp2是要保证前面的已被执行
//      2.关机过程中擦除Flash，完成之后红绿灯同时亮一秒后灭		 
		if(temp1==1&&temp2==1&&flag2==0)   //关机机状态显示
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