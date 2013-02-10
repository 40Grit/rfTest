#include "rf.h"

BYTE InByte(void)
{
	BYTE ofst, data=0;

	/* clock in the data from msb to lsb */
	for (ofst=0; ofst<8; ofst++)
	{
		RF_SCK = 1;
		data <<= 1;
		data |= RF_MISO;
		RF_SCK = 0;
   }
	return data;
}
void InData(BYTE data[], BYTE length)
{
	BYTE count;
	for (count = 0; count < length; count++)
		data[count] = InByte();
}
void ReadRxPayload(BYTE data[], BYTE length)
{
	RF_CSN = 0;
	OutByte(R_RX_PAYLOAD);
	InData(data, length);
	RF_CSN = 1;
}

/*OutByte(data): copy one byte of daya to the rf module*/
BYTE OutByte(BYTE byte)
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

      /* while the data is clocked out, the chip returns its status */
      RF_SCK = 1;
      dataIn <<= 1;
      dataIn |= RF_MISO;
      RF_SCK = 0;

      byte <<= 1;
   }

   return(dataIn);
}

/*FIXME: PROBABLY RETURNS A POINTER TO A RANDOM LOCATION*/
BYTE* OutData(BYTE *data, BYTE length)
{
	BYTE index = 0;
	BYTE *response;
	
	for(index = 0; index < length; index++)
	{
		response[index] = OutByte(data[index]);
	}

	return response;
}
BYTE OutCommand(BYTE command)
{
	BYTE dataIn;
	RF_CSN = 0;
	dataIn = OutByte(command);
	RF_CSN = 1;
	return dataIn;
}

BYTE OutCommandByte(BYTE command, BYTE byte)
{
	BYTE response;
	RF_CSN = 0;
	OutByte(command);
	response = OutByte(byte);
	RF_CSN = 1;
	return response;
}

BYTE* OutCommandData(BYTE command, BYTE *data, BYTE dataLength)
{
	BYTE *response;
	RF_CSN = 0;
	OutByte(command);
	response = OutData(data, dataLength);
	RF_CSN = 1;
	return response;
}

BYTE WriteRegister(BYTE reg, BYTE byte)
{
	BYTE command;
	//FIXME: return error if and 'ADRESS' register is provided
	//return error if non-existant register is given
	if ((reg > MAX_REGISTER_ADDRESS) && (reg != FEATURE) && (reg != DYNPD))
		return (0);

	command = W_REGISTER + reg;
	return OutCommandByte(command, byte);
}
BYTE* WriteAdrRegister(BYTE reg, BYTE data[], BYTE length)
{
	BYTE command;
	if((reg != RX_ADDR_P0) || (reg != RX_ADDR_P1) || (reg != TX_ADDR))
		return 0;

	//Create command to write given register, send command
	command = W_REGISTER + reg;
	return OutCommandData(command, data, length);
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
