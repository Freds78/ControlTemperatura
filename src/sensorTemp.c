/*=============================================================================
 * Author: Freds Amundaray Cruz <freds.amundaray@gmail.com>
 * Date: 2020/04/28
 * Version: 1
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include "sensorTemp.h"
#include "sapi.h"
#include <stdint.h>
#include <stdbool.h>
#include "ff.h"       	// Biblioteca FAT FS
#include "fssdc.h"		// API de bajo nivel para unidad "SDC:" en FAT FS
#include <math.h>
#include "Leds.h"

/*=====[Definition macros of private constants]==============================*/

#define AMARILLO LED1
#define ROJO LED2
#define VERDE LED3
#define SKIP_ROMCOM 0xCC
#define CONVERT_TEMP 0x44
#define READ_SCRATCH 0xBE
#define SEARCH_ROM 0xF0
#define MATCH_ROM 0x55
#define FILENAME1 "SDC:/Sensor1.txt"
#define FILENAME2 "SDC:/Sensor2.txt"
#define FILENAME3 "SDC:/Sensor3.txt"

/*=====[Definitions of extern global variables]==============================*/

typedef unsigned int	UINT;
static float Buffer1;
static float Buffer2;
static float Buffer3;
static uint8_t scratchSensor1[9];
static uint8_t scratchSensor2[9];
static uint8_t scratchSensor3[9];
static FIL fp;             			// <-- File object needed for each open file
static rtc_t rtc = {
   2020,			// año
   11,				// mes
   20,				// dia
   4,				// semana lunes martes etc
   10,				// hora
   13,				// minutos
   0				// segundos
};

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/


static void skip_Rom(uint8_t n);
static bool_t Read_Scratchpad(uint8_t n);
static uint8_t crc8_scrat(uint8_t n);
static bool_t initialize_secuence(uint8_t n);
static void send_bit(uint8_t bit_data,uint8_t n);
static uint8_t read_bit(uint8_t n);
static void send_byte(uint8_t data, uint8_t n);
static uint8_t read_byte(uint8_t n);
static void Control_Temperature(uint8_t n);
static void writeRegister(uint8_t n);
static void Conver_Temp(uint8_t n);



static void Conver_Temp(uint8_t n){

	delay_t delay;

	if(n == 1){
		gpioConfig( GPIO1, GPIO_INPUT );
		while(gpioRead( GPIO1 ) == 0){
			delayWrite(&delay,750);
			if(delayRead(&delay)){
			}
			printf("espera que llegue la confirmación del sensor 1\n");
		}
	}
	if(n == 2){
		gpioConfig( GPIO3, GPIO_INPUT );
		while(gpioRead( GPIO3 ) == 0){
			delayWrite(&delay,750);
			if(delayRead(&delay)){
			}
			printf("espera que llegue la confirmación del sensor 2\n");
		}
	}
	if(n == 3){
		gpioConfig( GPIO4, GPIO_INPUT );
		while(gpioRead( GPIO4 ) == 0){
			delayWrite(&delay,750);
			if(delayRead(&delay)){
			}
			printf("espera que llegue la confirmación del sensor 3\n");
		}
	}

}

static void skip_Rom(uint8_t n){

	send_byte(SKIP_ROMCOM, n );
}

static bool_t Read_Scratchpad(uint8_t n){

	int8_t i;
	uint8_t var_aux;

	bool_t result;

	var_aux = 0;

	if(n == 1){
		for (i = 0; i < 9; i++) {

			scratchSensor1[i] = read_byte(n);

			if(i == 8){

				var_aux = crc8_scrat(n);

				if(var_aux == scratchSensor1[i]){

					result = TRUE;

				}else{

					result = FALSE;
					initialize_secuence(n);
				}
			}
		}

	}


	if(n == 2){
		for (i = 0; i < 9; i++) {

			scratchSensor2[i] = read_byte(n);

			if(i == 8){

				var_aux = crc8_scrat(n);

				if(var_aux == scratchSensor2[i]){

					result = TRUE;

				}else{

					result = FALSE;
					initialize_secuence(n);
				}
			}
		}

	}

	if(n == 3){
		for (i = 0; i < 9; i++) {

			scratchSensor3[i] = read_byte(n);

			if(i == 8){

				var_aux = crc8_scrat(n);

				if(var_aux == scratchSensor3[i]){

					result = TRUE;

				}else{

					result = FALSE;
					initialize_secuence(n);
				}
			}
		}

	}
	return result;
}


