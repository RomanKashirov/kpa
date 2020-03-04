/*************************************************************************
Подключение 1986ВЕ91Т к Ethernet с помощью 5600ВГ1У. Интерфейс параллельный.
10.02.2020
*************************************************************************/


#include "config.h"
#ifdef TEST


#endif




// Инициализация тактового генератора
void Initialize_CLK(void)
{
	
	/* Включение HSE осциллятора (внешнего кварцевого резонатора)*/
	//Генератор запускается при появлении питания UCC и сигнала разрешения HSEON в регистре HS_CONTROL.
	MDR_RST_CLK->HS_CONTROL = 0x00000001;
	
	//При выходе в нормальный режим работы устанавливается бит  HSERDY в регистре CLOCK_STATUS.
	while((MDR_RST_CLK->CLOCK_STATUS&0x00000004)!=0x00000004){}
	
	MDR_RST_CLK->PLL_CONTROL = 0x00000704; //Включение схемы PLL и установка коэффициента умножения PLL K_mul = 7+1 (clk = 64 MHz)
	while((MDR_RST_CLK->CLOCK_STATUS & 0x00000002) != 0x00000002){} //Ожидание включения PLL
					
	/*Скорость  доступа  во  Flash-память  ограничена  и  составляет  порядка  40  нс,  в  результате 
	выдача новых значений из Flash-памяти может происходить с частотой не более 25 МГц. При  возникновении  переходов
	в  выполнении  программы,  когда  из памяти  программ  не 
	выбраны нужные инструкции, возникает пауза в несколько тактов процессора для того, чтобы 
	данные успели считаться из Flash. Число тактов паузы зависит от тактовой частоты процессора; 
	так при работе с частотой ниже 25 МГц пауза не требуется,  поскольку Flash-память успевает 
	выдать новые данные за один такт, при частоте от 25 до 50 МГц требуется один такт паузы, и 
	так далее. Число тактов паузы задается в регистре EEPROM_CMD битами Delay[2:0].*/
	MDR_EEPROM->CMD = 0x0000018;	//Delay = 3
	
	/*Выбор источника для CPU_C1 – HSE, выбор источника для CPU_C2 – PLLCPUo, выбор делителя CPU_C3 = CPU_C2,
	выбор источника для HCLK – CPU_C3*/
	MDR_RST_CLK->CPU_CLOCK = 0x00000106;
	MDR_RST_CLK->PER_CLOCK |= (1<<25)|(1<<21)|(1<<22)|(1<<23)|(1<<24)|(1<<29)|(1<<30); // Разрешение тактирования портов A, B, C, D, E, F, EXT_BUS_CNTRL
}



