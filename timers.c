/*************************************************************************
Работа с таймерами
5.03.2020
*************************************************************************/
#include "timers.h"

extern unsigned int Acvt_read_channels;

typedef enum {ON, OFF} STATUS;

STATUS Led_status = OFF;

void Initialize_timer_100ms(void)
{
	MDR_RST_CLK->TIM_CLOCK |= 0x7; // Делитель тактовой частоты = 128. 64 МГц/128 = 500 кГц
	MDR_TIMER1->PSG = 0x1F3; // Значение предварительного делителя счетчика = 500. Основной счетчик считает на частоте: CLK = TIM_CLK/(PSG+1) = 1000 Гц
	MDR_TIMER1->ARR = 1000; // Частота = 1 Гц
	MDR_TIMER1->CNT = 0;
	MDR_RST_CLK->TIM_CLOCK |= (1<<24); // Включение таймера 1
	
	 //Настройка прерывания 
  NVIC_EnableIRQ(Timer1_IRQn);      //Включение прерываний от TIMER1 
  NVIC_SetPriority(Timer1_IRQn, 0); //Установка приоритета прерываний 0-15 
 
    //Включение прерывания при равенстве нулю значения TIMER1 
  MDR_TIMER1->IE |= (1<<0);
   
  //Запуск таймера 
  MDR_TIMER1->CNTRL |= (1<<0);
}


void Timer1_IRQHandler() 
{
//Проверка что причина прерывания – обновление таймера 
	if(TIMER_GetITStatus(MDR_TIMER1, TIMER_STATUS_CNT_ZERO))
	{
		if (Led_status == OFF)
		{
			SET_LED1();
			Led_status = ON;
		}
		else 
		{
			RESET_LED1();
			Led_status = OFF;
		}
		Get_adcvt_data(Acvt_read_channels);
//Очистка флага прерывания (это необходимо делать в конце)
	TIMER_ClearITPendingBit(MDR_TIMER1, TIMER_STATUS_CNT_ZERO);
	}
}

