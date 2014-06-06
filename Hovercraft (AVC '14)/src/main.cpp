#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pin.h"
#include "pwm.h"
#include "uart.h"

#include "mpu6050.h"


#define rLED  PWM1_5 
#define gLED  PWM1_7
#define bLED  PWM1_6


int main(void)
{
    // Clock (80MHz)
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);


    // Init peripherals
    //  UART0
    UART_Init(UART0);
    UART_Enable(UART0);
    setbuf(stdout, NULL);

    //  I2C0
    I2C_Init(I2C0);
    I2C_Enable(I2C0);

    //  PWM0/1
    PWM_Init(PWM1, 1000);
    PWM_Enable(rLED);
    PWM_Enable(gLED);
    PWM_Enable(bLED);
    PWM_Set(rLED, 0);
    PWM_Set(gLED, 0);
    PWM_Set(bLED, 0);
   
    
    // Init devices
    MPU6050 mpu6050 = MPU6050(I2C0);


    // NVIC
    IntMasterEnable();

    printf("\r-HI-\r\n\n");

    while(1);
}

