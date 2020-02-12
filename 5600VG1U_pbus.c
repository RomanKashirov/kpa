/*************************************************************************
Подключение 1986ВЕ91Т к Ethernet с помощью 5600ВГ1У. Интерфейс параллельный.
10.02.2020
*************************************************************************/
#include "5600VG1U_pbus.h"

#define NUMRXDESCRIPTOR	16	//количество дескрипротов принимаемых пакетов (минимум 1, максимум 32)
#define NUMTXDESCRIPTOR	16	//количество дескрипротов отправляемых пакетов (минимум 1, максимум 32)
//Структура для дескриптора принимаемых пакетов
typedef struct
{
	unsigned int	StartAddress;			//начальный адрес дескриптора в памяти контроллера 1986ВЕ91Т1
	unsigned short	Status;					//стату дескриптора
	unsigned short	PacketLength;			//длина принятого пакета
	unsigned short	PacketStartAddressH;	//адрес в буфере, где расположен первый байт пакета (старшая часть, всегда 0х0000)
	unsigned short	PacketStartAddressL;	//адрес в буфере, где расположен первый байт пакета (младшая часть)
	unsigned short	LastDesc;				//признак последнего дескриптора (1 - последний дескриптор, 0 - не последний дескриптор)
} _rx_descriptor;

//Структура для работы с текущим дескриптором принимаемых пакетов
typedef struct
{
	_rx_descriptor* RxCurrentDescriptor;	//указатель на текущий дескриптор принимаемых пакетов
	unsigned short Number;					//номер текущего дескриптора (принимает значения от 0 до NUMRXDESCRIPTOR-1)
} _rx_current_descriptor;

//Структура для дескриптора отправляемых пакетов
typedef struct
{
	unsigned int	StartAddress;			//начальный адрес дескриптора в памяти контроллера 1986ВЕ91Т1
	unsigned short	Status;					//стату дескриптора
	unsigned short	PacketLength;			//длина принятого пакета
	unsigned short	PacketStartAddressH;	//адрес в буфере, где расположен первый байт пакета (старшая часть, всегда 0х0000)
	unsigned short	PacketStartAddressL;	//адрес в буфере, где расположен первый байт пакета (младшая часть)
	unsigned short	LastDesc;				//признак последнего дескриптора (1 - последний дескриптор, 0 - не последний дескриптор)
} _tx_descriptor;

//Структура для работы с текущим дескриптором отпарвляемых пакетов
typedef struct
{
	_tx_descriptor* TxCurrentDescriptor;	//указатель на текущий дескриптор отправляемых пакетов
	unsigned short Number;					//номер текущего дескриптора (принимает значения от 0 до NUMTXDESCRIPTOR-1)
	unsigned int FirstEmptyWord;			//адрес первого свободного слова в буфере передатчика
} _tx_current_descriptor;


//массив дескрипторов отправляемых пакетов
_tx_descriptor ATxDescriptor[32] =  {{0x60006000,0x2000,0,0,0,0},{0x60006010,0x2000,0,0,0,0},{0x60006020,0x2000,0,0,0,0},{0x60006030,0x2000,0,0,0,0},
									 {0x60006040,0x2000,0,0,0,0},{0x60006050,0x2000,0,0,0,0},{0x60006060,0x2000,0,0,0,0},{0x60006070,0x2000,0,0,0,0},
									 {0x60006080,0x2000,0,0,0,0},{0x60006090,0x2000,0,0,0,0},{0x600060A0,0x2000,0,0,0,0},{0x600060B0,0x2000,0,0,0,0},
									 {0x600060C0,0x2000,0,0,0,0},{0x600060D0,0x2000,0,0,0,0},{0x600060E0,0x2000,0,0,0,0},{0x600060F0,0x2000,0,0,0,0},
									 {0x60006100,0x2000,0,0,0,0},{0x60006110,0x2000,0,0,0,0},{0x60006120,0x2000,0,0,0,0},{0x60006130,0x2000,0,0,0,0},
									 {0x60006140,0x2000,0,0,0,0},{0x60006150,0x2000,0,0,0,0},{0x60006160,0x2000,0,0,0,0},{0x60006170,0x2000,0,0,0,0},
									 {0x60006180,0x2000,0,0,0,0},{0x60006190,0x2000,0,0,0,0},{0x600061A0,0x2000,0,0,0,0},{0x600061B0,0x2000,0,0,0,0},
									 {0x600061C0,0x2000,0,0,0,0},{0x600061D0,0x2000,0,0,0,0},{0x600061E0,0x2000,0,0,0,0},{0x600061F0,0x6000,0,0,0,1}};

