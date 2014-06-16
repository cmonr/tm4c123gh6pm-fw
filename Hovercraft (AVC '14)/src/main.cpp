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


    
    // Init devices
    MPU6050 mpu6050 = MPU6050(I2C0);



    // Initalize MPU6050
    printf("Initialize MPU6050...\r\n");
    mpu6050.initialize();
    printf(" Initialized.\r\n");

    // Verify connection
    printf("Testing I2C <> MPU6050 connection...\r\n");
    if (mpu6050.testConnection() == false)
    {
        printf(" Failed.\r\n");
        while(1);   // Block here
    }
    printf(" Success!\r\n");

    // Leave message on screen for 3s
    SysCtlDelay(SysCtlClockGet());

    // Clear screen
    printf("%c[2J", 27);
    
    
    while(1)
    {
        int16_t ax, ay, az;
        int16_t gx, gy, gz;
       
        // Move to top of terminal
        printf("%c[0;0H", 27);
    
        // Clear screen
        printf("%c[2J", 27);
    

        // Read Raw Values
        mpu6050.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

        // Print values
        printf("Ax: %f\r\n", ax / 2048.0);
        printf("Ay: %f\r\n", ay / 2048.0);
        printf("Az: %f\r\n", az / 2048.0);
        printf("Gx: %d\r\n", gx);
        printf("Gy: %d\r\n", gy);
        printf("Gz: %d\r\n\n", gz);

        // Blink LED
        Pin_Toggle(rLED);

        // Delay 100ms
        SysCtlDelay(SysCtlClockGet() / 3 / 10);
    }
}



