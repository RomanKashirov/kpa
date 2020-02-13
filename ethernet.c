/*************************************************************************
Подключение 1986ВЕ91Т к Ethernet с помощью 5600ВГ1У. Интерфейс параллельный.
10.02.2020
*************************************************************************/
#include "ethernet.h"
#include <MDR32F9Qx_port.h>
extern _rx_current_descriptor RxCurrentDesc;	//экземпляр структуры _rx_current_descriptor
extern _tx_current_descriptor TxCurrentDesc;	//экземпляр структуры _tx_current_descriptor
extern _tx_descriptor ATxDescriptor[32];



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
int Answear_ARP(_ethernet_packet* Dt);




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

		
	
		
		
		switch(TypeOfProtocol) //0x0006 //слова в памяти разбиты по 2 байта! (считывать массив кратный 2 байтам!)
    {
					//Если принят ARP-пакет
			case 0x0806:
				MDR_PORTD->RXTX = (1<<12)|(MDR_PORTD->RXTX & 0xFFE0);
				Answear_ARP(&Packet);
				break;
			
			//Если принят IP-пакет
			case 0x0800:
//				Answear_IP(&Packet);
				break;
			
			default:
				break;
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


//Функция для формирования ответного ARP-пакета
//Dt - указатель на пакет
//возвращает 0
int Answear_ARP(_ethernet_packet* Dt)
{
	Receive_IP[0] = Packet.Data[38];
                                        Receive_IP[1] = Packet.Data[39];
                                        Receive_IP[2] = Packet.Data[40];
                                        Receive_IP[3] = Packet.Data[41];

                                        if( (Receive_IP[0] == My_IP[0])&&(Receive_IP[1] == My_IP[1])&&(Receive_IP[2] == My_IP[2])&&(Receive_IP[3] == My_IP[3]))         //полное совпадение IP в пакете и нашего. Надо отвечать
                                        {
                                                        Packet.Remote_IP[0] = Packet.Data[28];
                                                        Packet.Remote_IP[1] = Packet.Data[29];
                                                        Packet.Remote_IP[2] = Packet.Data[30];
                                                        Packet.Remote_IP[3] = Packet.Data[31];

                                                        Packet.Remote_MAC[0] = Packet.Data[22];
                                                        Packet.Remote_MAC[1] = Packet.Data[23];
                                                        Packet.Remote_MAC[2] = Packet.Data[24];
                                                        Packet.Remote_MAC[3] = Packet.Data[25];
                                                        Packet.Remote_MAC[4] = Packet.Data[26];
                                                        Packet.Remote_MAC[5] = Packet.Data[27];

                                                        unsigned char Send[42];
        unsigned short Temp;
		unsigned int* MyPointer;
	
        while(Read_Tx_Descriptor(TxCurrentDesc.TxCurrentDescriptor) != 0)
		{
			TxCurrentDesc.TxCurrentDescriptor++;
			TxCurrentDesc.Number++;
			if(TxCurrentDesc.Number == NUMTXDESCRIPTOR)
			{
				TxCurrentDesc.Number = 0;
				TxCurrentDesc.TxCurrentDescriptor = ATxDescriptor;
			}	
		}
		
        //первые 6 байт - удаленный MAC - адрес
        Send[0] = Dt->Remote_MAC[0];
        Send[1] = Dt->Remote_MAC[1];
        Send[2] = Dt->Remote_MAC[2];
        Send[3] = Dt->Remote_MAC[3];
        Send[4] = Dt->Remote_MAC[4];
        Send[5] = Dt->Remote_MAC[5];

        //далее 6 байт - мой MAC-адрес
        Send[6] = My_MAC[0];
        Send[7] = My_MAC[1];
        Send[8] = My_MAC[2];
        Send[9] = My_MAC[3];
        Send[10] = My_MAC[4];
        Send[11] = My_MAC[5];

        //далее заполняем всё так же как в полученном пакете до 19 байта (включительно)
        for(Temp = 12; Temp < 20; Temp++)
        {
                Send[Temp] = Dt->Data[Temp];
        }

        //20 и 21 байты - Opcode: для ответа должен быть равен 2
        Send[21] = 0x02;
        Send[20] = 0x00;

        //с 22 по 27 (включительно) идет мой MAC

        Send[22] = My_MAC[0];
        Send[23] = My_MAC[1];
        Send[24] = My_MAC[2];
        Send[25] = My_MAC[3];
        Send[26] = My_MAC[4];
        Send[27] = My_MAC[5];

        //с 28 по 31 (включительно) идет мой IP

        Send[28] = My_IP[0];
        Send[29] = My_IP[1];
        Send[30] = My_IP[2];
        Send[31] = My_IP[3];

        //с 32 по 37 (включительно) идет удаленный MAC

        Send[32] = Dt->Remote_MAC[0];
        Send[33] = Dt->Remote_MAC[1];
        Send[34] = Dt->Remote_MAC[2];
        Send[35] = Dt->Remote_MAC[3];
        Send[36] = Dt->Remote_MAC[4];
        Send[37] = Dt->Remote_MAC[5];

        //с 38 по 41 (включительно) идет удаленный IP

        Send[38] = Dt->Remote_IP[0];
        Send[39] = Dt->Remote_IP[1];
        Send[40] = Dt->Remote_IP[2];
        Send[41] = Dt->Remote_IP[3];
		
		MyPointer = (unsigned int*)TxCurrentDesc.FirstEmptyWord;

		for(Temp=0;Temp<42;Temp+=2)
		{
			*MyPointer++ = Send[Temp]|(Send[Temp+1]<<8);
			if((unsigned int)MyPointer > 0x60005FFC) MyPointer = (unsigned int*)0x60004000;
		}
        
        Write_Tx_Descriptor(0x002A,&TxCurrentDesc);     //42 bytes

 		TxCurrentDesc.TxCurrentDescriptor++;
		TxCurrentDesc.Number++;
		if(TxCurrentDesc.Number == NUMTXDESCRIPTOR)
		{
			TxCurrentDesc.Number = 0;
			TxCurrentDesc.TxCurrentDescriptor = ATxDescriptor;
		}

#if PRINTSTATUSON
		Packet.CounterTxPacket++;
#endif	//PRINTSTATUSON

		
                                        }
																				return 0;
}
		
		
		
		
		

