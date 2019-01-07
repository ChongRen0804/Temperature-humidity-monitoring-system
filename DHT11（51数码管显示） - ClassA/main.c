#include "dht11.h"
#include <reg52.h>

//sbit LED0=P1^0;
//sbit LED1=P1^1;
//sbit LED2=P1^2;
//sbit LED3=P1^3;
//sbit LED4=P1^4;
//sbit LED5=P1^5;
//sbit LED6=P1^6;
//sbit LED7=P1^7;
//数码管位定义
//sbit DULA = P2^6;
//sbit WELA = P2^7;
//#define SET_DULA() (DULA = 1)
//#define RESET_DULA() (DULA = 0)
//#define SET_WELA() (WELA = 1)
//#define RESET_WELA() (WELA = 0)

sbit LSA=P2^2;
sbit LSB=P2^3;
sbit LSC=P2^4;

sbit led=P2^0;	 //定义P20口是led

sbit motoa=P1^0;	//电机OUTA输出
sbit motob=P1^1;	//电机OUTB输出
sbit k1=P3^0;  		//按键K1（开发板已连接）负责切显示模式
//sbit k3=P3^2;		//按键K3 （开发板已连接）负责控制是否进入掉电模式  //在class A模式下，该口改为接受SETB的中断信号。
sbit k4=P3^3;  		//按键K4（开发板已连接）负责关闭报警器及直流电机
sbit beep=P1^5;	 	//蜂鸣器（开发板已连接）

sbit setA=P1^2;
sbit setB=P3^2;		//连接中断0，



//中断计数标志
int flag=0;

//电机使能
int flag_dianji=0;

//setB=1后允许主函数循环的次数
int main_count=10;

//上传数据计数标志
int flag_lora=0;

//蜂鸣器是否响
int flag_beep=0;

//蜂鸣器使能
int flag_beep2=0;

//有效下行命令标志
int flag_en=0;

//掉电标志
int flag_diaodian=0;

//下行命令选项
int command=-1;

//温度报警时上传次数,默认1次
int flag_upcount1=1;

//湿度报警时上传次数，
int flag_upcount2=1;

int flag_LCD=0;

//显示模式按下k1切换显示模式，默认0为LCD显示，1为数码管显示
int display_mode=0;

//最低温度
int temp_min=10;

//最高温度
int temp_max=23;

//最小湿度
int humi_min=30;

//最大湿度
int humi_max=90;

//接收数据帧长度
int recv_len=0;

//上报周期，默认为30s,s=up_period/100,便于测试
int up_period=3000;

//上报周期值副本
int up_periodini=3000;


//接收数据帧数组
idata unsigned short receiveData[10]=0;

//数码管编码
idata unsigned char tab[4];

//unsigned char seg_we[]={0xfe,0xfd,0xfb,0xf7};
code unsigned char seg_we[]={0xfe,0xfd,0xfb,0xf7};

//unsigned char seg_du[]={0xa0,0xbb,0x62,0x2a,0x39,0x2c,0x24,0xba,0x20,0x28};
code unsigned char seg_du[]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,
					0x7f,0x6f};

extern int temp_value, humi_value; //温湿度数值（是实际值的10倍）

static void InitTime(void);		//设定定时器初始化

void Delay_1ms(unsigned int ms);   //不精确延迟1ms

int DecToHex(int dec);	 //十进制转同形式十六进制（如20->0x20）

int HexToDec(unsigned char hex);	//十六进制转同形式十进制（如0x20->20）

unsigned short CRC_GetModbus16(unsigned char * pData, int len);		//返回CRC校验码，pData为待校验的数据（如01FF，2，返回6040）
//static void SMG_Display(unsigned int value);  

code char Disp1[]=" Join  Success! ";
code char Disp2[]="  Hello World!  ";


/**********************************
LCD函数声明
**********************************/
/*在51单片机12MHZ时钟下的延时函数*/
extern void Lcd1602_Delay1ms(int c);   //误差 0us
/*LCD1602写入8位命令子函数*/
extern void LcdWriteCom(char com);
/*LCD1602写入8位数据子函数*/	
extern void LcdWriteData(char dat);
/*LCD1602初始化子程序*/		
extern void LcdInit();	

void delay(int i)
{
	while(i--);	
}


