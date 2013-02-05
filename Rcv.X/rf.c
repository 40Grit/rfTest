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

void RfShockBurstRxInit(void)
{
	BYTE addr[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};

	RF_CE = 0;
	RF_CSN = 1;
	RF_SCK = 0;
	nop();

	WriteRegister(CONFIG, CONFIG_SHOCK_BURST_RX);
	
	WriteRegister(EN_AA, EN_AA_P0_ON);		//Enable Auto Acknowledge
	
	WriteRegister(SETUP_RETR, 0x0F);		//Set retransmit # and time

	OutCommand_(ACTIVATE, 0x73);				//Activate Extra Features

	WriteRegister(FEATURE, 0x04);			//Enable Dynamic payload feature
	WriteRegister(DYNPD, 0x01);				//Enable Dynamic payload on pipe 0

	WriteAdrRegister(RX_ADDR_P0, addr, 5);	//Set Rx address
	WriteAdrRegister(TX_ADDR, addr, 5);		//Set Tx address
}

void RfShockBurstTxInit(void)
{
	BYTE addr[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};

	RF_CE = 0;
	RF_CSN = 1;
	RF_SCK = 0;
	nop();

	WriteRegister(CONFIG, CONFIG_SHOCK_BURST_TX);

	WriteRegister(EN_AA, EN_AA_P0_ON);		//Enable Auto Acknowledge

	WriteRegister(SETUP_RETR, 0x0F);		//Set retransmit # and time

	OutCommand_(ACTIVATE, 0x73);			//Activate Extra Features

	WriteRegister(FEATURE, 0x04);			//Enable Dynamic payload feature
	WriteRegister(DYNPD, 0x01);				//Enable Dynamic payload on pipe 0

	WriteAdrRegister(RX_ADDR_P0, addr, 5);	//Set Rx address
	WriteAdrRegister(TX_ADDR, addr, 5);		//Set Tx address
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

   RF_CE = 1;
}

/*XmitPacket: send a data byte/block to the rf module*/
void XmitPacket(BYTE data)
{
	WriteRegister(STATUS,0x70);  /* clear previous ints */

	WriteRegister(CONFIG, 0x3A); /* PWR_UP = 1, interrupts masked, transmit mode */

	OutCommand(FLUSH_TX);

	OutCommand_(W_TX_PAYLOAD, data);

   RF_CE = 1;		 /* pulse CE to start transmission (1 msec) */
   for (data=0; data<150; data++)
      nop();
   RF_CE = 0;
}

/*XmitPacket: send a data byte/block to the rf module*/
void XmitPacket2(BYTE data[], BYTE length)
{
	BYTE count;
	
	WriteRegister(STATUS, 0x70);	/* clear previous ints */

	WriteRegister(CONFIG, 0x3A);	/* PWR_UP = 1, interrupts masked, transmit mode */

	OutCommand(FLUSH_TX);			/* clear TX fifo */

	OutCommand(W_TX_PAYLOAD);
	OutData(data, length);			/* (payload) data to be sent (can send additional bytes) */

	RF_CE = 1;		 /* pulse CE to start transmission (1 msec) */
	for (count=0; count<150; count++)
		nop();
	RF_CE = 0;
}

/*RecvPacket: check/receive an packet from the rf module*/
BYTE RecvPacket(BYTE *data)
{
   /* make sure data is present */
	if (!CheckInterrupt(STATUS_RX_DR))
		return(0);

	/* read RX payload (repeat for as many incoming bytes) */
	*data = OutCommand_(R_RX_PAYLOAD, 0x00);

	OutCommand(FLUSH_RX);

	WriteRegister(STATUS, 0x40); /*reset intrupts*/

   return(1);
}

/*RecvPacket: check/receive an packet from the rf module*/
BYTE RecvPacket2(BYTE *payload)
{
   BYTE length;

   /* make sure data is present */
   if (RF_IRQ != 0)
      return(0);

   if (!CheckInterrupt(STATUS_RX_DR))
	   return(0);

   /*get length of packet*/
   //TODO: figure out when length is returned
   length = OutCommand_(R_RX_PL_WID, 0x00);

   /* read RX payload, which is 'length' bytes long*/
   OutCommand(R_RX_PAYLOAD);
   InData(payload,length);

   OutCommand(FLUSH_RX);    /* Flush RX FIFO */

   WriteRegister(STATUS, STATUS_RX_DR); /* reset interrupt */

   return(length);
}

BYTE CheckInterrupt(BYTE intrupt)
{
	BYTE status;
	status = OutCommand(NOP);
	return (intrupt & status) ? (1):(0);
}

/*pause(max): delay for "max" times .5 msec*/
void pause(BYTE max)
{
   BYTE ofst;

   /* this delays for .5 msec */
   for (ofst=0; ofst<97; ofst++)
      nop();
}

void nop(void)
{
}


