#include <lpc21xx.h>

#define LCD_D 0XFF << 0  
#define RS 1 << 8        
#define E 1 << 9          

typedef unsigned int U32;

void delay_ms(U32 millisecond)
{
    T0PR = 15000 - 1;
    T0TCR = 0x01;
    while (T0TC < millisecond);
    T0TCR = 0x03;
    T0TCR = 0x00;
}

void LCD_INIT(void);  
void LCD_CMD(unsigned char cmd);  
void LCD_DATA(unsigned char data);
void LCD_STRING(const char* str);

int main()
{
 
    char number[] = "EMBEDDED";  
    LCD_INIT();  
    while (1)
    {
        LCD_CMD(0X80);  
        LCD_STRING(number);  
    }
}

void LCD_INIT(void)
{
    IODIR0 = LCD_D | RS | E;  
    LCD_CMD(0x01);  
    LCD_CMD(0x02);
    LCD_CMD(0x0C);  
    LCD_CMD(0x38);
    LCD_CMD(0x80);  
}

void LCD_CMD(unsigned char cmd)
{
    IOCLR0 = LCD_D;  
    IOSET0 = cmd;  
    IOCLR0 = RS;    
    IOSET0 = E;    
    delay_ms(2);    
    IOCLR0 = E;    
}

void LCD_DATA(unsigned char data)
{
    IOCLR0 = LCD_D;  
    IOSET0 = data;  
    IOSET0 = RS;    
    IOSET0 = E;      
    delay_ms(2);    
    IOCLR0 = E;    
}

void LCD_STRING(const char* str)
{
    while (*str)  
    {
        LCD_DATA(*str++);  
    }
}
