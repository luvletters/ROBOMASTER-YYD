
#include "stm32f10x.h"
#include "delay.h"
#include "Servo.h"
#include "stm32f10x.h"
#include "bsp_usart.h"
#include "bsp_adc.h"
#include "motor.h"
#include "math.h"
#include "oled.h"
#include "stdtimer.h"
#include "EncoderR.h"
#include "EncoderL.h"

float pid_s,pid_m_R,pid_m_L;
void Servo_open(void)
{
	TIM_SetCompare1(TIM_SERVO,pid_s);
	TIM_SetCompare2(TIM_SERVO,pid_s);
	TIM_SetCompare3(TIM_SERVO,pid_s);
}
//--------------------------------------------------------------------------------^^
// ADC1转换的电压值通过MDA方式传到SRAM
extern __IO uint16_t ADC_ConvertedValue[NOFCHANEL];

// 局部变量，用于保存转换计算后的电压值 	 
float ADC_ConvertedValueLocal[NOFCHANEL];    
float LastSensorDevitation=0,Lastsensor;
float away=1;
//lalala
int i=0;
int j=0;
int k=0;

// 软件延时
void Delay(__IO uint32_t nCount)
{
  for(; nCount != 0; nCount--);
} 


//--------------------------------- main start here ------------------------------------------------
int main()
{
	
	
	OLED_init();
	
	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%   INIT1  &&&&&&&&&&&&&&&&&&&&&&&&&
	GPIO_InitTypeDef GPIO_InitStructure;	//定义一个GPIO结构体变量
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;//定义一个定时器结构体变量
	TIM_ICInitTypeDef TIM_ICInitStructure;	//定义一个定时器编码器结构体变量

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD|RCC_APB2Periph_AFIO, ENABLE);//使能GPIOD时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);//使能定时器4

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13;		//配置D12-> TIM4_CH1   D13-> TIM4_CH2 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//设置50MHz时钟
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;		//设置为下拉输入模式
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	//开启重映射
	GPIO_PinRemapConfig(GPIO_Remap_TIM4, ENABLE);
	TIM_TimeBaseStructure.TIM_Period = 0xffff;	//计数器最大值	
	TIM_TimeBaseStructure.TIM_Prescaler = 0;	//时钟不分频
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;	// 使用的采样频率之间的分频比例
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	//向上计数
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);	//初始化定时器4

	TIM_EncoderInterfaceConfig(TIM4, TIM_EncoderMode_TI12,TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);//在AB相计数
	TIM_ICStructInit(&TIM_ICInitStructure);
	TIM_ICInitStructure.TIM_ICFilter = 0;
	TIM_ICInit(TIM4, &TIM_ICInitStructure);
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&


	TIM4->CNT = 0;//cnt清0
	TIM_Cmd(TIM4, DISABLE);  //准备开始计数//初始化编码器
	
	
	TIM_Cmd(TIM4, ENABLE);//打开编码器定时器

	
	short Encoder_v1=0;//编码器当前速度值
	int Encoder_a1=0;//编码器当前转角
	
	Encoder_Init2();//初始化编码器
	Encoder_Start2();//打开编码器定时器
	
	short Encoder_v2=0;//编码器当前速度值
	int Encoder_a2=0;//编码器当前转角
	
	delay_init();	//延时初始化
	TIM_PWM_Init();	//定时器PWM初始化
	
	// 配置串口
	USART_Config();
	
	// ADC 初始化
	ADCx_Init();
	
	//motor set
	tim_motor_init();
	
MY_TIM6_Init(71,9999);