static uint8_t crc8_scrat (uint8_t n) {

	uint8_t div = 0b10001100; // Rotated poly
	uint8_t crc = 0;
	int8_t i;

	if(n == 1){

		for (i = 0; i < 8; i++) {

			uint8_t byte = scratchSensor1[i];
			int8_t j;

			for (j = 0; j < 8; j++) {

				uint8_t crc_carry = crc & 1;
				crc = crc >> 1;
				uint8_t byte_carry = byte & 1;
				byte = byte >>1;

				if (crc_carry ^ byte_carry){
					crc ^= div;
				}
			}
		}

	}

	if(n == 2){

		for (i = 0; i < 8; i++) {

			uint8_t byte = scratchSensor2[i];
			int8_t j;

			for (j = 0; j < 8; j++) {

				uint8_t crc_carry = crc & 1;
				crc = crc >> 1;
				uint8_t byte_carry = byte & 1;
				byte = byte >>1;

				if (crc_carry ^ byte_carry){
					crc ^= div;
				}
			}
		}

	}

	if(n == 3){

		for (i = 0; i < 8; i++) {

			uint8_t byte = scratchSensor3[i];
			int8_t j;

			for (j = 0; j < 8; j++) {

				uint8_t crc_carry = crc & 1;
				crc = crc >> 1;
				uint8_t byte_carry = byte & 1;
				byte = byte >>1;

				if (crc_carry ^ byte_carry){
					crc ^= div;
				}
			}
		}

	}
	return crc;

}



static void send_byte(uint8_t data, uint8_t n) {	//Sends 8 bit in a row, LSB first

	int8_t i;

	for (i = 0; i < 8; i++) {
		send_bit(data & 0x01, n);
		data = data >> 1;
	}
}


static void send_bit(uint8_t bit_data, uint8_t n) {


	if(n == 1){
		gpioConfig( GPIO1, GPIO_OUTPUT );
		gpioWrite( GPIO1, 0 );
		delayInaccurateUs( 2 );
		if (bit_data) {
			gpioWrite( GPIO1, 1 );
			delayInaccurateUs( 60 );
		} else {
			delayInaccurateUs( 60 );
			gpioWrite( GPIO1, 1 );
			delayInaccurateUs( 10 );
		}
	}

	if(n == 2){
		gpioConfig( GPIO3, GPIO_OUTPUT );
		gpioWrite( GPIO3, 0 );
		delayInaccurateUs( 2 );
		if (bit_data) {
			gpioWrite( GPIO3, 1 );
			delayInaccurateUs( 60 );
		} else {
			delayInaccurateUs( 60 );
			gpioWrite( GPIO3, 1 );
			delayInaccurateUs( 10 );
		}
	}

	if(n == 3){
		gpioConfig( GPIO4, GPIO_OUTPUT );
		gpioWrite( GPIO4, 0 );
		delayInaccurateUs( 2 );
		if (bit_data) {
			gpioWrite( GPIO4, 1 );
			delayInaccurateUs( 60 );
		} else {
			delayInaccurateUs( 60 );
			gpioWrite( GPIO4, 1 );
			delayInaccurateUs( 10 );
		}
	}

}


static uint8_t read_byte(uint8_t n) {		//Reads a byte, LSB first
	uint8_t byte = 0;
	int8_t i;
	uint8_t j;

	if(n == 1){
		for (i=0; i < 8; i++) {
			j = read_bit(n);
			j = j << i;
			byte = byte | j;
		}

	}

	if(n == 2){
		for (i=0; i < 8; i++) {
			j = read_bit(n);
			j = j << i;
			byte = byte | j;
		}

	}

	if(n == 3){
		for (i=0; i < 8; i++) {
			j = read_bit(n);
			j = j << i;
			byte = byte | j;
		}

	}

	return byte;
}

static uint8_t read_bit(uint8_t n) {

	uint8_t read;

	if(n == 1){
		gpioConfig( GPIO1, GPIO_OUTPUT );
		gpioWrite( GPIO1, 0 );
		delayInaccurateUs( 3 );
		gpioConfig( GPIO1, GPIO_INPUT );
		delayInaccurateUs( 10 );
		read = gpioRead( GPIO1 );
		delayInaccurateUs( 55 );
	}

	if(n == 2){
		gpioConfig( GPIO3, GPIO_OUTPUT );
		gpioWrite( GPIO3, 0 );
		delayInaccurateUs( 3 );
		gpioConfig( GPIO3, GPIO_INPUT );
		delayInaccurateUs( 10 );
		read = gpioRead( GPIO3 );
		delayInaccurateUs( 55 );
	}

	if(n == 3){
		gpioConfig( GPIO4, GPIO_OUTPUT );
		gpioWrite( GPIO4, 0 );
		delayInaccurateUs( 3 );
		gpioConfig( GPIO4, GPIO_INPUT );
		delayInaccurateUs( 10 );
		read = gpioRead( GPIO4 );
		delayInaccurateUs( 55 );
	}

	return read;
}