void display(unsigned int value)					 //数码管显示温度函数
{
	unsigned char i;
	tab[0]=value/1000;
	tab[1]=value%1000/100;
	tab[2]=value%1000%100/10;
	tab[3]=value%10;

	for(i=0;i<4;i++)
	{
		switch(i)	 //位选，选择点亮的数码管，
		{
			case(0):
				LSA=0;LSB=0;LSC=0; break;//显示第0位
			case(1):
				LSA=1;LSB=0;LSC=0; break;//显示第1位
			case(2):
				LSA=0;LSB=1;LSC=0; break;//显示第2位
			case(3):
				LSA=1;LSB=1;LSC=0; break;//显示第3位
			case(4):
				LSA=0;LSB=0;LSC=1; break;//显示第4位
			case(5):
				LSA=1;LSB=0;LSC=1; break;//显示第5位
			case(6):
				LSA=0;LSB=1;LSC=1; break;//显示第6位
			case(7):
				LSA=1;LSB=1;LSC=1; break;//显示第7位	
		}
		//P2=seg_we[i];
		P0=seg_du[tab[i]];
		Delay_1ms(1);	
	}
}

void display2(unsigned int value)					 //数码管显示湿度函数
{
	unsigned char i;
	tab[0]=value/1000;
	tab[1]=value%1000/100;
	tab[2]=value%1000%100/10;
	tab[3]=value%10;

	for(i=4;i<8;i++)
	{
		switch(i)	 //位选，选择点亮的数码管，
		{
			case(0):
				LSA=0;LSB=0;LSC=0; break;//显示第0位
			case(1):
				LSA=1;LSB=0;LSC=0; break;//显示第1位
			case(2):
				LSA=0;LSB=1;LSC=0; break;//显示第2位
			case(3):
				LSA=1;LSB=1;LSC=0; break;//显示第3位
			case(4):
				LSA=0;LSB=0;LSC=1; break;//显示第4位
			case(5):
				LSA=1;LSB=0;LSC=1; break;//显示第5位
			case(6):
				LSA=0;LSB=1;LSC=1; break;//显示第6位
			case(7):
				LSA=1;LSB=1;LSC=1; break;//显示第7位	
		}
		//P2=seg_we[i];
		P0=seg_du[tab[i-4]];
		Delay_1ms(1);	
	}
}


void display_LCD()
{
	
/*	int i=0;
	char Disp2[]="  Hello World!  ";
	LcdWriteCom(0x01);  //清屏
	for(i=0;i<16;i++)
	{
		LcdWriteData(Disp2[i]);	
	}
*/
	LcdWriteCom(0x01);  //清屏
	LcdWriteData('T');
	LcdWriteData('E');
	LcdWriteData('M');
	LcdWriteData(':');
	LcdWriteData(temp_value/100+48);
	LcdWriteData((temp_value/10)%10+48);
	LcdWriteData(' ');
	LcdWriteData('R');
	LcdWriteData('A');
	LcdWriteData('N');
	LcdWriteData(':');
	LcdWriteData(temp_min/10+48);
	LcdWriteData(temp_min%10+48);
	LcdWriteData('-');
	LcdWriteData(temp_max/10+48);
	LcdWriteData(temp_max%10+48);
	LcdWriteCom(0x80+0x40);
	LcdWriteData('H');
	LcdWriteData('U');
	LcdWriteData('M');
	LcdWriteData(':');
	LcdWriteData(humi_value/100+48);
	LcdWriteData((humi_value/10)%10+48);
	LcdWriteData(' ');
	LcdWriteData('R');
	LcdWriteData('A');
	LcdWriteData('N');
	LcdWriteData(':');
	LcdWriteData(humi_min/10+48);
	LcdWriteData(humi_min%10+48);
	LcdWriteData('-');
	LcdWriteData(humi_max/10+48);
	LcdWriteData(humi_max%10+48);

}

void display_LCDJOINS()
{
	int i=0;

	LcdWriteCom(0x01);  //清屏
	for(i=0;i<16;i++)
	{
		LcdWriteData(Disp1[i]);	
	}
	LcdWriteCom(0x80+0x40);
	for(i=0;i<16;i++)
	{
		LcdWriteData(Disp2[i]);	
	}
	Lcd1602_Delay1ms(5000);//延迟5s
}