// Инициализация портов ввода-вывода
void Initialize_GPIO(void)
{
//	#define jopa
	
#ifdef jopa	
	//Data Bus[0:15]
		MDR_PORTA->ANALOG = 0xFFFF;
		MDR_PORTA->PWR = 0xAAAAAAAA;
		MDR_PORTA->FUNC = 0x55555555;

//OE-WE & Buttons
		MDR_PORTC->ANALOG = 0xFFFF; // OE-WE
		MDR_PORTC->PWR = 0xAAAAAAAA; // OE-WE
		MDR_PORTC->FUNC = 0x00000014; // OE-WE
//		PORTC->PD   = 0x00000000; // OE-WE//Светодиоды
//		PORTC->RXTX = 0x0000;
//		PORTC->OE = 0x0080;
//LED

        MDR_PORTD->RXTX = 0;
        MDR_PORTD->OE = 0x7C00; //PORTD10 - PORTD14 - outputs
        MDR_PORTD->FUNC = 0x00000000; //PORTD - ports
        MDR_PORTD->ANALOG = 0xFF00; //PORTD - digital
        MDR_PORTD->PWR = 0xAAAA0000; //PORTD8 - PORTD15 - fast edge

//Address Bus[0:12]
		MDR_PORTE->ANALOG = 0xFFFF; // CS
		MDR_PORTE->PWR = 0xAAAAAAAA; // CS
		MDR_PORTE->RXTX = 0x0000;
		MDR_PORTE->OE = 0x0030;
		MDR_PORTE->FUNC = 0x01000000; // CS

		MDR_PORTF->ANALOG = 0xFFFF;
		MDR_PORTF->PWR = 0xFFFFFFFF;
		MDR_PORTF->FUNC = 0x55555555;

		MDR_PORTB->RXTX = 0;
        MDR_PORTB->OE = 0x8000; //PORTD10 - PORTD14 - outputs
        MDR_PORTB->FUNC = 0x00000000; //PORTD - ports
        MDR_PORTB->ANALOG = 0xC000; //PORTD - digital
        MDR_PORTB->PWR = 0xC0000000; //PORTD8 - PORTD15 - fast edge

#else 
	//Выводы под внешнюю шину
	//Шина данных[0:15] на PORTA
	MDR_PORTA->ANALOG = 0xFFFF; // Цифровой режим
	MDR_PORTA->PWR = 0xAAAAAAAA; // 10 – быстрый фронт (порядка 20 нс)
	MDR_PORTA->FUNC = 0x55555555; //Настройка порта на 01 – основная функция
	
	//OE, WE на PC1, PC2
	MDR_PORTC->ANALOG |= (1<<1)|(1<<2); // Цифровой режим PC1, PC2
	MDR_PORTC->PWR |= 0x28; // 10 – быстрый фронт (порядка 20 нс) 
	MDR_PORTC->FUNC |= 0x00000014; // Настройка PC1, PC2 порта на 01 – основная функция 
	
	//Шина адреса[0:12] на PF2-PF14
	MDR_PORTF->ANALOG |= 0x7FFC; //Цифровой режим PF2-PF14
	MDR_PORTF->PWR |= 0x3FFFFFF0; // 11 – максимально быстрый фронт (порядка 10 нс)
	MDR_PORTF->FUNC |= 0x15555550; // Настройка PF2-PF14 порта на  01 – основная функция 
	
	//Настройка вывода сброса 5600ВГ1У - nRST на PB11
//	MDR_PORTB->RXTX &= ~((1<<11)|0x001F); // Сброс
 
	MDR_PORTB->OE |= (1<<11);  //PB11 - выход
	MDR_PORTB->FUNC &= 0xFF3FFFFF; //PB11 - порт ввода-вывода
	MDR_PORTB->ANALOG |= (1<<11); //Цифровой режим работы вывода PB11
//	MDR_PORTB->PWR &= 0xFF3FFFFF;
	MDR_PORTB->PWR |= 0x400000;
	MDR_PORTB->PD |= (1<<11);
	MDR_PORTB->RXTX = (1<<11)|(MDR_PORTB->RXTX & 0xFFE0);



//Установка 11 and 12 битов PORTB (TODO узнать, зачем 11 (это сброс) и почему управляем 12 вручную)
//	MDR_PORTB->RXTX = (1<<11)|(MDR_PORTB->RXTX & 0xFFE0); //В ножки JTAG нельзя писать единицы, при записи в порт всегда делайте маску и сбрасывайте все биты JTAG в ноль
//	MDR_PORTB->OE |= (1<<11);  //PORTB_11,PORTB_12, PORTB_13, PORTB_15 - Выходы
//	MDR_PORTB->PWR &= 0x400000;
//	MDR_PORTB->ANALOG = 0x800;
	
//	MDR_PORTB->FUNC = 0xA8000000; //PORTB_13 - PORTF_15: альтернативная функция, PORTB_11, PORTB_12 - порты ввода-вывода

	
#if defined _1986_EVBRD_
//Настройка выбора ИМС 5600ВГ1У - CS на PE28
	MDR_PORTE->ANALOG = 0xFFFF; // CS
	MDR_PORTE->PWR = 0xAAAAAAAA; // CS
	MDR_PORTE->FUNC = 0x01000000; // CS
#elif defined _6115_160_
	//Настройка выбора ИМС 5600ВГ1У - CS на PC0
	MDR_PORTC->RXTX &= ~((1<<0)|0x001F); // CS
	MDR_PORTC->OE |= (1<<0);  //PC0 - выход
	MDR_PORTC->FUNC &= 0xFFFFFFFC; //PC0 - порт ввода-вывода
	MDR_PORTC->ANALOG |= (1<<0); //Цифровой режим работы вывода PC0
	MDR_PORTC->PWR &= 0xFFFFFFFC;
	MDR_PORTC->PWR |= 0x1;
#endif

	MDR_PORTE->ANALOG = 0xFFFF; // CS
	MDR_PORTE->PWR = 0xAAAAAAAA; // CS
	MDR_PORTE->FUNC = 0x01000000; // CS
	
	//Светодиоды
	// Сброс битов 10-14 PORTD
	MDR_PORTD->RXTX &= ~((1<<10)|(1<<11)|(1<<12)|(1<<13)|(1<<14)|0x001F); //В ножки JTAG нельзя писать единицы, при записи в порт всегда делайте маску и сбрасывайте все биты JTAG в ноль 
	MDR_PORTD->OE |= (1<<10)|(1<<11)|(1<<12)|(1<<13)|(1<<14); //PORTD 10-14 - Выходы
	MDR_PORTD->FUNC &= 0xC00FFFFF; //PORTD 10-14 - Порты ввода-вывода
	MDR_PORTD->ANALOG |= (1<<10)|(1<<11)|(1<<12)|(1<<13)|(1<<14); //Цифровой режим работы выводов PORTD 10-14
	MDR_PORTD->PWR &= 0xC00FFFFF;
	MDR_PORTD->PWR |= 0x2AAA0000; //PORTD10-14 - быстрый фронт (порядка 20 нс) 
#endif
}


//Функция для настройки внешней системной шины
__inline void Initialize_ExtBus()
{
	
	MDR_EBC->CONTROL = 0xF002;  //1111 – 17 тактов HCLK, RAM mode
}

