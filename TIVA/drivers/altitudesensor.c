/*
 * altitudesensor.c
 *
 *  Created on: Apr 28, 2018
 *      Author: Anay Gondhalekar
 */
#include <stdarg.h>
#include <stdbool.h>
#include <math.h>
#include <stdint.h>
#include "inc/hw_i2c.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/i2c.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "altitudesensor.h"

void i2c_read(char addre, unsigned long data,short *receive)
{
    unsigned long k=0;
    I2CMasterSlaveAddrSet(I2C5_BASE,addre,false); //false
    I2CMasterDataPut(I2C5_BASE,data);
    I2CMasterControl(I2C5_BASE,I2C_MASTER_CMD_BURST_SEND_START);
    while(!I2CMasterBusy(I2C5_BASE));
    //SysCtlDelay(10);
    while(I2CMasterBusy(I2C5_BASE));
    I2CMasterSlaveAddrSet(I2C5_BASE,addre,true);

    I2CMasterControl(I2C5_BASE,I2C_MASTER_CMD_SINGLE_RECEIVE);

    while(!I2CMasterBusy(I2C5_BASE));

    while(I2CMasterBusy(I2C5_BASE));

    k=I2CMasterErr(I2C5_BASE);

    k=I2CMasterDataGet(I2C5_BASE);

    *receive=k;
    I2CMasterControl(I2C5_BASE,I2C_MASTER_CMD_BURST_RECEIVE_FINISH);


}

unsigned long i2c_write(char address, unsigned long reg,unsigned long data)
{
    unsigned long k=0;
    I2CMasterSlaveAddrSet(I2C5_BASE,address,false);
    I2CMasterDataPut(I2C5_BASE,reg);
    I2CMasterControl(I2C5_BASE,I2C_MASTER_CMD_BURST_SEND_START);
        while(!I2CMasterBusy(I2C5_BASE));

    while(I2CMasterBusy(I2C5_BASE));
    k=I2CMasterErr(I2C5_BASE);
    I2CMasterDataPut(I2C5_BASE,data);
    k=I2CMasterErr(I2C5_BASE);
    I2CMasterControl(I2C5_BASE,I2C_MASTER_CMD_BURST_SEND_FINISH);
    k=I2CMasterErr(I2C5_BASE);


return k;
}

int ConfigureAltitude(void)
{
    short k = 0;
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C5);
    SysCtlPeripheralReset(SYSCTL_PERIPH_I2C5);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    GPIOPinConfigure(GPIO_PB4_I2C5SCL);
    GPIOPinConfigure(GPIO_PB5_I2C5SDA);
    GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_4);
    GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_5);

    I2CMasterInitExpClk(I2C5_BASE, 120000000U, false);
    //SysCtlDelay(g_ui32SysClock / 2 / 3);
    i2c_read(0x60,0x0C, &k);
    k=i2c_write(0x60, 0x26,0xBB); //for one by one send
    k=i2c_write(0x60, 0x27,0x02);
    i2c_read(0x60,0x26, &k);
    i2c_read(0x60,0x27, &k);
    return 0;
}

float get_altitude(void)
{
    short  data[4]={0,0,0,0};
    float convert = 0;

    i2c_read(0x60,0x01, &data[0]);
    i2c_read(0x60,0x02, &data[1]);
    i2c_read(0x60,0x03, &data[2]);
    convert=(float)((short)(data[0]<<8)|(data[1])) + (float)(data[2]>>4)*0.0625 - 30;
    return convert;
}


