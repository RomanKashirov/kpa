/*************************************************************************
Подключение 1986ВЕ91Т к Ethernet с помощью 5600ВГ1У. Интерфейс параллельный.
10.02.2020
*************************************************************************/
#include "ethernet.h"

#define ip_data 0x22                  // область IP данных
#define UDP_scrport    ip_data 		  // порт отправителя
#define UDP_destport   UDP_scrport+2  // порт получателя
#define UDP_len        UDP_destport+2 // длина
#define UDP_cksum      UDP_len+2	  // контрольная сумма

#define ip_proto       0x17           // протокол (ICMP=1, TCP=6, UDP=11)
#define ip_scraddres   0x1A			  // IP адрес источника
#define ip_destaddr	   0x1E			  // IP адрес назначения

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
unsigned short UpTypeOfProtocol; //тип протокола верхнего уровня

int Read_Packet(_ethernet_packet* Dt);
int Answear_ARP(_ethernet_packet* Dt);
int Answear_IP(_ethernet_packet* Dt);
unsigned short CheckSum_IP(_ethernet_packet* Dt);
unsigned short CheckSum_ICMP(_ethernet_packet* Dt);
int Answear_ICMP(_ethernet_packet* Dt, unsigned char* );




void Polling_Ethernet(void)
{
	if(Read_Rx_Descriptor(RxCurrentDesc.RxCurrentDescriptor) == 0) //проверка наличия принятого пакета
	{
		SET_LED2(); //LED2 - Принят пакет с нашим или широковещательным адресом
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
				Answear_ARP(&Packet);
				break;
			
			//Если принят IP-пакет
			case 0x0800:
				Answear_IP(&Packet);
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


int Answear_IP(_ethernet_packet* Dt)
{
	Receive_IP[0] = Dt->Data[30];
  Receive_IP[1] = Dt->Data[31];
  Receive_IP[2] = Dt->Data[32];
  Receive_IP[3] = Dt->Data[33];

  if((Receive_IP[0] == My_IP[0])&&(Receive_IP[1] == My_IP[1])&&(Receive_IP[2] == My_IP[2])&&(Receive_IP[3] == My_IP[3]))  //получили IP-пакет с нашим IP адресом
  {
		int Temp = CheckSum_IP(Dt);
    if(Temp == ((Dt->Data[24] << 8)&0xFF00|Dt->Data[25]))  //если контрольная сумма IP-протокола пакета и вычисленная совпадают, то работаем дальше, иначе откидываем пакет
    {
			SET_LED4();
			Dt->Remote_IP[0] = Dt->Data[26];
			Dt->Remote_IP[1] = Dt->Data[27];
			Dt->Remote_IP[2] = Dt->Data[28];
			Dt->Remote_IP[3] = Dt->Data[29];

			Dt->Remote_MAC[0] = Dt->Data[6];
			Dt->Remote_MAC[1] = Dt->Data[7];
			Dt->Remote_MAC[2] = Dt->Data[8];
			Dt->Remote_MAC[3] = Dt->Data[9];
			Dt->Remote_MAC[4] = Dt->Data[10];
			Dt->Remote_MAC[5] = Dt->Data[11];

			//проверить следующий протокол (ICMP)
			if(Dt->Data[23] == 0x01)     //далее следует ICMP-протокол
			{																			
				if(Dt->Data[34] == 0x08) //приняли echo request
        {
					Answear_ICMP(Dt, Answear_buffer);
				}
			}	
		}
	}
	return 0;
}

//--------------------------------------------------------------------------------------
//Функция для формирования ответа на ICMP-запрос
//Параметр:	указатель на принятый Ethernet-пакет
//Возвращает 0
//--------------------------------------------------------------------------------------
int Answear_ICMP(_ethernet_packet* Dt, unsigned char* ICMP_Packet)
{
	int	Temp = CheckSum_ICMP(Dt);
	if(Temp == ((Dt->Data[36] << 8)&0xFF00|Dt->Data[37]))	//проверка совпадения контрольной суммы ICMP-пакета и вычисленной
	{
		SET_LED5();
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
		
        //Remote MAC-Address
    ICMP_Packet[0] = Dt->Remote_MAC[0];
    ICMP_Packet[1] = Dt->Remote_MAC[1];
    ICMP_Packet[2] = Dt->Remote_MAC[2];
    ICMP_Packet[3] = Dt->Remote_MAC[3];
    ICMP_Packet[4] = Dt->Remote_MAC[4];
    ICMP_Packet[5] = Dt->Remote_MAC[5];

        //My MAC-Address
    ICMP_Packet[6] = My_MAC[0];
    ICMP_Packet[7] = My_MAC[1];
    ICMP_Packet[8] = My_MAC[2];
    ICMP_Packet[9] = My_MAC[3];
    ICMP_Packet[10] = My_MAC[4];
    ICMP_Packet[11] = My_MAC[5];

        //IP-protocol
    ICMP_Packet[12] = 0x08;
    ICMP_Packet[13] = 0x00;
//Закнчили формировать Eth2-пакет

//Формируем IP-пакет
        //Field of IP-protocol
    for(Temp = 14; Temp < 24; Temp++)
    {
			ICMP_Packet[Temp] = Dt->Data[Temp];
    }
		ICMP_Packet[26] = Dt->Data[30];
    ICMP_Packet[27] = Dt->Data[31];
    ICMP_Packet[28] = Dt->Data[32];
    ICMP_Packet[29] = Dt->Data[33];
        //Remote IP
    ICMP_Packet[30] = Dt->Data[26];
    ICMP_Packet[31] = Dt->Data[27];
    ICMP_Packet[32] = Dt->Data[28];
    ICMP_Packet[33] = Dt->Data[29];

    Temp = CheckSum_IP(Dt);

       //IP CheckSum
    ICMP_Packet[24] = (unsigned char)(Temp >> 8);
    ICMP_Packet[25] = (unsigned char)Temp;

//Закнчили формировать IP-пакет
//Формируем ICMP-пакет
    ICMP_Packet[34] = 0x00; //Echo reply
    ICMP_Packet[35] = 0x00;

    Dt->Data[34] = 0x00;
    Dt->Data[35] = 0x00;

    Temp = CheckSum_ICMP(Dt);
    ICMP_Packet[36] = (unsigned char)(Temp >> 8);
    ICMP_Packet[37] = (unsigned char)Temp;

    for(Temp = 38; Temp < Dt->Length - 4; Temp++)
    {
			ICMP_Packet[Temp] = Dt->Data[Temp];
    }

		MyPointer = (unsigned int*)TxCurrentDesc.FirstEmptyWord;

    if((Dt->Length & 0x0001) == 1)
    {
			ICMP_Packet[Dt->Length - 4] = Dt->Data[Dt->Length - 4];
      for(Temp=0;Temp<Dt->Length - 3;Temp+=2)
			{
				*MyPointer++ = ICMP_Packet[Temp]|(ICMP_Packet[Temp+1]<<8);
				if((unsigned int)MyPointer > 0x60005FFC) MyPointer = (unsigned int*)0x60004000;
			}
    }
    else
    {
			for(Temp=0;Temp<Dt->Length - 4;Temp+=2)
			{
				*MyPointer++ = ICMP_Packet[Temp]|(ICMP_Packet[Temp+1]<<8);
				if((unsigned int)MyPointer > 0x60005FFC) MyPointer = (unsigned int*)0x60004000;
			}
    }
//Закончили формировать ICMP-пакет

    Write_Tx_Descriptor(Dt->Length-4,&TxCurrentDesc);     //74 bytes

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


//Функция для формирования ответного ARP-пакета
//Dt - указатель на пакет
//возвращает 0
int Answear_ARP(_ethernet_packet* Dt)
{
	Receive_IP[0] = Dt->Data[38];
  Receive_IP[1] = Dt->Data[39];
  Receive_IP[2] = Dt->Data[40];
  Receive_IP[3] = Dt->Data[41];

  if( (Receive_IP[0] == My_IP[0])&&(Receive_IP[1] == My_IP[1])&&(Receive_IP[2] == My_IP[2])&&(Receive_IP[3] == My_IP[3]))         //полное совпадение IP в пакете и нашего. Надо отвечать
  {
		SET_LED3(); //LED3 - Принят ARP пакет с нашим IP
		Dt->Remote_IP[0] = Dt->Data[28];
    Dt->Remote_IP[1] = Dt->Data[29];
    Dt->Remote_IP[2] = Dt->Data[30];
    Dt->Remote_IP[3] = Dt->Data[31];
    Dt->Remote_MAC[0] = Dt->Data[22];
    Dt->Remote_MAC[1] = Dt->Data[23];
    Dt->Remote_MAC[2] = Dt->Data[24];
    Dt->Remote_MAC[3] = Dt->Data[25];
    Dt->Remote_MAC[4] = Dt->Data[26];
    Dt->Remote_MAC[5] = Dt->Data[27];

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
		
		
//--------------------------------------------------------------------------------------
//Функция для вычисления контрольной суммы IP-пакета
//Параметр:	указатель на принятый Ethernet-пакет
//Возвращает контрольную сумму IP-пакета
//--------------------------------------------------------------------------------------
unsigned short CheckSum_IP(_ethernet_packet* Dt)
{
        unsigned long Temp, Check = 0;
        for(Temp = 0; Temp < 20; Temp += 2)
        {
                if(Temp == 10) continue;
                else Check += ((Dt->Data[Temp + 14] << 8)&0xFF00)|Dt->Data[Temp + 15];
        }
        Check = (Check >> 16) + (Check & 0xFFFF);
        return (unsigned short)(~Check);
}
//--------------------------------------------------------------------------------------
//Функция для вычисления контрольной суммы ICMP-пакета
//Параметр:	указатель на принятый Ethernet-пакет
//Возвращает контрольную сумму ICMP-пакета
//--------------------------------------------------------------------------------------
unsigned short CheckSum_ICMP(_ethernet_packet* Dt)
{
        unsigned long Temp, Check = 0;
        for(Temp = 0; Temp < Dt->Length - 38; Temp += 2)      //реализовано для пакета ICMP объёмом до 1450 байт
        {                                                     //Dt->Length - 38,т.к. 34 байта - это Eth2 и IP заголовки, 4 байта - CRС32 Eth2 пакета
                if(Temp == 2) continue;
                else Check += ((Dt->Data[Temp + 34] << 8)&0xFF00)|Dt->Data[Temp + 35];
        }
        Check = (Check >> 16) + (Check & 0xFFFF);
        return (unsigned short)(~Check);
}		
		
		

