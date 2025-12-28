#include "stm32f4xx.h"

static void delay(volatile uint32_t count);
static void gpio(void);

int main(void)
{
	gpio();
	while(1)                                  //infinte loop
	{
	    GPIOA->ODR ^= (1U << 5);             //toggling the bit

	                                         /*
	                                         * we can also use BSRR to set or reset the PA5 directly
	                                         * to set
	                                             GPIOA->BSRR = (1U << 5)
	                                         * to reset after delay function
	                                             GPIOA->BSRR = (1U <<(5+16))
	                                         */

	    delay(200000);                       //Delay function
	}

}

static void gpio(void)
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
static void delay(volatile uint32_t count)
{
    while (count--)
    {
    	__asm volatile ("nop");                  //the loop that cannot be optimized away
    }
}
