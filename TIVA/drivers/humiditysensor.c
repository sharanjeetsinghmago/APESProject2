/*
 * humiditysensor.c
 *
 *  Created on: Apr 28, 2018
 *      Author: Anay
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
#include "humiditysensor.h"


int ConfigureHumidity(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
    SysCtlPeripheralReset(SYSCTL_PERIPH_I2C0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    GPIOPinConfigure(GPIO_PB2_I2C0SCL);
    GPIOPinConfigure(GPIO_PB3_I2C0SDA);
    GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
    GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);

    I2CMasterInitExpClk(I2C0_BASE, 120000000U, false);
    return 0;
}


float get_humidity(void)
{

    uint8_t msb, lsb;
    I2CMasterSlaveAddrSet(I2C0_BASE, 0x40, false);
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);
    I2CMasterDataPut(I2C0_BASE, 0xE5);

    while(I2CMasterBusy(I2C0_BASE));
    I2CMasterSlaveAddrSet(I2C0_BASE, 0x40, true);
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);

    while(I2CMasterBusy(I2C0_BASE));
    msb = I2CMasterDataGet(I2C0_BASE);
    I2CMasterSlaveAddrSet(I2C0_BASE, 0x40, true);
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);

    while(I2CMasterBusy(I2C0_BASE));
    lsb = I2CMasterDataGet(I2C0_BASE);

    float humidity = (((msb * 256 + lsb) * 125.0) / 65536.0) - 6;
    return humidity;
}


float get_temp(void){

    uint8_t msb, lsb;
    I2CMasterSlaveAddrSet(I2C0_BASE, 0x40, false);
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);
    I2CMasterDataPut(I2C0_BASE, 0xE3);

    while(I2CMasterBusy(I2C0_BASE));
    I2CMasterSlaveAddrSet(I2C0_BASE, 0x40, true);
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);

    while(I2CMasterBusy(I2C0_BASE));
    msb = I2CMasterDataGet(I2C0_BASE);
    I2CMasterSlaveAddrSet(I2C0_BASE, 0x40, true);
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);

    while(I2CMasterBusy(I2C0_BASE));
    lsb = I2CMasterDataGet(I2C0_BASE);

    float temp = (((msb * 256 + lsb) * 175.72) / 65536.0) - 46.85;

    return temp;
}

