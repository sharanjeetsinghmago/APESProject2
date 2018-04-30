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

uint32_t g_ui32SysClock;
int main(void)
{

    float ftest;
    int test;
        g_ui32SysClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                SYSCTL_OSC_MAIN | SYSCTL_USE_PLL |
                SYSCTL_CFG_VCO_480), 120000000);    	PinoutSet(false, false);

    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_1);


    ConfigureUART();

    UARTprintf("UART Configured \n");

    test = ConfigureAltitude();
        if(test == 0)
            UARTprintf("[Success] Sensor Configuring \n");
        else
            UARTprintf("[Failure] Sensor Configuring\n");

   ftest = get_altitude();
   if(ftest > 1700 || ftest < 1600)
               UARTprintf("[Success] Getting Altitude\n");
           else
               UARTprintf("[Failure] Getting Altitude\n");

   return 0;                  

}
