/*=============================================================================
该程序由三个部分组成：信号检测程序、通信程序和控制逻辑程序

***检测：暗箱前的传感器检测到桃子来临，发出相机拍照的命令；
	 气缸前的传感器检测到桃子来临，则判断桃子类别，进而判断
	 是否应该将桃子推出
	 
***通信：采用异步收发传输（UART），这部分由三步组成：UART初
   始化程序、写串口程序、读串口程序,对应的函数分别是
	 UART0_Init ();Uart0_Transmit();UART0_ISR(void) interrupt 4
	 
***控制：暗箱前的传感器检测到桃后，给上位机发送命令拍照并识别
	 该桃的类别，上位机将结果返回给下位机，下位机接受后存储到数
	 组peachGrade[30]中，桃的个数限制在30个以内，超出后则从头开
	 始记录。后面的五个传感器负责记录桃的个数，从而找到当前遇到
	 的桃属于数组中第几个，然后查询其类别，判断是否应该推出该桃
==============================================================================*/
#include <c8051f340.h>                 // SFR declarations
#include <stdio.h>

//-----------------------------------------------------------------------------
// Global CONSTANTS
//-----------------------------------------------------------------------------

#define SYSCLK      12000000           // SYSCLK frequency in Hz
#define BAUDRATE        9600           // Baud rate of UART in bps

//-----------------------------------------------------------------------------
// Function PROTOTYPES
//-----------------------------------------------------------------------------

void SYSCLK_Init (void);
void PORT_Init (void);
void Timer2_Init (int);

void UART0_Init (void);
void Uart0_Transmit(unsigned char tmp);
void sendstr(unsigned char *tmp);

void Ext_Interrupt_Init (void);        // Configure External Interrupts (/INT0
                                       // and /INT1);
																			 //P0.0/P0.1两个外部中断入口
sbit SW1 = P2^0;                       // Push-button switch on board
sbit SW2 = P2^1;                       // Push-button switch on board
sbit LED1 = P2^2;                      // Green LED
sbit LED2 = P2^3;                      // Green LED	

/*=============================================================================
检测信号来自于六个红外传感器，传感器信号输出线连接单片机六个引脚
分别为P02/P03/P06/P07/P24/P25

对五个气阀的控制信号，分别通过单片机5个引脚给出
分别为P31/P32/P33/P34/P35
==============================================================================*/
sbit sensor0 = P0^2;	//暗箱前的传感器
sbit sensor1 = P0^3;	//后面五个传感器
sbit sensor2 = P0^6;	//
sbit sensor3 = P0^7;	//
sbit sensor4 = P2^4;	//
sbit sensor5 = P2^5;	//

sbit valve1 = P3^1;	//五个气阀
sbit valve2 = P3^2;
sbit valve3 = P3^3;
sbit valve4 = P3^4;
sbit valve5 = P3^5;


unsigned char inputcharacter;       // Used to store character from UART	
unsigned char camera_order = '6';
unsigned char peachGrade[30] = {0};
void delay1ms(unsigned int i);

unsigned int sensor0_record = 0;
unsigned int sensor1_record = 0;//记录第N个阀门处经过的桃的个数
unsigned int sensor2_record = 0;
unsigned int sensor3_record = 0;
unsigned int sensor4_record = 0;
unsigned int sensor5_record = 0;
	

unsigned int push1 = 0;		//记录第N个阀门推出桃的个数
unsigned int push2 = 0;
unsigned int push3 = 0;
unsigned int push4 = 0;
unsigned int push5 = 0;



unsigned int i = 1;		//记录桃的等级
unsigned int j = 0;
unsigned int k = 0;		//
//-----------------------------------------------------------------------------
// MAIN Routine
//-----------------------------------------------------------------------------

