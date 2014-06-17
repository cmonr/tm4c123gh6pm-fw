#include "i2c.h"


tI2C _i2c[4] = 
{
    {SYSCTL_PERIPH_I2C0, I2C0_BASE, PB3, PB2, GPIO_PB3_I2C0SDA, GPIO_PB2_I2C0SCL, I2C_MODE_STD, false},
    {SYSCTL_PERIPH_I2C1, I2C1_BASE, PA7, PA6, GPIO_PA7_I2C1SDA, GPIO_PA6_I2C1SCL, I2C_MODE_STD, false},
    {SYSCTL_PERIPH_I2C2, I2C2_BASE, PE5, PE4, GPIO_PE5_I2C2SDA, GPIO_PE4_I2C2SCL, I2C_MODE_STD, false},
    {SYSCTL_PERIPH_I2C3, I2C3_BASE, PD1, PD0, GPIO_PD1_I2C3SDA, GPIO_PD0_I2C3SCL, I2C_MODE_STD, false}
};

tI2C_Err I2C_Init(tI2C* i2c)
{
     // Check if object already exists
    if (i2c -> isInit == true)
        return I2C_IS_INITIALIZED;

    // Power I2C Peripheral
    SysCtlPeripheralEnable(i2c -> periph);

    // Configure I2C
    I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), false);

    //  Set timeout
    I2CMasterTimeoutSet(I2C0_BASE, 0xFF);

    // Init complete
    i2c -> isInit = true;

    return I2C_NO_ERR;
}

void I2C_SetMode(tI2C* i2c, tI2C_Mode mode)
{
    switch(mode)
    {
        case I2C_MODE_STD:
            I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), false);
            break;
        case I2C_MODE_FAST:
            I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), false);
            break;
        case I2C_MODE_FAST_PLUS:
        case I2C_MODE_HIGH_SPEED:
            // Ignore for now...
            return;
        default:
            return;
    }

    i2c -> mode = mode;
}

void I2C_Enable(tI2C* i2c)
{
    // Enable Pin Port
    Pin_Init(i2c -> sda_pin);
    Pin_Init(i2c -> scl_pin);
    
    // Set GPIO Pin Mux
    GPIOPinTypeI2C(pins[i2c -> sda_pin].port.base, pins[i2c -> sda_pin].offset);
    GPIOPinTypeI2CSCL(pins[i2c -> scl_pin].port.base, pins[i2c -> scl_pin].offset);
    GPIOPinConfigure(i2c -> sda_pin_mux);
    GPIOPinConfigure(i2c ->scl_pin_mux);

    // Enable I2C
    I2CMasterEnable(i2c -> base);
}

void I2C_Disable(tI2C* i2c)
{
    // Disable I2C
    I2CMasterDisable(i2c -> base);

    // Set GPIO Pin
    GPIOPinTypeGPIOInput(pins[i2c -> sda_pin].port.base, pins[i2c -> sda_pin].offset);
    GPIOPinTypeGPIOInput(pins[i2c -> scl_pin].port.base, pins[i2c -> scl_pin].offset);
}


bool I2C_Write(tI2C* i2c, unsigned char addr, unsigned char data)
{
    I2CMasterSlaveAddrSet(i2c -> base, addr, false);
    I2CMasterDataPut(i2c -> base, data);
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);
    while(I2CMasterBusy(I2C0_BASE));

    return I2CMasterErr(i2c -> base) == I2C_MASTER_ERR_NONE;
}

bool I2C_WriteNoStop(tI2C* i2c, unsigned char addr, unsigned char data)
{
    I2CMasterSlaveAddrSet(i2c -> base, addr, false);
    I2CMasterDataPut(i2c -> base, data);
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);
    while(I2CMasterBusy(I2C0_BASE));

    return I2CMasterErr(i2c -> base) == I2C_MASTER_ERR_NONE;
}