void UsartInit()
{
//	SCON=0X50;			//设置为工作方式1
//	TMOD=0X20;			//设置计数器工作方式2
//	PCON=0X80;			//波特率加倍
//	TH1=0XFA;				//计数器初始值设置，注意波特率是9600的
//	TL1=0XFA;
//	ES=1;						//打开接收中断
//	EA=1;						//打开总中断
//	TR1=1;					//打开计数器

	SCON=0x50;
	T2CON=0x34;
//	T2MOD=0x00;
//	TH2=0xfd;
//	TL2=0xfd;
	RCAP2H=0xFF;		   //波特率设置为38400
	RCAP2L=0xF7;
	EA=1;
	ES=1;
	PCON=0;
} 	 

void sendlora()
{

//	setA=0;
//	
////	Delay_1ms(5);
//
//	SBUF=0xFA; 
//	while(!TI);//等特数据传送
//	TI=0;//清除数据传送标志
////	Delay_1ms(500);
//
//	SBUF=0x01; 
//	while(!TI);//等特数据传送
//	TI=0;//清除数据传送标志
////	Delay_1ms(500);
//
//	SBUF=0xFF; 
//	while(!TI);//等特数据传送
//	TI=0;//清除数据传送标志
////	Delay_1ms(500);
//
//	SBUF=0x60; 
//	while(!TI);//等特数据传送
//	TI=0;//清除数据传送标志
////	Delay_1ms(500);
//
//	SBUF=0x40; 
//	while(!TI);//等特数据传送
//	TI=0;//清除数据传送标志
////	Delay_1ms(500);
//
//	SBUF=0xF5; 
//	while(!TI);//等特数据传送
//	TI=0;//清除数据传送标志
////	Delay_1ms(500);
//
//	setA=1;



//
//	SBUF=0xFA; 
//	while(!TI);//等特数据传送
//	TI=0;//清除数据传送标志
////	Delay_1ms(500);
//
//	SBUF=0x05; 
//	while(!TI);//等特数据传送
//	TI=0;//清除数据传送标志
////	Delay_1ms(500);
//
//	SBUF=0x01; 
//	while(!TI);//等特数据传送
//	TI=0;//清除数据传送标志
////	Delay_1ms(500);
//
//	SBUF=0x02; 
//	while(!TI);//等特数据传送
//	TI=0;//清除数据传送标志
////	Delay_1ms(500);
//
//	SBUF=0x03; 
//	while(!TI);//等特数据传送
//	TI=0;//清除数据传送标志
////	Delay_1ms(500);
//
//	SBUF=0x04; 
//	while(!TI);//等特数据传送
//	TI=0;//清除数据传送标志
////	Delay_1ms(500);
//
//	SBUF=0x05; 
//	while(!TI);//等特数据传送
//	TI=0;//清除数据传送标志
////	Delay_1ms(500);
//
//	SBUF=0xF5; 
//	while(!TI);//等特数据传送
//	TI=0;//清除数据传送标志
////	Delay_1ms(500);
//
//	SBUF=0x0E; 
//	while(!TI);//等特数据传送
//	TI=0;//清除数据传送标志
////	Delay_1ms(500);
//
//	SBUF=0xF5; 
//	while(!TI);//等特数据传送
//	TI=0;//清除数据传送标志
////	Delay_1ms(500);



  	unsigned char a[3];
    unsigned short b;
	unsigned short b1;
	unsigned short b2;
	a[0]=0x02;
    a[1]=DecToHex(temp_value/10);
    a[2]=DecToHex(humi_value/10);
	b=CRC_GetModbus16(a,3);
	b1=(b&0xFF00)>>8;
	b2=(b&0x00FF);

	setA=0;
	Delay_1ms(5);

	SBUF=0xFA; 
	while(!TI);//等特数据传送
	TI=0;//清除数据传送标志
//	Delay_1ms(500);

	SBUF=0x02; 
	while(!TI);//等特数据传送
	TI=0;//清除数据传送标志
//	Delay_1ms(500);

	SBUF=DecToHex(temp_value/10); 
	while(!TI);//等特数据传送
	TI=0;//清除数据传送标志
//	Delay_1ms(500);

	SBUF=DecToHex(humi_value/10); 
	while(!TI);//等特数据传送
	TI=0;//清除数据传送标志
//	Delay_1ms(500);

	SBUF=b1; 
	while(!TI);//等特数据传送
	TI=0;//清除数据传送标志
//	Delay_1ms(500);

	SBUF=b2; 
	while(!TI);//等特数据传送
	TI=0;//清除数据传送标志
//	Delay_1ms(500);


	SBUF=0xF5; 
	while(!TI);//等特数据传送
	TI=0;//清除数据传送标志
//	Delay_1ms(500);

	setA=1;

}

