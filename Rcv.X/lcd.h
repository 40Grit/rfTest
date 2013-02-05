/*lcd.h*/

//pic18f250 - http://ww1.microchip.com/downloads/en/DeviceDoc/39631E.pdf
#include <p18f2520.h>


#define LCD_RS_CNTL    0
#define LCD_RS_DATA    1
#define LCD_RW_WRITE   0
#define LCD_RW_READ    1

#define LCD_E      PORTCbits.RC7
#define LCD_RW     PORTCbits.RC6
#define LCD_RS     PORTCbits.RC5

#define LCD_DATA       PORTB
#define LCD_PORT_DIR   TRISB
#define LCD_PORT_OUT   0x00
#define LCD_PORT_IN    0xff

#define BYTE unsigned char
#define WORD unsigned int



void Pause(BYTE max);
void Msec2(void);
void nop(void);

void LcdInit(void);
void LcdText(BYTE row, BYTE col, char *textp);
void LcdConst(BYTE row, BYTE col, const char *textp);
void LcdWrite(BYTE reg, BYTE data);
void LcdBusy(BYTE reg, BYTE data);
