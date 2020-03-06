/*************************************************************************
Подключение 1986ВЕ91Т к плате АЦПВТ. Интерфейс параллельный.
5.03.2020
*************************************************************************/
#include "adcvt.h"

unsigned int current_time = 12345;

measure_buf Adcvt_data;

unsigned int Get_angle(Channel_Adc_VT ch);

// Считывание угла из данного канала АЦПВТ
unsigned int Get_angle(Channel_Adc_VT ch)
{
	static int unsigned res = 0;
	return res++;
}

// Считывание данных из заданного канала АЦПВТ
void Get_channel_data(Channel_Adc_VT ch)
{
	Adcvt_data.data_buf[Adcvt_data.head].angle = Get_angle(ch);
	Adcvt_data.data_buf[Adcvt_data.head].Ch = ch;
	Adcvt_data.data_buf[Adcvt_data.head].time = current_time;
	Adcvt_data.head = (Adcvt_data.head + 1) % ADCVT_BUFFER_SIZE;
	if(Adcvt_data.head == Adcvt_data.tail)
	{
		Adcvt_data.tail = (Adcvt_data.tail + 1) % ADCVT_BUFFER_SIZE;
	}
}


// Считывание данных из активных каналов АЦПВТ
void Get_adcvt_data(unsigned int channels)
{
	if(channels & (1<<OBP1))
	{
		Get_channel_data(OBP1);
	}
	
	if(channels & (1<<RBP1))
	{
		Get_channel_data(RBP1);
	}
	
	if(channels & (1<<OBP2))
	{
		Get_channel_data(OBP2);
	}
	
	if(channels & (1<<RBP2))
	{
		Get_channel_data(RBP2);
	}
		
	if(channels & (1<<OKP))
	{
		Get_channel_data(OKP);
	}
	
	if(channels & (1<<RKP))
	{
		Get_channel_data(RKP);
	}
}

// Инициализация циклического буфера для хранения результатов измерения
void Initialize_adc_vt(void)
{
	Adcvt_data.head = 0;
	Adcvt_data.tail = 0;
}