bool I2C_WriteBuff(tI2C* i2c, unsigned char addr, unsigned char* data, unsigned int len)
{
    I2CMasterSlaveAddrSet(i2c -> base, addr, false);
    I2CMasterDataPut(i2c -> base, *data);
    if (len == 1)
    {
        I2CMasterControl(i2c -> base, I2C_MASTER_CMD_SINGLE_SEND);
        while(I2CMasterBusy(i2c -> base));

        return I2CMasterErr(i2c -> base) == I2C_MASTER_ERR_NONE;
    }
    
    
    // Start sending consecutive data
    I2CMasterControl(i2c -> base, I2C_MASTER_CMD_BURST_SEND_START);
    while(I2CMasterBusy(i2c -> base));
    if (I2CMasterErr(i2c -> base) != I2C_MASTER_ERR_NONE)
        return false;
    len--;
    data++;

    
    // Continue sending consecutive data
    while(len > 1){
        I2CMasterDataPut(i2c -> base, *data);
        I2CMasterControl(i2c -> base, I2C_MASTER_CMD_BURST_SEND_CONT);
        while(I2CMasterBusy(i2c -> base));
        if (I2CMasterErr(i2c -> base) != I2C_MASTER_ERR_NONE)
            return false;
        len--;
        data++;
    }

    // Send last piece of data
    I2CMasterDataPut(i2c -> base, *data);
    I2CMasterControl(i2c -> base, I2C_MASTER_CMD_BURST_SEND_FINISH);
    while(I2CMasterBusy(i2c -> base));
   
    return I2CMasterErr(i2c -> base) == I2C_MASTER_ERR_NONE;
}

bool I2C_WriteBuffNoStop(tI2C* i2c, unsigned char addr, unsigned char* data, unsigned int len)
{
    I2CMasterSlaveAddrSet(i2c -> base, addr, false);
    I2CMasterDataPut(i2c -> base, *data);
    
    
    // Start sending consecutive data
    I2CMasterControl(i2c -> base, I2C_MASTER_CMD_BURST_SEND_START);
    while(I2CMasterBusy(i2c -> base));
    if (I2CMasterErr(i2c -> base) != I2C_MASTER_ERR_NONE)
        return false;
    len--;
    data++;

    
    // Continue sending consecutive data
    while(len > 0){
        I2CMasterDataPut(i2c -> base, *data);
        I2CMasterControl(i2c -> base, I2C_MASTER_CMD_BURST_SEND_CONT);
        while(I2CMasterBusy(i2c -> base));
        if (I2CMasterErr(i2c -> base) != I2C_MASTER_ERR_NONE)
            return false;
        len--;
        data++;
    }
   
    return I2CMasterErr(i2c -> base) == I2C_MASTER_ERR_NONE;
}

bool I2C_WriteBuffToReg(tI2C* i2c, unsigned char addr, unsigned char reg, unsigned char* data, unsigned int len)
{
    // No data to send. Success!
    if(len == 0)
        return true;
    
    // Send device sddress
    I2CMasterSlaveAddrSet(i2c -> base, addr, false);
   
    // Send register
    I2CMasterDataPut(i2c -> base, reg);
    I2CMasterControl(i2c -> base, I2C_MASTER_CMD_BURST_SEND_START);
    while(I2CMasterBusy(i2c -> base));
    if (I2CMasterErr(i2c -> base) != I2C_MASTER_ERR_NONE)
        return false;
   
    // Send data
    while(len > 1){
        I2CMasterDataPut(i2c -> base, *data);
        I2CMasterControl(i2c -> base, I2C_MASTER_CMD_BURST_SEND_CONT);
        while(I2CMasterBusy(i2c -> base));
        if (I2CMasterErr(i2c -> base) != I2C_MASTER_ERR_NONE)
            return false;
        len--;
        data++;
    }

    // Finish sending data
    I2CMasterDataPut(i2c -> base, *data);
    I2CMasterControl(i2c -> base, I2C_MASTER_CMD_BURST_SEND_FINISH);
    while(I2CMasterBusy(i2c -> base));
   
    return I2CMasterErr(i2c -> base) == I2C_MASTER_ERR_NONE;
}

bool I2C_Read(tI2C* i2c, unsigned char addr, unsigned char* data)
{
    I2CMasterSlaveAddrSet(i2c -> base, addr, true);
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);
    while(I2CMasterBusy(I2C0_BASE));
    *data = I2CMasterDataGet(i2c -> base);

    return I2CMasterErr(i2c -> base) == I2C_MASTER_ERR_NONE;
}

