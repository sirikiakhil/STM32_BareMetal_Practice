/*
 * Sending Button Press Duration using UART TX
 *
 * Concept:
 * - Measure how long a button is pressed
 * - Use TIMER2 to count time
 * - Use UART (USART2) to print the duration
 * - Button press detected using edge detection
 */

#include"stm32f4xx.h"
#include<stdio.h>     // Required for printf() and sprintf()

#define HIGH 1
#define LOW  0
#define BAUDRATE     115200U
#define CLOCK_FREQ   16000000U  // MCU Clock Speed (16 MHz)

/*
 * Global variable to count timer overflows.
 *
 * IMPORTANT:
 * - This variable is modified inside an ISR (TIM2_IRQHandler)
 * - Therefore it MUST be declared as volatile
 * - static is NOT used because ISR and main must share this variable
 */

volatile uint32_t num_of_over_flows;

/* Function declarations */
static void uart_config(void);
static uint32_t Baudrate_config(uint32_t Clk_freq,uint32_t Baudrate);
static void Uart_tx(char ch);
static void gpio_config(void);
static void timer_config(void);
static float cal_fun(void);
static void nvic_config(void);

/*==========================================================*/
/*
 * Retargeting printf to UART
 *
 * Whenever printf() is used, internally it calls __io_putchar()
 * Here we redirect that character to UART transmit function.
 *
 * Without this function:
 * - printf() will NOT work on UART
 */
int __io_putchar(int ch){
	Uart_tx(ch);   // Send character via UART
	return ch;
}
/*==========================================================*/

int main(void)
{
    gpio_config();             // Configure button GPIO (PC13)
	timer_config();            // Configure TIMER2
	uart_config();             // Configure USART2 for TX

	char Duration[20];         // Buffer to store formatted time string
	uint8_t curr_state;        // Current button state
    uint8_t prev_state = HIGH; // Assume button initially released (pull-up)

	// Infinite loop
    while (1)
    {
       curr_state = (((GPIOC->IDR) >> 13) & 1);                // Read PC13 input value (button state)

       for(volatile uint32_t num = 0; num < 20000; num++);     // Simple software debounce delay

       /*
        * FALLING EDGE detection
        * HIGH -> LOW means button is pressed
        */
       if ((prev_state == HIGH) && (curr_state == LOW))
       {
              TIM2->SR &= ~TIM_SR_UIF;     // Clear update interrupt flag
              TIM2->CNT = 0;              // Reset timer counter
              TIM2->CR1 |= TIM_CR1_CEN;   // Start the timer
       }

       /*
        * RISING EDGE detection
        * LOW -> HIGH means button is released
        */
       else if ((prev_state == LOW) && (curr_state == HIGH))
       {
              TIM2->CR1 &= ~TIM_CR1_CEN;  // Stop the timer

              // Calculate pressed duration in seconds
              float sec = cal_fun();

              /*
               * sprintf():
               * - Converts float value into formatted string
               * - Stores result into character array
               * - "%.2f" means print float with 2 decimal places
               */
              sprintf(Duration, "%.2f\r\n", sec);

              // Print the duration through UART
              printf("%s", Duration);
       }

        // Store current state as previous state, Used for next loop iteration to detect edges
       prev_state = curr_state;
    }
}

/*==========================================================*/
/*
 * UART configuration function
 * - Uses USART2
 * - TX pin = PA2
 */

static void uart_config(void)
{
	// Enable clock for GPIOA (USART2 TX pin)
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	(void)RCC->AHB1ENR;   // Dummy read for clock stabilization

	// Configure PA2 as Alternate Function
	GPIOA->MODER &= ~(3U << 4);
	GPIOA->MODER |=  (2U << 4);

	// Select AF7 (USART2) for PA2
	GPIOA->AFR[0] &= ~(0xFU << 8);   // Clear bits 11:8
	GPIOA->AFR[0] |=  (7U << 8);     // AF7 = USART2_TX

	// Enable clock for USART2
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	(void)RCC->APB1ENR;

	// Disable USART before configuration
	USART2->CR1 &= ~USART_CR1_UE;

	// Set baud rate
	USART2->BRR = Baudrate_config(CLOCK_FREQ, BAUDRATE);

	// Enable Transmitter
	USART2->CR1 |= USART_CR1_TE;

	// Enable USART2
	USART2->CR1 |= USART_CR1_UE;
}

