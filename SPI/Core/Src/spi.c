
#include "SPI.h"

/*
    PA5 -> SPI1_SCK
    PA6 -> SPI1_MISO
    PA7 -> SPI1_MOSI

    CS (Chip Select):
    PA3 is used as a MANUAL CS pin (GPIO output).
    This is for SPI practice and future real-device usage.
*/

void spi1_gpio_config(void)
{
    // Enable clock for GPIOA
    RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOAEN);
    (void)RCC->AHB1ENR;

    /*
        PA5, PA6, PA7 -> Alternate Function mode
        SPI hardware controls direction internally.
    */
    GPIOA->MODER &= ~((3U << 10) | (3U << 12) | (3U << 14));
    GPIOA->MODER |=  ((2U << 10) | (2U << 12) | (2U << 14));

    // AF5 for SPI1
    GPIOA->AFR[0] &=  ~((15 << 16) | (15 << 20) | (15 << 24));
    GPIOA->AFR[0] |=   ((5 << 16) | (5 << 20) | (5 << 24));

    /*
        PA3 configured as GPIO OUTPUT.
        Used as MANUAL Chip Select (CS).
        SPI1 does NOT use hardware NSS because SSM + SSI are enabled.
    */
    GPIOA->MODER &= ~(3 << 6);
    GPIOA->MODER |=  (1 << 6);
}

/************************************************************/

void spi1_config(void)
{
    /*
        ADDED:
        Enable SPI1 peripheral clock.
        GPIO clock alone is NOT enough.
    */
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    (void)RCC->APB2ENR;

    // Disable SPI before configuration
    SPI1->CR1 &= ~(SPI_CR1_SPE);

    // Baud rate (Master controls SPI clock)
    SPI1->CR1 |= (1U << 3);

    // CPOL = 1, CPHA = 1 (must match slave)
    SPI1->CR1 |= (SPI_CR1_CPOL);
    SPI1->CR1 |= (SPI_CR1_CPHA);

    // MSB first
    SPI1->CR1 &= ~(SPI_CR1_LSBFIRST);

    // 8-bit data frame
    SPI1->CR1 &= ~(1U << 11);

    // Master mode
    SPI1->CR1 |= (SPI_CR1_MSTR);

    /*
        Software NSS:
        - Prevents MODF error
        - SPI internally assumes NSS = HIGH
        - Does NOT control any external pin
    */
    SPI1->CR1 |= (SPI_CR1_SSM);
    SPI1->CR1 |= (SPI_CR1_SSI);

    // Enable SPI1
    SPI1->CR1 |= (SPI_CR1_SPE);
}

/************************************************************/

void spi2_gpio_config(void)
{
    // Enable clock for GPIOB
    RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOBEN);
    (void)RCC->AHB1ENR;

    /*
        SPI2 pins:
        PB13 -> SCK
        PB14 -> MISO
        PB15 -> MOSI
        Set to AF mode (AF5).
    */
    GPIOB->MODER &= ~((3U << 8) | (3U << 10) | (3U << 12));
    GPIOB->MODER |=  ((2U << 8) | (2U << 10) | (2U << 12));

    GPIOB->AFR[1] &=  ~((15 << 20) | (15 << 24) | (15 << 28));
    GPIOB->AFR[1] |=   ((5 << 20) | (5 << 24) | (5 << 28));

    /*
        NOTE:
        SPI2_NSS (PB12) is NOT configured.
        Because SPI2 uses Software NSS (SSM + SSI),
        SPI2 hardware ignores the NSS pin completely.
    */
}

/************************************************************/

void spi2_config(void)
{
    /*
        ADDED:
        Enable SPI2 peripheral clock.
    */
    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
    (void)RCC->APB1ENR;

    // Disable SPI2 before configuration
    SPI2->CR1 &= ~(SPI_CR1_SPE);

    // Slave does not control baud rate

    // CPOL and CPHA must match master
    SPI2->CR1 |= (SPI_CR1_CPOL);
    SPI2->CR1 |= (SPI_CR1_CPHA);

    // MSB first
    SPI2->CR1 &= ~(SPI_CR1_LSBFIRST);

    // 8-bit data frame
    SPI2->CR1 &= ~(1U << 11);

    // Slave mode
    SPI2->CR1 &= ~(SPI_CR1_MSTR);

    /*
        Software NSS for SPI2:
        - SPI2 is internally always enabled
        - NSS pin is ignored
        - Works for learning / internal testing
    */
    SPI2->CR1 |= (SPI_CR1_SSM);
    SPI2->CR1 |= (SPI_CR1_SSI);

    // Enable SPI2
    SPI2->CR1 |= (SPI_CR1_SPE);
}

/************************************************************/

void cs_enable(void)
{
    /*
        Pull CS LOW.
        Required for real SPI devices.
        Here it is kept for SPI practice and future compatibility.
    */
    GPIOA->ODR &= ~(1U << 3);
}

void cs_disable(void)
{
    /*
        Pull CS HIGH.
        Marks end of SPI transaction.
    */
    GPIOA->ODR |= (1U << 3);
}