bool I2C_ReadBuff(tI2C* i2c, unsigned char addr, unsigned char* data, unsigned int len)
{
    //while(len == 0);    // Halt program if len == 0
    if (len == 0)
        return false;    // For now...
    
    I2CMasterSlaveAddrSet(i2c -> base, addr, true);
    if (len == 1)
    {
        I2CMasterControl(i2c -> base, I2C_MASTER_CMD_SINGLE_RECEIVE);
        while(I2CMasterBusy(i2c -> base));
        *data = I2CMasterDataGet(i2c -> base);

        return I2CMasterErr(i2c -> base) == I2C_MASTER_ERR_NONE;
    }
    
    
    // Start receiving consecutive data
    I2CMasterControl(i2c -> base, I2C_MASTER_CMD_BURST_RECEIVE_START);
    while(I2CMasterBusy(i2c -> base));
    *data = I2CMasterDataGet(i2c -> base);
    if (I2CMasterErr(i2c -> base) != I2C_MASTER_ERR_NONE)
        return false;
    len--;
    data++;

    
    // Continue receiving consecutive data
    while(len > 1){
        I2CMasterControl(i2c -> base, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
        while(I2CMasterBusy(i2c -> base));
        *data = I2CMasterDataGet(i2c -> base);
        if (I2CMasterErr(i2c -> base) != I2C_MASTER_ERR_NONE)
            return false;
        len--;
        data++;
    }

    // Receive last piece of data
    I2CMasterControl(i2c -> base, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
    while(I2CMasterBusy(i2c -> base));
    *data = I2CMasterDataGet(i2c -> base);
   
    return I2CMasterErr(i2c -> base) == I2C_MASTER_ERR_NONE;
}




//
// I2Cdev functions (Modified)
//

bool i2c_writeBit(tI2C* i2c, uint8_t addr, uint8_t reg, uint8_t bitOffset, uint8_t data)
{
    uint8_t b = 0;  // Initialize to silence compiler
    i2c_readByte(i2c, addr, reg, &b);
    b = (data != 0) ? (b | (1 << bitOffset)) : (b & ~(1 << bitOffset));
    return i2c_writeByte(i2c, addr, reg, b);

}

bool i2c_writeBitW(tI2C* i2c, uint8_t addr, uint8_t reg, uint8_t bitOffset, uint16_t data)
{
    uint16_t w = 0; // Initialize to silence compiler
    i2c_readWord(i2c, addr, reg, &w);
    w = (data != 0) ? (w | (1 << bitOffset)) : (w & ~(1 << bitOffset));
    return i2c_writeWord(i2c, addr, reg, w);
}

bool i2c_writeByte(tI2C* i2c, uint8_t addr, uint8_t reg, uint8_t data)
{
    return i2c_writeBytes(i2c, addr, reg, 1, &data);
}

bool i2c_writeWord(tI2C* i2c, uint8_t addr, uint8_t reg, uint16_t data)
{
    return i2c_writeWords(i2c, addr, reg, 1, &data);
}


bool i2c_writeBits(tI2C* i2c, uint8_t addr, uint8_t reg, uint8_t bitOffset, uint8_t bitSize, uint8_t data)
{
    //      010 value to write
    // 76543210 bit numbers
    //    xxx   args: bitOffset=4, bitSize=3
    // 00011100 mask byte
    // 10101111 original value (sample)
    // 10100011 original & ~mask
    // 10101011 masked | value
    uint8_t b;
    if (i2c_readByte(i2c, addr, reg, &b) != 0) {
        uint8_t mask = ((1 << bitSize) - 1) << (bitOffset - bitSize + 1);
        data <<= (bitOffset - bitSize + 1); // shift data into correct position
        data &= mask; // zero all non-important bits in data
        b &= ~(mask); // zero all important bits in existing byte
        b |= data; // combine data with existing byte
        return i2c_writeByte(i2c, addr, reg, b);
    } else {
        return false;
    }
}

bool i2c_writeBitsW(tI2C* i2c, uint8_t addr, uint8_t reg, uint8_t bitOffset, uint8_t bitSize, uint16_t data)
{
    //              010 value to write
    // fedcba9876543210 bit numbers
    //    xxx           args: bitOffset=12, bitSize=3
    // 0001110000000000 mask word
    // 1010111110010110 original value (sample)
    // 1010001110010110 original & ~mask
    // 1010101110010110 masked | value
    uint16_t w;
    if (i2c_readWord(i2c, addr, reg, &w) != 0) {
        uint16_t mask = ((1 << bitSize) - 1) << (bitOffset - bitSize + 1);
        data <<= (bitOffset - bitSize + 1); // shift data into correct position
        data &= mask; // zero all non-important bits in data
        w &= ~(mask); // zero all important bits in existing word
        w |= data; // combine data with existing word
        return i2c_writeWord(i2c, addr, reg, w);
    } else {
        return false;
    }
}

bool i2c_writeBytes(tI2C* i2c, uint8_t addr, uint8_t reg, uint8_t size, uint8_t* data)
{
    return I2C_WriteBuffToReg(i2c, addr, reg, data, size);
}

bool i2c_writeWords(tI2C* i2c, uint8_t addr, uint8_t reg, uint8_t size, uint16_t* data)
{
    // TODO: Verify that this acutally works.
    //  Should work due to to the MCU being little endian
    return I2C_WriteBuffToReg(i2c, addr, reg, (uint8_t*) data, size*2);
}



uint8_t i2c_readBit(tI2C* i2c, uint8_t addr, uint8_t reg, uint8_t bitOffset, uint8_t* data)
{
    uint8_t b = 0; // Initialize to silence compiler
    uint8_t count = i2c_readByte(i2c, addr, reg, &b);
    *data = b & (1 << bitOffset);
    return count;

}

uint8_t i2c_readBitW(tI2C* i2c, uint8_t addr, uint8_t reg, uint8_t bitOffset, uint16_t* data)
{
    uint16_t b = 0; // Initialize to silence compiler
    uint8_t count = i2c_readWord(i2c, addr, reg, &b);
    *data = b & (1 << bitOffset);
    return count;

}

uint8_t i2c_readByte(tI2C* i2c, uint8_t addr, uint8_t reg, uint8_t* data)
{
    return i2c_readBytes(i2c, addr, reg, 1, data);
}

uint8_t i2c_readWord(tI2C* i2c, uint8_t addr, uint8_t reg, uint16_t* data)
{
    return i2c_readWords(i2c, addr, reg, 1, data);
}


uint8_t i2c_readBits(tI2C* i2c, uint8_t addr, uint8_t reg, uint8_t bitOffset, uint8_t bitSize, uint8_t* data)
{
    // 01101001 read byte
    // 76543210 bit numbers
    //    xxx   args: bitOffset=4, bitSize=3
    //    010   masked
    //   -> 010 shifted
    uint8_t count, b;
    if ((count = i2c_readByte(i2c, addr, reg, &b)) != 0) {
        uint8_t mask = ((1 << bitSize) - 1) << (bitOffset - bitSize + 1);
        b &= mask;
        b >>= (bitOffset - bitSize + 1);
        *data = b;
    }
    return count;

}

uint8_t i2c_readBitsW(tI2C* i2c, uint8_t addr, uint8_t reg, uint8_t bitOffset, uint8_t bitSize, uint16_t* data)
{
    // 1101011001101001 read byte
    // fedcba9876543210 bit numbers
    //    xxx           args: bitOffset=12, bitSize=3
    //    010           masked
    //           -> 010 shifted
    uint8_t count;
    uint16_t w;
    if ((count = i2c_readWord(i2c, addr, reg, &w)) != 0) {
        uint16_t mask = ((1 << bitSize) - 1) << (bitOffset - bitSize + 1);
        w &= mask;
        w >>= (bitOffset - bitSize + 1);
        *data = w;
    }
    return count;

}

uint8_t i2c_readBytes(tI2C* i2c, uint8_t addr, uint8_t reg, uint8_t size, uint8_t* data)
{
    I2C_WriteNoStop(i2c, addr, reg);

    // TODO: If repeated start is needed, we need to fix it here...
    if (I2C_ReadBuff(i2c, addr, data, size))
        return size;
    return 0;
}

uint8_t i2c_readWords(tI2C* i2c, uint8_t addr, uint8_t reg, uint8_t size, uint16_t* data)
{
    // TODO: Actually implement this...
    return 0;
}
