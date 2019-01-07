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
//�����λ����
//sbit DULA = P2^6;
//sbit WELA = P2^7;
//#define SET_DULA() (DULA = 1)
//#define RESET_DULA() (DULA = 0)
//#define SET_WELA() (WELA = 1)
//#define RESET_WELA() (WELA = 0)

sbit LSA=P2^2;
sbit LSB=P2^3;
sbit LSC=P2^4;

sbit led=P2^0;	 //����P20����led

sbit motoa=P1^0;	//���OUTA���
sbit motob=P1^1;	//���OUTB���
sbit k1=P3^0;  		//����K1�������������ӣ���������ʾģʽ
//sbit k3=P3^2;		//����K3 �������������ӣ���������Ƿ�������ģʽ  //��class Aģʽ�£��ÿڸ�Ϊ����SETB���ж��źš�
sbit k4=P3^3;  		//����K4�������������ӣ�����رձ�������ֱ�����
sbit beep=P1^5;	 	//�������������������ӣ�

sbit setA=P1^2;
sbit setB=P3^2;		//�����ж�0��



//�жϼ�����־
int flag=0;

//���ʹ��
int flag_dianji=0;

//setB=1������������ѭ���Ĵ���
int main_count=10;

//�ϴ����ݼ�����־
int flag_lora=0;

//�������Ƿ���
int flag_beep=0;

//������ʹ��
int flag_beep2=0;

//��Ч���������־
int flag_en=0;

//�����־
int flag_diaodian=0;

//��������ѡ��
int command=-1;

//�¶ȱ���ʱ�ϴ�����,Ĭ��1��
int flag_upcount1=1;

//ʪ�ȱ���ʱ�ϴ�������
int flag_upcount2=1;

int flag_LCD=0;

//��ʾģʽ����k1�л���ʾģʽ��Ĭ��0ΪLCD��ʾ��1Ϊ�������ʾ
int display_mode=0;

//����¶�
int temp_min=10;

//����¶�
int temp_max=23;

//��Сʪ��
int humi_min=30;

//���ʪ��
int humi_max=90;

//��������֡����
int recv_len=0;

//�ϱ����ڣ�Ĭ��Ϊ30s,s=up_period/100,���ڲ���
int up_period=3000;

//�ϱ�����ֵ����
int up_periodini=3000;


//��������֡����
idata unsigned short receiveData[10]=0;

//����ܱ���
idata unsigned char tab[4];

//unsigned char seg_we[]={0xfe,0xfd,0xfb,0xf7};
code unsigned char seg_we[]={0xfe,0xfd,0xfb,0xf7};

//unsigned char seg_du[]={0xa0,0xbb,0x62,0x2a,0x39,0x2c,0x24,0xba,0x20,0x28};
code unsigned char seg_du[]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,
					0x7f,0x6f};

extern int temp_value, humi_value; //��ʪ����ֵ����ʵ��ֵ��10����

static void InitTime(void);		//�趨��ʱ����ʼ��

void Delay_1ms(unsigned int ms);   //����ȷ�ӳ�1ms

int DecToHex(int dec);	 //ʮ����תͬ��ʽʮ�����ƣ���20->0x20��

int HexToDec(unsigned char hex);	//ʮ������תͬ��ʽʮ���ƣ���0x20->20��

unsigned short CRC_GetModbus16(unsigned char * pData, int len);		//����CRCУ���룬pDataΪ��У������ݣ���01FF��2������6040��
//static void SMG_Display(unsigned int value);  

code char Disp1[]=" Join  Success! ";
code char Disp2[]="  Hello World!  ";


/**********************************
LCD��������
**********************************/
/*��51��Ƭ��12MHZʱ���µ���ʱ����*/
extern void Lcd1602_Delay1ms(int c);   //��� 0us
/*LCD1602д��8λ�����Ӻ���*/
extern void LcdWriteCom(char com);
/*LCD1602д��8λ�����Ӻ���*/	
extern void LcdWriteData(char dat);
/*LCD1602��ʼ���ӳ���*/		
extern void LcdInit();	

void delay(int i)
{
	while(i--);	
}


