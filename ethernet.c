/*************************************************************************
Подключение 1986ВЕ91Т к Ethernet с помощью 5600ВГ1У. Интерфейс параллельный.
10.02.2020
*************************************************************************/
#include "ethernet.h"
#include <MDR32F9Qx_port.h>
extern _rx_current_descriptor RxCurrentDesc;	//экземпляр структуры _rx_current_descriptor 


//Структура для работы с принятыми Ethernet-пакетами
typedef struct
{
        unsigned char Data[1600];       //Данные пакета
        unsigned short Length;          //Размер принятого пакета
        unsigned short Address;         //Адрес, по которому расположен пакет в буфере приемника
        unsigned char Remote_IP[4];     //удаленный IP-адрес (IP-адрес отправителя принятого пакета)
        unsigned char Remote_MAC[6];    //удаленный MAC-адрес (MAC-адрес отправителя принятого пакета)

#if PRINTSTATUSON
		unsigned int CounterTxPacket;	//Счетчик отправленных пакетов
		unsigned int CounterRxPacket;	//Счетчик принятых пакетов
#endif	//PRINTSTATUSON

} _ethernet_packet;

unsigned char My_IP[] = {192, 168, 1, 11}, My_MAC[] = {0x0A, 0x1B, 0x2C, 0x3D, 0x4E, 0x5F};		//ip-адрес нашего устройства, MAC-адрес нашего устройства
unsigned char Receive_IP[4];			//ip-адрес принятого пакета
unsigned char Answear_buffer[1500]; 	 	//буфер для формирования ответного ICMP-пакета
_ethernet_packet Packet;				//экземпляр структуры _ethernet_packet
unsigned short TypeOfProtocol;	//TypeOfProtocol - тип протокола пакета, вложенного в Eth2 пакет.

int Read_Packet(_ethernet_packet* Dt);




void Polling_Ethernet(void)
{
	if(Read_Rx_Descriptor(RxCurrentDesc.RxCurrentDescriptor) == 0) //проверка наличия принятого пакета
	{
		
		Packet.Length = Read_Packet_Length(RxCurrentDesc.RxCurrentDescriptor);
    Packet.Address = Read_Packet_Start_Address(RxCurrentDesc.RxCurrentDescriptor);
    Read_Packet(&Packet);				   //считали полученный пакет

		Ready_Rx_Descriptor(RxCurrentDesc.RxCurrentDescriptor);

#if	PRINTSTATUSON
						Packet.CounterRxPacket++;
#endif	//PRINTSTATUSON						

		Ethernet->RXBF_HEAD = (Ethernet->RXBF_TAIL-1)&0x07FF;

		TypeOfProtocol = (Packet.Data[12]<<8)|Packet.Data[13];
		if(TypeOfProtocol == 0x0806)
		{
			MDR_PORTD->RXTX = (1<<12)|(MDR_PORTD->RXTX & 0xFFE0);	
		}
			
                        
	}
   
}

//--------------------------------------------------------------------------------------
//Функция для считывания пакета
//Параметр:	указатель на принятый Ethernet-пакет
//Возвращает 0
//--------------------------------------------------------------------------------------
int Read_Packet(_ethernet_packet* Dt)
{
        unsigned short Temp,Val;
		unsigned int* MyPointer;
		MyPointer = (unsigned int*)((unsigned int)RxBuffer + (unsigned int)(Dt->Address<<2));
		for(Temp=0;Temp<Dt->Length;Temp+=2)
		{
			Val = *MyPointer++;
			Dt->Data[Temp] = (unsigned char)Val;
			Dt->Data[Temp+1] = (unsigned char)(Val>>8);
			if( ((int)MyPointer & 0x00002000) == 0x00002000) MyPointer = RxBuffer;
		}
        if((Dt->Length & 0x0001) == 1)  //если пакет содержит нечетное кол-во байт, то необходимо занулить предпоследний
                                        //считанный байт в буфере, так как он влияет на контрольную сумму ICMP-пакета.
        {
                Dt->Data[Dt->Length-4] = 0;
        }
		
        return 0;
}
