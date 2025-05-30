#include<LPC21xx.h>                                
																/* defines.h */
#ifndef __DEFINES_H__
#define __DEFINES_H__

#define SETBIT(WORD,BITPOS)         (WORD|=1<<BITPOS)
#define CLRBIT(WORD,BITPOS)         (WORD&=~(1<<BITPOS))
#define CPLBIT(WORD,BITPOS)         (WORD^=(1<<BITPOS))
#define WRITEBIT(WORD,BITPOS,BIT)   (BIT ? SETBIT(WORD,BITPOS): CLRBIT(WORD,BITPOS))
#define READBIT(WORD,BITPOS)        ((WORD>>BITPOS)&1)

typedef unsigned int u32;
typedef signed int s32;
typedef unsigned short int u16;
typedef signed short int s16;
typedef unsigned char u8;
typedef signed char s8;
typedef float f32;
typedef double f64;

#endif

u8 SPI0(u8 data);

/* spi_defines.h */
#ifndef __SPI_DEFINES_H__
#define __SPI_DEFINES_H__

#define CS 7

//defines for clock /baudrate

#define FOSC 12000000
#define CCLK (5*FOSC)
#define PCLK (CCLK/4)
#define SPCCR_VAL 60
#define SPI_RATE (u8)(PCLK/SPCCR_VAL)

// SPCR Bits Setting 
#define Mode_0     0x00  // CPOL 0 CPHA 0
#define Mode_1     0x08  // CPOL 0 CPHA 1
#define Mode_2     0x10  // CPOL 1 CPHA 0
#define Mode_3     0x18  // CPOL 1 CPHA 1
#define MSTR_BIT   5     // SPI0 as Master 
#define LSBF_BIT   6     // default MSB first,if set LSB first
#define SPIE_BIT   7     //SPI Interrupt Enable Bit

// SPSR bits
#define SPIF_BIT   7    // Data Transfer Completion Flag

//SPINT bit
#define SPIINTF_BIT 0   //SPI Interrupt Flag Bit
#endif


#define lcd_d 0xff<<8
#define rs 1<<16
//#define rw 1<<18
#define e 1<<18


void delay_milliseconds(u32 milliseconds)
{
	
	T0PR = 15000-1;
	T0TCR = 0X01;
	while(T0TC<milliseconds);
	T0TCR = 0x03;
	T0TCR = 0x00;
}

void lcd_cmd(u8 d)
{
	IOPIN0 = d<<8;
	IOCLR0 = rs;
	IOSET0 =e;
	delay_milliseconds(2);
	IOCLR0 =e;
}

void lcd_data(u8 d)
{
	IOPIN0 = d<<8;
	IOSET0 = rs;
	IOSET0 =e;
	delay_milliseconds(2);
	IOCLR0 =e;
}

void lcd_init(void)
{
	IODIR0 |= lcd_d|rs|e;
	lcd_cmd(0x01);
	lcd_cmd(0x02);
	lcd_cmd(0x0c);
	lcd_cmd(0x38);
	lcd_cmd(0x80);
}

void str(u8 *s)
{
	while(*s)
		lcd_data(*s++);
}

void lcd_integer(int n)
{
	s8 arr[5],i=0;
	if(n==0)
		lcd_data('0');
	else
	{
		   if(n<0)
		   {
				 lcd_data('-');
				 n=-n;
       }
	      while(n>0)
      	{
		     arr[i++]=n%10;
		     n=n/10;
        }
        for(--i;i>=0;i--)
   	      lcd_data(arr[i]+48);
  }
}
void Float_display(f32 f_val)
{
  u32 number;
  number = f_val;
	number=(f_val-number)*100;
  lcd_integer(number);
  lcd_data('.');
  lcd_integer(00);	
}


       

f32 Read_ADC_MCP3204(u8 channelNo)
{
  u8 hByte,lByte;
  u32 adcVal=0;
   
  
  CLRBIT(IOPIN0,CS);

	
  SPI0(0x06);
  hByte = SPI0(channelNo<<6);
  lByte = SPI0(0x00);
	

	SETBIT(IOPIN0,CS);
	
  adcVal=((hByte&0x0f)<<8)|lByte;
  return ((adcVal*3.3)/4096);
}
                     
void Init_SPI0(void)
{
	
	PINSEL0 |=0X00001500;//P0.4 to P0.6 as SPI pins 
  S0SPCCR = SPI_RATE;             // cfg bit clock rate 
  S0SPCR  = (1<<MSTR_BIT|Mode_3); //spi module in master mode,
                                  //CPOL =1,CCPHA = 1. MSB first
	SETBIT(IOPIN0,CS);
	SETBIT(IODIR0,CS);
}

u8 SPI0(u8 data)
{
   u8 stat;
   stat = S0SPSR;    //clear SPIF 
   S0SPDR = data;   // load spi tx reg
   while(READBIT(S0SPSR,SPIF_BIT)==0); // wait for transmission to complete
   return S0SPDR;    // read data from SPI data reg, place into buffer 
}
int main()
{
	f32 f;
	Init_SPI0(); //hw SPI initialisation
	lcd_init();	
	str("MCP3204--");
  while(1)
  {
    f=Read_ADC_MCP3204(0);
		lcd_cmd(0xc0);
		Float_display(f);
		delay_milliseconds(1500);
	}	
}
				