void display(unsigned int value)					 //�������ʾ�¶Ⱥ���
{
	unsigned char i;
	tab[0]=value/1000;
	tab[1]=value%1000/100;
	tab[2]=value%1000%100/10;
	tab[3]=value%10;

	for(i=0;i<4;i++)
	{
		switch(i)	 //λѡ��ѡ�����������ܣ�
		{
			case(0):
				LSA=0;LSB=0;LSC=0; break;//��ʾ��0λ
			case(1):
				LSA=1;LSB=0;LSC=0; break;//��ʾ��1λ
			case(2):
				LSA=0;LSB=1;LSC=0; break;//��ʾ��2λ
			case(3):
				LSA=1;LSB=1;LSC=0; break;//��ʾ��3λ
			case(4):
				LSA=0;LSB=0;LSC=1; break;//��ʾ��4λ
			case(5):
				LSA=1;LSB=0;LSC=1; break;//��ʾ��5λ
			case(6):
				LSA=0;LSB=1;LSC=1; break;//��ʾ��6λ
			case(7):
				LSA=1;LSB=1;LSC=1; break;//��ʾ��7λ	
		}
		//P2=seg_we[i];
		P0=seg_du[tab[i]];
		Delay_1ms(1);	
	}
}

void display2(unsigned int value)					 //�������ʾʪ�Ⱥ���
{
	unsigned char i;
	tab[0]=value/1000;
	tab[1]=value%1000/100;
	tab[2]=value%1000%100/10;
	tab[3]=value%10;

	for(i=4;i<8;i++)
	{
		switch(i)	 //λѡ��ѡ�����������ܣ�
		{
			case(0):
				LSA=0;LSB=0;LSC=0; break;//��ʾ��0λ
			case(1):
				LSA=1;LSB=0;LSC=0; break;//��ʾ��1λ
			case(2):
				LSA=0;LSB=1;LSC=0; break;//��ʾ��2λ
			case(3):
				LSA=1;LSB=1;LSC=0; break;//��ʾ��3λ
			case(4):
				LSA=0;LSB=0;LSC=1; break;//��ʾ��4λ
			case(5):
				LSA=1;LSB=0;LSC=1; break;//��ʾ��5λ
			case(6):
				LSA=0;LSB=1;LSC=1; break;//��ʾ��6λ
			case(7):
				LSA=1;LSB=1;LSC=1; break;//��ʾ��7λ	
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
	LcdWriteCom(0x01);  //����
	for(i=0;i<16;i++)
	{
		LcdWriteData(Disp2[i]);	
	}
*/
	LcdWriteCom(0x01);  //����
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

	LcdWriteCom(0x01);  //����
	for(i=0;i<16;i++)
	{
		LcdWriteData(Disp1[i]);	
	}
	LcdWriteCom(0x80+0x40);
	for(i=0;i<16;i++)
	{
		LcdWriteData(Disp2[i]);	
	}
	Lcd1602_Delay1ms(5000);//�ӳ�5s
}

void UsartInit()
{
//	SCON=0X50;			//����Ϊ������ʽ1
//	TMOD=0X20;			//���ü�����������ʽ2
//	PCON=0X80;			//�����ʼӱ�
//	TH1=0XFA;				//��������ʼֵ���ã�ע�Ⲩ������9600��
//	TL1=0XFA;
//	ES=1;						//�򿪽����ж�
//	EA=1;						//�����ж�
//	TR1=1;					//�򿪼�����

	SCON=0x50;
	T2CON=0x34;
//	T2MOD=0x00;
//	TH2=0xfd;
//	TL2=0xfd;
	RCAP2H=0xFF;		   //����������Ϊ38400
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
//	while(!TI);//�������ݴ���
//	TI=0;//������ݴ��ͱ�־
////	Delay_1ms(500);
//
//	SBUF=0x01; 
//	while(!TI);//�������ݴ���
//	TI=0;//������ݴ��ͱ�־
////	Delay_1ms(500);
//
//	SBUF=0xFF; 
//	while(!TI);//�������ݴ���
//	TI=0;//������ݴ��ͱ�־
////	Delay_1ms(500);
//
//	SBUF=0x60; 
//	while(!TI);//�������ݴ���
//	TI=0;//������ݴ��ͱ�־
////	Delay_1ms(500);
//
//	SBUF=0x40; 
//	while(!TI);//�������ݴ���
//	TI=0;//������ݴ��ͱ�־
////	Delay_1ms(500);
//
//	SBUF=0xF5; 
//	while(!TI);//�������ݴ���
//	TI=0;//������ݴ��ͱ�־
////	Delay_1ms(500);
//
//	setA=1;



//
//	SBUF=0xFA; 
//	while(!TI);//�������ݴ���
//	TI=0;//������ݴ��ͱ�־
////	Delay_1ms(500);
//
//	SBUF=0x05; 
//	while(!TI);//�������ݴ���
//	TI=0;//������ݴ��ͱ�־
////	Delay_1ms(500);
//
//	SBUF=0x01; 
//	while(!TI);//�������ݴ���
//	TI=0;//������ݴ��ͱ�־
////	Delay_1ms(500);
//
//	SBUF=0x02; 
//	while(!TI);//�������ݴ���
//	TI=0;//������ݴ��ͱ�־
////	Delay_1ms(500);
//
//	SBUF=0x03; 
//	while(!TI);//�������ݴ���
//	TI=0;//������ݴ��ͱ�־
////	Delay_1ms(500);
//
//	SBUF=0x04; 
//	while(!TI);//�������ݴ���
//	TI=0;//������ݴ��ͱ�־
////	Delay_1ms(500);
//
//	SBUF=0x05; 
//	while(!TI);//�������ݴ���
//	TI=0;//������ݴ��ͱ�־
////	Delay_1ms(500);
//
//	SBUF=0xF5; 
//	while(!TI);//�������ݴ���
//	TI=0;//������ݴ��ͱ�־
////	Delay_1ms(500);
//
//	SBUF=0x0E; 
//	while(!TI);//�������ݴ���
//	TI=0;//������ݴ��ͱ�־
////	Delay_1ms(500);
//
//	SBUF=0xF5; 
//	while(!TI);//�������ݴ���
//	TI=0;//������ݴ��ͱ�־
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
	while(!TI);//�������ݴ���
	TI=0;//������ݴ��ͱ�־
//	Delay_1ms(500);

	SBUF=0x02; 
	while(!TI);//�������ݴ���
	TI=0;//������ݴ��ͱ�־
//	Delay_1ms(500);

	SBUF=DecToHex(temp_value/10); 
	while(!TI);//�������ݴ���
	TI=0;//������ݴ��ͱ�־
//	Delay_1ms(500);

	SBUF=DecToHex(humi_value/10); 
	while(!TI);//�������ݴ���
	TI=0;//������ݴ��ͱ�־
//	Delay_1ms(500);

	SBUF=b1; 
	while(!TI);//�������ݴ���
	TI=0;//������ݴ��ͱ�־
//	Delay_1ms(500);

	SBUF=b2; 
	while(!TI);//�������ݴ���
	TI=0;//������ݴ��ͱ�־
//	Delay_1ms(500);


	SBUF=0xF5; 
	while(!TI);//�������ݴ���
	TI=0;//������ݴ��ͱ�־
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
	b1=(b&0xFF00)>>8;				 //ȡ���ߵ�λ
	b2=(b&0x00FF);

	setA=0;
	Delay_1ms(5);

	SBUF=0xFA; 
	while(!TI);//�������ݴ���
	TI=0;//������ݴ��ͱ�־

	SBUF=0x04; 
	while(!TI);//�������ݴ���
	TI=0;//������ݴ��ͱ�־

	SBUF=DecToHex(temp_min); 
	while(!TI);//�������ݴ���
	TI=0;//������ݴ��ͱ�־

	SBUF=DecToHex(temp_max); 
	while(!TI);//�������ݴ���
	TI=0;//������ݴ��ͱ�־

	SBUF=DecToHex(humi_min); 
	while(!TI);//�������ݴ���
	TI=0;//������ݴ��ͱ�־

	SBUF=DecToHex(humi_max); 
	while(!TI);//�������ݴ���
	TI=0;//������ݴ��ͱ�־

	SBUF=b1; 
	while(!TI);//�������ݴ���
	TI=0;//������ݴ��ͱ�־

	SBUF=b2; 
	while(!TI);//�������ݴ���
	TI=0;//������ݴ��ͱ�־

	SBUF=0xF5; 
	while(!TI);//�������ݴ���
	TI=0;//������ݴ��ͱ�־

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
	b1=(b&0xFF00)>>8;				 //ȡ���ߵ�λ
	b2=(b&0x00FF);

	setA=0;
	Delay_1ms(5);

	SBUF=0xFA; 
	while(!TI);//�������ݴ���
	TI=0;//������ݴ��ͱ�־

	SBUF=0x01; 
	while(!TI);//�������ݴ���
	TI=0;//������ݴ��ͱ�־

	SBUF=DecToHex(up_period/100); 
	while(!TI);//�������ݴ���
	TI=0;//������ݴ��ͱ�־

	SBUF=b1; 
	while(!TI);//�������ݴ���
	TI=0;//������ݴ��ͱ�־

	SBUF=b2; 
	while(!TI);//�������ݴ���
	TI=0;//������ݴ��ͱ�־

	SBUF=0xF5; 
	while(!TI);//�������ݴ���
	TI=0;//������ݴ��ͱ�־	

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
	//����INT0
	IT0=1;//�����س�����ʽ���½��أ�
	EX0=1;//��INT0���ж�����	
	EA=1;//�����ж�	
}


void Usart() interrupt 4			 //�����ж�
{

	if(RI==1)
	{
		int i=0;
		unsigned short temp=SBUF;
		RI = 0;//��������жϱ�־λ
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
			for(i=0;i<=recv_len;i++)		 //����յ�������֡�������ã�
			{
				SBUF=receiveData[i]; 
				while(!TI);//�������ݴ���
				TI=0;//������ݴ��ͱ�־
			}
			if(receiveData[1]==0xFB&&receiveData[2]==0x03&&receiveData[3]==0xB1&&receiveData[4]==0x02)	//��������0����ʾ�����ɹ�
			{
				command=0;
			}
			if(receiveData[0]==0xFB&&receiveData[1]==0x01&&receiveData[2]==0xAA) 	//����1��Ҫ�󷵻ص�ǰ��ֵ����վ
			{
				//sendlimit();
				//sendlora();
				command=1;
			}
			if(receiveData[0]==0xFB&&receiveData[1]==0x05&&receiveData[2]==0xAA)	//����2������������ʪ����ֵ
			{
				command=2;
			}
			if(receiveData[0]==0xFB&&receiveData[1]==0x01&&receiveData[2]==0xBB)	//����3�����Ƶ���ر�
			{
				command=3;
			}
			if(receiveData[0]==0xFB&&receiveData[1]==0x01&&receiveData[2]==0xCC)	 //����4�����Ʒ������ر�
			{
				command=4;
			}
			if(receiveData[0]==0xFB&&receiveData[1]==0x01&&receiveData[2]==0xDD)	 //����5�����������ϴ�����
			{
				command=5;
			}
			if(receiveData[0]==0xFB&&receiveData[1]==0x02&&receiveData[2]==0xDD)	 //����6�����������ϴ�����
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
	//	receiveData=SBUF;//��ȥ���յ�������
	//	RI = 0;//��������жϱ�־λ
	//	SBUF=receiveData;//�����յ������ݷ��뵽���ͼĴ���
	//	while(!TI);			 //�ȴ������������
	//	TI=0;						 //���������ɱ�־λ
	}
	else
	{
	  //TI = 0; 
	}
	

}




int main(void)
{
	//�ȵ��ϵ��ȶ�
	int mode=0;
	Delay_1ms(1000);
	//��Ϊ��һ������dht11 �Żᴥ��һ�βɼ�����.
	//������ʹ������ʱ�ɼ�һ������
	ReadTempAndHumi();
	//��Ϊ�����βɼ�������һ����ʱ����,���ﻹ�ɼ���
	Delay_1ms(3000);
	//�趨��ʱ��
	InitTime();	  //��ʱ����ʼ��
	UsartInit();  //���ڳ�ʼ��
	LcdInit(); 	  //LCD��ʼ��
	Int0Init();  //	�����ⲿ�ж�0
	
	setA=1;
	setB=1;

	while(1)
	{
		
		if(flag == 100)	  //һ���һ����ʪ��
		{

			flag = 0;
			/*
			//����ʪ��,�ɼ�⺯�������Ƿ�ʧ��,
			//��������OK(1)��ʾ�ɹ�,����ERROR(0)��ʾʧ��
			//OK��ERROR����DHT11.H�ж���ĺ�
			*/
			ReadTempAndHumi();
		}
	/*	if((temp_value>(temp_max*10))&&(flag_dianji==0))	  //�¶ȴ���temp_max�ȿ�ʼֱ�����������ʼ����
		{
			//motoa=0;
		//	motob=1;
			if(flag_upcount1==1)
			{
				sendlora();		  //����һ����ʪ������
				flag_upcount1=0;
			}  
			//up_period=500; //�ϴ����ڸ�Ϊ5s
		}
		else
		{
		 	motoa=0;
			motob=0;
		//	flag_dianji=0;		  //���ʹ��
			flag_upcount1=1;	  //�ָ�����
		//	up_period=3000; //�ϴ����ڻָ�Ϊ30s			//�˴����������ڳ�ͻ
		}	   *


		if((humi_value>(humi_max*10))&&(flag_beep2==0))	  //ʪ�ȴ���90�ȷ���������
		{
			flag_beep=1;
			if(flag_upcount2==1)
			{
				sendlora();		  //����һ����ʪ������
				flag_upcount2=0;
			}
		//	up_period=500; //�ϴ����ڸ�Ϊ5s				//�˴����������ڳ�ͻ
		}
		else
		{
		 	flag_beep=0;
			flag_upcount2=1;
		//	up_period=3000; //�ϴ����ڻָ�Ϊ30s
		} */

	/*	if(k1==0)		//�ж�K1�Ƿ��£�K1�������л���ʾģʽ
		{
			delay(1000);	//����
			if(k1==0)
			{	
				while(!k1);
				if(display_mode)
					display_mode=0;		//��ʾģʽ0,LCD��ϸ��ʾ
				else
					display_mode=1;		//��ʾģʽ1,����ܼ�����ʾ
			}
		}	


		if(k4==0)		//�ж�K4�Ƿ��£�K4�������ֶ��رյ������������Ҫ�ǲ�����
		{
			delay(1000);	//����
			if(k4==0)
			{	
				while(!k4);
				motoa=0;
				motob=0;
				flag_beep2=1; //���ùرշ�����
				flag_dianji=1;//���ùرյ��
			}
		}	 */
	

		if(display_mode==1)
		{
			//�������ʾ�¶�
			display(temp_value);
			//�������ʾʪ��
			display2(humi_value);
		}
		if(flag_LCD==100&&display_mode==0)
		{
			flag_LCD=0;
			display_LCD();
		}

		if(flag_lora>=up_period)  //up_period/100���ϴ�һ����ʪ��		  	//��ֹ����up_periodͻȻ�仯�����
		{
			flag_lora=0;
			sendlora();		  //������ʪ������
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
				Delay_1ms(100); //�����ϴ����쵼�������г�ͻ	    //�ؼ��ʣ���˫��
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
				flag_dianji=1;//���ùرյ��
				command=-1;
         	  	break;
			case 4:
				flag_beep2=1; //���ùرշ�����
				command=-1;
         	  	break;
			case 5:
				Delay_1ms(100); //�����ϴ����쵼�������г�ͻ		 //�ؼ��ʣ���˫��
				sendperiod();	//��������
				command=-1;
         	  	break;
			case 6:
				up_period=HexToDec(receiveData[3])*100;//�����ϴ����ڣ���Ҫ��100
				flag_lora=0;		//���¿�ʼ����
				command=-1;
         	  	break;
    		default:
          		break;
		}
		if(flag_diaodian)
		{
			LcdWriteCom(0x08);  //����ʾ����ʾ���
			//PCON |= (1<<1);        //����ģʽ
			PCON=0X01;  //����ģʽ
		}
		else
		{
			LcdWriteCom(0x0c);  //����ʾ����ʾ���
			PCON=0x00;            //����ָ�
		}
		if(setB==1)
			flag_diaodian=1;
			
		
	
			
	}
	return 0;

	
}

//�趨��ʱ����ʼ��,��ʱ10ms
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

void timer(void) interrupt 1		//��ȷ��ʱ10ms
{
//	TH0 =(65535 - 50000);	// (unsigned char)((65535 - 50000) >> 8);
//	TL0 =(65535 - 50000);	// (unsigned char)(65535 - 50000);
	TH0 =0XDC;	
	TL0 =0X00;
	if(flag<100)		   //��ֹ����100�����Զ�޷�������ʪ��
	{
		flag++;
	}
	if(flag_lora<up_period)	   //��ֹ����up_period����ɲ�����Զ����������
	{
		flag_lora++;
	}
	if(flag_beep==1)
		beep=~beep;
	if(flag_LCD<100)		   //��ֹ����100�����Զ�޷�������ʪ��
	{
		flag_LCD++;
	}
}

void Int0()	interrupt 0		//�ⲿ�ж�0���жϺ���
{	
	if(setB==0)
	{
		delay(1000);	 //��ʱ����
		if(setB==0)
		{
			led=~led;			 //led1����ָʾ��,������
			flag_diaodian=0;
		}
	}
}
