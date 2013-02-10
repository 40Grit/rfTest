#include <p18f2520.h>
#include "lcd.h"
#include "rf.h"

/* Set up the configuration bits */
#pragma config OSC		= INTIO67
#pragma config STVREN	= OFF
#pragma config PWRT		= ON
#pragma config BOREN	= OFF
#pragma config WDT		= OFF
#pragma config PBADEN	= OFF
#pragma config CCP2MX	= PORTC
#pragma config MCLRE	= OFF
#pragma config LVP		= OFF

void oneByteTest(void);
void RangeTestRx(void);
void intToCharArray(int num, char array[]);
void ModeSelectInit(void);
void RangeTestTx(void);
void ShockBurstRxTest(void);
void ShockBurstTxTest(void);

void main(void)
{
	ModeSelectInit();
	RfPicInit();

	/*if input RC0 is high, run reciever test else run transmiter test*/
	PORTCbits.RC0 ? (ShockBurstRxTest()):(ShockBurstTxTest());
	//PORTCbits.RC0 ? (RangeTestRx()):(RangeTestTx());
}

void RangeTestTx(void)
{
	int count;
	BYTE testData = 0x01;
	XmitInit();
	while (1)
	{
		for(count=0;count<200;count++);  //Pause to let receiver 'cath-up'
	
		XmitPacket(testData++);			 //Send the test BYTE and increment it
		
		if (testData > 100)				 //if 100 bytes have been sent
		{
			XmitPacket(0xFF);			 //send indicator 0xFF
			testData = 0x01;			 //reset testData
		}
	}

}

void RangeTestRx(void)
{
    BYTE data;
    int count = 0;
	int sum = 0;
    long missCount = 0;
    char countString[8] = "       ";
	char sumString[8]	= "       ";
    char lostRf[8] = "Lost RF";

	LcdInit();
	RecvInit();

    while(1)
    {
        while (RecvPacket(&data) == 0)
        {
            if (missCount++ >= 100000)
            {
				LcdBusy(LCD_RS_CNTL, 0x01);
				LcdText(0,0,lostRf);
				missCount = 0;
            }
        }

        //if 100 bytes indicator received, 0xFF, isn't received continue
        if (data <= 100)
		{
			sum += data;
			count++;
            continue;
		}

        //turn the count into characters for display
        intToCharArray(count, countString);
		intToCharArray(sum, sumString);
        LcdBusy(LCD_RS_CNTL, 0x01);         //clear lcd screen
        LcdText(0,0, countString);          //display the count
		LcdText(0x40,0, sumString);			//display the sum
		sum = 0;
        count = 0;                          //reset
    }

}

void ShockBurstRxTest(void)
{
	char initialText[12] = "InitialText";
	BYTE addr[] = {0xE7,0xE7,0xE7,0xE7,0xE7};
	BYTE data[32];
   
	/*Initialize LCD screen*/
	LcdInit();
	/*Initialize nRF24l01+ for enhance shockburst in recieve mode */
	RfShockBurstInit(addr, MODE_RX);
	/*write iniital text to lcd*/
	LcdText(0,0,initialText);

	/*block while no data has been received*/
	/*print data to lcd screen when received*/
	while(1)
	{
		while (RecvPacket2(data));
		LcdText(0,0, data);
	}
}

void ShockBurstTxTest(void)
{
	BYTE testData[32] = "ShockBurst \0";
	BYTE addr[] = {0xE7,0xE7,0xE7,0xE7,0xE7};
	/*Initialize nRF24l01+ for enhance shockburst in transmit mode */
	RfShockBurstInit(addr, MODE_TX);

	/*Constantly send the test string*/
	while (1)
	{
		pause(10);
		XmitPacket2(testData, 32);		//Send the test BYTE and increment it
	}
}
/*Determines the string representation
 of an int input and places in character string*/
void intToCharArray(int num, char array[8])
{
    char digit = num;
    char length = 0;
    char index  = 0;

    char buf[9] = "        ";
    
    //Grab each digit in given integer add 0x30 to get to ascii,
    //Breaks if all digits have been placed in the buffer
    while(1)
    {
        buf[length++] = (num % 10) + 0x30;
        num /= 10;
        if (!num) break;
    }
    
    //Place buffer into given array, back to front, for proper display
    for(index=0; index < length; index++)
    {
        array[index] = buf[length - 1 - index];
    }

    //terminate string
    array[index] = '\0';
}

void oneByteTest(void)
{
   BYTE data;
   char okay[5]   = {"okay"};
   char bad[4]    = {"bad"};
   while(1)
   {
    while (RecvPacket(&data) == 0);
    if(data == 0xF4)
        LcdText(0,0,okay);
    else
        LcdText(0,0,bad);
   }
}

/*initializes a bit on port c as input*/
void ModeSelectInit(void)
{
	TRISCbits.RC0 = 1;
}
