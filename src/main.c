/*
 * main.c
 *
 *  Created on: 28 abr. 2020
 *      Author: freds
 */

/*=====[Main function, program entry point after power on or reset]==========*/
#include "sensorTemp.h"
#include "sapi.h"       // <= sAPI header
#include "ff.h"       // <= Biblioteca FAT FS
#include "fssdc.h"	// API de bajo nivel para unidad "SDC:" en FAT FS

int main( void )
{
   // ----- Setup -----------------------------------
   boardInit();
   uartConfig( UART_USB, 115200 );

   /* Configurar PWM */
   pwmConfig( 0, PWM_ENABLE );
   pwmConfig( PWM0, PWM_ENABLE_OUTPUT );


   // SPI configuration
   spiConfig( SPI0 );

   // Inicializar el conteo de Ticks con resolucion de 10ms,
   // con tickHook diskTickHook
   tickConfig( 10 );
   tickCallbackSet( diskTickHook, NULL );

   FSSDC_InitSPI ();
   FATFS fs;
   if( f_mount( &fs, "SDC:", 0 ) != FR_OK ){
	   gpioToggle( LEDR );
	   printf("SD no disponible\n");
	   delay_t temp;
	   delayWrite(&temp,1000);
	   if(delayRead(&temp)){
	   }
   }

   rtcInit(); // Inicializar RTC

   conection_t com;

   sensorInit(&com);

   // ----- Repeat for ever -------------------------
   while( TRUE ) {

		masterTx_Control(&com);
   }

   // YOU NEVER REACH HERE, because this program runs directly or on a
   // microcontroller and is not called by any Operating System, as in the
   // case of a PC program.
   return 0;
}