/*==========================================================*/
/*
 * GPIO configuration for button
 * - Button connected to PC13
 * - Input mode with pull-up
 */

static void gpio_config(void)
{
	// Enable clock for GPIOC
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
	(void)RCC->AHB1ENR;

	// Configure PC13 as input mode
	GPIOC->MODER &= ~(3U << 26);

	// Set low speed (not critical for input, but kept clean)
	GPIOC->OSPEEDR &= ~(3U << 26);

	// Enable pull-up resistor on PC13
    GPIOC->PUPDR &= ~(3U << 26);
	GPIOC->PUPDR |=  (1U << 26);
}

/*==========================================================*/
/*
 * Baudrate calculation function
 * Formula based on STM32 reference manual
 */

static uint32_t Baudrate_config(uint32_t Clk_freq,uint32_t Baudrate)
{
	return ((Clk_freq + (Baudrate/2)) / Baudrate);
}

/*==========================================================*/
/*
 * UART transmit function
 * - Polls TXE flag
 * - Sends one character
 */

static void Uart_tx(char ch)
{
	// Wait until transmit data register is empty
	while(!(USART2->SR & USART_SR_TXE)) {}

	// Write data to DR to transmit
	USART2->DR = ch;
}

/*==========================================================*/
/*
 * TIMER2 configuration
 * - Used to measure button press duration
 * - Prescaler creates 1ms tick
 * - Overflow interrupt enabled
 */

static void timer_config(void)
{
	// Enable clock for TIMER2
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	(void)RCC->APB1ENR;

	// Disable timer before configuration
	TIM2->CR1 &= ~TIM_CR1_CEN;

	/*
	 * Prescaler:
	 * 16 MHz / 16000 = 1000 Hz
	 * => Timer increments every 1 ms
	 */
	TIM2->PSC = 15999;


	TIM2->ARR = 100000;                // Auto-reload value (overflow after 100000 ms)


	TIM2->DIER |= TIM_DIER_UIE;         // Enable update interrupt


	TIM2->SR &= ~TIM_SR_UIF;            // Clear update interrupt flag

	// Reset counter
	TIM2->CNT = 0;

	// Configure NVIC for TIM2 interrupt
	nvic_config();
}

/*==========================================================*/
/*
 * NVIC configuration for TIMER2 interrupt
 * IRQ number for TIM2 = 28
 */

static void nvic_config(void)
{
	// Enable TIM2 interrupt in NVIC
	NVIC->ISER[0] |= (1U << 28);

	// Set priority for TIM2 interrupt
	NVIC->IP[28] &= ~(0xFU << 4);
	NVIC->IP[28] |=  (1U << 4);
}

/*==========================================================*/
/*
 * Function to calculate time duration
 * Uses:
 * - Timer counter value
 * - Number of overflows
 */

static float cal_fun(void)
{
	uint32_t count = TIM2->CNT;

	/*
	 * Total time in milliseconds:
	 * (overflow_count * ARR) + current_counter
	 * Divide by 1000 to convert to seconds
	 */
	float sec = (((num_of_over_flows * 100000) + count) / 1000.0f);


	TIM2->CNT = 0;               // Reset timer and overflow count
	num_of_over_flows = 0;

	return sec;
}

/*==========================================================*/
/*
 * TIMER2 Interrupt Service Routine
 * -Triggered on timer overflow
 * -Increments overflow counter
 */

void TIM2_IRQHandler(void)
{

	TIM2->SR &= ~TIM_SR_UIF;      // Clear update interrupt flag

	num_of_over_flows++;          // Increment overflow count
}
