#include <p18F2520.h>


/* Set up the configuration bits */
#pragma config OSC = INTIO67
#pragma config STVREN = OFF
#pragma config PWRT = ON
#pragma config BOREN = OFF
#pragma config WDT = OFF
#pragma config PBADEN = OFF
#pragma config CCP2MX = PORTC
#pragma config MCLRE = OFF
#pragma config LVP = OFF



#define BYTE unsigned char
#define WORD unsigned int


#define RS_CNTL    0
#define RS_DATA    1
#define RW_WRITE   0
#define RW_READ    1

#define LCD_E      PORTCbits.RC7
#define LCD_RW     PORTCbits.RC6
#define LCD_RS     PORTCbits.RC5

#define LCD_DATA   PORTB
#define LCD_TRIS   TRISB
#define TRIS_OUT   0x00
#define TRIS_IN    0xff


void Pause(BYTE max);
void Msec2(void);
void nop(void);

void LcdInit(void);
void LcdText(BYTE row, BYTE col, char *textp);
void LcdConst(BYTE row, BYTE col, const char *textp);
void LcdWrite(BYTE reg, BYTE data);
void LcdBusy(BYTE reg, BYTE data);


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

/* nop
 * do nothing and return
 */
void nop(void)
{
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
   LCD_TRIS = TRIS_IN;

   Pause(25);
   LcdWrite(RS_CNTL, 0x30);
   Pause(5);
   LcdWrite(RS_CNTL, 0x30);
   Msec2();
   LcdWrite(RS_CNTL, 0x30);

   LcdBusy(RS_CNTL, 0x38);
   LcdBusy(RS_CNTL, 0x0c);
   LcdBusy(RS_CNTL, 0x01);
   LcdBusy(RS_CNTL, 0x06);
   LcdBusy(RS_CNTL, 0x02);
}


/* Lcdtext
 * write a string of text to the display
 */
void LcdText(BYTE row, BYTE col, char *textp)
{
     if (row != 0)
         row = 0x40;

     LcdBusy(RS_CNTL, (BYTE)(0x80 | row | col));

     for ( ; *textp!='\0'; textp++)
        LcdBusy(RS_DATA, *textp);
}


/* Lcdwrite
 * write a byte to an lcd register
 */
void LcdWrite(BYTE reg, BYTE data)
{
   LCD_RS = reg;
   LCD_RW = RW_WRITE;
   LCD_DATA = data;
   LCD_TRIS = TRIS_OUT;
   LCD_E = 1;
   nop();
   LCD_E = 0;
   LCD_TRIS = TRIS_IN;
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
      LCD_RS = RS_CNTL;
      LCD_RW = RW_READ;
      LCD_TRIS = TRIS_IN;
      LCD_E = 1;
      nop();
      res = LCD_DATA;
      LCD_E = 0;
      if ((res & 0x80) == 0)
         break;
   }

   LcdWrite(reg, data);
}


/* init
 * pic general initialization
 */
void PicInit(void)
{
   // configure the internal oscillator
   OSCCON |= 0x70;            // sets 8 mhz operation

}


void main(void)
{
   /* pic initialization */
   PicInit();
   LcdInit();

   /* main loop */

   while (1)
   {
   }

}
