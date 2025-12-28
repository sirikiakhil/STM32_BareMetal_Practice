/*
 * Toggle LED on each button press(edge detection manually)
 */
#include "stm32f4xx.h"

#define OFF 0
#define ON  1

static void gpio_config(void);
// void method_2(void);

int main(void)
{
	gpio_config();
	uint8_t current_state;
	uint8_t previous_state=1;                           //active high because button not pressed its logic at 1
	while(1)                                      //infinte loop
	{
		//another method not edge detection
		//method_2();
		current_state = (((GPIOC->IDR) >> (13)) & (1));
		if((current_state == 0) && (previous_state == 1))
		{
			for(volatile uint32_t count=0;count<20000;count++);   //delay to overcome bouncing effect
			current_state = (((GPIOC->IDR) >> (13)) & (1));
			if(current_state == 0)
			{
				if(GPIOA->ODR & (1U << 5))
				{
					GPIOA->BSRR = (1U <<(5+16));
				}
				else
				{
					GPIOA->BSRR = (1U << 5);
				}
			}
		}
		previous_state = current_state;
	}

}

static void gpio_config(void)
{
	    // GPIOA is on AHB1 bus
		RCC->AHB1ENR |=RCC_AHB1ENR_GPIOAEN ;      //Enableing the clock for GPIO

		// GPIOC is on AHB1 bus
		RCC->AHB1ENR |=RCC_AHB1ENR_GPIOCEN ;      //Enableing the clock for GPIOA
		(void)RCC->AHB1ENR;                       // read-back to ensure clock is active
		//-------------------------------------

		GPIOA->MODER &= ~(3U << 10);             //Clearing the moder PA5 bits
		GPIOA->MODER |= (1U << 10);              // setting the moder  PA5 bits as output configuration

		GPIOA->OTYPER &= ~(1U << 5);               //Push -Pull type

		GPIOA->OSPEEDR &= ~(3U << 10);           //Clearing the output speed PA5 bits

		GPIOA->PUPDR &= ~(3U << 10);            //Clearing the PA5 PUPDR bits
		//--------------------------------------

		GPIOC->MODER &= ~(3U << 26);             //Clearing the moder PC13 bits to config as input mode

		GPIOC->OSPEEDR &= ~(3U << 26);           //Clearing the output speed PC13 bits to set  low speed

		GPIOC->PUPDR &= ~(3U << 26);            //Clearing the PC13 PUPDR bits
		GPIOC->PUPDR |= (1U << 26);             // config as pull-up

}

/*
 void method_2(void)
{
	if(!(GPIOC->IDR & (1U << 13)))
			{
				for(volatile uint32_t count=0;count<20000;count++);   //delay to overcome bouncing effecT
				if(!(GPIOC->IDR & (1U << 13)))
				{
					if(current_state == OFF)
					{
						GPIOA->BSRR = (1U << 5);                           //LED on
						current_state =ON;
					}
					else
					{
						GPIOA->BSRR = (1U <<(5+16));                       //LED off
						current_state=OFF;
					}
				}
			}
			while(!(GPIOC->IDR & (1U << 13)));                    //wait untile switch relese
}
*/