void sendlimit()
{

	unsigned char a[5];
    unsigned short b;
	unsigned short b1;
	unsigned short b2;
	a[0]=0x04;
    a[1]=DecToHex(temp_min);
	a[2]=DecToHex(temp_max);
    a[3]=DecToHex(humi_min); 
	a[4]=DecToHex(humi_max);
	b=CRC_GetModbus16(a,5);
	b1=(b&0xFF00)>>8;				 //取出高低位
	b2=(b&0x00FF);

	setA=0;
	Delay_1ms(5);

	SBUF=0xFA; 
	while(!TI);//等特数据传送
	TI=0;//清除数据传送标志

	SBUF=0x04; 
	while(!TI);//等特数据传送
	TI=0;//清除数据传送标志

	SBUF=DecToHex(temp_min); 
	while(!TI);//等特数据传送
	TI=0;//清除数据传送标志

	SBUF=DecToHex(temp_max); 
	while(!TI);//等特数据传送
	TI=0;//清除数据传送标志

	SBUF=DecToHex(humi_min); 
	while(!TI);//等特数据传送
	TI=0;//清除数据传送标志

	SBUF=DecToHex(humi_max); 
	while(!TI);//等特数据传送
	TI=0;//清除数据传送标志

	SBUF=b1; 
	while(!TI);//等特数据传送
	TI=0;//清除数据传送标志

	SBUF=b2; 
	while(!TI);//等特数据传送
	TI=0;//清除数据传送标志

	SBUF=0xF5; 
	while(!TI);//等特数据传送
	TI=0;//清除数据传送标志

	setA=1;
}

void sendperiod()
{
 	unsigned char a[2];
    unsigned short b;
	unsigned short b1;
	unsigned short b2;
	a[0]=0x01;
    a[1]=DecToHex(up_period/100);
	b=CRC_GetModbus16(a,2);
	b1=(b&0xFF00)>>8;				 //取出高低位
	b2=(b&0x00FF);

	setA=0;
	Delay_1ms(5);

	SBUF=0xFA; 
	while(!TI);//等特数据传送
	TI=0;//清除数据传送标志

	SBUF=0x01; 
	while(!TI);//等特数据传送
	TI=0;//清除数据传送标志

	SBUF=DecToHex(up_period/100); 
	while(!TI);//等特数据传送
	TI=0;//清除数据传送标志

	SBUF=b1; 
	while(!TI);//等特数据传送
	TI=0;//清除数据传送标志

	SBUF=b2; 
	while(!TI);//等特数据传送
	TI=0;//清除数据传送标志

	SBUF=0xF5; 
	while(!TI);//等特数据传送
	TI=0;//清除数据传送标志	

	setA=1;
}



//void PutString(unsigned char *TXStr)    
//{                  
//    ES=0;       
//     while(*TXStr!=0)   
//    {                        
//        SBUF=*TXStr;  
//        while(TI==0);  
//        TI=0;      
//        TXStr++;  
//    }  
//    ES=1;   
//}


void Int0Init()
{
	//设置INT0
	IT0=1;//跳变沿出发方式（下降沿）
	EX0=1;//打开INT0的中断允许。	
	EA=1;//打开总中断	
}


