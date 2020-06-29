/*
 * Leds.h
 *
 *  Created on: 10 may. 2020
 *      Author: freds
 */

#ifndef MYPROGRAM_S_TEMP_INC_LEDS_H_
#define MYPROGRAM_S_TEMP_INC_LEDS_H_


#include "sapi.h"

typedef gpioMap_t lamp_t;

void turnOn(lamp_t lamp);
void turnOff(lamp_t lamp);






#endif /* MYPROGRAM_S_TEMP_INC_LEDS_H_ */
