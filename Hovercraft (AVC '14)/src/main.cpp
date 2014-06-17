#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "driverlib/sysctl.h"

#include "pin.h"
#include "pwm.h"
#include "uart.h"

#include "mpu6050.h"


#define rLED  PF1

int main(void)
{
    // Clock (80MHz)
    SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);


    // Init peripherals
    //  UART0
    UART_Init(UART0);
    UART_Enable(UART0);
    setbuf(stdout, NULL);

    //  I2C0
    I2C_Init(I2C0);
    I2C_Enable(I2C0);

    //  PWM0/1
    Pin_Init(rLED);
    Pin_Set(rLED, LOW);
   
    // NVIC
    IntMasterEnable();


    
    // Initialize Drivers
    MPU6050 mpu6050 = MPU6050(I2C0);



    // Initalize MPU6050
    printf("Initialize MPU6050...\r\n");
    mpu6050.initialize();
    printf(" Initialized.\r\n");

    //  Verify connection
    printf("Testing I2C <> MPU6050 connection...\r\n");
    if (mpu6050.testConnection() == false)
    {
        printf(" Failed.\r\n");
        while(1);   // Block here
    }
    printf(" Success!\r\n");

    //  Initialize DMP
    printf(" Initializing DMP...\r\n");

    switch(mpu6050.dmpInitialize())
    {
    case 0:
        break;
    case 1:
        printf("  DMP Memory Load Failed.\r\n");
        while(1);
        break;
    case 2:
        printf("  DMP Config Updates Failed.");
        while(1);
        break;
    }

    mpu6050.setXGyroOffset(220);
    mpu6050.setYGyroOffset(76);
    mpu6050.setZGyroOffset(-85);
    mpu6050.setZAccelOffset(1788);
    
    printf(" Enabling DMP...\r\n");
    mpu6050.setDMPEnabled(true);
    printf(" DMP ready!\r\n");


    // Leave message on screen for 10s
    SysCtlDelay(SysCtlClockGet() / 3 * 10);

    while(1)
    {
        Quaternion q;  
        uint8_t fifoBuffer[64];

        if (mpu6050.getIntStatus() & 0x02)
        {
            // Move to top of terminal
            printf("%c[0;0H", 27);
        
            // Clear screen
            printf("%c[2J", 27);
        

            // Read Quaternion
            while(mpu6050.dmpPacketAvailable() == false);
            mpu6050.getFIFOBytes(fifoBuffer, mpu6050.dmpGetFIFOPacketSize());
            mpu6050.dmpGetQuaternion(&q, fifoBuffer);

            // Print values
            printf("Quaternion:\r\n");
            printf("w: %f\r\n", q.w);
            printf("x: %f\r\n", q.x);
            printf("y: %f\r\n", q.y);
            printf("z: %f\r\n", q.z);


            // Blink LED
            Pin_Toggle(rLED);

            // Delay 100ms
            SysCtlDelay(SysCtlClockGet() / 3 / 10);
        }
    }
}



