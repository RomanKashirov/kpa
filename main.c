/*************************************************************************
Подключение 1986ВЕ91Т к Ethernet с помощью 5600ВГ1У. Интерфейс параллельный.
10.02.2020
*************************************************************************/

#include "main.h"


int main()
{
	
	Initialize_CLK(); 
	Initialize_GPIO();
	Initialize_ExtBus();

	Initialize_5600VG1U_parallel();
//	Initialize_ethernet();
	
	while(1)
	{
//		Polling_Ethernet();
	}
}