void Usart() interrupt 4			 //接收中断
{

	if(RI==1)
	{
		int i=0;
		unsigned short temp=SBUF;
		RI = 0;//清除接收中断标志位
	/*	if((temp==0x02||temp==0x03)&&recv_len==1)
		{
			flag_en=0;
			recv_len=0;
		}
	*/
		if(temp==0xFB&&recv_len==0&&flag_en==0)
		{
			flag_en=1;
			receiveData[recv_len]=temp;
			recv_len++;
		}
		else if(temp==0xF5&&recv_len>=5&&flag_en==1)
		{
			receiveData[recv_len]=temp;
			for(i=0;i<=recv_len;i++)		 //输出收到的数据帧（测试用）
			{
				SBUF=receiveData[i]; 
				while(!TI);//等特数据传送
				TI=0;//清除数据传送标志
			}
			if(receiveData[1]==0xFB&&receiveData[2]==0x03&&receiveData[3]==0xB1&&receiveData[4]==0x02)	//特殊命令0：表示入网成功
			{
				command=0;
			}
			if(receiveData[0]==0xFB&&receiveData[1]==0x01&&receiveData[2]==0xAA) 	//命令1：要求返回当前阈值至网站
			{
				//sendlimit();
				//sendlora();
				command=1;
			}
			if(receiveData[0]==0xFB&&receiveData[1]==0x05&&receiveData[2]==0xAA)	//命令2：下行设置温湿度阈值
			{
				command=2;
			}
			if(receiveData[0]==0xFB&&receiveData[1]==0x01&&receiveData[2]==0xBB)	//命令3：控制电机关闭
			{
				command=3;
			}
			if(receiveData[0]==0xFB&&receiveData[1]==0x01&&receiveData[2]==0xCC)	 //命令4：控制蜂鸣器关闭
			{
				command=4;
			}
			if(receiveData[0]==0xFB&&receiveData[1]==0x01&&receiveData[2]==0xDD)	 //命令5：设置数据上传周期
			{
				command=5;
			}
			if(receiveData[0]==0xFB&&receiveData[1]==0x02&&receiveData[2]==0xDD)	 //命令6：设置数据上传周期
			{
				command=6;
			}
			recv_len=0;
			flag_en=0;
	
		}
		else if(flag_en==1)
		{
		 	receiveData[recv_len]=temp;
			recv_len++;
		}
	
		//RI = 0;


	//	unsigned short receiveData;
	//
	//	receiveData=SBUF;//出去接收到的数据
	//	RI = 0;//清除接收中断标志位
	//	SBUF=receiveData;//将接收到的数据放入到发送寄存器
	//	while(!TI);			 //等待发送数据完成
	//	TI=0;						 //清除发送完成标志位
	}
	else
	{
	  //TI = 0; 
	}
	

}




