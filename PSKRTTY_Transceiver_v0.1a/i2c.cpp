/*

The routines bypass timer0 processing used by Wire library and talks directly to the metal.
Cannot use interupt driver I2C from another interupt - bad things happen

Routines adapted from  http://www.embedds.com/programming-avr-i2c-interface/

*/

#include <inttypes.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "i2c.h"

#define I2C_START 0x08
#define I2C_START_RPT 0x10
#define I2C_SLA_W_ACK 0x18
#define I2C_SLA_R_ACK 0x40
#define I2C_DATA_ACK 0x28
#define I2C_WRITE 0b11000000
#define I2C_READ  0b11000001

uint8_t i2cStart(void)
{
  TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);

  while (!(TWCR & (1<<TWINT))) ;

  return (TWSR & 0xF8);
}

void i2cStop(void)
{
  TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);

  while ((TWCR & (1<<TWSTO))) ;
}

uint8_t i2cByteSend(uint8_t data)
{
  TWDR = data;

  TWCR = (1<<TWINT) | (1<<TWEN);

  while (!(TWCR & (1<<TWINT))) ;

  return (TWSR & 0xF8);
}

uint8_t i2cByteRead(void)
{
  TWCR = (1<<TWINT) | (1<<TWEN);

  while (!(TWCR & (1<<TWINT))) ;

  return (TWDR);
}

uint8_t i2cSendRegister(uint8_t reg, uint8_t data)
{
  uint8_t stts;
  
  stts = i2cStart();
  if (stts != I2C_START) return 1;

  stts = i2cByteSend(I2C_WRITE);
  if (stts != I2C_SLA_W_ACK) return 2;

  stts = i2cByteSend(reg);
  if (stts != I2C_DATA_ACK) return 3;

  stts = i2cByteSend(data);
  if (stts != I2C_DATA_ACK) return 4;

  i2cStop();

  return 0;
}

uint8_t i2cSendRepeatedRegister(uint8_t reg, uint8_t bytes, uint8_t *data)
{
  uint8_t stts, i;
  
  stts = i2cStart();
  if (stts != I2C_START) return 1;

  stts = i2cByteSend(I2C_WRITE);
  if (stts != I2C_SLA_W_ACK) return 2;

  stts = i2cByteSend(reg);
  if (stts != I2C_DATA_ACK) return 3;

  for (i=0; i<bytes; i++) {
    stts = i2cByteSend(data[i]);
    if (stts != I2C_DATA_ACK) return 4; 
  }

  i2cStop();

  return 0;
}



uint8_t i2cReadRegister(uint8_t reg, uint8_t *data)
{
  uint8_t stts;
  
  stts = i2cStart();
  if (stts != I2C_START) return 1;

  stts = i2cByteSend(I2C_WRITE);
  if (stts != I2C_SLA_W_ACK) return 2;
 
  stts = i2cByteSend(reg);
  if (stts != I2C_DATA_ACK) return 3;

  stts = i2cStart();
  if (stts != I2C_START_RPT) return 4;

  stts = i2cByteSend(I2C_READ);
  if (stts != I2C_SLA_R_ACK) return 5;

  *data = i2cByteRead();

  i2cStop();

  return 0;
}

// Init TWI (I2C)
//
void i2cInit()
{
  TWBR = 92;            
  TWSR = 0;
  TWDR = 0xFF;
}
