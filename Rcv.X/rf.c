#include "rf.h"

/*RfInit(): configure the hardware*/
void RfPicInit(void)
{
   OSCCON |= 0x70;            // sets 8 mhz operation
   PORTA = 0;				  //clear port A
   TRISA = 0x30;              //set port A pins (0:3) as output, (4:5)intput; no concern(6:7)
   //FIXME: disable a/d only for relevent pins
   ADCON1 = 0x0F;             //disable AD for relevant pins
   //FIXME: disable comparator only for relevean pins
   CMCON = 0x07;              //disable comparator for relevent pins
}

void RfShockBurstInit(BYTE address[], BYTE mode)
{
	RF_CE = 0;
	RF_CSN = 1;
	RF_SCK = 0;
	nop();
	
	//Close Pipes
	WriteRegister(EN_RXADDR,0x00);
	WriteRegister(EN_AA, 0x00);
	//Open Pipe 0
	WriteRegister(EN_RXADDR, 0x01);
	//Pipe 0 autoack
	WriteRegister(EN_AA, 0x01);
	//CONFIG:16 bit crc
	RfConfigure(CRCO,1);
	//auto retransmit 	//15 tranmist with 500us
	WriteRegister(SETUP_RETR, 0x0F);

	//Set 5 byte address width
	WriteRegister(SETUP_AW,0x03);
	//set tx address
	WriteAdrRegister(TX_ADDR, address, 5);
	//set pipe 0 address
	WriteAdrRegister(RX_ADDR_P0, address, 5);
	//set mode
	if(mode == MODE_TX)
		RfConfigure(PRIM_RX,0);
	else
	{
		RfConfigure(PRIM_RX,1);
		WriteRegister(RX_PW_P0, 32);
	}

	//Set rf channel
	WriteRegister(RF_CH, 0x12);
	//turn on power
	RfConfigure(PWR_UP,1);
	//start a timer
	pause(5);
	//wait for timer

	/*FIXME: Implement state*/
	//set state

}

/*XmitInit: Initialize the rf circuit for transmitting*/
void XmitInit(void)
{
	BYTE address[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};

	RF_CE = 0;
	RF_CSN = 1;
	RF_SCK = 0;
	nop();

   /* select transmit, mask out interrupts */
   WriteRegister(CONFIG, 0x38);
   /* disable auto retransmit */
   WriteRegister(SETUP_RETR,0x00);
   /* address width = 5 bytes */
   WriteRegister(SETUP_AW,0x03);
   /* data rate = 1 MB, 0dBM */
   WriteRegister(RF_SETUP, 0x07);
   /* channel 2 */
   WriteRegister(RF_CH, 0x02);

   /* set address E7E7E7E7E7 */
   WriteAdrRegister(TX_ADDR, address, 5);
   WriteAdrRegister(RX_ADDR_P0, address, 5);

   /* disable auto retransmit*/
   WriteRegister(EN_AA, 0x00);

}

/*RecvInit: Initialize the rf circuit for receiving*/
void RecvInit(void)
{
	BYTE address[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
	RF_CE = 0;
	RF_CSN = 1;
	RF_SCK = 0;
	nop();

   /* select receive, mask out interrupts */
   WriteRegister(CONFIG, 0x39);
   /* disable auto-ack for all channels */
   WriteRegister(EN_AA,0x00);
   /* address width = 5 bytes */
   WriteRegister(SETUP_AW,0x03);
   /* data rate = 1 MB */
   WriteRegister(RF_SETUP, 0x07);
   /* 1 byte payload width expected */
   WriteRegister(RX_PW_P0, 0x01);
   /* channel 2 */
   WriteRegister(RF_CH, 0x02);
   /* set address E7E7E7E7E7 */
   WriteAdrRegister(TX_ADDR, address, 5);
   /* PWR_UP = 1 */
   WriteRegister(CONFIG, 0x3B);

   RfConfigure(PWR_UP, 1);
   pause(5);
}

/*XmitPacket: send a data byte/block to the rf module*/
void XmitPacket(BYTE data)
{
	RF_CE = 0;
	
	/* clear previous intrupts */
	WriteRegister(STATUS,(STATUS_MAX_RT + STATUS_RX_DR + STATUS_TX_DS));

	WriteRegister(CONFIG, 0x3A); /* PWR_UP = 1, interrupts masked, transmit mode */

	OutCommand(FLUSH_TX);

	OutCommandByte(W_TX_PAYLOAD, data);

	PulseCe(); /*Pulse Chip Enable to do RF*/
}

/*RecvPacket: check/receive an packet from the rf module*/
BYTE RecvPacket(BYTE *data)
{
	/* make sure data is present */
	if (!CheckInterrupt(STATUS_RX_DR))
		return (0);

	RF_CE = 0;
	/* read RX payload (repeat for as many incoming bytes) */
	*data = OutCommandByte(R_RX_PAYLOAD, 0xFF);

	OutCommand(FLUSH_RX);
	WriteRegister(STATUS, STATUS_RX_DR); /*reset intrupts*/

	RF_CE = 1;
	pause(5);
	return (1);
}

/*XmitPacket: send a data byte/block to the rf module*/
BYTE XmitPacket2(BYTE *data, BYTE length)
{
	RF_CE = 0;
	//early return if packet length is too long
	if (length>32)
		return 1;
	//load tx fifo
	OutCommandData(W_TX_PAYLOAD, data, length);
	//send packet
	PulseCe();

	while(1)
	{
		if(CheckInterrupt(STATUS_MAX_RT))
			return 2;
		if(CheckInterrupt(STATUS_TX_DS))
			return 0;
	}
	/*FIXME:implement state*/
	//set state
}

/*RecvPacket: check/receive an packet from the rf module*/
BYTE RecvPacket2(BYTE *payload)
{
   /* make sure data is present */
   if (!CheckInterrupt(STATUS_RX_DR))
	   return(1);

   /* read RX payload, which is 'length' bytes long*/
   payload = OutCommandData(R_RX_PAYLOAD, payload, 32);

   OutCommand(FLUSH_RX);				/* Flush RX FIFO */

   WriteRegister(STATUS, STATUS_RX_DR); /* reset interrupt */

   return(0);
}

//Write individual bits of the configure register
void RfConfigure(BYTE bitNum, BYTE enable)
{
	BYTE status;
	status = OutCommand(nop);
	if (enable)
		status |=  bitNum;
	else
		status &= ~bitNum;
	WriteRegister(CONFIG, status);
}

BYTE CheckInterrupt(BYTE intrupt)
{
	BYTE status;
	if (RF_IRQ)
		return 0;
	status = OutCommand(NOP);
	return (intrupt & status) ? (1):(0);
}

void PulseCe(void)
{
	BYTE count = 0;
	RF_CE = 1;		 /* pulse CE to start transmission (1 msec) */
	for (count=0; count<150; count++)
		nop();
	RF_CE = 0;
}

/*pause(max): delay for "max" times .5 msec*/
void pause(BYTE max)
{
   BYTE ofst;
   BYTE count;
   for(count = 0; count < max; count++)
   {
	   /* this delays for .5 msec */
	   for (ofst=0; ofst<97; ofst++)
		  nop();
   }
}

void nop(void)
{
}


