#ifndef _I2C_H_
#define _I2C_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <inc/hw_memmap.h>
#include <inc/hw_ints.h>
#include <driverlib/sysctl.h>
#include <driverlib/gpio.h>
#include <driverlib/i2c.h>
#include <driverlib/pin_map.h>

#include "pin.h"


#define I2C0 &_i2c[0]
#define I2C1 &_i2c[1]
#define I2C2 &_i2c[2]
#define I2C3 &_i2c[3]

/*typedef enum
{
    I2C_IRQ_FLAGS
} tI2C_IRQ_Flag;
*/

typedef enum
{
    I2C_MODE_STD,
    I2C_MODE_FAST,
    I2C_MODE_FAST_PLUS,
    I2C_MODE_HIGH_SPEED,
    I2C_MODES
} tI2C_Mode;

typedef enum
{
    I2C_NO_ERR,
    I2C_IS_INITIALIZED

} tI2C_Err;

typedef struct
{
    unsigned long periph;
    unsigned long base;

    tPinName sda_pin;
    tPinName scl_pin;
    unsigned int sda_pin_mux;
    unsigned int scl_pin_mux;

    // Not going to mess with I2C Interrupts for now...
    /*unsigned int nvic_ndx;
    unsigned long irq_flags;
    void (*irq[UART_IRQ_FLAGS])( void );*/

    tI2C_Mode mode;

    bool isInit;
} tI2C;

extern tI2C _i2c[4];



tI2C_Err I2C_Init( tI2C* );
void I2C_SetMode( tI2C*, tI2C_Mode );

bool I2C_Write( tI2C*, unsigned char, unsigned char);
bool I2C_WriteBuff( tI2C*, unsigned char, unsigned char*, unsigned int );

bool I2C_Read( tI2C*, unsigned char, unsigned char* );
bool I2C_ReadBuff( tI2C*, unsigned char, unsigned char*, unsigned int );

// TODO: Implement config options
//  IE: Speed, Timeout
//bool i2c_config( tI2C* );

void I2C_Enable( tI2C* );
void I2C_Disable( tI2C* );



// Porting I2CDev
bool i2c_writeBit( tI2C*, uint8_t addr, uint8_t reg, uint8_t bitOffset, uint8_t data);
bool i2c_writeBitW( tI2C*, uint8_t addr, uint8_t reg, uint8_t bitOffset, uint16_t data);
bool i2c_writeByte( tI2C*, uint8_t addr, uint8_t reg, uint8_t data);
bool i2c_writeWord( tI2C*, uint8_t addr, uint8_t reg, uint16_t data);

bool i2c_writeBits( tI2C*, uint8_t addr, uint8_t reg, uint8_t bitOffset, uint8_t bitSize, uint8_t data);
bool i2c_writeBitsW( tI2C*, uint8_t addr, uint8_t reg, uint8_t bitOffset, uint8_t bitSize, uint16_t data);
bool i2c_writeBytes( tI2C*, uint8_t addr, uint8_t reg, uint8_t size, uint8_t* data);
bool i2c_writeWords( tI2C*, uint8_t addr, uint8_t reg, uint8_t size, uint16_t* data);


// TODO: Add Timeout???
//  Probably not... Can be a config option as a part of the device, not a parameter sent to the function
uint8_t i2c_readBit( tI2C*, uint8_t addr, uint8_t reg, uint8_t bitOffset, uint8_t* data);
uint8_t i2c_readBitW( tI2C*, uint8_t addr, uint8_t reg, uint8_t bitOffset, uint16_t* data);
uint8_t i2c_readByte( tI2C*, uint8_t addr, uint8_t reg, uint8_t* data);
uint8_t i2c_readWord( tI2C*, uint8_t addr, uint8_t reg, uint16_t* data);

uint8_t i2c_readBits( tI2C*, uint8_t addr, uint8_t reg, uint8_t bitOffset, uint8_t bitSize, uint8_t* data);
uint8_t i2c_readBitsW( tI2C*, uint8_t addr, uint8_t reg, uint8_t bitOffset, uint8_t bitSize, uint16_t* data);
uint8_t i2c_readBytes( tI2C*, uint8_t addr, uint8_t reg, uint8_t size, uint8_t* data);
uint8_t i2c_readWords( tI2C*, uint8_t addr, uint8_t reg, uint8_t size, uint16_t* data);


#ifdef __cplusplus
}
#endif

#endif
