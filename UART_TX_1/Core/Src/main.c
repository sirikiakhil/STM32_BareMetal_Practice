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

	USART2->BRR = Baudrate_config(CLOCK_FREQ,BAUDRATE); //UART does not store baud rate directly , insted it stores a divider value
    //Divider = (clock / Baudrate)

	USART2->CR1 |= USART_CR1_TE;  // TE

	USART2->CR1 |= USART_CR1_UE;    //Enable the UART2
}

static uint32_t Baudrate_config(uint32_t Clk_freq,uint32_t Baudrate)
{
	return ((Clk_freq + (Baudrate/2))/Baudrate); //this formula is used to rounding the integer division value
}

static void Uart_tx(char ch)
{
	while(!(USART2->SR & USART_SR_TXE)){}

		USART2->DR = ch;
}


/*
====================== UART TRANSMIT FLOW (WHY EACH STEP EXISTS) ======================

1. Configure UART peripheral (ONCE)
   - Select baud rate (based on peripheral clock).
   - Configure word length (8/9 bits).
   - Configure parity (none/even/odd).
   - Configure stop bits (usually 1).
   - Enable transmitter (TE bit).

2. Enable UART peripheral
   - UART hardware is inactive until UE bit is set.
   - TX line stays idle (HIGH) until transmission starts.

3. Wait until TXE flag is set
   - TXE = Transmit data register empty.
   - Indicates DR is ready to accept new data.
   - Prevents overwriting the transmit buffer.

4. Write data to UART Data Register (DR)
   - Writing DR starts transmission automatically.
   - Hardware adds START bit, parity (if enabled), and STOP bit.
   - Data is shifted out on TX line.

5. Wait until TC flag is set
   - TC = Transmission complete.
   - Ensures last STOP bit is fully transmitted.
   - Required before disabling UART or changing direction.

6. Repeat steps 3–5 for more data
   - Each byte is framed and transmitted independently.

===============================================================================
KEY IDEA:
UART is asynchronous: no clock line exists.
Each byte is framed with START and STOP bits,
and TXE and TC flags synchronize software with hardware timing.
===============================================================================
*/


/*
====================== UART RECEIVE FLOW (WHY EACH STEP EXISTS) ======================

1. Configure UART peripheral (ONCE)
   - Select baud rate (must match transmitter).
   - Configure word length, parity, and stop bits.
   - Enable receiver (RE bit).

2. Enable UART peripheral
   - UART hardware starts monitoring the RX line.
   - RX line remains idle HIGH until a START bit arrives.

3. Wait until RXNE flag is set
   - RXNE = Receive data register not empty.
   - Indicates a full frame (START + data + STOP) is received.
   - Data is now safe to read.

4. Read data from UART Data Register (DR)
   - Retrieves the received byte.
   - Clears RXNE flag automatically.
   - Must be read to avoid overrun (ORE) error.

5. Repeat steps 3–4 for more data
   - Each received byte is independent.
   - UART hardware handles framing for every byte.

===============================================================================
KEY IDEA:
UART reception is asynchronous.
The receiver detects the START bit automatically,
samples data using the configured baud rate,
and RXNE synchronizes software with received data.
===============================================================================
*/
