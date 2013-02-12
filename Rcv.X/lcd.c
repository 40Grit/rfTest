#include "lcd.h"


/* Pause
 * delay for a duration (in milliseconds)
 */
void Pause(BYTE max)
{
   for ( ; max != 0; max--)
   {
      /* the function msec2 delays for 1/2 millisecond */
      Msec2();
      Msec2();
   }
}


/* Msec2
 * delay for 1/2 millisecond
 */
void Msec2(void)
{
   BYTE ofst;

   /* this delays for .5 msec */
   for (ofst=0; ofst<98; ofst++)
      nop();
}

/* LcdInit
 * initialize the lcd character display
 */
void LcdInit(void)
{

   PORTC = 0;
   DDRCbits.RC5 = 0;
   DDRCbits.RC6 = 0;
   DDRCbits.RC7 = 0;

   LCD_DATA = 0;
   LCD_PORT_DIR = LCD_PORT_IN;

   Pause(25);
   LcdWrite(LCD_RS_CNTL, 0x30);
   Pause(5);
   LcdWrite(LCD_RS_CNTL, 0x30);
   Msec2();
   LcdWrite(LCD_RS_CNTL, 0x30);

   LcdBusy(LCD_RS_CNTL, 0x38);
   LcdBusy(LCD_RS_CNTL, 0x0c);
   LcdBusy(LCD_RS_CNTL, 0x01);
   LcdBusy(LCD_RS_CNTL, 0x06);
   LcdBusy(LCD_RS_CNTL, 0x02);
}


/* Lcdtext
 * write a string of text to the display
 */
void LcdText(BYTE row, BYTE col, char *textp)
{
     if (row != 0)
         row = 0x40;

     LcdBusy(LCD_RS_CNTL, (BYTE)(0x80 | row | col));

     for ( ; *textp!='\0'; textp++)
        LcdBusy(LCD_RS_DATA, *textp);
}


/* Lcdwrite
 * write a byte to an lcd register
 */
void LcdWrite(BYTE reg, BYTE data)
{
   LCD_RS = reg;
   LCD_RW = LCD_RW_WRITE;
   LCD_DATA = data;
   LCD_PORT_DIR = LCD_PORT_OUT;
   LCD_E = 1;
   nop();
   LCD_E = 0;
   LCD_PORT_DIR = LCD_PORT_IN;
}

/* LcdBusy
 * wait for the lcd to be idle (not busy)
 * then write a byte to an lcd register
 */
void LcdBusy(BYTE reg, BYTE data)
{
   BYTE res;

   while (1)
   {
      LCD_RS = LCD_RS_CNTL;
      LCD_RW = LCD_RW_READ;
      LCD_PORT_DIR = LCD_PORT_IN;
      LCD_E = 1;
      nop();
      res = LCD_DATA;
      LCD_E = 0;
      if ((res & 0x80) == 0)
         break;
   }

   LcdWrite(reg, data);
}

void LcdClear(void)
{
	LcdWrite(LCD_RS_CNTL, 0x01);
}