#ifndef PORT_H
#define PORT_H
#include "FreeRTOSConfig.h"

#define portINITAL_XPSR         ( 0x01000000 )
#define portSTART_ADDRESS_MASK  ( ( StackType_t ) 0xfffffffeUL )

static void prvTaskExitError( void );

StackType_t *pxPortInitialiseStack(StackType_t *pxTopOfStack,
                                   TaskFunction_t pxCode,
                                   void *pvParameters);

void vPortEnterCritical( void )

#endif /* PORT_H */