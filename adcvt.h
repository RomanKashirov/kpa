/*************************************************************************
Подключение 1986ВЕ91Т к плате АЦПВТ. Интерфейс параллельный.
5.03.2020
*************************************************************************/
#ifndef _ADCVT_H_
#define _ADCVT_H_

#define ADCVT_BUFFER_SIZE 32

typedef enum  {OBP1 = 0, RBP1 = 1, OBP2 = 2, RBP2 = 3, OKP = 4, RKP = 5} Channel_Adc_VT;


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
	measure_data data_buf[ADCVT_BUFFER_SIZE];	//буфер структур
	unsigned int head;					//голова
	unsigned int tail;			//хвост
} measure_buf;



void Get_adcvt_data(unsigned int channels);

void Initialize_adc_vt(void);

#endif /* _ADCVT_H_ */

