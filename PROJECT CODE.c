#include <LPC21XX.H>

// --- MACROS ---
#define LCD_D 0xff<<8
#define RS (1<<16)
#define RW (1<<19)
#define E  (1<<17)
#define CS 7

#define FOSC 12000000
#define CCLK (5*FOSC)
#define PCLK (CCLK/4)
#define SPCCR_VAL 60
#define SPI_RATE (PCLK/SPCCR_VAL)

// --- DELAY FUNCTIONS ---
void delay_milliseconds(int milliseconds) {
    T0PR = 15000 - 1;
    T0TCR = 0x01;
    while (T0TC < milliseconds);
    T0TCR = 0x03;
    T0TCR = 0x00;
}

void delay_seconds(int seconds) {
    T0PR = 15000000 - 1;
    T0TCR = 0x01;
    while (T0TC < seconds);
    T0TCR = 0x03;
    T0TCR = 0x00;
}

// --- UART FUNCTIONS ---
void UART0_INIT(void) {
    PINSEL0 |= 0x00000005;
    U0LCR = 0x83;
    U0DLL = 97;
    U0LCR = 0x03;
}

void UART0_Tx(unsigned char d) {
    while (!(U0LSR & (1 << 5)));
    U0THR = d;
}

void UART0_INTEGER(int n) {
    unsigned char arr[5];
    signed int i = 0;

    if (n == 0)
        UART0_Tx('0');
    else {
        if (n < 0) {
            UART0_Tx('-');
            n = -n;
        }
        while (n != 0) {
            arr[i++] = n % 10;
            n = n / 10;
        }
        for (--i; i >= 0; i--)
            UART0_Tx(arr[i] + 48);
    }
}

void UART0_fp(float f) {
    int temp = f;
    UART0_INTEGER(temp);
    UART0_Tx('.');
    temp = (f - temp) * 1000;
    UART0_INTEGER(temp);
}

void UART0_STR(char *s) {
    while (*s)
        UART0_Tx(*s++);
}

void UART0_time(char h, char m, char s) {
    UART0_Tx('0' + (h / 10));
    UART0_Tx('0' + (h % 10));
    UART0_Tx(':');
    UART0_Tx('0' + (m / 10));
    UART0_Tx('0' + (m % 10));
    UART0_Tx(':');
    UART0_Tx('0' + (s / 10));
    UART0_Tx('0' + (s % 10));
}

// --- LCD FUNCTIONS ---
void LCD_COMMAND(unsigned char cmd) {
    IOCLR0 = LCD_D;
    IOSET0 = cmd << 8;
    IOCLR0 = RS;
    IOCLR0 = RW;
    IOSET0 = E;
    delay_milliseconds(2);
    IOCLR0 = E;
}

void LCD_DATA(unsigned char d) {
    IOCLR0 = LCD_D;
    IOSET0 = d << 8;
    IOSET0 = RS;
    IOCLR0 = RW;
    IOSET0 = E;
    delay_milliseconds(2);
    IOCLR0 = E;
}

void LCD_INIT(void) {
    IODIR0 |= LCD_D | RS | RW | E;
    LCD_COMMAND(0x01);
    LCD_COMMAND(0x02);
    LCD_COMMAND(0x0C);
    LCD_COMMAND(0x38);
    LCD_COMMAND(0x80);
}

void LCD_STR(unsigned char *s) {
    while (*s)
        LCD_DATA(*s++);
}

void LCD_INTEGER(int n) {
    unsigned char arr[5];
    signed int i = 0;

    if (n == 0)
        LCD_DATA('0');
    else {
        if (n < 0) {
            LCD_DATA('-');
            n = -n;
        }
        while (n != 0) {
            arr[i++] = n % 10;
            n = n / 10;
        }
        for (--i; i >= 0; i--)
            LCD_DATA(arr[i] + 48);
    }
}

void LCD_fp(float f) {
    int temp = f;
    LCD_INTEGER(temp);
    LCD_DATA('.');
    temp = (f - temp) * 1000;
    LCD_INTEGER(temp);
}

// --- SPI FUNCTIONS ---
void Init_SPI0(void) {
    PINSEL0 |= 0x00001500;
    S0SPCCR = SPI_RATE;
    S0SPCR = (1 << 5) | 0x18;  // Master mode, CPOL=1, CPHA=1
    IODIR0 |= (1 << CS);
    IOSET0 = (1 << CS);
}

unsigned char SPI0(unsigned char data) {
    S0SPSR;
    S0SPDR = data;
    while (!(S0SPSR & (1 << 7)));
    return S0SPDR;
}

float Read_ADC_MCP3204(unsigned char channelNo) {
    unsigned char hByte, lByte;
    unsigned int adcVal;

    IOCLR0 = (1 << CS);
    SPI0(0x06);
    hByte = SPI0(channelNo << 6);
    lByte = SPI0(0x00);
    IOSET0 = (1 << CS);

    adcVal = ((hByte & 0x0F) << 8) | lByte;
    return ((adcVal * 3.3) / 4096.0) * 100;
}

// --- Time Display on LCD ---
void LCD_display_time(char h, char m, char s) {
    LCD_DATA('0' + (h / 10));
    LCD_DATA('0' + (h % 10));
    LCD_DATA(':');
    LCD_DATA('0' + (m / 10));
    LCD_DATA('0' + (m % 10));
    LCD_DATA(':');
    LCD_DATA('0' + (s / 10));
    LCD_DATA('0' + (s % 10));
}

// --- MAIN FUNCTION ---
int main(void) {
    float temp;
    char sec, min, hr;

    Init_SPI0();
    LCD_INIT();
    UART0_INIT();

    // RTC Setup
    CCR = 0x01;
    SEC = 0;
    MIN = 0;
    HOUR = 12;

    PREINT = 0x01C8;     // For 15MHz PCLK
    PREFRAC = 0x61C0;

    LCD_COMMAND(0x80);
    LCD_STR("Temp:      ");
    LCD_COMMAND(0xC0);
    LCD_STR("Time:      ");

    while (1) {
        sec = SEC;
        min = MIN;
        hr = HOUR;

        temp = Read_ADC_MCP3204(0);  // Channel 0

        // LCD Update
        LCD_COMMAND(0x86);
        LCD_fp(temp);

        LCD_COMMAND(0xC6);
        LCD_display_time(hr, min, sec);

        // UART Update
        UART0_STR("Temp: ");
        UART0_fp(temp);
        UART0_STR("  Time: ");
        UART0_time(hr, min, sec);
        UART0_Tx('\n');

        delay_seconds(1);
    }
}
