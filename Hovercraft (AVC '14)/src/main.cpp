#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pin.h"
#include "pwm.h"
#include "uart.h"

#include "roboticsbp.h"
#include "tests.h"

#define rLED  PWM1_5 
#define gLED  PWM1_7
#define bLED  PWM1_6


#define CMD_DELIM ';'

DRV8800* motors[4] = {&m0, &m1, &m2, &m3};
Servo* servos[6] = {&s0, &s1, &s2, &s3, &s4, &s5};


unsigned char u1Buf[1024];
unsigned char* u1BufPtr;

unsigned char* cmdHead;
volatile unsigned char* cmdTail;


void parseCmd()
{   
    static float value;
    static unsigned char ndx;

    /* Valid commands:
    
    //  LED Comamnds
    #           [r,g,b,*]
    L#E         Enable LED #
    L#D         Disable LED #
    L#=0.6      Set LED to 60%

    //  Motor Commands
    #           [0-3, *]
    M#E         Enable Motor #
    M#D         Disable Motor #
    M#I         Invert Motor #
    M#T=0.7     Throttle Motor # to 70%
    M#=0.3      Set Motor # to 30% Fwd
    M#=-0.8     Set Motor # to 80% Rev
   
    //  Servo Commands
    #           [0-5, *]
    S#E         Enable Servo #
    S#D         Disable Servo #
    S#I         Invert Servo #
    S#L=0.4,0.6 Set Servo 3 limits to 40% - 60%
    S#=0.2      Set Servo 2 to 20%

    TODO: Add more targets later
    */
   
    // Determine object to do command on
    switch(cmdHead[0])
    {
        // 
        // LED Commands
        // 
        case 'l':
        case 'L':
            switch(cmdHead[1])
            {
                case '*':           // Command all LEDs
                    switch(cmdHead[2])
                    {
                        case 'e':   // Enable LEDs
                        case 'E':
                            PWM_Enable(rLED);
                            PWM_Enable(gLED);
                            PWM_Enable(bLED);

                            PWM_Set(rLED, 0);
                            PWM_Set(gLED, 0);
                            PWM_Set(bLED, 0);

                            //printf(" All LEDs enabled\r\n All LEDs set to  0%%\r\n");
                            break;

                        case 'd':   // Disable LEDs
                        case 'D':
                            PWM_Disable(rLED);
                            PWM_Disable(gLED);
                            PWM_Disable(bLED);

                            //printf(" All LEDs disabled\r\n");
                            break;

                        case '=':   // Set LEDs
                             // Parse  value
                            value = strtof(reinterpret_cast<const char*>(&cmdHead[3]), NULL); 

                            // Check value range
                            if (value > 1 || value < 0 )
                            {
                                //printf("Invalid led value: %s\r\n", cmdHead);
                                break;
                            }
                            
                            // Set LEDs
                            PWM_Set(rLED, value);
                            PWM_Set(gLED, value);
                            PWM_Set(bLED, value);

                            //printf(" LEDs set to % 2d%%\r\n", (unsigned char) (value*100));
                            break;


                        //default:
                            //printf("Invalid LED command: %s\r\n", cmdHead);
                            
                    }
                    break;
                case 'r':
                case 'g':
                case 'b':
                    switch(cmdHead[2])
                    {
                        case 'e':   // Enable LED
                        case 'E':
                            if (cmdHead[1] == 'r')
                            {
                                PWM_Enable(rLED);
                                PWM_Set(rLED, 0);

                                //printf(" Red LED enabled\r\n Red LED set to  0%%\r\n");
                            }
                            else if (cmdHead[1] == 'g')
                            {
                                PWM_Enable(gLED);
                                PWM_Set(gLED, 0);
                               
                                //printf(" Green LED enabled\r\n Green LED set to  0%%\r\n");
                            }
                            else
                            {
                                PWM_Enable(bLED);
                                PWM_Set(bLED, 0);
                               
                                //printf(" Blue LED enabled\r\n Blue LED set to  0%%\r\n");
                            }

                            break;

                        case 'd':   // Disable LED
                        case 'D':
                            if (cmdHead[1] == 'r')
                            {
                                PWM_Disable(rLED);

                                //printf(" Red LED disabled\r\n");
                            }
                            else if (cmdHead[1] == 'g')
                            {
                                PWM_Disable(gLED);
                               
                                //printf(" Green LED disabled\r\n");
                            }
                            else
                            {
                                PWM_Disable(bLED);
                               
                                //printf(" Blue LED disabled\r\n");
                            }

                            break;

                        case '=':   // Set LED
                             // Parse  value
                            value = strtof(reinterpret_cast<const char*>(&cmdHead[3]), NULL); 

                            // Check value range
                            if (value > 1 || value < 0 )
                            {
                                //printf("Invalid led value: %s\r\n", cmdHead);
                                break;
                            }
                            
                            // Set LED
                            if (cmdHead[1] == 'r')
                            {
                                PWM_Set(rLED, value);

                                //printf(" Red LED set to % 2d%%\r\n", (unsigned char) (value * 100));
                            }
                            else if (cmdHead[1] == 'g')
                            {
                                PWM_Set(gLED, value);
                               
                                //printf(" Green LED set to % 2d%%\r\n", (unsigned char) (value * 100));
                            }
                            else
                            {
                                PWM_Set(bLED, value);
                               
                                //printf(" Blue LED set to % 2d%%\r\n", (unsigned char) (value * 100));
                            }

                            break;


                        //default:
                            //printf("Invalid LED command: %s\r\n", cmdHead);
                            
                    }

                    break;
                //default:
                    //printf("Invalid LED index: %s\r\n", cmdHead);

            }
            break;

        
        //
        // MOTOR Commands
        //
        case 'm':
        case 'M':
            switch(cmdHead[1])
            {
                case '*':           // Command all Motors
                    switch(cmdHead[2])
                    {
                        case 'e':   // Enable Motors
                        case 'E':
                            Motors_Enable();
                            //printf(" All motors enabled\r\n");
                            break;

                        case 'd':   // Disable Motors
                        case 'D':
                            Motors_Disable();
                            //printf(" All motors disabled\r\n");
                            break;

                        case 'i':   // Invert Motors
                        case 'I':
                            m0.invert();
                            m1.invert(); 
                            m2.invert(); 
                            m3.invert(); 
                            //printf(" All motors inverted\r\n All motors set to  0%%\r\n");
                            break;

                        case 't':   // Throttle Motors
                        case 'T':
                            // Parse throttle value
                            value = strtof(reinterpret_cast<const char*>(&cmdHead[4]), NULL); 
                            /*

                            if (sscanf(&cmdHead[3], "=%f", &value) != 1)
                            {
                                //printf("Invalid motor trottle value: %s\r\n", cmdHead);
                                break;
                            }*/

                            // Check throttle value range
                            if (value > 1 || value < 0)
                            { 
                                //printf("Invalid motor throttle value: %s\r\n", cmdHead);
                                break;
                            }
                            
                            // Throttle motors
                            m0.throttle(value);
                            m1.throttle(value);
                            m2.throttle(value);
                            m3.throttle(value);

                            //printf(" All motors throttled to % 2d%%\r\n All motors set to 0%%", (unsigned char) (value*100));
                            break;

                        case '=':   // Set Motors
                            // Parse set value
                            value = strtof(reinterpret_cast<const char*>(&cmdHead[3]), NULL); 
                            /* 
                            if (sscanf(&cmdHead[2], "=%f", &value) != 1)
                            {
                                //printf("Invalid motor value: %s\r\n", cmdHead);
                                break;
                            }*/

                            // Check set value range
                            if (value > 1 || value < -1)
                            {
                                //printf("Invalid motor value: %s\r\n", cmdHead);
                                break;
                            } 
                            
                            // Set motors
                            m0.set(value);
                            m1.set(value);
                            m2.set(value);
                            m3.set(value);

                            //printf(" All motors set to % 2d%%\r\n", (unsigned char) (value*100));
                            break;

                        //default:
                            //printf("Invalid motor command: %s\r\n", cmdHead);
                    }
                    break; 

                case '0':
                case '1':
                case '2':
                case '3':
                    ndx = cmdHead[1] - '0';

                    switch(cmdHead[2])
                    { 
                        case 'e':   // Enable Motor
                        case 'E':
                            // TODO 

                            //printf(" Motor %d enabled\r\n", ndx);
                            break;

                        case 'd':   // Disable Motor
                        case 'D':
                            // TODO 

                            //printf(" Motor %d disabled\r\n", ndx);
                            break;

                        case 'i':   // Invert Motor(s)
                        case 'I':
                            motors[ndx] -> invert();
                            //printf(" Motor %d inverted\r\n Motor %d set to  0%%\r\n", ndx, ndx);
                            break;

                        case 't':   // Throttle Motor(s)
                        case 'T':
                            // Parse throttle value
                            value = strtof(reinterpret_cast<const char*>(&cmdHead[4]), NULL); 
                            /*
                            if (sscanf(&cmdHead[3], "=%f", &value) != 1)
                            {
                                //printf("Invalid motor trottle value: %s\r\n", cmdHead);
                                break;
                            }*/

                            // Check throttle value range
                            if (value > 1 || value < 0)
                            {
                                //printf("Invalid motor throttle value: %s\r\n", cmdHead);
                                break;
                            }
                            
                            // Throttle motors
                            motors[ndx] -> throttle(value);

                            //printf(" Motor %d throttled to % 2d%%\r\n Mmotor %d set to 0%%", (unsigned char) (value*100), ndx, ndx);
                            break;

                        case '=':   // Set Motor
                            // Parse throttle value
                            value = strtof(reinterpret_cast<const char*>(&cmdHead[3]), NULL); 
                            /*
                            if (sscanf(&cmdHead[2], "=%f", &value) != 1)
                            {
                                //printf("Invalid motor value: %s\r\n", cmdHead);
                                break;
                            }*/

                            // Check throttle value range
                            if (value > 1 || value < -1)
                            {
                                //printf("Invalid motor value: %s\r\n", cmdHead);
                                break;
                            }
                            
                            // Throttle motors
                            motors[ndx] -> set(value);

                            //printf(" Motor %d set to % 2d%%\r\n", (unsigned char) (value*100), ndx);
                            break;

                        //default:
                            //printf("Invalid motor command: %s\r\n", cmdHead);
                    }
                    break;

                //default:
                    //printf("Invalid Motor index: %s\r\n", cmdHead);
            }
            break;


        //
        // SERVO Commands
        //
        case 's':
        case 'S':
            switch(cmdHead[1])
            {
                case '*':           // Command all Servos
                    switch(cmdHead[2])
                    {
                        case 'e':   // Enable Servos
                        case 'E':
                            s0.enable();
                            s1.enable();
                            s2.enable();
                            s3.enable();
                            s4.enable();
                            s5.enable();
                            //printf(" All servos enabled\r\n");
                            break;

                        case 'd':   // Disable Servos
                        case 'D':
                            s0.disable();
                            s1.disable();
                            s2.disable();
                            s3.disable();
                            s4.disable();
                            s5.disable();
                            //printf(" All servos enabled\r\n");
                            break;

                        case 'i':   // Invert Servos
                        case 'I':
                            s0.invert();
                            s1.invert();
                            s2.invert();
                            s3.invert();
                            s4.invert();
                            s5.invert();
                            //printf(" All servos inverted\r\n All servos set to 50%%\r\n");
                            break;

                        case 'l':   // Limit Servos
                        case 'L':
                            // Parse limits
                            char* endPtr;
                            float max;

                            value = strtof(reinterpret_cast<const char*>(&cmdHead[4]), &endPtr);
                            max = strtof(reinterpret_cast<const char*>(endPtr), NULL);

                            // Check min limit range
                            if (value > 1 || value < 0)
                            {
                                //printf("Invalid servo min limit: %s\r\n", cmdHead);
                                break;
                            }
                            
                            // Check max limit range
                            if (max > 1 || max < 0)
                            {
                                //printf("Invalid servo max limit: %s\r\n", cmdHead);
                                break;
                            }

                            // Limit servos
                            s0.limit(value, max);
                            s1.limit(value, max);
                            s2.limit(value, max);
                            s3.limit(value, max);
                            s4.limit(value, max);
                            s5.limit(value, max);

                            //printf(" All servos limited to % 2d%% and % 2d%%\r\n", (unsigned char) (value*100), (unsigned char) (value * 100));
                            break;

                        case '=':   // Set Servos
                            // Parse set value
                            value = strtof(reinterpret_cast<const char*>(&cmdHead[4]), NULL);

                            // Check set value range
                            if (value > 1 || value < 0)
                            {
                                //printf("Invalid motor throttle value: %s\r\n", cmdHead);
                                break;
                            }

                            // Set servos
                            s0.set(value);
                            s1.set(value);
                            s2.set(value);
                            s3.set(value);
                            s4.set(value);
                            s5.set(value);

                            //printf(" All servos set to % 2d%%\r\n", (unsigned char) (value*100));
                            break;

                        //default:
                            //printf("Invalid servo command: %s\r\n", cmdHead);
                    }
                    break;

                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                    ndx = cmdHead[1] - '0';

                    switch(cmdHead[2])
                    {
                        case 'e':   // Enable Servo
                        case 'E':
                            servos[ndx] -> enable();
                            //printf(" Servo %d enabled\r\n", ndx);
                            break;

                        case 'd':   // Disable Servos
                        case 'D':
                            servos[ndx] -> disable();
                            //printf(" Servo %d disabled\r\n", ndx);
                            break;

                        case 'i':   // Invert Servos
                        case 'I':
                            servos[ndx] -> invert();
                            //printf(" Servo %d inverted\r\n Servo %d set to 50%%\r\n", ndx, ndx);
                            break;

                        case 'l':   // Limit Servos
                        case 'L':
                            // Parse limits
                            char* endPtr;
                            float max;

                            value = strtof(reinterpret_cast<const char*>(&cmdHead[3]), &endPtr);
                            max = strtof(reinterpret_cast<const char*>(endPtr), NULL);

                            // Check min limit range
                            if (value > 1 || value < 0)
                            {
                                //printf("Invalid servo min limit: %s\r\n", cmdHead);
                                break;
                            }
                            
                            // Check max limit range
                            if (max > 1 || max < 0)
                            {
                                //printf("Invalid servo max limit: %s\r\n", cmdHead);
                                break;
                            }

                            // Limit servos
                            servos[ndx] -> limit(value, max);

                            //printf(" Servo %d limited to % 2d%% and % 2d%%\r\n", ndx, (unsigned char) (value*100), (unsigned char) (value * 100));
                            break;

                        case '=':   // Set Servos
                            // Parse set value
                            value = strtof(reinterpret_cast<const char*>(&cmdHead[3]), NULL);

                            // Check set value range
                            if (value > 1 || value < 0)
                            {
                                //printf("Invalid motor throttle value: %s\r\n", cmdHead);
                                break;
                            }

                            // Set servos
                            servos[ndx] -> set(value);

                            //printf(" Servo %d set to % 2d%%\r\n", ndx, (unsigned char) (value*100));
                            break;

                        //default:
                            //printf("Invalid servo command: %s\r\n", cmdHead);
                    }
                    break;

                //default:
                    //printf("Invalid Servo index: %s\r\n", cmdHead);
            }
            break;

        //default:
            //printf("Invalid Motor target: %s\r\n", cmdHead);
    }   
}

void UART1_RX_IRQ()
{
    // Read cahracter
    unsigned char data = UART_ReadChar(UART1);

    // Put data into buffer
    *u1BufPtr = data;
    u1BufPtr++;


    if (data == CMD_DELIM)
    {
        // Simple buffer mantenance
        //  Don't use Ring Buffer, since we want string functions to
        //  be able to act on the buffer instead of copying it every
        //  time, which is time consuming
        if (u1BufPtr > u1Buf + 1000)
        {
            // Reset pointer to beginning of buffer
            u1BufPtr = u1Buf;
        }

        // Disabling local echo due to non-fifo U0TX implementation
        // Blocks, which drops data

        // Echo new line to USB
        //UART_WriteChar(UART0, ';');
    }
    else
    {
        // Echo to USB 
        //UART_WriteChar(UART0, data);
    }
}


int main(void)
{
    // Clock (50MHz)
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);
    
    
    // Init peripherals
    //  UART0/1
    UART_Init(UART0);
    UART_Enable(UART0);

    UART_Init(UART1);
    UART_SetIRQ(UART1, UART_RX_IRQ, &UART1_RX_IRQ);
    UART_IntEnable(UART1, UART_RX_IRQ);
    UART_Enable(UART1);
    setbuf(stdout, NULL);
    u1BufPtr = u1Buf;

    //  I2C0
    I2C_Init(I2C0);
    I2C_Enable(I2C0);

    //  PWM0/1
    PWM_Init(PWM0, 50);
    PWM_Init(PWM1, 1000);
    PWM_Enable(rLED);
    PWM_Enable(gLED);
    PWM_Enable(bLED);
    PWM_Set(rLED, 0);
    PWM_Set(gLED, 0);
    PWM_Set(bLED, 0);
    

    // Init Robotics BoosterPack
    RoboticsBP_Init();

    // NVIC
    IntMasterEnable();

    // As soon as UART1 IRW is enabled, a character is
    //  incorrectly received. Correct the issue by simply
    //  resetting the buffer pointer.
    //  Might be a bug in UART implementation of PAL
    u1BufPtr = u1Buf;
   

    
    // Start parsing commands as they become available
    cmdHead = u1Buf;
    cmdTail = u1Buf;

    while(1)
    {
        // Wait until buffPtr moves
        //  Might be possible to use wfi() here
        if (cmdTail == u1BufPtr)
            continue;
       
        // This is apparently needed, otherwise everything doesn't work...
        printf("%c%d", *cmdTail, *cmdTail == ';');
        
        // Increment pointer
        cmdTail++;

        // If command is complete
        if (*cmdTail == ';')
        {
            printf("\r\n");

            // Fake the end of a string
            *cmdTail = 0;
            
            // Execute command
            parseCmd();
            
            // Update pointers
            //  Check if we need to loop back
            if (cmdTail > u1Buf + 1000)
            {
                // Reset pointers to beginning of buffer
                cmdHead = u1Buf;
                cmdTail = u1Buf;
            }
            else
            {
                // Upadte pointers
                cmdHead = ((unsigned char*) cmdTail) + 1;
                cmdTail = cmdHead;
            }
        }
    }
}

