/*
 * altitudesensor.h
 *
 *  Created on: Apr 28, 2018
 *      Author: Anay Gondhalekar
 */

#ifndef DRIVERS_ALTITUDESENSOR_H_
#define DRIVERS_ALTITUDESENSOR_H_


void i2c_read(char addre, unsigned long data,short *receive);
unsigned long i2c_write(char address, unsigned long reg,unsigned long data);
int ConfigureAltitude(void);
float get_altitude(void);

#endif /* DRIVERS_ALTITUDESENSOR_H_ */
