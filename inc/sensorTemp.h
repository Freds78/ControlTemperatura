/*=============================================================================
 * Author: Freds Amundaray Cruz <freds.amundaray@gmail.com>
 * Date: 2020/04/28
 * Version: 1
 *===========================================================================*/

/*=====[Avoid multiple inclusion - begin]====================================*/

#ifndef __S_TEMP_H__
#define __S_TEMP_H__

/*=====[Inclusions of public function dependencies]==========================*/

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "sapi.h"

/*=====[C++ - begin]=========================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*=====[Definition macros of public constants]===============================*/


/*=====[Public function-like macros]=========================================*/

/*=====[Definitions of public data types]====================================*/

typedef enum{
	INITIALIZATION,
	SKIPROMCOM,
	CONVERTTEMP,
	READSCRAT
}RomFun_Comm_t;

typedef struct{
	RomFun_Comm_t mode;
	delay_t delay;
	uint8_t contador;
}conection_t;

/*=====[Prototypes (declarations) of public functions]=======================*/
void masterTx_Control(conection_t *command);
void sensorInit(conection_t *command);
void diskTickHook( void *ptr );

/*=====[Prototypes (declarations) of public interrupt functions]=============*/

/*=====[C++ - end]===========================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Avoid multiple inclusion - end]======================================*/

#endif /* __S_TEMP_H__ */
