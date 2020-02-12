/*************************************************************************
Подключение 1986ВЕ91Т к Ethernet с помощью 5600ВГ1У. Интерфейс параллельный.
10.02.2020
*************************************************************************/
#include "5600VG1U_pbus.h"

_ethernet* MyEth;
void EthCfg(void);

void Initialize_5600VG1U_parallel(void)
{
	MyEth = Ethernet;
	EthCfg();
}

//--------------------------------------------------------------------------------------
//Функция для инициализации Ethernet-контроллера 5600ВГ1У
//Параметр:	нет
//Возвращаемых значений нет
//--------------------------------------------------------------------------------------
void EthCfg()
{
	Ethernet->GCTRL = 0x8000;	//Reset 5600VG1U        
	for(int i=0;i<1000;i++){}		//Little delay
	Ethernet->GCTRL = 0x4382;						   	
	Ethernet->MAC_CTRL = 0x0200;  //Receive broadcast packet and packet with my MAC-address, littel endian ON
	Ethernet->MinFrame = 0x0040;
  Ethernet->CollConfig = 0;
  Ethernet->IPGTx = 0x000A;
  Ethernet->MAC_ADDR_T = 0x1B0A;
  Ethernet->MAC_ADDR_M = 0x3D2C;
  Ethernet->MAC_ADDR_H = 0x5F4E;
  Ethernet->PHY_CTRL = 0x31D0;
}
