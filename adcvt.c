/*************************************************************************
Подключение 1986ВЕ91Т к плате АЦПВТ. Интерфейс параллельный.
5.03.2020
*************************************************************************/
#include "adcvt.h"

unsigned int current_time = 12345;

measure_buf Adcvt_data;

unsigned int Get_angle(Channel_Adc_VT ch)
{
	static int unsigned res = 0;
	return res++;
}

void Get_adcvt_data(void)
{
	Adcvt_data.data_buf[Adcvt_data.head].angle = Get_angle(OBP1);
	Adcvt_data.data_buf[Adcvt_data.head].Ch = OBP1;
	Adcvt_data.data_buf[Adcvt_data.head].time = current_time;
	Adcvt_data.head++;
}

void Initialize_adc_vt(void)
{
	Adcvt_data.head = 0;
	Adcvt_data.tail = 0;
}


