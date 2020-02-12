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
	PORT_SetBits(MDR_PORTB, PORT_Pin_11);
	Initialize_ExtBus();

	Initialize_5600VG1U_parallel();
//	Initialize_ethernet();
	#ifdef TEST
		reg_sample = get_sample_reg();
		port_sample = MDR_PORTB->RXTX;
		#endif
	
	while(1)
	{
		
	SET_LED5();
		port_sample++;
		PORT_SetBits(MDR_PORTB, PORT_Pin_11);
//		Polling_Ethernet();
	}
}
