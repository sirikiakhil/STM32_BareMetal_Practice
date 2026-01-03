/*
 * Sending Character Using UART TX
 */
#include"stm32f4xx.h"

#define BAUDRATE     115200U
#define CLOCK_FREQ   16000000U  //MCU Clock Speed(16MHz)


static void uart_config(void);
static uint32_t Baudrate_config(uint32_t Clk_freq,uint32_t Baudrate);
static void Uart_tx(char ch);

int main(void)
{
	uart_config();
	while(1)
	{
		Uart_tx('U');
		for(volatile uint16_t i=0;i<50000;i++);
	}
}


static void uart_config(void)
{
	//Enable clock for PORTA
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	(void)RCC->AHB1ENR;

	//config PORTA Pin2 mode as Alternative function
	GPIOA->MODER &= ~(3U << 4);
	GPIOA->MODER |= (2U << 4);

	//Config AFRL
	GPIOA->AFR[0] &= ~(0xFU << 8);   // clear bits 11:8
	GPIOA->AFR[0] |=  (7U << 8);     // AF7 = USART2_TX


	//Enable clock for UART2
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	(void)RCC->APB1ENR;


	USART2->CR1 &= ~USART_CR1_UE;  //disabled uart to configure

	USART2->BRR = Baudrate_config(CLOCK_FREQ,BAUDRATE);

	USART2->CR1 |= USART_CR1_TE;  // TE

	USART2->CR1 |= USART_CR1_UE;    //Enable the UART2
}

static uint32_t Baudrate_config(uint32_t Clk_freq,uint32_t Baudrate)
{
	return ((Clk_freq + (Baudrate/2))/Baudrate);
}

static void Uart_tx(char ch)
{
	while(!(USART2->SR & USART_SR_TXE)){}

		USART2->DR = ch;
}
