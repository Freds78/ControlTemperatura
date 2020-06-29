/*
 * Leds.c
 *
 *  Created on: 10 may. 2020
 *      Author: freds
 */


#include "Leds.h"
#include "sapi.h"



void turnOn(lamp_t lamp){
	gpioWrite(lamp, ON);

}
void turnOff(lamp_t lamp){
	gpioWrite(lamp, OFF);
}




