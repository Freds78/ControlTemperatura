/*=============================================================================
 * Author: Freds Amundaray Cruz <freds.amundaray@gmail.com>
 * Date: 2020/04/28
 * Version: 1
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/
#include "Leds.h"
#include "sensorTemp.h"
#include "sapi.h"
#include <stdint.h>
#include <stdbool.h>
#include "ff.h"       // <= Biblioteca FAT FS
#include "fssdc.h"	// API de bajo nivel para unidad "SDC:" en FAT FS
#include <math.h>
/*=====[Definition macros of private constants]==============================*/
#define AMARILLO LED1
#define ROJO LED2
#define VERDE LED3
#define SKIP_ROMCOM 0xCC
#define CONVERT_TEMP 0x44
#define READ_SCRATCH 0xBE
#define FILENAME "SDC:/Temp.txt"


/*=====[Definitions of extern global variables]==============================*/
typedef unsigned int	UINT;
static float Buffer;
static uint8_t scratch[9];
static  FIL fp;             // <-- File object needed for each open file
static rtc_t rtc = {
   2020,
   6,
   23,
   4,
   13,
   10,
   0
};
/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

static void skip_Rom();
static void Read_Scratchpad();
static uint8_t crc8 ();
static void inicialize_secuence();
static void send_bit(uint8_t bit_data);
static uint8_t read_bit();
static void send_byte(uint8_t data);
static uint8_t read_byte();
static void Control_Temperature();
static void writeRegister();

static void skip_Rom(){
	send_byte(SKIP_ROMCOM);
}

static void Read_Scratchpad(){

	int8_t i;
	for (i = 0; i < 9; i++) {
		scratch[i] = read_byte();
	}
	if (crc8() != 0){
		inicialize_secuence();
	}
}


static uint8_t crc8 () {
 uint8_t div = 0b10001100; // Rotated poly
 uint8_t crc = 0;
 int8_t i;

 for (i = 0; i < 8; i++) {
	 uint8_t byte = scratch[i];
	 int8_t j;
	 for (j = 0; j < 8; j++) {
		 uint8_t crc_carry = crc & 1;
		 crc = crc >> 1;
		 uint8_t byte_carry = byte & 1;
		 byte = byte >>1;
		 if (crc_carry ^ byte_carry)
			 crc ^= div;
	 }
 }
 return crc;
}

/* Sends 8 bit in a row, LSB first */
static void send_byte(uint8_t data) {
 int8_t i;

 for (i = 0; i < 8; i++) {
	 send_bit(data & 0x01);
	 data = data >> 1;
 }
}


static void send_bit(uint8_t bit_data) {


	gpioConfig( GPIO2, GPIO_OUTPUT );
	gpioWrite( GPIO2, 0 );
	delayInaccurateUs( 2 );
    if (bit_data) {
    	gpioWrite( GPIO2, 1 );
    	delayInaccurateUs( 60 );
     } else {
    	delayInaccurateUs( 60 );
    	gpioWrite( GPIO2, 1 );
    	delayInaccurateUs( 10 );
    }

}

/* Reads a byte, LSB first */
static uint8_t read_byte() {
	uint8_t byte = 0;
	int8_t i;
	uint8_t j;

	for (i=0; i < 8; i++) {
		j = read_bit();
		j = j << i;
		byte = byte | j;
	}
	return byte;
}

static uint8_t read_bit() {

	uint8_t read;
	gpioConfig( GPIO2, GPIO_OUTPUT );
	gpioWrite( GPIO2, 0 );
	delayInaccurateUs( 3 );
	gpioConfig( GPIO2, GPIO_INPUT );
	delayInaccurateUs( 10 );
	read = gpioRead( GPIO2 );
	delayInaccurateUs( 55 );
	return read;
}

static void inicialize_secuence(){

	gpioConfig( GPIO2, GPIO_OUTPUT );
	gpioWrite( GPIO2, 0 );
	delayInaccurateUs( 500 );
	gpioConfig( GPIO2, GPIO_INPUT );
	delayInaccurateUs( 90 );
	delayInaccurateUs( 410 );
}

// FUNCION que se ejecuta cada vezque ocurre un Tick
 void diskTickHook( void *ptr )
{
   disk_timerproc();   // Disk timer process
}


static void writeRegister(){

	rtcRead( &rtc );
	char bufferRegistre[100];

	UINT n = sprintf( bufferRegistre, "La temperatura es: %.2f C %04d-%02d-%02d %02d:%02d:%02d\r\n",
			Buffer,
			rtc.year,
			rtc.month,
			rtc.mday,
			rtc.hour,
			rtc.min,
			rtc.sec
	);

	if( f_open( &fp, FILENAME, FA_WRITE | FA_OPEN_APPEND  ) == FR_OK ){ //| FA_OPEN_APPEND

        UINT nbytes;
        f_write( &fp, bufferRegistre, n, &nbytes );

        f_close(&fp);

		if( nbytes == n ){
			printf("Escribio correctamente\n");
			gpioWrite( LEDG, ON );
		} else {
			gpioWrite( LEDR, ON );
			printf("Escribio %d bytes\n", nbytes);
		}

	} else{
		printf("Error al abrir el archivo\n");
		gpioWrite( LEDR, ON );
	}
}


static void Control_Temperature(){

	float remaining_count, count_per_degree, answer;
	int reading;
	uint8_t dutyCycle = 0;
	reading = (scratch[1] << 8) + scratch[0];

	if (reading & 0x8000) { // negative degrees C
		reading = 0-((reading ^ 0xffff) + 1); // 2's comp then convert to signed int
	}

	answer = reading +0.0; // convert to floating point
	answer = answer / 16.0f;
	printf("La Temperatura es: %3.1fC\r\n", answer );

	Buffer = answer;

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

void sensorInit(conection_t *pcommand){

	rtcWrite( &rtc ); // Establecer fecha y hora
	delayWrite(&pcommand->delay,2000);
	if(delayRead(&pcommand->delay)){
	}
	pcommand->mode = INITIALIZATION;
	pcommand->contador = 0;
 }

 void masterTx_Control(conection_t *pcommand){

	 switch(pcommand->mode){
	 case INITIALIZATION:
		 inicialize_secuence();
		 pcommand->mode = SKIPROMCOM;
		 break;
	 case SKIPROMCOM:
		 skip_Rom();

		 if(pcommand->contador == 1){
			 pcommand->mode = READSCRAT;
			 pcommand->contador = 0;
		 }else{
			 pcommand->mode = CONVERTTEMP;
			 pcommand->contador++;
		 }
		 break;
	 case CONVERTTEMP:
		 send_byte(CONVERT_TEMP);
		 gpioConfig( GPIO2, GPIO_INPUT );
		 while(gpioRead( GPIO2 ) == 0){
			 delay(750);
			 printf("espera que llegue la confirmación\n");
		 }
		 pcommand->mode = INITIALIZATION;
		 break;
	 case READSCRAT:
		 send_byte(READ_SCRATCH);
		 Read_Scratchpad();
		 Control_Temperature();
		 writeRegister();
		 delay(3000);
		 pcommand->mode = INITIALIZATION;
		 break;
	 default:
		 pcommand->mode = INITIALIZATION;
		 break;
	 }
 }


