/*rf.h
 *1/31/2013
 */

//pic18f250 - http://ww1.microchip.com/downloads/en/DeviceDoc/39631E.pdf
#include <p18f2520.h>

/* hardware control lines */
  /*SPI*/
          //symbol   -    Port        -   Pin      -     Purpose          -   Direction
    #define RF_CE      PORTAbits.RA0      //2           RF Radio Enable         output
    #define RF_CSN     PORTAbits.RA1      //3           SPI Chip Select         output
    #define RF_SCK     PORTAbits.RA2      //4           SPI Clock               output
    #define RF_MOSI    PORTAbits.RA3      //5           SPI Tx                  output
    #define RF_MISO    PORTAbits.RA4      //6           SPI Rx                  input
    #define RF_IRQ     PORTAbits.RA5      //7           RF Radio Interrupt      input



#define BYTE unsigned char
#define WORD unsigned int


/*************COMMAND DEFINITIONS***********************/
//Refer to page chapter 8 (pg.46) of data sheet for functionality
#define R_RX_PAYLOAD        0x61
#define W_TX_PAYLOAD        0xA0
#define FLUSH_TX            0xE1
#define FLUSH_RX            0xE2
#define REUSE_TX_PL         0xE3
#define ACTIVATE            0x50    //followed by 0x73
#define R_RX_PL_WID         0x60
#define W_ACK_PAYLOAD       10101   //choose pipe
#define W_TX_PAYLOAD_NO_ACK 0xB0
#define NOP                 0xFF


#define R_REGISTER          0x00
#define W_REGISTER          0x20

//Register addresses
#define CONFIG              0x00
    #define MASK_RX_DR      6
    #define MASK_TX_DS      5
    #define MASK_MAX_RT     4
    #define EN_CRC          3
    #define CRCO            2
    #define PWR_UP          1
    #define PRIM_RX         0
        #define MODE_TX         0
        #define MODE_RX         1

    #define CONFIG_ENHANCED_SHOCKBURST 0b00001100
#define EN_AA               0x01
    #define EN_AA_P0_ON 0b00000001
#define EN_RXADDR           0x02
#define SETUP_AW            0x03
#define SETUP_RETR          0x04
#define RF_CH               0x05
#define RF_SETUP            0x06
#define STATUS              0x07
    #define STATUS_RX_DR 0x40
    #define STATUS_TX_DS 0x20
    #define STATUS_MAX_RT 0x10
#define OBSERVE_TX          0x08
#define CD                  0x09

#define RX_ADDR_P0          0x0A
#define RX_ADDR_P1          0x0B
#define RX_ADDR_P2          0x0C
#define RX_ADDR_P3          0x0D
#define RX_ADDR_P4          0x0E
#define RX_ADDR_P5          0x0F

#define TX_ADDR             0x10

#define RX_PW_P0            0x11
#define RX_PW_P1            0x12
#define RX_PW_P2            0x13
#define RX_PW_P3            0x14
#define RX_PW_P4            0x15
#define RX_PW_P5            0x16
#define FIFO_STATUS         0x17
#define MAX_REGISTER_ADDRESS 0x17

#define DYNPD               0x1C
#define FEATURE             0x1D



/* function prototypes */
void RfPicInit(void);
void XmitInit(void);
void RecvInit(void);
void RfShockBurstInit(BYTE address[], BYTE mode, BYTE payloadWidth);


BYTE CheckInterrupt(BYTE intrupt);
void pause(BYTE max);
BYTE calc(BYTE ofst, BYTE chan);
void nop(void);

BYTE XmitPacket(BYTE *data, BYTE length);
BYTE RecvPacket(BYTE *payload, BYTE length);


/*Data Operation Prototypes*/
BYTE OutInByte(BYTE data);
void OutData(BYTE data[], BYTE length);
BYTE OutCommand(BYTE command);
BYTE OutCommandByte(BYTE command, BYTE byte);
BYTE OutCommandData(BYTE command, BYTE data[], BYTE dataLength);
BYTE WriteRegister(BYTE reg, BYTE byte);
BYTE WriteAdrRegister(BYTE reg, BYTE data[], BYTE length);
BYTE ReadRegister(BYTE reg);
BYTE InByte(void);
void InData(BYTE data[], BYTE length);
void PulseCe(void);
void RfConfigure(BYTE bitNum, BYTE enable);
void ReadRxPayload(BYTE data[], BYTE length);
void WriteTxPayload(BYTE data[], BYTE length);