static bool_t initialize_secuence(uint8_t n){

	bool_t val;
	uint8_t bit;

	if(n == 1){
		gpioConfig( GPIO1, GPIO_OUTPUT );
		gpioWrite( GPIO1, 0 );
		delayInaccurateUs( 500 );
		gpioConfig( GPIO1, GPIO_INPUT );
		delayInaccurateUs( 60 );
		bit = gpioRead( GPIO1 );
		if (bit == 0){
			val = TRUE;
		}else{
			val = FALSE;
		}
		delayInaccurateUs( 410 );
	}

	if(n == 2){
			gpioConfig( GPIO3, GPIO_OUTPUT );
			gpioWrite( GPIO3, 0 );
			delayInaccurateUs( 500 );
			gpioConfig( GPIO3, GPIO_INPUT );
			delayInaccurateUs( 60 );
			bit = gpioRead( GPIO3 );
			if (bit == 0){
				val = TRUE;
			}else{
				val = FALSE;
			}
			delayInaccurateUs( 410 );
		}

	if(n == 3){
			gpioConfig( GPIO4, GPIO_OUTPUT );
			gpioWrite( GPIO4, 0 );
			delayInaccurateUs( 500 );
			gpioConfig( GPIO4, GPIO_INPUT );
			delayInaccurateUs( 60 );
			bit = gpioRead( GPIO4 );
			if (bit == 0){
				val = TRUE;
			}else{
				val = FALSE;
			}
			delayInaccurateUs( 410 );
		}

	return val;
}

// FUNCION que se ejecuta cada vezque ocurre un Tick
 void diskTickHook( void *ptr )
{
   disk_timerproc();   // Disk timer process
}


 static void writeRegister(uint8_t n){

	 rtcRead( &rtc );
	 char bufferRegistre[100];

	 if(n == 1){

		 if( f_open( &fp, FILENAME1, FA_WRITE | FA_OPEN_APPEND  ) == FR_OK ){

			 UINT n = sprintf( bufferRegistre, "Temperatura sensor_1 es: %.2f C %04d-%02d-%02d %02d:%02d:%02d\r\n",
					 Buffer1,
					 rtc.year,
					 rtc.month,
					 rtc.mday,
					 rtc.hour,
					 rtc.min,
					 rtc.sec
			 );

			 UINT nbytes;
			 f_write( &fp, bufferRegistre, n, &nbytes );

			 f_close(&fp);

			 if( nbytes == n ){
				 // printf("Escribio correctamente\n");
				 gpioWrite( LEDG, ON );
			 } else {
				 gpioWrite( LEDR, ON );
				 // printf("Escribio %d bytes\n", nbytes);
			 }

		 } else{
			 // printf("Error al abrir el archivo\n");
			 gpioWrite( LEDR, ON );
		 }
	 }

	 if(n == 2){

		 if( f_open( &fp, FILENAME2, FA_WRITE | FA_OPEN_APPEND  ) == FR_OK ){ //| FA_OPEN_APPEND

			 UINT n = sprintf( bufferRegistre, "Temperatura sensor_2 es: %.2f C %04d-%02d-%02d %02d:%02d:%02d\r\n",
					 Buffer2,
					 rtc.year,
					 rtc.month,
					 rtc.mday,
					 rtc.hour,
					 rtc.min,
					 rtc.sec
			 );

			 UINT nbytes;
			 f_write( &fp, bufferRegistre, n, &nbytes );

			 f_close(&fp);

			 if( nbytes == n ){
				 // printf("Escribio correctamente\n");
				 gpioWrite( LEDG, ON );
			 } else {
				 gpioWrite( LEDR, ON );
				 // printf("Escribio %d bytes\n", nbytes);
			 }

		 } else{
			 //printf("Error al abrir el archivo\n");
			 gpioWrite( LEDR, ON );
		 }
	 }

	 if(n == 3){

		 if( f_open( &fp, FILENAME3, FA_WRITE | FA_OPEN_APPEND  ) == FR_OK ){

			 UINT n = sprintf( bufferRegistre, "Temperatura sensor_3 es: %.2f C %04d-%02d-%02d %02d:%02d:%02d\r\n",
					 Buffer3,
					 rtc.year,
					 rtc.month,
					 rtc.mday,
					 rtc.hour,
					 rtc.min,
					 rtc.sec
			 );

			 UINT nbytes;
			 f_write( &fp, bufferRegistre, n, &nbytes );

			 f_close(&fp);

			 if( nbytes == n ){
				 //printf("Escribio correctamente\n");
				 gpioWrite( LEDG, ON );
			 } else {
				 gpioWrite( LEDR, ON );
				 //printf("Escribio %d bytes\n", nbytes);
			 }

		 } else{
			 //printf("Error al abrir el archivo\n");
			 gpioWrite( LEDR, ON );
		 }
	 }



 }


