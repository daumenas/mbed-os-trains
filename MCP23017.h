/* MCP23017 - drive the Microchip MCP23017 16-bit Port Extender using I2C
* Copyright (c) 2010 Wim Huiskamp, Romilly Cocking (original version for SPI)
*
* Changed by Jacco van Splunter:
* The _read and _write functions are moved from protected -> public
*
* Released under the MIT License: http://mbed.org/license/mit
*
* version 0.2 Initial Release
* version 0.3 Cleaned up
* version 0.4 Fixed problem with _read method
* version 0.5 Added support for 'Banked' access to registers
*/
#include "mbed.h"

#ifndef  MCP23017_H
#define  MCP23017_H

// All register addresses assume IOCON.BANK = 0 (POR default)
#define IODIRA   0x00
#define IODIRB   0x01
#define IPOLA    0x02
#define IPOLB    0x03
#define GPINTENA 0x04
#define GPINTENB 0x05
#define DEFVALA  0x06
#define DEFVALB  0x07
#define INTCONA  0x08
#define INTCONB  0x09
#define IOCONA   0x0A
#define IOCONB   0x0B
#define GPPUA    0x0C
#define GPPUB    0x0D
#define INTFA    0x0E
#define INTFB    0x0F
#define INTCAPA  0x10
#define INTCAPB  0x11
#define GPIOA    0x12
#define GPIOB    0x13
#define OLATA    0x14
#define OLATB    0x15

// The following register addresses assume IOCON.BANK = 1 
#define IODIRA_BNK   0x00
#define IPOLA_BNK    0x01
#define GPINTENA_BNK 0x02
#define DEFVALA_BNK  0x03
#define INTCONA_BNK  0x04
#define IOCONA_BNK   0x05
#define GPPUA_BNK    0x06
#define INTFA_BNK    0x07
#define INTCAPA_BNK  0x08
#define GPIOA_BNK    0x09
#define OLATA_BNK    0x0A

#define IODIRB_BNK   0x10
#define IPOLB_BNK    0x11
#define GPINTENB_BNK 0x12
#define DEFVALB_BNK  0x13
#define INTCONB_BNK  0x14
#define IOCONB_BNK   0x15
#define GPPUB_BNK    0x16
#define INTFB_BNK    0x17
#define INTCAPB_BNK  0x18
#define GPIOB_BNK    0x19
#define OLATB_BNK    0x1A

// This array allows structured access to Port_A and Port_B registers for both bankModes
const int IODIR_AB[2][2]   = {{IODIRA, IODIRB},     {IODIRA_BNK, IODIRB_BNK}};
const int IPOL_AB[2][2]    = {{IPOLA, IPOLB},       {IPOLA_BNK, IPOLB_BNK}};
const int GPINTEN_AB[2][2] = {{GPINTENA, GPINTENB}, {GPINTENA_BNK, GPINTENB_BNK}};
const int DEFVAL_AB[2][2]  = {{DEFVALA, DEFVALB},   {DEFVALA_BNK, DEFVALB_BNK}};
const int INTCON_AB[2][2]  = {{INTCONA, INTCONB},   {INTCONA_BNK, INTCONB_BNK}};
const int IOCON_AB[2][2]   = {{IOCONA, IOCONB},     {IOCONA_BNK, IOCONB_BNK}};
const int GPPU_AB[2][2]    = {{GPPUA, GPPUB},       {GPPUA_BNK, GPPUB_BNK}};
const int INTF_AB[2][2]    = {{INTFA, INTFB},       {INTFA_BNK, INTFB_BNK}};
const int INTCAP_AB[2][2]  = {{INTCAPA, INTCAPB},   {INTCAPA_BNK, INTCAPB_BNK}};
const int GPIO_AB[2][2]    = {{GPIOA, GPIOB},       {GPIOA_BNK, GPIOB_BNK}};
const int OLAT_AB[2][2]    = {{OLATA, OLATB},       {OLATA_BNK, OLATB_BNK}};


// Control settings
#define IOCON_BANK      0x80 // Banked registers for Port A and B
#define IOCON_BYTE_MODE 0x20 // Disables sequential operation, Address Ptr does not increment
                             //   If Disabled and Bank = 0, operations toggle between Port A and B registers
                             //   If Disabled and Bank = 1, operations do not increment registeraddress                             
#define IOCON_HAEN      0x08 // Hardware address enable

#define INTERRUPT_POLARITY_BIT 0x02
#define INTERRUPT_MIRROR_BIT   0x40

#define PORT_DIR_OUT   0x00
#define PORT_DIR_IN    0xFF

enum Polarity { ACTIVE_LOW , ACTIVE_HIGH };
enum Port { PORT_A=0, PORT_B=1 };
enum Bank { NOT_BNK=0, BNK=1 };

class MCP23017 {
public:
    /** Create an MCP23017 object connected to the specified I2C object and using the specified deviceAddress
    *
    * @param I2C &i2c the I2C port to connect to 
    * @param char deviceAddress the address of the MCP23017
    */
    MCP23017(I2C &i2c, char deviceAddress);

    /** Set I/O direction of specified MCP23017 Port
    *
    * @param Port Port address (Port_A or Port_B)
    * @param char direction pin direction (0 = output, 1 = input)
    */
    void direction(Port port, char direction);

    /** Set Pull-Up Resistors on specified MCP23017 Port
    *
    * @param Port Port address (Port_A or Port_B)
    * @param char offOrOn per pin (0 = off, 1 = on)
    */    
    void configurePullUps(Port port, char offOrOn);

    void configureBanked(Bank bankmode);  
    void interruptEnable(Port port, char interruptsEnabledMask);
    void interruptPolarity(Polarity polarity);
    void mirrorInterrupts(bool mirror);
    void defaultValue(Port port, char valuesToCompare);
    void interruptControl(Port port, char interruptControlBits);

    /** Read from specified MCP23017 Port
    *
    * @param Port Port address (Port_A or Port_B)
    * @returns data from Port 
    */
    char read(Port port);
    
    /** Write to specified MCP23017 Port
    *
    * @param Port Port address (Port_A or Port_B)
    * @param char byte data to write
    */    
    void write(Port port, char byte);
    
    /** Write to specified MCP23017 register
    *
    * @param char address the internal registeraddress of the MCP23017
    */
    void _write(char address, char byte);

    /** Read from specified MCP23017 register
    *
    * @param char address the internal registeraddress of the MCP23017
    * @returns data from register 
    */
    char _read(char address);

protected:
    I2C &_i2c;
    char _readOpcode;
    char _writeOpcode;
    Bank _bankMode; 
    
    /** Init MCP23017
    *
    * @param
    * @returns 
    */    
    void _init();
};

#endif