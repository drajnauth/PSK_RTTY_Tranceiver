#ifndef I2C_H
#define I2C_H

#include <inttypes.h>

void i2cInit();
uint8_t i2cSendRegister(uint8_t reg, uint8_t data);
uint8_t i2cReadRegister(uint8_t reg, uint8_t *data);
uint8_t i2cSendRepeatedRegister(uint8_t reg, uint8_t bytes, uint8_t *data);

uint8_t i2cStart(void);
void i2cStop(void);
uint8_t i2cByteSend(uint8_t data);
uint8_t i2cByteRead(void);


#endif //I2C_H
