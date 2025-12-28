/*
 * LED Delay using TIM2 (polling)
 */
#include "stm32f4xx.h"

static void gpio_config(void);
static void timer_config(void);

int main(void)
{
	gpio_config();
	timer_config();

	while(1)
	{
		if((TIM2->SR) & TIM_SR_UIF)
			{
				TIM2->SR &= ~(TIM_SR_UIF);                       //clearing UIF flag (without touching other bits)
				GPIOA->ODR ^= (1U << 5);                      //LED Toggle
			}

	}
}

static void gpio_config(void)
{

	// GPIOA is on AHB1 bus
	RCC->AHB1ENR |=RCC_AHB1ENR_GPIOAEN ;      //Enableing the clock for GPIOA
	(void)RCC->AHB1ENR;                       // read-back to ensure clock is active

	GPIOA->MODER &= ~(3U << 10);             //Clearing the moder PA5 bits
	GPIOA->MODER |= (1U << 10);              // setting the moder  PA5 bits as output configuration

	GPIOA->OTYPER &= ~(1U << 5);               //Push -Pull type

	GPIOA->OSPEEDR &= ~(3U << 10);           //Clearing the output speed PA5 bits

	GPIOA->PUPDR &= ~(3U << 10);            //Clearing the PA5 PUPDR bits

}

static void timer_config(void)
{
	//  is on AHB1 bus
	RCC->APB1ENR |=RCC_APB1ENR_TIM2EN ;      //Enableing the clock for TIMER2
	(void)RCC->APB1ENR;                       // read-back to ensure clock is active

	TIM2->CR1 &= ~(TIM_CR1_CEN);               //Disableing the TIM2

	TIM2->PSC = 15999;                       //setting prescaler value (for 16MHz clk freq) , at this value the tick period 1000Hz (1msec)
	TIM2->ARR = 1000;                        //setting ARR to 1000 (0-1000) , at this value the overflow time 1000 x 1mHz = 1sec

	TIM2->SR &= ~TIM_SR_UIF;                       //clearing UIF flag

	TIM2->CR1 |= TIM_CR1_CEN;               //Enableing the TIM2
}
