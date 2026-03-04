/*
 * LED Fading using PWM
 * Timer2 Channel 1 is used to generate PWM signal
 * By changing duty cycle gradually, LED brightness changes smoothly.
 */

#include "stm32f4xx.h"

static void gpio_config(void);
static void timer_config(void);

int main(void)
{
	gpio_config();     // Setup PA5 as PWM output pin
	timer_config();    // Setup Timer2 for PWM generation

	while(1)
	{
		// Increase duty cycle slowly → LED brightness increases
		for(volatile uint16_t i = 1; i <= 1000; i++)
		{
			TIM2->CCR1 = i;                            // Update compare value (controls ON time)
			for(volatile int d = 0; d < 8000; d++);   // Small delay for smooth fading
		}

		// Decrease duty cycle slowly → LED brightness decreases
		for(volatile uint16_t i = 1000; i >= 1; i--)
		{
			TIM2->CCR1 = i;                           // Reduce compare value
			for(volatile int d = 0; d < 8000; d++);   // Small delay for smooth fading
		}
	}
}

static void gpio_config(void)
{
	// Enable clock for GPIOA since PA5 is used
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	(void)RCC->AHB1ENR;  // Dummy read to confirm clock activation

	// Set PA5 to Alternate Function mode
	GPIOA->MODER &= ~(3U << 10);
	GPIOA->MODER |=  (2U << 10);

	// Configure output type as Push-Pull
	GPIOA->OTYPER &= ~(1U << 5);

	// Set medium output speed
	GPIOA->OSPEEDR &= ~(3U << 10);
	GPIOA->OSPEEDR |=  (2U << 10);

	// Disable pull-up and pull-down
	GPIOA->PUPDR &= ~(3U << 10);

	// Select AF1 (TIM2_CH1) for PA5
	GPIOA->AFR[0] &= ~(15U << 20);
	GPIOA->AFR[0] |=  (1U << 20);
}

static void timer_config(void)
{
	// Enable clock for TIM2 (APB1 bus)
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	(void)RCC->APB1ENR;  // Ensure clock is stable

	// Disable timer before making configurations
	TIM2->CR1 &= ~(TIM_CR1_CEN);

	// Prescaler: Divides timer clock
	TIM2->PSC = 15;

	// Auto-reload register: Defines PWM period
	TIM2->ARR = 999;

	// Select Channel 1 as output
	TIM2->CCMR1 &= ~(3U << 0);

	// Set PWM Mode 1 for Channel 1
	TIM2->CCMR1 &= ~(7U << 4);
	TIM2->CCMR1 |=  (6U << 4);

	// Enable preload for CCR1 (safe update of duty cycle)
	TIM2->CCMR1 |=  (1U << 3);

	// Initial duty cycle = 25%
	TIM2->CCR1 = 0;

	// Enable auto-reload preload (prevents sudden glitches)
	TIM2->CR1 |= TIM_CR1_ARPE;

	// Enable Channel 1 output
	TIM2->CCER |= (1U << 0);

	// Start Timer2
	TIM2->CR1 |= TIM_CR1_CEN;
}




/*
================================================================================
Basic PWM Explanation:

PWM (Pulse Width Modulation) is a technique used to control power by varying
the ON and OFF time of a digital signal.

Duty Cycle = (ON Time / Total Period) × 100%

If duty cycle is:
  0%   → LED completely OFF
  50%  → LED medium brightness
  100% → LED fully ON

In this program:
- ARR (999) defines total period.
- CCR1 controls ON time.
- Increasing CCR1 increases duty cycle → LED gets brighter.
- Decreasing CCR1 decreases duty cycle → LED gets dimmer.

So by changing CCR1 from 1 to 1000 and back,
we create a smooth fading effect.
================================================================================
*/

/*
===============================================================================
PWM Frequency Concept:

PWM Frequency means how many PWM cycles occur in one second.

Formula:

    PWM Frequency = Timer Clock / ((PSC + 1) × (ARR + 1))

Where:
    PSC → Prescaler value
    ARR → Auto Reload Register value

How it works:

1. Timer clock comes from MCU clock.
2. Prescaler (PSC) divides the timer clock.
3. ARR defines how many counts are required to complete one PWM period.
4. When counter reaches ARR, one full PWM cycle is completed.

In this program:

    Timer Clock = 16 MHz
    PSC = 15
    ARR = 999

So,

    Timer Frequency after prescaler
    = 16,000,000 / (15 + 1)
    = 1,000,000 Hz (1 MHz)

    PWM Frequency
    = 1,000,000 / (999 + 1)
    = 1000 Hz (1 kHz)

That means the LED PWM signal runs at 1 kHz.

Important Notes:

- Changing ARR changes PWM frequency.
- Changing CCR1 changes duty cycle (brightness).
- Frequency controls how fast the signal repeats.
- Duty cycle controls how long the signal stays HIGH.

For LED brightness control:
- Frequency should be high enough so human eye cannot detect flicker.
- Duty cycle is adjusted to change brightness.

===============================================================================
*/
