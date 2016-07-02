/**
*****************************************************************************
**
**  File        : main.c
**
**  Abstract    : main function.
**
**  Functions   : main
**
**  Environment : Atollic TrueSTUDIO(R)
**
**  Distribution: The file is distributed "as is", without any warranty
**                of any kind.
**
**  (c)Copyright Atollic AB.
**  You may use this file as-is or modify it according to the needs of your
**  project. This file may only be built (assembled or compiled and linked)
**  using the Atollic TrueSTUDIO(R) product. The use of this file together
**  with other tools than Atollic TrueSTUDIO(R) is not permitted.
**
*****************************************************************************
*/

/* Includes */
#include "stm32l476xx.h"
#include "core_cmFunc.h"
#include "core_cmInstr.h"
#include "core_cm4.h"

/* Private macro */

/* Private variables */
volatile int sysTickFlag = 0;
volatile uint32_t turnOnOff = 1;
volatile uint32_t blinkMode = 1;
uint32_t delay = 10;

/* Private function prototypes */
void IRQInit();
void PINInit();
void SysInit();
void blink_LED(int, uint32_t *);
void Delay_ms(int);

/**
**===========================================================================
**
**  Abstract: main program
**
**===========================================================================
*/
int main(void)
{
  /* TODO - Add your application code here */
	SysInit();
	PINInit();
	IRQInit();

  /* Infinite loop */
  while (1){
	  if (turnOnOff){
	  blink_LED(blinkMode,&delay);
	  }
  }
} /* main*/

/* Private functions */
void blink_LED(int mode, uint32_t *blinkSpeed){
	int i = 0;

	switch (mode){
	case 1:
		Delay_ms(*blinkSpeed);
		GPIOB->BSRR = GPIO_BSRR_BS2;
		GPIOE->BRR = GPIO_BRR_BR8;
		Delay_ms(*blinkSpeed);
		GPIOB->BRR = GPIO_BRR_BR2;
		GPIOE->BSRR = GPIO_BSRR_BS8;
	break;
	case 2:
		GPIOE->BRR = GPIO_BRR_BR8;
		GPIOB->BRR = GPIO_BRR_BR2;
		for(i=0;i<5;i++){
			GPIOB->BSRR = GPIO_BSRR_BS2;
			Delay_ms(*blinkSpeed);
			GPIOB->BRR = GPIO_BRR_BR2;
			Delay_ms(*blinkSpeed);
		}
		for(i=0;i<5;i++){
			GPIOE->BSRR = GPIO_BSRR_BS8;
			Delay_ms(*blinkSpeed);
			GPIOE->BRR = GPIO_BRR_BR8;
			Delay_ms(*blinkSpeed);
		}
	break;
	}
}

void Delay_ms(int czas_ms)
{
	int j = 0;
	while(j<czas_ms){
		if(sysTickFlag == 1){
			j++;
			sysTickFlag = 0;
		}
	}
}
__attribute__((interrupt)) void SysTick_Handler(void){
		sysTickFlag = 1;
	}
__attribute__((interrupt)) void EXTI0_IRQHandler(void){
	EXTI->PR1 = (1<<0);
	if (turnOnOff == 0)  turnOnOff = 1;
	else turnOnOff = 0;
	}
__attribute__((interrupt)) void EXTI1_IRQHandler(void){
	EXTI->PR1 = (1<<1);
	blinkMode++;
	if (blinkMode>2) blinkMode=1;
	}
__attribute__((interrupt)) void EXTI3_IRQHandler(void){
	EXTI->PR1 = (1<<3);
	delay += 10;
	}
__attribute__((interrupt)) void EXTI9_5_IRQHandler(void){
	EXTI->PR1 = (1<<5);
	delay -= 10;
	if (delay<=10) delay = 10;
	}


void IRQInit(){
	 /* konfiguracja zewntrznych przerwañ
	  * na liniach GPIOA - przycisk generuje przerwanie */
	  SYSCFG->EXTICR[0] = SYSCFG_EXTICR1_EXTI0_PA ;
	  SYSCFG->EXTICR[1] = SYSCFG_EXTICR1_EXTI0_PA ;

	  /*wybór przerwañ które beda przesylane do rdzenia*/
	  NVIC_EnableIRQ(EXTI0_IRQn);
	  NVIC_EnableIRQ(EXTI1_IRQn);
	  NVIC_EnableIRQ(EXTI2_IRQn);
	  NVIC_EnableIRQ(EXTI3_IRQn);
	  NVIC_EnableIRQ(EXTI9_5_IRQn);

	  /* RTSR1 - wybór zbocza narastajacego przy wykrywaniu sygnalu
	   * IMR1 - uruchomienie przerwan na liniach 0,1,3,5 */
	  EXTI->RTSR1 =(0b111111);
	  EXTI->IMR1 = (0b101011);
}

void PINInit(){
	  GPIOB->MODER =  GPIO_MODER_MODE2_0;		// output mode GPIOB (LED red)
	  GPIOE->MODER =  GPIO_MODER_MODE8_0;		// output mode GPIOE (LED green)
	  GPIOA->MODER = ((uint32_t)0xABFFF000U) ;	// input mode (pad)

	  /* Pull-down dla wybranych przyciskow */
	  GPIOA->PUPDR = (GPIO_PUPDR_PUPD0_1
			  	  	| GPIO_PUPDR_PUPD1_1
			  	  	| GPIO_PUPDR_PUPD2_1
			  	  	| GPIO_PUPDR_PUPD3_1
			  	  	| GPIO_PUPDR_PUPD5_1);
}

void SysInit(){
	  SysTick_Config(8000); 					//inicjalizacja zegara systemowego - generator przerwan

	  RCC->CR = RCC_CR_MSION; 					// wybór zaegara taktujacego
	  RCC->AHB2ENR = (RCC_AHB2ENR_GPIOBEN
			  	   |  RCC_AHB2ENR_GPIOEEN
			  	   |  RCC_AHB2ENR_GPIOAEN);		// uruchomienie taktowania GPIOA, GPIOB i GPIOE
}
