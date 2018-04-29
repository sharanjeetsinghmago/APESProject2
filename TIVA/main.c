
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <math.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_i2c.h"
#include "driverlib/gpio.h"
#include "drivers/pinout.h"
#include "drivers/humiditysensor.h"
#include "drivers/altitudesensor.h"
#include "inc/hw_gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/i2c.h"
#include "utils/uartstdio.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "limits.h"
#include "string.h"
#include <time.h>


uint32_t g_ui32SysClock;
TimerHandle_t xTimer1,xTimer2;
TaskHandle_t xTaskHandle3;
QueueHandle_t HQueue, AQueue;
volatile float humidity;
void AltitudeTask(void *pvParameters);
void HumidityTask(void *pvParameters);
void LoggerTask(void *pvParameters);

#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

struct Message
{
    uint8_t log_id;
    uint8_t log_level;
    float data;
    char timestamp[25];
    char c;
}xMessage;

#define ALT  0x01
#define HUMID  0X02

//*****************************************************************************
//
// Configure the UART and its pins.  This must be called before UARTprintf().
//
//*****************************************************************************
void ConfigureUART(void)
{

    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    UARTStdioConfig(0, 115200, g_ui32SysClock);
}

void uart7_init(void)
{
    //
    // Enable the GPIO Peripheral used by the UART.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);

    //
    // Enable UART2
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART7);

    //
    // Configure GPIO Pins for UART mode.
    //
    ROM_GPIOPinConfigure(GPIO_PC4_U7RX);
    ROM_GPIOPinConfigure(GPIO_PC5_U7TX);
    ROM_GPIOPinTypeUART(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5);

    //
    // Initialize the UART for console I/O.
    //
    //UARTStdioConfig(0, 9600, g_ui32SysClock);
    ROM_UARTConfigSetExpClk(UART7_BASE, g_ui32SysClock, 9600,
                                (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                                 UART_CONFIG_PAR_NONE));
}

void uart7_send(const uint8_t *pui8Buffer, uint32_t ui32Count)
{
    // Loop while there are more characters to send.
    while(ui32Count--)
    {
        // Write the next character to the UART.
         UARTCharPut(UART7_BASE, *pui8Buffer++);
    }
}


void vTimerCallback1( TimerHandle_t xTimer1 )
{

    struct Message px1Message;
    float altitude;
    time_t a = time(NULL);
    char* temp=ctime(&a);
    strcpy(px1Message.timestamp,temp);
    altitude = get_altitude();
    //Tick_Count = xTaskGetTickCount();
    px1Message.data = altitude;
    px1Message.log_id = 1;
    px1Message.log_level = 1;
    //strcpy( pxMessage.logstring, "Anay here");

    /*
    AQueue = xQueueCreate( 10, sizeof( struct Message ) );
    xQueueSend( AQueue, &pxMessage, 10 );
    */
    xQueueSendToBack( HQueue, &px1Message, 10 );
    //UARTprintf("%d", (int)altitude);
    //xTaskNotify( xTaskHandle3,ALT , eSetBits );

}

void vTimerCallback2( TimerHandle_t xTimer2 )
{

    struct Message pxMessage;
    time_t a = time(NULL);
    char* temp=ctime(&a);
    strcpy(pxMessage.timestamp,temp);
    float humidity1 = get_humidity();
            if((humidity - humidity1) > 10.0 || (humidity1 - humidity) > 10.0)
                    {
                        humidity1 = humidity;
                    }
            pxMessage.data = humidity;
            humidity = humidity1;


    pxMessage.log_id = 2;
    pxMessage.log_level = 1;
    xQueueSendToBack( HQueue, &pxMessage, 10 );

}
void AltitudeTask(void *pvParameters)
{

        // Turn on LED 1
        xTimer1 = xTimerCreate("timer1",pdMS_TO_TICKS( 500 ),pdTRUE,( void * ) 0,vTimerCallback1);
        xTimerStart( xTimer1, 0 );
        for(;;) ;

}