void main (void) 
{
		unsigned char flag0 = '0';
		unsigned char flag1 = '0';
		unsigned char flag2 = '0';
		unsigned char flag3 = '0';
		unsigned char flag4 = '0';
		unsigned char flag5 = '0';
		unsigned char inputcharacter;
	
		unsigned int push_delay1 = 0;		//此标志作为推桃的延迟信号
		unsigned int push_delay2 = 0;
		unsigned int push_delay3 = 0;
		unsigned int push_delay4 = 0;
		unsigned int push_delay5 = 0;

		unsigned int delay00 = 0;
		unsigned int delay01 = 0;
		unsigned int delay02 = 0;
		unsigned int delay03 = 0;
		unsigned int delay1 = 0;
		unsigned int delay2 = 0;
		unsigned int delay3 = 0;
		unsigned int delay4 = 0;
		unsigned int delay5 = 0;

unsigned int camera_counts[4] = {0,0,0,0};

   PCA0MD &= ~0x40;                    // WDTE = 0 (clear watchdog timer 
                                       // enable)
   PORT_Init();                        // Initialize Port I/O
   SYSCLK_Init ();                     // Initialize Oscillator

	 Ext_Interrupt_Init();               // Initialize External Interrupts
   UART0_Init(); 
	 EA = 1;
	 ES0 = 1;

	valve1 = 0;	//五个气阀
	valve2 = 0;
	valve3 = 0;
	valve4 = 0;
	valve5 = 0;
	
   while (1)
	 {
/*============================
		 sensor0
==============================*/
			if(sensor0 == 1)
			{
				delay1ms(10);
				if(sensor0 == 1)
				{
					delay1ms(10);
					if(sensor0 == 1)//三次判断是为了消除干扰信号
					{
						flag0 = 'a';//说明桃子已走去
					}
				}
			}
		 
			if(sensor0 == 0)
			{
				delay1ms(10);
				if(sensor0 == 0)
				 {
						delay1ms(10);
						if(sensor0 == 0)
						{
							if(flag0 == 'a')
							{
								flag0 = '0';
								camera_counts[sensor0_record] = 1;
								sensor0_record++;			//记录桃子过来的个数，处理后使其减少一个
								if(sensor0_record == 4)
								{
									sensor0_record = 0;
								}
								
							}
							
						}
				  }
				}

			if(camera_counts[0] == 1)
			{
					delay00++;
					if(delay00 == 110 )
					{
						camera_counts[0] = 0;
						Uart0_Transmit(camera_order);
						delay00 = 0;
					}
			}
			if(camera_counts[1] == 1)
			{
					delay01++;
					if(delay01 == 110 )
					{
						camera_counts[1] = 0;
						Uart0_Transmit(camera_order);
						delay01 = 0;
					}
			}
			
			if(camera_counts[2] == 1)
			{
					delay02++;
					if(delay02 == 110 )
					{
						camera_counts[2] = 0;
						Uart0_Transmit(camera_order);
						delay02 = 0;
					}
			}
	  	if(camera_counts[3] == 1)
			{
					delay03++;
					if(delay03 == 110 )//此参数可以控制相机延时拍照
					{
						camera_counts[3] = 0;
						Uart0_Transmit(camera_order);
						delay03 = 0;
					}
			}
		
			
/*============================
		 sensor1
==============================*/
			if(sensor1 == 1)
			{
				delay1ms(10);
				if(sensor1 == 1)
				{
					delay1ms(10);
					if(sensor1 == 1)
					{
						flag1 = 'a';//说明桃子已走去
					}
				}
			}
		 
			if(sensor1 == 0)
			{
				delay1ms(10);
				if(sensor1 == 0)
				 {
						delay1ms(10);
						if(sensor1 == 0)
						{
							
							if(flag1 == 'a')
							{
								flag1 = '0';
									 {//此处功能为判断决策是否推桃
										 sensor1_record++;
										 if(sensor1_record == 30)//如果超出数组范围，则从头开始查询
										 {
											sensor1_record = 1;
										 }
										if(peachGrade[sensor1_record] == '0')
										{
											push_delay1 = 1;
										}
										
									}
							}
							
						}
				  }
				}
				
				if(push_delay1 >= 1)
				{
					push_delay1++;
					if(push_delay1 == 30)//此参数可以控制气缸退延迟推出
					{
						push_delay1 = 0;
						delay1 = 1;
						valve1 = 1;//使阀门工作，把桃退推出，并将推出的桃记为A
						LED1 = 1;																						
						peachGrade[sensor1_record] = 'A';

					}					
				}
			
				if(delay1 >= 1)
					{
						delay1++;
						if(delay1 == 20)//此参数可以控制气缸延迟拉回
						{
							delay1 = 0;
							valve1 = 0;
							LED1 = 0;
						}
					}
/*============================
		 sensor2
==============================*/
			if(sensor2 == 1)
			{
				delay1ms(10);
				if(sensor2 == 1)
				{
					delay1ms(10);
					if(sensor2 == 1)
					{
						flag2 = 'a';//说明桃子已走去
					}
				}
			}
		 
			if(sensor2 == 0)
			{
				delay1ms(10);
				if(sensor2 == 0)
				 {
						delay1ms(10);
						if(sensor2 == 0)
						{
							
							if(flag2 == 'a')
							{
								flag2 = '0';
										{
												sensor2_record++;
												 if(sensor2_record == 30)
												 {
													sensor2_record = 1;
												 }
												while(peachGrade[sensor2_record] == 'A')
												{
													sensor2_record++;
													if(sensor2_record == 30)
												 {
													sensor2_record = 1;
												 }
												}
												if(peachGrade[sensor2_record] == '1')
													{
														push_delay2 = 1;
													}
										 }
							}
							
						}
				  }
				}
			
				if(push_delay2 >= 1)
				{
					push_delay2++;
					if(push_delay2 == 30)
					{
						push_delay2 = 0;
						valve2 = 1;//使阀门工作，把桃退推出，并将推出的桃记为等级A
						LED1 = 1;																						
						peachGrade[sensor2_record] = 'A';
						delay2 = 1;
					}					
				}
			
				if(delay2 >= 1)
					{
						delay2++;
						if(delay2 == 20)
						{
							delay2 = 0;
							valve2 = 0;
							LED1 = 0;
						}
					}
/*============================
		 sensor3
==============================*/
			if(sensor3 == 1)
			{
				delay1ms(10);
				if(sensor3 == 1)
				{
					delay1ms(10);
					if(sensor3 == 1)
					{
						flag3 = 'a';//说明桃子已走去
					}
				}
			}
		 
			if(sensor3 == 0)
			{
				delay1ms(10);
				if(sensor3 == 0)
				 {
						delay1ms(10);
						if(sensor3 == 0)
						{
							
							if(flag3 == 'a')
							{
								flag3 = '0';
								 {
									sensor3_record++;
									 if(sensor3_record == 30)
									 {
										sensor3_record = 1;
									 }
									while(peachGrade[sensor3_record] == 'A')
									{
										sensor3_record++;
										if(sensor3_record == 30)
									 {
										sensor3_record = 1;
									 }
									}
									if(peachGrade[sensor3_record] == '2')
										{
											push_delay3 = 1;
										}
								 }
							}
							
						}
				  }
				}
			
				if(push_delay3 >= 1)
				{
					push_delay3++;
					if(push_delay3 == 30)
					{
						push_delay3 = 0;
						valve3 = 1;//使阀门工作，把桃退推出，并将推出的桃记为等级A
						LED1 = 1;																						
						peachGrade[sensor3_record] = 'A';
						delay3 = 1;
					}					
				}
			
				if(delay3 >= 1)
					{
						delay3++;
						if(delay3 == 20)
						{
							delay3 = 0;
							valve3 = 0;
							LED1 = 0;
						}
					}
/*============================
		 sensor4
==============================*/
			if(sensor4 == 1)
			{
				delay1ms(10);
				if(sensor4 == 1)
				{
					delay1ms(10);
					if(sensor4 == 1)
					{
						flag4 = 'a';//说明桃子已走去
					}
				}
			}
		 
			if(sensor4 == 0)
			{
				delay1ms(10);
				if(sensor4 == 0)
				 {
						delay1ms(10);
						if(sensor4 == 0)
						{
							
							if(flag4 == 'a')
							{
								flag4 = '0';
								 {
									sensor4_record++;
									 if(sensor4_record == 30)
									 {
										sensor4_record = 1;
									 }
									while(peachGrade[sensor4_record] == 'A')
									{
										sensor4_record++;
										if(sensor4_record == 30)
									 {
										sensor4_record = 1;
									 }
									}
									if(peachGrade[sensor4_record] == '3')
										{
											push_delay4 = 1;
										}
								 }
							}
							
						}
				  }
				}
			
				if(push_delay4 >= 1)
				{
					push_delay4++;
					if(push_delay4 == 30)
					{
						push_delay4 = 0;
						valve4 = 1;//使阀门工作，把桃退推出，并将推出的桃记为等级A
						LED1 = 1;																						
						peachGrade[sensor4_record] = 'A';
						delay4 = 1;
					}					
				}
			
				if(delay4 >= 1)
					{
						delay4++;
						if(delay4 == 20)
						{
							delay4 = 0;
							valve4 = 0;
							LED1 = 0;
						}
					}
/*============================
		 sensor5
==============================*/
			if(sensor5 == 1)
			{
				delay1ms(10);
				if(sensor5 == 1)
				{
					delay1ms(10);
					if(sensor5 == 1)
					{
						flag5 = 'a';//说明桃子已走去
					}
				}
			}
		 
			if(sensor5 == 0)
			{
				delay1ms(10);
				if(sensor5 == 0)
				 {
						delay1ms(10);
						if(sensor5 == 0)
						{
							
							if(flag5 == 'a')
							{
								flag5 = '0';
								 {
									sensor5_record++;
									 if(sensor5_record == 30)
									 {
										sensor5_record = 1;
									 }
									while(peachGrade[sensor5_record] == 'A')
									{
										sensor5_record++;
										if(sensor5_record == 30)
									 {
										sensor5_record = 1;
									 }
									}
									if(peachGrade[sensor5_record] == '4')
										{
											push_delay5 = 1;
										}
								 }
							}
							
						}
				  }
				}			
				
				if(push_delay5 >= 1)
				{
					push_delay5++;
					if(push_delay5 == 30)
					{
						push_delay5 = 0;
						valve5 = 1;//使阀门工作，把桃退推出，并将推出的桃记为等级A
						LED1 = 1;																						
						peachGrade[sensor5_record] = 'A';
						delay5 = 1;
					}					
				}
			
				if(delay5 >= 1)
					{
						delay5++;
						if(delay5 == 20)
						{
							delay5 = 0;
							valve5 = 0;
							LED1 = 0;
						}
					}

			
			
		}	

	 

/*		 {
      printf ("\nEnter character: ");
      inputcharacter = getchar ();
      printf ("\nCharacter entered : %c",inputcharacter);
      printf ("\n      Value in Hex: %bx",inputcharacter);
   }*/
}

