// ADC example using LDR sensor

#include "stm32f4xx.h"
#include <stdio.h>

#define BAUDRATE     960U
#define CLOCK_FREQ   16000000U  // MCU clock frequency (16 MHz)

void uart_config(void);
static uint32_t Baudrate_config(uint32_t Clk_freq,uint32_t Baudrate);
void gpio_config();
void ADC_config();
void Uart_tx(uint16_t value);

int main(void)
{
	gpio_config();   // Configure GPIO pin used for ADC input
	ADC_config();    // Initialize ADC peripheral
	uart_config();   // Setup UART for sending ADC data

	uint16_t value;

	while(1)
	{
        // Wait until ADC conversion is completed
		while(!(ADC1->SR & 2U)){}

        // Read converted value from ADC data register
		value = ADC1->DR;

        // Send ADC value through UART
		Uart_tx(value);

        // Small delay between transmissions
		for(volatile int i = 0; i < 50000; i++);
	}
}

void gpio_config()
{
	// Enable clock for GPIOA (ADC pin is on port A)
	RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOAEN);
	(void)RCC->AHB1ENR;

    // Set PA0 to analog mode (11)
	GPIOA->MODER &= ~(3U << 0);
	GPIOA->MODER |= (3U << 0);

    // Disable pull-up and pull-down resistors
	GPIOA->PUPDR &= ~(3U << 0);
}

void ADC_config()
{
    // Enable clock for ADC1 (on APB2 bus)
	RCC->APB2ENR |= (RCC_APB2ENR_ADC1EN);
	(void)RCC->APB2ENR;

    // Disable ADC before starting configuration
	ADC1->CR2 &= ~(1U);

	// Configure ADC common control register
	ADC->CCR &= ~(0x1F);      // Clear prescaler bits
    ADC->CCR &= ~(3U << 16);  // Clear multi-mode configuration

	// Basic ADC configuration
	ADC1->CR1 &= ~(1U << 8);   // Disable scan mode

	ADC1->CR1 &= ~(3U << 24);  // Set resolution to 12-bit

	ADC1->CR2 |= (1U << 1);    // Enable continuous conversion mode

	ADC1->CR2 &= ~(1U << 11);  // Right data alignment

    // Set sampling time for channel 0
	ADC1->SMPR2 |= (7U);

    // Configure regular sequence length (only 1 conversion)
	ADC1->SQR1 &= ~(15U << 20);

    // Select channel 0 as first conversion in sequence
	ADC1->SQR3 &= ~(0x1F << 0);
	ADC1->SQR3 |=  (0 << 0);

    // Enable ADC peripheral
	ADC1->CR2 |= (1 << 0);

    // Start ADC conversion
	ADC1->CR2 |= (1U <<30);
}

void uart_config(void)
{
	// Enable clock for GPIOA (USART2 TX pin)
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	(void)RCC->AHB1ENR;

	// Configure PA2 as Alternate Function (USART2_TX)
	GPIOA->MODER &= ~(3U << 4);
	GPIOA->MODER |= (2U << 4);

	// Select AF7 for USART2 TX
	GPIOA->AFR[0] &= ~(0xFU << 8);
	GPIOA->AFR[0] |=  (7U << 8);

	// Enable clock for USART2 peripheral
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	(void)RCC->APB1ENR;

	// Disable USART before configuration
	USART2->CR1 &= ~USART_CR1_UE;

    // Configure baud rate
	USART2->BRR = Baudrate_config(CLOCK_FREQ,BAUDRATE);
    // UART does not store baud rate directly.
    // Instead it stores a divider value:
    // Divider = Clock frequency / Baudrate

	USART2->CR1 |= USART_CR1_TE;  // Enable transmitter

	USART2->CR1 |= USART_CR1_UE;  // Enable USART2
}

static uint32_t Baudrate_config(uint32_t Clk_freq,uint32_t Baudrate)
{
    // This formula helps to round the divider value correctly
	return ((Clk_freq + (Baudrate/2))/Baudrate);
}

void Uart_tx(uint16_t value)
{
    unsigned char byte[50];

    // Convert integer value to string so it can be sent over UART
    sprintf((char*)byte, "%d\r\n", value);

    // Send each character one by one
    for(uint8_t i = 0; byte[i] != '\0'; i++)
    {
        // Wait until transmit register is empty
        while(!(USART2->SR & USART_SR_TXE));

        // Load character into data register
        USART2->DR = byte[i];
    }

    // Wait until transmission is fully completed
    while(!(USART2->SR & USART_SR_TC));
}



/*
===============================================================================
ADC WORKING FLOW (Quick Reminder)

1. Enable GPIO Clock
   - Turn ON clock for the GPIO port where the analog pin is connected.

2. Configure GPIO Pin as ANALOG
   - Set MODER = 11 for the ADC pin (ex: PA0).
   - Disable pull-up / pull-down resistors.

3. Enable ADC Peripheral Clock
   - Enable clock for ADC in RCC APB2ENR register.

4. Configure ADC Basic Settings
   - Set resolution (usually 12-bit).
   - Disable scan mode if only one channel is used.
   - Set continuous or single conversion mode.
   - Configure data alignment (usually right aligned).

5. Set Sampling Time
   - Configure SMPR register for required sampling time.
   - Longer sampling gives more stable reading for sensors.

6. Select ADC Channel
   - Configure SQR registers to choose which channel to convert.
   - Example: Channel 0 → PA0.

7. Enable ADC
   - Set ADON bit in CR2 register.

8. Start Conversion
   - Set SWSTART bit in CR2 register.

9. Wait for Conversion Complete
   - Check EOC (End Of Conversion) flag in ADC_SR.

10. Read Converted Value
    - Read ADC_DR register to get digital value.

11. Process or Transmit Data
    - Send value to UART / display / control system.

-------------------------------------------------------------------------------
Extra Notes:

ADC Resolution (12-bit in STM32):
    Digital Range = 0 → 4095

Conversion Formula:

    Digital Value = (Input Voltage / Reference Voltage) × 4095

Example:
    If Vref = 3.3V
    Vin = 1.65V

    ADC Value ≈ (1.65 / 3.3) × 4095 ≈ 2047

-------------------------------------------------------------------------------
In this program:

LDR → produces analog voltage
ADC → converts voltage to digital value
UART → sends that value to PC

===============================================================================
*/