//массив дескрипторов принимаемых пакетов
_rx_descriptor	ARxDescriptor[32] = {{0x60002000,0xA000,0,0,0,0},{0x60002010,0xA000,0,0,0,0},{0x60002020,0xA000,0,0,0,0},{0x60002030,0xA000,0,0,0,0},
									 {0x60002040,0xA000,0,0,0,0},{0x60002050,0xA000,0,0,0,0},{0x60002060,0xA000,0,0,0,0},{0x60002070,0xA000,0,0,0,0},
									 {0x60002080,0xA000,0,0,0,0},{0x60002090,0xA000,0,0,0,0},{0x600020A0,0xA000,0,0,0,0},{0x600020B0,0xA000,0,0,0,0},
									 {0x600020C0,0xA000,0,0,0,0},{0x600020D0,0xA000,0,0,0,0},{0x600020E0,0xA000,0,0,0,0},{0x600020F0,0xA000,0,0,0,0},
									 {0x60002100,0xA000,0,0,0,0},{0x60002110,0xA000,0,0,0,0},{0x60002120,0xA000,0,0,0,0},{0x60002130,0xA000,0,0,0,0},
									 {0x60002140,0xA000,0,0,0,0},{0x60002150,0xA000,0,0,0,0},{0x60002160,0xA000,0,0,0,0},{0x60002170,0xA000,0,0,0,0},
									 {0x60002180,0xA000,0,0,0,0},{0x60002190,0xA000,0,0,0,0},{0x600021A0,0xA000,0,0,0,0},{0x600021B0,0xA000,0,0,0,0},
									 {0x600021C0,0xA000,0,0,0,0},{0x600021D0,0xA000,0,0,0,0},{0x600021E0,0xA000,0,0,0,0},{0x600021F0,0xE000,0,0,0,1}};

_tx_current_descriptor TxCurrentDesc;	//экземпляр структуры _tx_current_descriptor
_rx_current_descriptor RxCurrentDesc;	//экземпляр структуры _rx_current_descriptor


_ethernet* MyEth;
void EthCfg(void);  //функция конфигурирования контроллера 5600ВГ1У
void InitTxDescriptor(void);						// функция инициализации дескрипторов отправляемых пакетов
void InitRxDescriptor(void);						// функция инициализации дескрипторов принимаемых пакетов


int get_sample_reg(void)
{
	return Ethernet->GCTRL;
}

void Initialize_5600VG1U_parallel(void)
{
	MyEth = Ethernet;
	EthCfg();
	InitTxDescriptor();	
	InitRxDescriptor();
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

//--------------------------------------------------------------------------------------
//Функция для инициализации дескрипторов передаваемых пакетов
//Параметр:	нет
//Возвращаемых значений нет
//--------------------------------------------------------------------------------------
void InitTxDescriptor()
{
	unsigned int* TDescPointer;

	TxCurrentDesc.TxCurrentDescriptor = ATxDescriptor;
	TxCurrentDesc.FirstEmptyWord = 0x60004000;

	for(int i=0;i<NUMTXDESCRIPTOR;i++)
	{
		TDescPointer = (unsigned int*)(TxCurrentDesc.TxCurrentDescriptor->StartAddress);
		*TDescPointer = TxCurrentDesc.TxCurrentDescriptor->Status;
		TxCurrentDesc.TxCurrentDescriptor++;
	}

	TxCurrentDesc.TxCurrentDescriptor--;
	TxCurrentDesc.TxCurrentDescriptor->LastDesc = 1;	//установка признака последнего дескриптора
	TDescPointer = (unsigned int*)(TxCurrentDesc.TxCurrentDescriptor->StartAddress);
	*TDescPointer = 0x4000;								//последний обрабатываемый дескриптор

	TxCurrentDesc.Number = 0;
	TxCurrentDesc.TxCurrentDescriptor = ATxDescriptor;
}
//--------------------------------------------------------------------------------------
//Функция для инициализации дескрипторов принимаемых пакетов
//Параметр:	нет
//Возвращаемых значений нет
//--------------------------------------------------------------------------------------
void InitRxDescriptor()
{
	unsigned int* RDescPointer;

	RxCurrentDesc.RxCurrentDescriptor = ARxDescriptor;

	for(int i=0;i<NUMRXDESCRIPTOR;i++)
	{
		RDescPointer = (unsigned int*)(RxCurrentDesc.RxCurrentDescriptor->StartAddress);
		*RDescPointer = RxCurrentDesc.RxCurrentDescriptor->Status;
		RxCurrentDesc.RxCurrentDescriptor++;
	}

	RxCurrentDesc.RxCurrentDescriptor--;
	RxCurrentDesc.RxCurrentDescriptor->LastDesc = 1;	//установка признака последнего дескриптора
	RDescPointer = (unsigned int*)(RxCurrentDesc.RxCurrentDescriptor->StartAddress);
	*RDescPointer = 0xC000;								//последний обрабатываемый дескриптор

	RxCurrentDesc.Number = 0;
	RxCurrentDesc.RxCurrentDescriptor = ARxDescriptor;
}




