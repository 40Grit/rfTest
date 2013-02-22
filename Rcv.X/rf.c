/*rf.c contains functions to initialize an nRF24L01+
 *in enhanced shockburst mode as a primary receiver
 *or primary transmitter, send/receive packets,
 *and check IRQ interrupts
 *NOTE: consider this unfinished
 */

#include "rf.h"

/*RfInit(): configure the hardware*/
void RfPicInit(void)
{
	OSCCON |= 0x70; // sets 8 mhz operation
	PORTA = 0; //clear port A
	TRISA = 0x30; //set port A pins (0:3) as output, (4:5)intput; no concern(6:7)
	//FIXME: disable a/d only for relevent pins
	ADCON1 = 0x0F; //disable AD for relevant pins
	//FIXME: disable comparator only for relevean pins
	CMCON = 0x07; //disable comparator for relevent pins
}

/*Provide a 'device address', mode (MODE_TX, MODE_RX), and a static payload width*/
void RfShockBurstInit(BYTE address[], BYTE mode, BYTE payloadWidth)
{
	RF_CE = 0;
	RF_CSN = 1;
	RF_SCK = 0;
	nop();

	//Close Pipes
	WriteRegister(EN_RXADDR, 0x00);
	WriteRegister(EN_AA, 0x00);
	//Open Pipe 0
	WriteRegister(EN_RXADDR, 0x01);
	//Pipe 0 autoack
	WriteRegister(EN_AA, 0x01);
	//CONFIG:16 bit crc
	RfConfigure(CRCO, 1);
	//auto retransmit 	//15 tranmist with 500us
	WriteRegister(SETUP_RETR, 0x1F);

	//Set 5 byte address width
	WriteRegister(SETUP_AW, 0x03);
	//set tx address
	WriteAdrRegister(TX_ADDR, address, 5);
	//set pipe 0 address
	WriteAdrRegister(RX_ADDR_P0, address, 5);

	//set mode: tx or rx
	if (mode == MODE_TX)
		RfConfigure(PRIM_RX, 0);
	else
	{
		RfConfigure(PRIM_RX, 1);
		WriteRegister(RX_PW_P0, payloadWidth);
		RF_CE = 1;
	}

	//Set rf channel
	WriteRegister(RF_CH, 0x12);
	//turn on power
	RfConfigure(PWR_UP, 1);
	//FIXME: start a timer
	pause(5);
	//wait for timer
	/*TODO: Implement state?*/

}

/*XmitPacket: send a data byte/block to the rf module*/
BYTE XmitPacket(BYTE *data, BYTE length)
{
	RF_CE = 0;

	//early return if packet length is too long
	if (length > 32)
		return 1;
	
	//Clear Interrupts
	WriteRegister(STATUS, (STATUS_TX_DS | STATUS_RX_DR | STATUS_MAX_RT));

	//load tx fifo
	WriteTxPayload(data, length);

	//send packet
	PulseCe();

	//block until acknowledge or timeout
	while (1)
	{
		if (CheckInterrupt(STATUS_MAX_RT))
		{
			OutCommand(FLUSH_TX);
			return 2;
		}
		if (CheckInterrupt(STATUS_TX_DS))
			return 0;
	}

	/*FIXME:implement state?*/
	//set state
}

/*RecvPacket: check/receive a packet from the rf module*/
BYTE RecvPacket(BYTE *payload, BYTE length)
{
	/* make sure data is present */
	if (!CheckInterrupt(STATUS_RX_DR))
		return (1);
	
	RF_CE = 0;//Disable radio
	/* read RX payload, which is blah bytes long*/
	ReadRxPayload(payload, length);

	/* Flush RX FIFO */
	OutCommand(FLUSH_RX);

	/* reset interrupt */
	WriteRegister(STATUS, STATUS_RX_DR);
	RF_CE = 1;//Re-enable radio
	return (0);
}

//Write individual bits of the configure register
void RfConfigure(BYTE bitNum, BYTE enable)
{
	BYTE config = 0;
	config = ReadRegister(CONFIG);
	if (enable)
		config |= 1 << bitNum;
	else
		config &= ~(1 << bitNum);
	WriteRegister(CONFIG, config);
}

//Check for interrupt specified by intrupt
BYTE CheckInterrupt(BYTE intrupt)
{
	BYTE status;
	/*Check IRQ pin, return early if not active*/
	if (RF_IRQ)
		return 0;

	/*Interrupt thrown, get it from status
	 Mask pipe bits*/
	status = OutCommand(NOP) & 0xF0;
	/*return 1 if intrupt is thrown*/
	return (intrupt & status) ? (1) : (0);
}

//Toggles RF_CE used to transmit one packet from TX Fifo
void PulseCe(void)
{
	BYTE count = 0;
	RF_CE = 1; /* pulse CE to start transmission (1 msec) */
	for (count = 0; count < 150; count++)
		nop();
	RF_CE = 0;
}

/*pause(max): delay for "max" times .5 msec*/
/*FIXME: TRUTH???^^^^*/
void pause(BYTE max)
{
	BYTE ofst;
	BYTE count;
	for (count = 0; count < max; count++)
	{

		/* this delays for .5 msec */
		//FIXME:TRUTH?^^^^^^^
		for (ofst = 0; ofst < 97; ofst++)
			nop();
	}
}

void nop(void)
{
}


