#include <LPC21xx.H>
#define LED_1 1<<9
#define LED_2 1<<10


void delay(unsigned int ms)
{
	T0PR =15000-1;
	T0TCR =0X01;
	while(T0TC < ms);
	T0TCR =0X03;
	T0TCR =0X00;
}
int main(){
	int i;
	PINSEL0 = 0X00000000;
	IODIR0 = LED_1|LED_2;
	for(i =0;i<10;i++)
	{
		IOSET0 =LED_1|LED_2;
		delay(100);
		IOCLR0=LED_1|LED_2;
		delay(100);
	}
	while (1) {};
}
