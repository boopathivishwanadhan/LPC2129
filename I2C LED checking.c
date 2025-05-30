#include <LPC21xx.h>

/* ---------------- Type Definitions ---------------- */
typedef unsigned int u32;
typedef unsigned char u8;

/* ---------------- I2C Constants ---------------- */
#define I2C_EEPROM_SA 0x50 // 7-bit EEPROM address
#define gLED (1<<18)       // Green LED at P1.18
#define rLED (1<<17)       // Red LED at P1.17

#define SCL_EN  0x00000010
#define SDA_EN  0x00000040

#define CCLK      60000000
#define PCLK      (CCLK / 4)
#define I2C_SPEED 100000
#define LOADVAL   ((PCLK / I2C_SPEED) / 2)

#define AA_BIT   2
#define SI_BIT   3
#define STO_BIT  4
#define STA_BIT  5
#define I2EN_BIT 6

/* ---------------- Delay Function ---------------- */
void delay_ms(u32 sec)
{
    T0PR = 15000 - 1;
    T0TCR = 0x01;
    while (T0TC < sec);
    T0TCR = 0x03;
    T0TCR = 0x00;
}

/* ---------------- I2C Functions ---------------- */
void init_i2c(void)
{
    PINSEL0 |= SCL_EN | SDA_EN;
    I2SCLL = LOADVAL;
    I2SCLH = LOADVAL;
    I2CONSET = 1 << I2EN_BIT;
}

void i2c_start(void)
{
    I2CONSET = 1 << STA_BIT;
    while (((I2CONSET >> SI_BIT) & 1) == 0);
    I2CONCLR = 1 << STA_BIT;
}

void i2c_restart(void)
{
    I2CONSET = 1 << STA_BIT;
    I2CONCLR = 1 << SI_BIT;
    while (((I2CONSET >> SI_BIT) & 1) == 0);
    I2CONCLR = 1 << STA_BIT;
}

void i2c_write(u8 dat)
{
    I2DAT = dat;
    I2CONCLR = 1 << SI_BIT;
    while (((I2CONSET >> SI_BIT) & 1) == 0);
}

void i2c_stop(void)
{
    I2CONSET = 1 << STO_BIT;
    I2CONCLR = 1 << SI_BIT;
}

u8 i2c_nack(void)
{
    I2CONSET = 0x00;
    I2CONCLR = 1 << SI_BIT;
    while (((I2CONSET >> SI_BIT) & 1) == 0);
    return I2DAT;
}

u8 i2c_masterack(void)
{
    I2CONSET = 1 << AA_BIT;
    I2CONCLR = 1 << SI_BIT;
    while (((I2CONSET >> SI_BIT) & 1) == 0);
    I2CONCLR = 1 << AA_BIT;
    return I2DAT;
}

/* ---------------- EEPROM Functions ---------------- */
void i2c_eeprom_write(u8 slaveAddr, u8 wBuffAddr, u8 dat)
{
    i2c_start();
    i2c_write(slaveAddr << 1);     // SLA+W
    i2c_write(wBuffAddr);          // EEPROM memory address
    i2c_write(dat);                // data to write
    i2c_stop();
    delay_ms(10);                  // EEPROM internal write delay
}

u8 i2c_eeprom_read(u8 slaveAddr, u8 rBuffAddr)
{
    u8 dat;
    i2c_start();
    i2c_write(slaveAddr << 1);     // SLA+W
    i2c_write(rBuffAddr);          // EEPROM memory address
    i2c_restart();
    i2c_write((slaveAddr << 1) | 1); // SLA+R
    dat = i2c_nack();
    i2c_stop();
    return dat;
}

/* ---------------- Main Function ---------------- */
int main(void)
{
    u8 ch;
    init_i2c();
    IODIR1 = gLED | rLED;

    i2c_eeprom_write(I2C_EEPROM_SA, 0x00, 'F');     // Write to EEPROM
    ch = i2c_eeprom_read(I2C_EEPROM_SA, 0x00);      // Read from EEPROM

    delay_ms(100);

    if (ch == 'F')          // Match with written character
    {
        IOSET1 = gLED;
			IOCLR1 = rLED;
    }
    else
    {
        IOSET1 = rLED;
			IOCLR1 = gLED;
    }

    while (1); // Infinite loop
}