void delay1ms(unsigned int i)
{
    unsigned int j;
	while(i--)
 	{
 		for(j=0;j<125;j++)	  //1ms??????
 		{
  			;
 		}
 	} 
}
//-----------------------------------------------------------------------------
// PORT_Init
//-----------------------------------------------------------------------------
//
// P0.4   digital   push-pull    UART TX
// P0.5   digital   open-drain   UART RX
// 
//-----------------------------------------------------------------------------

void PORT_Init (void)
{
   P0MDOUT |= 0x10;                    // Enable UTX as push-pull output
   XBR0     = 0x01;                    // Enable UART on P0.4(TX) and P0.5(RX)                     
   XBR1     = 0x40;                    // Enable crossbar and weak pull-ups
}

//-----------------------------------------------------------------------------
// SYSCLK_Init
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// This routine initializes the system clock to use the internal oscillator
// at its maximum frequency.
// Also enables the Missing Clock Detector.
//-----------------------------------------------------------------------------

void SYSCLK_Init (void)
{
   OSCICN |= 0x03;                     // Configure internal oscillator for
                                       // its maximum frequency
   RSTSRC  = 0x04;                     // Enable missing clock detector
}

//-----------------------------------------------------------------------------
// UART0_Init
//-----------------------------------------------------------------------------
//
// Configure the UART0 using Timer1, for <BAUDRATE> and 8-N-1.
//-----------------------------------------------------------------------------

