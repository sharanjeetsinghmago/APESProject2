
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
#include "driverlib/ssi.h"
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

#define UART_COMM

TimerHandle_t xTimer1,xTimer2;
TaskHandle_t xHBTaskHandle;
QueueHandle_t HQueue, AQueue;
BaseType_t xHBTask;

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

uint32_t g_ui32SysClock;

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

void
InitSPI3(void)
{

    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI3);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    //
    // Configure the pin muxing for SSI3 functions on port H4, H5, H6 and H7.
    // This step is not necessary if your part does not support pin muxing.
    //
    GPIOPinConfigure(GPIO_PF3_SSI3CLK);
    GPIOPinConfigure(GPIO_PF2_SSI3FSS);
    GPIOPinConfigure(GPIO_PF0_SSI3XDAT1);
    GPIOPinConfigure(GPIO_PF1_SSI3XDAT0);

    GPIOPinTypeSSI(GPIO_PORTF_BASE, GPIO_PIN_3 | GPIO_PIN_1 | GPIO_PIN_0 |
                   GPIO_PIN_2);

    SSIConfigSetExpClk(SSI3_BASE, g_ui32SysClock, SSI_FRF_MOTO_MODE_2,
                       SSI_MODE_SLAVE, 100000, 8);

    SSIEnable(SSI3_BASE);
}

void spi3_send(const uint8_t *pui32Data, uint32_t ui32Index)
{
    while(ui32Index--)
    {
        SSIDataPut(SSI3_BASE , *pui32Data ++ );
    }
    while(SSIBusy(SSI3_BASE))
    {
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
    //xQueueSendToBack( HQueue, &px1Message, 10 );
    //UARTprintf("%d", (int)altitude);

    if(altitude < 0 || altitude > 8850)
    {
        xTaskNotify( xHBTaskHandle,ALT , eSetBits );
    }
    else
    {
        xQueueSendToBack( HQueue, &px1Message, 10 );
    }
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

    //xQueueSendToBack( HQueue, &pxMessage, 10 );
    pxMessage.log_id = 2;
    pxMessage.log_level = 1;
    //xQueueSendToBack( HQueue, &pxMessage, 10 );

    if(humidity < 0 || humidity > 100)
        {
            xTaskNotify( xHBTaskHandle,HUMID , eSetBits );
        }

    else
    {
        xQueueSendToBack( HQueue, &pxMessage, 10 );
    }
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

                         #ifdef UART_COMM
                         uart7_send((uint8_t *)&pxRxedMessage,sizeof(pxRxedMessage));
                         #else
                          spi3_send((uint8_t *)&pxRxedMessage, sizeof(pxRxedMessage));
                        #endif
                }
        //SysCtlDelay(g_ui32SysClock / 2 / 3);
    }
}

void HBTask(void *pvParameters)
{
    uint32_t val_recv;
    struct Message hbmsg;
    time_t a = time(NULL);
    while(1)
    {
        xHBTask = xTaskNotifyWait(0, 0xFF, &val_recv, portMAX_DELAY);
        if(xHBTask == pdTRUE)
        {
            if(val_recv & 0x01)
            {
                UARTprintf("Error in Heartbeat from Altitude \n");

            char* temp=ctime(&a);
            strcpy(hbmsg.timestamp,temp);

            hbmsg.data = -2;

            hbmsg.log_id = 1;
            hbmsg.log_level = 2;
            xQueueSendToBack( HQueue, &hbmsg, 10 );
            }

            if(val_recv & 0x02)
            {
            UARTprintf("Error in Heartbeat from Humidity \n");

            char* temp=ctime(&a);
            strcpy(hbmsg.timestamp,temp);

            hbmsg.data = -2;

            hbmsg.log_id = 2;
            hbmsg.log_level = 2;
            xQueueSendToBack( HQueue, &hbmsg, 10 );
            }
        }
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


    InitSPI3();
    UARTprintf("UART Configured");

    ConfigureAltitude();

    ConfigureHumidity();
    HQueue = xQueueCreate( 10, sizeof( struct Message ) );

    SysCtlDelay(g_ui32SysClock / 2 / 3);
    humidity = get_humidity();

    /* freertos based code */

    xTaskCreate(AltitudeTask, (const portCHAR *)"ALTITUDE_TASK",configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    xTaskCreate(HumidityTask, (const portCHAR *)"HUMIDITY_TASK",configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    xTaskCreate(LoggerTask, (const portCHAR *)"LOGGER_TASK",configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    xTaskCreate(HBTask, (const portCHAR *)"HB_TASK",configMINIMAL_STACK_SIZE, NULL, 1, &xHBTaskHandle);

    vTaskStartScheduler();

    UARTprintf("Ending Main");

    return 0;

}