int main(void)
{
	//先等上电稳定
	int mode=0;
	Delay_1ms(1000);
	//因为读一次数据dht11 才会触发一次采集数据.
	//即在先使用数据时采集一次数据
	ReadTempAndHumi();
	//因为在两次采集数据需一定的时间间隔,这里还可减少
	Delay_1ms(3000);
	//设定定时器
	InitTime();	  //定时器初始化
	UsartInit();  //串口初始化
	LcdInit(); 	  //LCD初始化
	Int0Init();  //	设置外部中断0
	
	setA=1;
	setB=1;

	while(1)
	{
		
		if(flag == 100)	  //一秒读一次温湿度
		{

			flag = 0;
			/*
			//读温湿度,可检测函数调用是否失败,
			//函数返回OK(1)表示成功,返回ERROR(0)表示失败
			//OK和ERROR是在DHT11.H中定义的宏
			*/
			ReadTempAndHumi();
		}
	/*	if((temp_value>(temp_max*10))&&(flag_dianji==0))	  //温度大于temp_max度开始直流电机工作开始降温
		{
			//motoa=0;
		//	motob=1;
			if(flag_upcount1==1)
			{
				sendlora();		  //发送一次温湿度数据
				flag_upcount1=0;
			}  
			//up_period=500; //上传周期改为5s
		}
		else
		{
		 	motoa=0;
			motob=0;
		//	flag_dianji=0;		  //电机使能
			flag_upcount1=1;	  //恢复计数
		//	up_period=3000; //上传周期恢复为30s			//此处与设置周期冲突
		}	   *


		if((humi_value>(humi_max*10))&&(flag_beep2==0))	  //湿度大于90度蜂鸣器报警
		{
			flag_beep=1;
			if(flag_upcount2==1)
			{
				sendlora();		  //发送一次温湿度数据
				flag_upcount2=0;
			}
		//	up_period=500; //上传周期改为5s				//此处与设置周期冲突
		}
		else
		{
		 	flag_beep=0;
			flag_upcount2=1;
		//	up_period=3000; //上传周期恢复为30s
		} */

	/*	if(k1==0)		//判断K1是否按下，K1作用是切换显示模式
		{
			delay(1000);	//消抖
			if(k1==0)
			{	
				while(!k1);
				if(display_mode)
					display_mode=0;		//显示模式0,LCD详细显示
				else
					display_mode=1;		//显示模式1,数码管简略显示
			}
		}	


		if(k4==0)		//判断K4是否按下，K4作用是手动关闭电机蜂鸣器，主要是测试用
		{
			delay(1000);	//消抖
			if(k4==0)
			{	
				while(!k4);
				motoa=0;
				motob=0;
				flag_beep2=1; //永久关闭蜂鸣器
				flag_dianji=1;//永久关闭电机
			}
		}	 */
	

		if(display_mode==1)
		{
			//数码管显示温度
			display(temp_value);
			//数码管显示湿度
			display2(humi_value);
		}
		if(flag_LCD==100&&display_mode==0)
		{
			flag_LCD=0;
			display_LCD();
		}

		if(flag_lora>=up_period)  //up_period/100秒上传一次温湿度		  	//防止出现up_period突然变化的情况
		{
			flag_lora=0;
			sendlora();		  //发送温湿度数据
		}
		

//		   	if(command==1)
//			{
//				sendlimit();
//				command=0;
//			}
//
//			if(command==2)
//			{
//				temp_min=HexToDec(receiveData[3]);
//				temp_max=HexToDec(receiveData[4]);
//				humi_min=HexToDec(receiveData[5]);
//				humi_max=HexToDec(receiveData[6]);
//				command=0;
//				//int i=0;
//			}

		 switch(command)
		{
			case 0:
				display_LCDJOINS();
				command=-1;
           		break;
			case 1:
				Delay_1ms(100); //发送上传过快导致与下行冲突	    //关键词：半双工
				sendlimit();
				command=-1;
         	  	break;
			case 2:
				temp_min=HexToDec(receiveData[3]);
				temp_max=HexToDec(receiveData[4]);
				humi_min=HexToDec(receiveData[5]);
				humi_max=HexToDec(receiveData[6]);
				command=-1;
          		break;
			case 3:
				motoa=0;
				motob=0;
				flag_dianji=1;//永久关闭电机
				command=-1;
         	  	break;
			case 4:
				flag_beep2=1; //永久关闭蜂鸣器
				command=-1;
         	  	break;
			case 5:
				Delay_1ms(100); //发送上传过快导致与下行冲突		 //关键词：半双工
				sendperiod();	//发送周期
				command=-1;
         	  	break;
			case 6:
				up_period=HexToDec(receiveData[3])*100;//设置上传周期，需要乘100
				flag_lora=0;		//重新开始计数
				command=-1;
         	  	break;
    		default:
          		break;
		}
		if(flag_diaodian)
		{
			LcdWriteCom(0x08);  //关显示不显示光标
			//PCON |= (1<<1);        //掉电模式
			PCON=0X01;  //空闲模式
		}
		else
		{
			LcdWriteCom(0x0c);  //开显示不显示光标
			PCON=0x00;            //掉电恢复
		}
		if(setB==1)
			flag_diaodian=1;
			
		
	
			
	}
	return 0;

	
}

//设定定时器初始化,定时10ms
static void InitTime(void)
{
//	TH0 = (65536-50000);    		//(unsigned char)((65535U - 50000) >> 8);
//	TL0 = (65536-50000);    		//(unsigned char)(65535U - 50000) ;
	TH0 =0XDC;	
	TL0 =0X00;
	TMOD = 0X01;
	TR0 = 1;
	ET0 = 1;
	EA = 1;
}

void timer(void) interrupt 1		//精确延时10ms
{
//	TH0 =(65535 - 50000);	// (unsigned char)((65535 - 50000) >> 8);
//	TL0 =(65535 - 50000);	// (unsigned char)(65535 - 50000);
	TH0 =0XDC;	
	TL0 =0X00;
	if(flag<100)		   //防止超过100造成永远无法更新温湿度
	{
		flag++;
	}
	if(flag_lora<up_period)	   //防止超过up_period次造成不能永远发送新数据
	{
		flag_lora++;
	}
	if(flag_beep==1)
		beep=~beep;
	if(flag_LCD<100)		   //防止超过100造成永远无法更新温湿度
	{
		flag_LCD++;
	}
}

void Int0()	interrupt 0		//外部中断0的中断函数
{	
	if(setB==0)
	{
		delay(1000);	 //延时消抖
		if(setB==0)
		{
			led=~led;			 //led1掉电指示灯,掉电亮
			flag_diaodian=0;
		}
	}
}