void HumidityTask(void *pvParameters)
{

        // Turn on LED 1
        xTimer2 = xTimerCreate("timer2",pdMS_TO_TICKS( 500 ),pdTRUE,( void * ) 0,vTimerCallback2);
        xTimerStart( xTimer2, 0 );
        for(;;) ;

}

void LoggerTask(void *pvParameters)
{
    struct Message pxRxedMessage;
    while(1)
    {

        if( xQueueReceive( HQueue, &( pxRxedMessage ), portMAX_DELAY ))
                {

                         if(pxRxedMessage.log_id == 1)
                                {
                                  UARTprintf("Log ID = %d  ",pxRxedMessage.log_id);
                                  UARTprintf("Altitude is %d\n",(int)pxRxedMessage.data);
                                  UARTprintf("Timestamp: %s\n",pxRxedMessage.timestamp );
                                  UARTprintf("Log Level = %d  ",pxRxedMessage.log_level);
                                }
                               else if(pxRxedMessage.log_id == 2)
                                {
                                   UARTprintf("Log ID = %d  ",pxRxedMessage.log_id);
                                  UARTprintf("Humidity is %d\n",(int)pxRxedMessage.data);
                                  UARTprintf("Timestamp: %s\n",pxRxedMessage.timestamp );
                                  UARTprintf("Log Level = %d  ",pxRxedMessage.log_level);
                                 }

                         uart7_send((uint8_t *)&pxRxedMessage,sizeof(pxRxedMessage));
                }
        //SysCtlDelay(g_ui32SysClock / 2 / 3);
    }
}

int main(void)
{

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
    uart7_init();

    UARTprintf("UART Configured");

    ConfigureAltitude();

    ConfigureHumidity();
    HQueue = xQueueCreate( 10, sizeof( struct Message ) );

    SysCtlDelay(g_ui32SysClock / 2 / 3);
    humidity = get_humidity();
    /*
    if(ConfigureHumidity() != 0 )
    {
        UARTprintf("Couldn't Configure the humidity sensor\n");
    }
    SysCtlDelay(g_ui32SysClock / 2 / 3);
    UARTprintf("Configured Humidity and Temperature sensor\n");

    SysCtlDelay(g_ui32SysClock / 2 / 3);
    ConfigureAltitude();
    UARTprintf("Configured Altitude sensor\n");
    SysCtlDelay(g_ui32SysClock / 2 / 3);




    SysCtlDelay(g_ui32SysClock / 2 / 3);
    humidity = get_humidity();

    SysCtlDelay(g_ui32SysClock / 2 / 3);

    temp = get_temp();

    SysCtlDelay(g_ui32SysClock / 2 / 3);
    while(1)
    {
        SysCtlDelay(g_ui32SysClock / 2 / 3);
        humidity1 = get_humidity();
        if((humidity - humidity1) > 10.0 || (humidity1 - humidity) > 10.0)
                {
                    humidity1 = humidity;
                }
        UARTprintf("Humidity is %d percent.\n",(int)humidity1);
        humidity = humidity1;
        SysCtlDelay(g_ui32SysClock / 2 / 3);


        temp1 = get_temp();
        if((temp - temp1) > 5.0 || (temp1 - temp) > 5.0)
        {
            temp1 = temp;
        }
        UARTprintf("Temperature is %d Celcius.\n",(int)temp1);
        temp = temp1;
        SysCtlDelay(g_ui32SysClock / 2 / 3);

        altitude = get_altitude();
        UARTprintf("Altitude = %d metres.\n ",(int)altitude);

        SysCtlDelay(g_ui32SysClock / 2 / 3);
    */

        /* freertos based code */


    xTaskCreate(AltitudeTask, (const portCHAR *)"ALTITUDE_TASK",configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    xTaskCreate(HumidityTask, (const portCHAR *)"HUMIDITY_TASK",configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    xTaskCreate(LoggerTask, (const portCHAR *)"LOGGER_TASK",configMINIMAL_STACK_SIZE, NULL, 1, &xTaskHandle3);

    vTaskStartScheduler();

    UARTprintf("Ending Main");

    return 0;

}

