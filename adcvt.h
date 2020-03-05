/*************************************************************************
Подключение 1986ВЕ91Т к плате АЦПВТ. Интерфейс параллельный.
5.03.2020
*************************************************************************/
#ifndef _ADCVT_H_
#define _ADCVT_H_


typedef enum  {OBP1, RBP1, OBP2, RBP2, OKP, RKP} Channel_Adc_VT;


//Структура для хранения результатов измерения
typedef struct
{
	Channel_Adc_VT Ch;	//канал
	unsigned int angle;					//угол
	unsigned int time;			//время
} measure_data;

//Структура для хранения кольцевого буфера с данными
typedef struct
{
	measure_data data_buf[16];	//буфер структур
	unsigned int head;					//голова
	unsigned int tail;			//хвост
} measure_buf;

unsigned int Get_angle(Channel_Adc_VT ch);

void Get_adcvt_data(void);

void Initialize_adc_vt(void);

#endif /* _ADCVT_H_ */