void UART0_Init (void)
{
   SCON0 = 0x10;                       // SCON0: 8-bit variable bit rate
                                       //        level of STOP bit is ignored
                                       //        RX enabled
                                       //        ninth bits are zeros
                                       //        clear RI0 and TI0 bits
   if (SYSCLK/BAUDRATE/2/256 < 1) {
      TH1 = -(SYSCLK/BAUDRATE/2);
      CKCON &= ~0x0B;                  // T1M = 1; SCA1:0 = xx
      CKCON |=  0x08;
   } else if (SYSCLK/BAUDRATE/2/256 < 4) {
      TH1 = -(SYSCLK/BAUDRATE/2/4);
      CKCON &= ~0x0B;                  // T1M = 0; SCA1:0 = 01                  
      CKCON |=  0x01;
   } else if (SYSCLK/BAUDRATE/2/256 < 12) {
      TH1 = -(SYSCLK/BAUDRATE/2/12);
      CKCON &= ~0x0B;                  // T1M = 0; SCA1:0 = 00
   } else {
      TH1 = -(SYSCLK/BAUDRATE/2/48);
      CKCON &= ~0x0B;                  // T1M = 0; SCA1:0 = 10
      CKCON |=  0x02;
   }

   TL1 = TH1;                          // Init Timer1
   TMOD &= ~0xf0;                      // TMOD: timer 1 in 8-bit autoreload
   TMOD |=  0x20;                       
   TR1 = 1;                            // START Timer1
   TI0 = 1;                            // Indicate TX0 ready
}

