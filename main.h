
#include "config.h"
//#include "5600VG1U_serial.h"
//#include "ethernet.h"
#include <MDR32F9Qx_port.h>

#define SET_LED5() MDR_PORTD->RXTX = (1<<14)|(MDR_PORTD->RXTX & 0xFFE0)
#define RESET_LED5() MDR_PORTD->RXTX &= ~((1<<14)|0x001F);
