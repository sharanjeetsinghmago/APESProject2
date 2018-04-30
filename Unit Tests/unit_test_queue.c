#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <math.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_i2c.h"
#include "driverlib/gpio.h"
#include "drivers/pinout.h"
#include "inc/hw_gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/i2c.h"
#include "utils/uartstdio.h"
#include "queue.h>

uint32_t g_ui32SysClock;
int main(void)
{
    int i;
    float ftest;
    int test;
    myqueue = xQueueCreate(5, 10);
    //
    // Run from the PLL at 120 MHz.
    //
    g_ui32SysClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                SYSCTL_OSC_MAIN | SYSCTL_USE_PLL |
                SYSCTL_CFG_VCO_480), 120000000);

    //
    // Configure the device pins.
    //
    PinoutSet(false, false);

    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_1);


    ConfigureUART();

    UARTprintf("UART Configured \n");

myqueue = xQueueCreate(5, 10);

            if(myqueue == NULL)
                UARTprintf("[Success] Queue Configuring \n");
            else
                UARTprintf("[Failure] Queue Configuring\n");

            char put[10]="Hi there\0";
            char get[10]={0};

            if( xQueueSendToBack( myqueue, &put, strlen(put) ) != pdPASS ){
                 UARTprintf("[Failure] Queue is full \n");
             }

             if( xQueueReceive( myqueue, &get, portMAX_DELAY ) != pdPASS ){
                 UARTprintf("[Failure] Error in queue\n");
             }

             for(i=0; i<11;i++)
             {
                 xQueueSendToBack( myqueue, &put, strlen(put)
             }

             if( xQueueSendToBack( myqueue, &put, strlen(put) ) != pdPASS )
             {
                  UARTprintf("[Failure] Queue is full \n");
             }

             if( xQueueReceive( myqueue, &get, portMAX_DELAY ) != pdPASS ){
                 UARTprintf("[Failure] Error in queue receive\n");
             }
             else{
                 UARTprintf("[Success] remove successful\n");
             }
             return 0;

}
