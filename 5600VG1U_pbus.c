/*************************************************************************
Подключение 1986ВЕ91Т к Ethernet с помощью 5600ВГ1У. Интерфейс параллельный.
10.02.2020
*************************************************************************/
#include "5600VG1U_pbus.h"

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
void Reset_5600VG1U(void);

int get_sample_reg(void)
{
	return Ethernet->MAC_ADDR_H;
}

void Initialize_5600VG1U_parallel(void)
{
	//MDR_PORTB->RXTX = (1<<11)|(MDR_PORTB->RXTX & 0xFFE0); // Снятие сброса
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
	Reset_5600VG1U();
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



//функция сброса контроллера 5600ВГ1У
void Reset_5600VG1U()
{
  unsigned short a;
	MDR_PORTB->RXTX &= ~((1<<11)|0x001F); // Сброс PORTB11 для подачи nRST на 5600ВГ1У
	for(a=0;a<1000;a++){}  // Минимальная длительность сигнала сброса 100 нс. TODO узнать какая сейчас задержка?
	MDR_PORTB->RXTX = (1<<11)|(MDR_PORTB->RXTX	& 0xFFE0); // Установка PORTB11 для снятия nRST
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


//--------------------------------------------------------------------------------------
//Функция для определения наличия принятого пакета
//Параметр:	RxDesc - указатель на дескриптор принимаемых пакетов
//Возвращает:	1 - пакет не принят
//				0 - пакет принят
//--------------------------------------------------------------------------------------
unsigned short Read_Rx_Descriptor(_rx_descriptor* RxDesc)
{
		unsigned int* RDescPointer;
		RDescPointer = (unsigned int*)RxDesc->StartAddress;
        if( (*RDescPointer & 0x8000) == 0x8000) return 1;	
        else return 0;                                      
}

//--------------------------------------------------------------------------------------
//Функция для получения размера приянтого пакета
//Параметр:	RxDesc - указатель на дескриптор принимаемых пакетов
//Возвращает:	размер пакета в байтах
//--------------------------------------------------------------------------------------
unsigned short Read_Packet_Length(_rx_descriptor* RxDesc)
{
		unsigned int* RDescPointer;
		RDescPointer = (unsigned int*)(RxDesc->StartAddress + 4);		//получим адрес длины пакета
        return (unsigned short)(*RDescPointer);       //Temp;
}


//--------------------------------------------------------------------------------------
//Функция для получения адреса, по которому расположен пакета в буфере приемника
//Параметр:	RxDesc - указатель на дескриптор принимаемых пакетов
//Возвращает:	адрес пакета в буфере
//--------------------------------------------------------------------------------------
unsigned short Read_Packet_Start_Address(_rx_descriptor* RxDesc)
{
        unsigned int* RDescPointer;
		RDescPointer = (unsigned int*)(RxDesc->StartAddress + 12);	//получим адрес стартового адреса пакета
        return (unsigned short)(*RDescPointer);
}

//--------------------------------------------------------------------------------------
//Функция для установки признака готовности к приему пакета дескриптором принимаемых пакетв
//Параметр:	RxDesc - указатель на дескриптор принимаемых пакетов
//Возвращает:	0
//--------------------------------------------------------------------------------------
int Ready_Rx_Descriptor(_rx_descriptor* RxDesc)
{
		unsigned int* MyDesc;
		MyDesc = (unsigned int*)(RxDesc->StartAddress);
		if(RxDesc->LastDesc == 1) *MyDesc = 0xC000;	//установка готовности дескриптора к приему пакета
		else *MyDesc = 0x8000;						//установка готовности дескриптора к приему пакета
        return 0;
}

//Функция для подготовки дескрипторов передатчика к отправке пакетов
//Параметр:	PacketLen - размер пакета в байтах,
//			TxCurrentDesc - указатель на текущий дескриптор отправляемых пакетов
//Возвращаемых значений нет
//--------------------------------------------------------------------------------------
int Write_Tx_Descriptor(unsigned short PacketLen, _tx_current_descriptor* TxCurrentDesc)
{
	unsigned int* MyDesc;
	MyDesc = (unsigned int*)(TxCurrentDesc->TxCurrentDescriptor->StartAddress);

	*(MyDesc+1) = PacketLen;
	*(MyDesc+3) = (TxCurrentDesc->FirstEmptyWord>>2)&0x0000FFFF;

	if(TxCurrentDesc->TxCurrentDescriptor->LastDesc == 1) *MyDesc = 0xC000;
	else *MyDesc = 0x8000;

	TxCurrentDesc->FirstEmptyWord += (PacketLen/2)<<2;	//адрес первого свободного 16-разрядного слова в буфере
												//сдвг PacketLen на 1 разряд, так как в передаваемом пакете
												//необходимо указывать размер в байтах, а в буфере - 16-разрядные слова!
	if(TxCurrentDesc->FirstEmptyWord > 0x60005FFC)
	{
		TxCurrentDesc->FirstEmptyWord -= 0x00006000;
		TxCurrentDesc->FirstEmptyWord |= 0x60004000;
	}
    return 0;
}

//--------------------------------------------------------------------------------------
//Функция для определения статуса отправляемого пакета
//Параметр:	TxDesc - указатель на дескриптор отправляемых пакетов
//Возвращает:	1 - пакет не отправлен
//				0 - пакет отправлен 
//--------------------------------------------------------------------------------------
unsigned short Read_Tx_Descriptor(_tx_descriptor* TxDesc)
{
		unsigned int* MyDesc;
		MyDesc = (unsigned int*)(TxDesc->StartAddress);
        if((*MyDesc & 0x8000) == 0x8000) return 1;            //пакет не отправлен
        else return 0;                                              //пакет отправлен
}
