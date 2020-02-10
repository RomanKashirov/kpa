/*************************************************************************
Подключение 1986ВЕ91Т к Ethernet с помощью 5600ВГ1У. Интерфейс параллельный.
10.02.2020
*************************************************************************/

#ifndef _CONFIG_H_
#define _CONFIG_H_


#include <MDR32F9Qx_config.h>

void Initialize_CLK(void);
void Initialize_SPI1(void);
void Initialize_GPIO(void);
void Initialize_ExtBus(void);

#endif
