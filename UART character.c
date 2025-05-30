#include <LPC21xx.H>

typedef unsigned int U32;
void delay_ms(U32 ms)
{
    T0PR = 15000 - 1;
    T0TCR = 0x01;
    while (T0TC < ms);
    T0TCR = 0x03;
}

void UART0_CONFIG(void);
void UART0_TX(unsigned char);
void delay_ms(U32 ms);

int main()
{
char ch;
int num;
    UART0_CONFIG();
   
    while (1)
    {
        for (ch = 'A'; ch <= 'Z'; ch++)
        {
            UART0_TX(ch);
 UART0_TX(' ');
            delay_ms(100);
        }
        UART0_TX('\n');

        for (num = '0'; num <= '9'; num++)
        {
            UART0_TX(num);
 UART0_TX(' ');
            delay_ms(100);
        }
while(1);
    }
}

void UART0_CONFIG(void)
{
    PINSEL0 = 0x00000005;
    U0LCR = 0x83;        
    U0DLL = 97;          
    U0LCR = 0x03;        
}

void UART0_TX(unsigned char d)
{
    while ((U0LSR & (1 << 5)) == 0);
    U0THR = d;                      
}
