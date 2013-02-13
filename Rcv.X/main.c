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

void intToCharArray(int num, char array[]);
void byteToCharArray(BYTE num, BYTE array[4]);
void ModeSelectInit(void);
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

void ShockBurstRxTest(void)
{
	char initialText[12] = "InitialText";
	BYTE addr[] = {0xE7,0xE7,0xE7,0xE7,0xE7};
	BYTE byteReceived;
	BYTE byteCounter;
	BYTE receivedString[4];
	BYTE counterString[4];
   
	/*Initialize LCD screen*/
	LcdInit();
	/*Initialize nRF24l01+ for enhance shockburst in recieve mode */
	RfShockBurstInit(addr, MODE_RX, 1);
	/*write iniital text to lcd*/
	LcdText(0,0,initialText);

	//Block until first byte is received,
	//This byte will track what future received bytes should be
	while(RecvPacket(&byteCounter, 1));
	while(1)
	{
		//Increment byteCounter for next Receive
		byteCounter++;
		while (RecvPacket(&byteReceived, 1));

		byteToCharArray(byteReceived, receivedString);
		byteToCharArray(byteCounter, counterString);

		LcdClear();
		LcdText(0,0, receivedString);
		LcdText(0x40,0,counterString);
	}
}

void ShockBurstTxTest(void)
{
	BYTE addr[] = {0xE7,0xE7,0xE7,0xE7,0xE7};
	BYTE testByte = 0;
	BYTE delay = 0;

	/*Initialize nRF24l01+ for enhance shockburst in transmit mode */
	RfShockBurstInit(addr, MODE_TX, 1);

	/*Constantly send the test string*/
	while (1)
	{
		for(delay = 0; delay<3; delay++) pause(255);
		XmitPacket(&testByte, 1);
		testByte++;
	}
}

/*initializes a bit on port c as input*/
void ModeSelectInit(void)
{
	TRISCbits.RC0 = 1;
}

void byteToCharArray(BYTE num, BYTE array[4])
{
    BYTE digit = num;
    BYTE length = 0;
    BYTE index  = 0;

    BYTE buf[4] = "   ";

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
