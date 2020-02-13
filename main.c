/*************************************************************************
Подключение 1986ВЕ91Т к Ethernet с помощью 5600ВГ1У. Интерфейс параллельный.
10.02.2020
*************************************************************************/

#include "main.h"
#include <MDR32F9Qx_port.h>

#ifdef TEST
int reg_sample;
int port_sample;
#endif


int main()
{
	
	Initialize_CLK(); 
	Initialize_GPIO();
	
	
	Initialize_ExtBus();

	Initialize_5600VG1U_parallel();
//	Initialize_ethernet();
	#ifdef TEST
	MDR_PORTD->RXTX = (1<<11)|(MDR_PORTD->RXTX & 0xFFE0);
		reg_sample = get_sample_reg();
	if(reg_sample == 0x5F4E)
	{
		SET_LED5();
	}
		#endif
	
	while(1)
	{
		
	
	
//		Polling_Ethernet();
	}
}