//--------------------------------
//串口UART0中断，接收桃的分类结果
//--------------------------------
//中断向量0x0023
void UART0_ISR(void) interrupt 4
{
	//Rx,Tx公用中断
	//接收中断
/*		EA = 0;
	if(RI0 == 1)
	{
		RI0 = 0;
		temp = SBUF0;
		LED2 = !LED2;
	}
	if(TI0 == 1)
	{
		TI0 = 0;
	}
	EA = 1;	*/
	unsigned char temp;
	if(!TI0)
	{
		RI0 = 0;
		temp = SBUF0;
		LED2 = !LED2;
		if(i == 30)
			i = 1;
		peachGrade[i] = temp;
		i++;
	}
	
	//发送中断
	else TI0 = 0;
}

//-----------------------------
//串口UART0发送函数
//-----------------------------
void Uart0_Transmit(unsigned char tmp)
{
	ES0 = 0; //关UART0中断
	EA = 0; //关全局中断
	SBUF0 = tmp;	
	
	while(TI0 == 0);
	TI0 = 0;
	ES0 = 1;
	EA = 1;
}

//-----------------------------
//发送字符串
//-----------------------------
void sendstr(unsigned char* tmp)
{
	while(*tmp)
	{
		Uart0_Transmit(*tmp++);
	}
}

//-----------------------------------------------------------------------------
// Ext_Interrupt_Init

// This function configures and enables /INT0 and /INT1 (External Interrupts)
// as negative edge-triggered.
//
//-----------------------------------------------------------------------------
void Ext_Interrupt_Init (void)				//外部中断初始化
{
   TCON = 0x05;                        // /INT 0 and /INT 1 are edge triggered

   IT01CF = 0x10;                      // /INT0 active low; /INT0 on P0.0;
                                       // /INT1 active low; /INT1 on P0.1

	 PX0 = 1;														//外部中断0位高优先级
	
   EX0 = 1;                            // Enable /INT0 interrupts
   EX1 = 1;                            // Enable /INT1 interrupts
}

/*=======================================
下面中断检测到桃来后，给相机发出命令
中断0由P00下降沿触发
========================================*/
/*
void INT0_ISR (void) interrupt 0			
{
		delay1ms(10);
	if(sensor0 == 0)
	{
		delay1ms(30);
	if(sensor0 == 0)
	{

		{
//		k++;
//		if(k == 2)
		{
//			k = 0;
			Uart0_Transmit(camera_order);
			LED1 = !LED1;
		}
		}
	}
	}
*/	
	

	
		



/*=======================================
由后面五个传感器触发;P01
========================================*/
void INT1_ISR (void) interrupt 2
{
	LED2 = !LED2;
}

 
	 
	 
//-----------------------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------------------