/* MCP23017 - drive the Microchip MCP23017 16-bit Port Extender using I2C
* Copyright (c) 2010 Wim Huiskamp, Romilly Cocking (original version for SPI)
*
* Released under the MIT License: http://mbed.org/license/mit
*
* version 0.2 Initial Release
* version 0.3 Cleaned up
* version 0.4 Fixed problem with _read method
* version 0.5 Added support for 'Banked' access to registers
*/

#include "mbed.h"
#include "MCP23017.h"

/** Create an MCP23017 object connected to the specified I2C object and using the specified deviceAddress
*
* @param I2C &i2c the I2C port to connect to 
* @param char deviceAddress the address of the MCP23017
*/
MCP23017::MCP23017(I2C &i2c, char deviceAddress) : _i2c(i2c) {
    _writeOpcode = deviceAddress & 0xFE; // low order bit = 0 for write
    _readOpcode  = deviceAddress | 0x01; // low order bit = 1 for read
    _init();
}

/** Read from specified MCP23017 register
*
* @param char address the internal registeraddress of the MCP23017
* @returns data from register 
*/
char MCP23017::_read(char address) {
    char data[2];

    data[0] = address;
    _i2c.write(_writeOpcode, data, 1);     // Select Register for reading
    _i2c.read(_readOpcode, data, 1);       // Read from selected Register
    
    return data[0];
}


/** Write to specified MCP23017 register
*
* @param char address the internal registeraddress of the MCP23017
*/
void MCP23017::_write(char address, char byte) {
    char data[2];

    data[0] = address;
    data[1] = byte;
    _i2c.write(_writeOpcode, data, 2);    // Write data to selected Register
}


/** Init MCP23017
*
* @param
* @returns 
*/
void MCP23017::_init() {
  
  _bankMode = NOT_BNK;  // This may not be true after software reset without hardware reset !!!
  
  _write(IOCON_AB[_bankMode][PORT_A], (IOCON_BYTE_MODE | IOCON_HAEN )); // Hardware addressing on, no-autoincrement, 16 bit mode (operations toggle between A and B registers)

}

/** Set I/O direction of specified MCP23017 Port
*
* @param Port Port address (Port_A or Port_B)
* @param char direction pin direction (0 = output, 1 = input)
*/
void MCP23017::direction(Port port, char direction) {
    _write(IODIR_AB[_bankMode][port], direction);
}

/** Set Pull-Up Resistors on specified MCP23017 Port
*
* @param Port Port address (Port_A or Port_B)
* @param char offOrOn per pin (0 = off, 1 = on)
*/
void MCP23017::configurePullUps(Port port, char offOrOn) {
  
  _write(GPPU_AB[_bankMode][port], offOrOn); 
}

/** Configere the Banked or Non-Banked mode
*
* @param Bank bankMode
* @param char offOrOn per pin (0 = off, 1 = on)
*/
void MCP23017::configureBanked(Bank bankMode) {

    if (bankMode == NOT_BNK) {
      // Non-Banked sequential registers (default POR)
      // Hardware addressing on, , no-autoincrement, 16 bit mode (operations do toggle between A and B registers)          
      _write(IOCON_AB[_bankMode][PORT_A], (IOCON_BYTE_MODE | IOCON_HAEN ));
      _bankMode = NOT_BNK;
    }  
    else {
      // Banked registers
      // Hardware addressing on, no-autoincrement, 8 bit mode           
      _write(IOCON_AB[_bankMode][PORT_A], (IOCON_BANK | IOCON_BYTE_MODE | IOCON_HAEN ));
      _bankMode = BNK;
    }
}


void MCP23017::interruptEnable(Port port, char interruptsEnabledMask) {
  
  _write(GPINTEN_AB[_bankMode][port], interruptsEnabledMask);    
   
}

void MCP23017::mirrorInterrupts(bool mirror) {
  char iocon = _read(IOCON_AB[_bankMode][PORT_A]);

  if (mirror) {
    iocon = iocon | INTERRUPT_MIRROR_BIT;
  }
  else {
    iocon = iocon & ~INTERRUPT_MIRROR_BIT;
  }

  _write(IOCON_AB[_bankMode][PORT_A], iocon);

}

void  MCP23017::interruptPolarity(Polarity polarity) {
    char iocon = _read(IOCON_AB[_bankMode][PORT_A]);
    
    if (polarity == ACTIVE_LOW) {
        iocon = iocon & ~INTERRUPT_POLARITY_BIT;
    } else {
        iocon = iocon | INTERRUPT_POLARITY_BIT;
    }
    _write(IOCON_AB[_bankMode][PORT_A], iocon);
}

void MCP23017::defaultValue(Port port, char valuesToCompare) {
    
  _write(DEFVAL_AB[_bankMode][port], valuesToCompare);
    
}

void MCP23017::interruptControl(Port port, char interruptControlBits) {
    
  _write(INTCON_AB[_bankMode][port], interruptControlBits);
    
}

/** Write to specified MCP23017 Port
*
* @param Port Port address (Port_A or Port_B)
* @param char byte data to write
*/
void MCP23017::write(Port port, char byte) {
    _write(OLAT_AB[_bankMode][port], byte);
}
    
/** Read from specified MCP23017 Port
*
* @param Port Port address (Port_A or Port_B)
* @returns data from Port 
*/
char MCP23017::read(Port port) {
    return _read(GPIO_AB[_bankMode][port]);
}

