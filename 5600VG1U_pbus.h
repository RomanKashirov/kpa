/*************************************************************************
Подключение 1986ВЕ91Т к Ethernet с помощью 5600ВГ1У. Интерфейс параллельный.
10.02.2020
*************************************************************************/
#ifndef _5600VG1U_PBUS_H_
#define _5600VG1U_PBUS_H_

//Управляющие регистры Ethernrt-контроллера 5600ВГ1У
typedef struct
{
	unsigned int MAC_CTRL;
    unsigned int MinFrame;
    unsigned int MaxFrame;
    unsigned int CollConfig;
    unsigned int IPGTx;
    unsigned int MAC_ADDR_T;
    unsigned int MAC_ADDR_M;
    unsigned int MAC_ADDR_H;
    unsigned int HASH0;
    unsigned int HASH1;
    unsigned int HASH2;
    unsigned int HASH3;
    unsigned int INT_MSK;
    unsigned int INT_SRC;
    unsigned int PHY_CTRL;
    unsigned int PHY_STAT;
    unsigned int RXBF_HEAD;
    unsigned int RXBF_TAIL;
    unsigned int dammy0;
    unsigned int dammy1;
    unsigned int STAT_RX_ALL;
    unsigned int STAT_RX_OK;
    unsigned int STAT_RX_OVF;
    unsigned int STAT_RX_LOST;
    unsigned int STAT_TX_ALL;
    unsigned int STAT_TX_OK;
    unsigned int base_RxBF;
    unsigned int base_TxBF;
    unsigned int base_RxBD;
    unsigned int base_TxBD;
    unsigned int base_RG;
    unsigned int GCTRL;
} _ethernet;


//Стартовый адрес буфера приемника
#define BASE_ETH_RXBuffer                  0x60000000  //Absolut ADDRESS = 0x0000

//Стартовый адрес буфера дескрипторов принимаемых пакетов
#define BASE_ETH_RXDescBuffer              0x60002000  //Absolut ADDRESS = 0x0800

//Стартовый адрес передатчика
#define BASE_ETH_TXBuffer                  0x60004000  //Absolut ADDRESS = 0x1000

//Стартовый адрес буфера дескрипторов передаваемых пакетов
#define BASE_ETH_TXDescBuffer              0x60006000  //Absolut ADDRESS = 0x1800

//Стартовый адрес управляющих регистров Ethernrt-контроллера 5600ВГ1У
#define BASE_ETHERNET                      0x60007F00  //Absolut ADDRESS = 0x1FC0

#define Ethernet              ((_ethernet *)BASE_ETHERNET)

#define RxBuffer              ((unsigned int *)BASE_ETH_RXBuffer)
#define RxDescriptor          ((unsigned int *)BASE_ETH_RXDescBuffer)

#define TxBuffer              ((unsigned int *)BASE_ETH_TXBuffer)
#define TxDescriptor          ((unsigned int *)BASE_ETH_TXDescBuffer)


#endif /* _5600VG1U_PBUS_H_ */