/*
	while(1)
	{
		  ADC_ConvertedValueLocal[0] =(float) ADC_ConvertedValue[0]/4096*3.3;
			ADC_ConvertedValueLocal[1] =(float) ADC_ConvertedValue[1]/4096*3.3;
			ADC_ConvertedValueLocal[2] =(float) ADC_ConvertedValue[2]/4096*3.3;
			ADC_ConvertedValueLocal[3] =(float) ADC_ConvertedValue[3]/4096*3.3;
			ADC_ConvertedValueLocal[4] =(float) ADC_ConvertedValue[4]/4096*3.3;
		motor_run(motor_1,100);//right negative
		motor_run(motor_2,180);//right positive
		motor_run(motor_3,100);//left negative
		motor_run(motor_4,180);//left positive
			TIM_SetCompare1(TIM_SERVO,1550);
	    TIM_SetCompare2(TIM_SERVO,1550);
	    TIM_SetCompare3(TIM_SERVO,1550);
		if(ADC_ConvertedValueLocal[0]>1.0||ADC_ConvertedValueLocal[2]>1.0)
		{
			float sv=100;
			TIM_SetCompare1(TIM_SERVO,1500-sv);
	    TIM_SetCompare2(TIM_SERVO,1500-sv);
	    TIM_SetCompare3(TIM_SERVO,1500-sv);
			delay_ms(2500);

			goto Label_1;
		}
	}
*/
	
	while(1)
	{ 
		//Label_1:
		
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   		ADC_ConvertedValueLocal[0] =(float) ADC_ConvertedValue[0]/4096*3.3;
			ADC_ConvertedValueLocal[1] =(float) ADC_ConvertedValue[1]/4096*3.3;
			ADC_ConvertedValueLocal[2] =(float) ADC_ConvertedValue[2]/4096*3.3;
			ADC_ConvertedValueLocal[3] =(float) ADC_ConvertedValue[3]/4096*3.3;
			ADC_ConvertedValueLocal[4] =(float) ADC_ConvertedValue[4]/4096*3.3;
		
				ADC_ConvertedValueLocal[1]-=0.26;
		
		if(	ADC_ConvertedValueLocal[0]<0.06){ADC_ConvertedValueLocal[0]=0;}//0,06
		if(	ADC_ConvertedValueLocal[1]<0.02){ADC_ConvertedValueLocal[1]=0;}//0.26
		if(	ADC_ConvertedValueLocal[2]<0.018){ADC_ConvertedValueLocal[2]=0;}//0.018
		if(	ADC_ConvertedValueLocal[3]<0.004){ADC_ConvertedValueLocal[3]=0;}//0,004
		if(	ADC_ConvertedValueLocal[4]<0.07){ADC_ConvertedValueLocal[4]=0;}//0.07
		
		
		ADC_ConvertedValueLocal[0]*=100;//                            voltage get 
		ADC_ConvertedValueLocal[1]*=100;
		ADC_ConvertedValueLocal[2]*=100;
		ADC_ConvertedValueLocal[3]*=100;
		ADC_ConvertedValueLocal[4]*=100;
		
		ADC_ConvertedValueLocal[0]/=1.80;
		ADC_ConvertedValueLocal[1]/=1.54;//1.54
		ADC_ConvertedValueLocal[2]/=1.86;
		ADC_ConvertedValueLocal[3]/=1.487;//1.487
		ADC_ConvertedValueLocal[4]/=2.4210;
		//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=
		float SensorDevitation;
		float r_s=85,tl=0;
		if(ADC_ConvertedValueLocal[4]<0.01)      //                          out left        
		{tl=200;}

    float enbig_1=500;
    float pid_P,pid_D,pid_I,pid_center=1550+tl,I_max=30;//                   servo  PID
		float origin_p=1.1,origin_i=0.0001,origin_d=0.001;
    float g_Proportion=origin_p,g_Differential=origin_d,g_Integral=origin_i;
    SensorDevitation = (sqrt(ADC_ConvertedValueLocal[3])-sqrt(ADC_ConvertedValueLocal[1]))/(ADC_ConvertedValueLocal[3]+ADC_ConvertedValueLocal[1]);
    SensorDevitation = SensorDevitation*enbig_1;
		
		
		if(SensorDevitation>50)
    {
			g_Proportion   = origin_p+0.1;
			g_Differential = origin_d;
			g_Integral     = origin_i;
		}
		
		else if(SensorDevitation>20&&SensorDevitation<50)
    {
			g_Proportion   = origin_p;
			g_Differential = origin_d;
			g_Integral     = origin_i;
		}
		
		else if(SensorDevitation<20)
		{
			g_Proportion   = origin_p-0.1;
			g_Differential = origin_d;
			g_Integral     = origin_i;
		}
		
    pid_P = SensorDevitation*g_Proportion;
    pid_D =(SensorDevitation-LastSensorDevitation)*g_Differential;
    pid_I += SensorDevitation * g_Integral;
		LastSensorDevitation = SensorDevitation;
		if(SensorDevitation-LastSensorDevitation>10)
		{
			away=1.0;
		}
		else if(SensorDevitation-LastSensorDevitation>5&&SensorDevitation-LastSensorDevitation<10)
		{
			away=1.0;
		}
		else if(SensorDevitation-LastSensorDevitation<5)
		{
			away=1.0;
		}
		
		if(pid_I > I_max){pid_I = I_max;}
		else if(pid_I < -I_max){pid_I = -I_max;}
    pid_s = pid_center+pid_P+pid_I+pid_D;
		Servo_open();

	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	
		
		if(TIM_GetFlagStatus(TIM6,TIM_IT_Update))
				{
					Encoder_v1=(short)TIM4->CNT;//Get_Encoder1();                basic timer 10ms
					Encoder_a1 += Encoder_v1;
					Encoder_v2=Get_Encoder2();
					Encoder_a2 += Encoder_v2;
						
					Encoder_Restart2();//清0 cnt值
					TIM4->CNT = 0;//Encoder_Restart1();//清0 cnt值
					
					TIM_ClearFlag(TIM6,TIM_IT_Update);
				}
		
		//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=
	
		float rp,lp;
		float conr=0.1818,conl=0.1818,ensmall_m=0.45;//0.45//0.072//0.0675
		float pid_2center=100;
		float orginr,orginl;
				orginr=(pid_2center+SensorDevitation*ensmall_m)*away;
				orginl=(pid_2center-SensorDevitation*ensmall_m)*away;//                   motor PID 
		float mP=1;
		float encoderDevitation_R,encoderDevitation_L;
		encoderDevitation_L=-orginl+Encoder_v2*conl;
		encoderDevitation_R=-orginr+Encoder_v1*conr;
				
				
				
				
	


		rp=orginr-encoderDevitation_L*mP;
		lp=orginl-encoderDevitation_R*mP;

		//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++==
		motor_run(motor_1,(0));//right negative
		motor_run(motor_3,(0));//left negative
		
		//rp=100;lp=100;
		
		if(rp>200){rp=200;}
		if(rp<0){rp=0;}
		motor_run(motor_2,rp);//right positive
		if(lp<0){lp=0;}
		if(lp>200){lp=200;}
		motor_run(motor_4,lp);//left positive                            speed max
		//                                                               motor run
		
		/* sample
		motor_run(motor_1,100);//right negative
		motor_run(motor_2,100);//right positive
		motor_run(motor_3,100);//left negative
		motor_run(motor_4,100);//left positive
		
		*/
		//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		
		if(ADC_ConvertedValueLocal[4]>110&&i==0)
		{
		motor_run(motor_1,100);//right negative
		motor_run(motor_2,180);//right positive
		motor_run(motor_3,100);//left negative
		motor_run(motor_4,180);//left positive
			
			i++;
					TIM_SetCompare1(TIM_SERVO,1500+0.8*r_s);//                  round island
					TIM_SetCompare2(TIM_SERVO,1500+0.8*r_s);
					TIM_SetCompare3(TIM_SERVO,1500+0.8*r_s);
				delay_ms(400);
			
					TIM_SetCompare1(TIM_SERVO,1500+2*r_s);
					TIM_SetCompare2(TIM_SERVO,1500+2*r_s);
					TIM_SetCompare3(TIM_SERVO,1500+2*r_s);
			  delay_ms(800);
			
			}
		//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		
	}
	
}
