/*************************************************************************
Подключение 1986ВЕ91Т к Ethernet с помощью 5600ВГ1У. Интерфейс параллельный.
10.02.2020
*************************************************************************/

#include "main.h"
/*Тестовая информация:
LED1 - Включился генератор и порты ввода-вывода Установлена связь с 5600ВГ1У
LED2 - Принят пакет с нашим или широковещательным адресом
LED3 - Принят ARP пакет с нашим IP
LED4 - Получили IP-пакет с нашим IP адресом и корректной чек-суммой
LED5 - Принят ICMP-пакет с корректной чек-суммой
*/
 
#ifdef TEST_MODE
#warning "TEST_MODE VERSION"
int reg_sample;
int port_sample;
#endif

int main()
{
	Initialize_CLK(); 
	Initialize_GPIO();
	Initialize_ExtBus();
	Initialize_5600VG1U_pbus();
	
#ifdef TEST_MODE // Идет проверка правильной инициализации 5600ВГ1У
	if(get_sample_reg() == 0x5F4E)
	{
		SET_LED1(); //LED1 - Включился генератор и порты ввода-вывода Установлена связь с 5600ВГ1У
	}
#endif
	
	while(1)
	{
		Polling_Ethernet();
	}
}