static void Control_Temperature(uint8_t n){

	float remaining_count, count_per_degree, answer;
	int reading;

	uint8_t dutyCycle = 0;

	if(n == 1){

		reading = (scratchSensor1[1] << 8) + scratchSensor1[0];

			if (reading & 0x8000) { // negative degrees C
				reading = 0-((reading ^ 0xffff) + 1); // 2's comp then convert to signed int
			}

			answer = reading +0.0; // convert to floating point
			answer = answer / 16.0f;
			printf("Temperatura sensor_1 es: %3.1fC\r\n", answer );

			Buffer1 = answer;

			if(answer < 5){
					turnOn(ROJO);
					dutyCycle = 200;
					pwmWrite( PWM0, dutyCycle );
				}else if(answer > 25){
					turnOn(ROJO);
					dutyCycle = 200;
					pwmWrite( PWM0, dutyCycle );
				}else{
					turnOff(ROJO);
					dutyCycle = 0;
					pwmWrite( PWM0, dutyCycle );
				}
	}

	if(n == 2){
			reading = (scratchSensor2[1] << 8) + scratchSensor2[0];

				if (reading & 0x8000) { // negative degrees C
					reading = 0-((reading ^ 0xffff) + 1); // 2's comp then convert to signed int
				}

				answer = reading +0.0; // convert to floating point
				answer = answer / 16.0f;
				printf("Temperatura sensor_2 es: %3.1fC\r\n", answer );

				Buffer2 = answer;

				if(answer < 5){
						turnOn(VERDE);
						dutyCycle = 200;
						pwmWrite( PWM0, dutyCycle );
					}else if(answer > 25){
						turnOn(VERDE);
						dutyCycle = 200;
						pwmWrite( PWM0, dutyCycle );
					}else{
						turnOff(VERDE);
						dutyCycle = 0;
						pwmWrite( PWM0, dutyCycle );
					}
		}

	if(n == 3){
			reading = (scratchSensor3[1] << 8) + scratchSensor3[0];

				if (reading & 0x8000) { // negative degrees C
					reading = 0-((reading ^ 0xffff) + 1); // 2's comp then convert to signed int
				}

				answer = reading +0.0; // convert to floating point
				answer = answer / 16.0f;
				printf("Temperatura sensor_3 es: %3.1fC\r\n", answer );

				Buffer3 = answer;

				if(answer < 5){
						turnOn(AMARILLO);
						dutyCycle = 200;
						pwmWrite( PWM0, dutyCycle );
					}else if(answer > 25){
						turnOn(AMARILLO);
						dutyCycle = 200;
						pwmWrite( PWM0, dutyCycle );
					}else{
						turnOff(AMARILLO);
						dutyCycle = 0;
						pwmWrite( PWM0, dutyCycle );
					}
		}


}

void sensorInit(conection_t *pcommand){

	rtcWrite( &rtc ); 								// Establecer fecha y hora
	delayWrite(&pcommand->delay,2000);
	if(delayRead(&pcommand->delay)){
	}
	pcommand->mode = INITIALIZATION;
	pcommand->contador_secuencia = 0;
	pcommand->contSensor = 1;

 }

void masterTx_Control(conection_t *pcommand){

	uint8_t Registro;
	Registro = 0;

	switch(pcommand->mode){

	case INITIALIZATION:

		if(pcommand->contSensor != 0){
			initialize_secuence(pcommand->contSensor);
			pcommand->mode = SKIPROMCOM;

		}

		break;

	case SKIPROMCOM:

		if(pcommand->contSensor != 0){
			skip_Rom(pcommand->contSensor);
		}

		if(pcommand->contador_secuencia  == 1){
			pcommand->mode = READSCRAT;
			pcommand->contador_secuencia = 0;
		}else{
			pcommand->mode = CONVERTTEMP;
			pcommand->contador_secuencia++;
		}
		break;

	case CONVERTTEMP:

		if(pcommand->contSensor != 0){
			send_byte(CONVERT_TEMP, pcommand->contSensor);
			Conver_Temp(pcommand->contSensor);
		}
		pcommand->mode = INITIALIZATION;
		break;

	case READSCRAT:

		send_byte(READ_SCRATCH, pcommand->contSensor);
		if(Read_Scratchpad(pcommand->contSensor)== TRUE){
			Control_Temperature(pcommand->contSensor);
			delayWrite(&pcommand->delay,60000);
			if(delayRead(&pcommand->delay)){
				Registro = 1;
				writeRegister(Registro);
				Registro = 2;
				writeRegister(Registro);
				Registro = 3;
				writeRegister(Registro);
			}

		}

		pcommand->contSensor++;
		if(pcommand->contSensor == 4){
			pcommand->contSensor = 1;
		}

		pcommand->mode = INITIALIZATION;
		break;

	default:

		pcommand->mode = INITIALIZATION;
		break;

	}
}


