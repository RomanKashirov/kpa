/*************************************************************************
Подключение 1986ВЕ91Т к Ethernet с помощью 5600ВГ1У. Интерфейс параллельный.
10.02.2020
*************************************************************************/
#ifndef _MAIN_H_
#define _MAIN_H_

#include "config.h"
//#include "5600VG1U_serial.h"
//#include "ethernet.h"
#include <MDR32F9Qx_port.h>

#define SET_LED5() MDR_PORTD->RXTX = (1<<14)|(MDR_PORTD->RXTX & 0xFFE0)
#define RESET_LED5() MDR_PORTD->RXTX &= ~((1<<14)|0x001F);

#endif /* _MAIN_H_ */
