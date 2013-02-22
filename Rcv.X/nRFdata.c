#include "rf.h"
/*Contains low level functions to transfer
 *data between micro and nRF24L01+
 *NOTE:consider this unfinished
 */


/*OutByte(data): byte is clocked out to the RF module,
 *while dataIn is clocked in and returned */
BYTE OutInByte(BYTE byte)
{
	BYTE ofst;
	BYTE dataIn = 0;

   /* clock out the data from msb to lsb */
   for (ofst=0; ofst<8; ofst++)
   {
      if ((byte & 0x80) != 0)
         RF_MOSI = 1;
      else
         RF_MOSI = 0;

      /* while the data is clocked out, the chip returns data*/
      RF_SCK = 1;
      dataIn <<= 1;
      dataIn |= RF_MISO;
      RF_SCK = 0;

      byte <<= 1;
   }
   return(dataIn);
}

/*Reads length bytes of data from nRF into data[]
 *Sends 0xFF(NOP) in case nRF isn't writing*/
void InData(BYTE data[], BYTE length)
{
	BYTE count;
	for (count = 0; count < length; count++)
		data[count] = OutInByte(0xFF);
}

/*Writes length bytes from *data to MOSI port*/
void OutData(BYTE *data, BYTE length)
{
	BYTE index = 0;
	for(index = 0; index < length; index++)
	{
		OutInByte(data[index]);
	}
}

/*COMMAND FUNCTIONS
 *These functions send 1, 2, and multi-byte commands respectively*/
BYTE OutCommand(BYTE command)
{
	BYTE status;
	RF_CSN = 0;
	status = OutInByte(command);
	RF_CSN = 1;
	return status;
}
BYTE OutCommandByte(BYTE command, BYTE byte)
{
	BYTE inByte;
	RF_CSN = 0;
	OutInByte(command);
	inByte = OutInByte(byte);
	RF_CSN = 1;
	return inByte;
}
BYTE OutCommandData(BYTE command, BYTE *data, BYTE dataLength)
{
	BYTE status;
	RF_CSN = 0;
	status = OutInByte(command);
	OutData(data, dataLength);
	RF_CSN = 1;
	return status;
}

/*Register IO function
 These functions will write & read the nRF's one byte registers
 WriteAdrRegister will write the 5 byte "device address" registers
 */
BYTE WriteRegister(BYTE reg, BYTE byte)
{
	BYTE command;
	//early return if non-existant register is provided
	//FIXME: disallow address registers
	if ((reg > MAX_REGISTER_ADDRESS) && (reg != FEATURE) && (reg != DYNPD))
		return (1);

	command = W_REGISTER + reg;
	OutCommandByte(command, byte);
	return (0);
}
BYTE WriteAdrRegister(BYTE reg, BYTE data[], BYTE length)
{
	BYTE command;
	//Early return if address register is not selected or address length is too long
	if((reg != RX_ADDR_P0) || (reg != RX_ADDR_P1) || (reg != TX_ADDR) || length > 5)
		return (1);

	//Create command to write given register, send command
	command = W_REGISTER + reg;
	OutCommandData(command, data, length);
	return (0);
}
BYTE ReadRegister(BYTE reg)
{
	BYTE command;
	//FIXME: return error if and 'ADRESS' register is provided
	//return error if non-existant register is given
	if ((reg > MAX_REGISTER_ADDRESS) && (reg != FEATURE) && (reg != DYNPD))
		return (0);

	command = R_REGISTER + reg;
	return OutCommandByte(command,0xFF);
}
//FIXME: make a ReadAdrRegister function

/*Convenience functions to read and write payloads*/
void ReadRxPayload(BYTE data[], BYTE length)
{
	RF_CSN = 0;
	OutInByte(R_RX_PAYLOAD);
	InData(data, length);
	RF_CSN = 1;
}
void WriteTxPayload(BYTE data[], BYTE length)
{
	OutCommandData(W_TX_PAYLOAD, data, length);
}