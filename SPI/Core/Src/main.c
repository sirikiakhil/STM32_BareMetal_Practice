//SPI (loopback method)

#include "spi.h"

/************************************************************/


int main(void)
{


	//Configure GPIO pins for SPI2
	spi2_gpio_config();
	spi2_config();   //SPI2 as SLAVE


	//Configure GPIO pins for SPI1
	spi1_gpio_config();
	spi1_config();   //SPI1 as MASTER



	//Transmit data from SPI1 (MASTER)
	uint8_t spi1_tx_data[3] = {0x77, 0x88, 0x99};

	//Receive buffers
	uint8_t spi2_rx_data[3];   //Data received by SPI2 (SLAVE)
	uint8_t spi1_rx_data;   //Dummy data received by SPI1 (MASTER)

	volatile uint8_t temp;

	/*
		*CS is enabled here as good SPI practice, In this project, SPI2 uses Software NSS (SSM + SSI)
		*so, CS is not strictly required for SPI2 to work
	*/
	cs_enable();



	for(uint8_t i = 0; i < 3; i++)
	{
		/* SPI LOOPBACK (SAME MCU) ,DEADLOCK FREE STEPS

		1. Wait until SPI1 TXE =1
		2. Write data to SPI1->DR   (clock starts)
		3. Wait until SPI1 BSY = 0
		4. Read SPI1->DR          (clear master RX / prevent OVR)
		5. Read SPI2->DR           (slave RX, no blocking)

		*/
		//Wait until TX buffer empty
		while(!(SPI1->SR & SPI_SR_TXE));

		//Write data (this generates clock)
	    SPI1->DR = spi1_tx_data[i];

	    // Wait until SPI1 BSY =0 (transfer completed )
	    while(SPI1->SR & SPI_SR_BSY);

	    //Read MASTER RX frist complete
	    while(!(SPI1->SR & SPI_SR_RXNE));
	    spi1_rx_data = SPI1->DR;

	    //Loopback:BSY =0 guarantees slave RX, do NOT poll SPI2_RXNE (if we use it it may cause deadlock risk)
	    //(deadlock means -> in loopback both (master and slave ) need cpu at same time)

	    spi2_rx_data[i] = SPI2->DR;
	}

			// Wait until SPI fully done
			while(SPI1->SR & SPI_SR_BSY);


	cs_disable();

	// Wait until SPI1 is no longer busy
	//while(SPI1->SR & SPI_SR_BSY){}


	/*
		OVR (Overrun) safety clear
		Reading DR followed by SR clears possible OVR flag
		This is defensive coding for future scalability
	*/

	temp = SPI1->DR;
	temp = SPI1->SR;

	temp = SPI2->DR;
	temp = SPI2->SR;


	while(1)
	{
		//verify using LED or UART
	}
}

/************************************************************/








// Steps to follow when we are dealing with real hardwares

   //steps to transmit
     /*
		1. Wait for the TXE bit to set in the Status Register

		2. Write the data to the Data Register

	    3. After the data has been transmitted, wait for the sy bit to reset in Status Register

	    4. Clear the Overrun flag by reading DR and SR
	 */


   //steps to recevie
    /*
		1. Wait for the BSY bit to reset in Status Register

		2. Send some Dummy data before reading the DATA

	 	3. Wait for the RXNE bit to Set in the status Register

	 	4. Read data from Data Register
    */

/*
====================== SPI TRANSMIT FLOW (WHY EACH STEP EXISTS) ======================

1. Configure SPI peripheral (ONCE)
   - Set MASTER or SLAVE mode.
   - Configure CPOL and CPHA (clock idle level and sampling edge).
   - Set data frame size (8-bit / 16-bit).
   - Configure baud rate (master only).
   - Configure NSS handling (SSM/SSI or hardware NSS).

2. Enable SPI peripheral
   - SPI hardware is inactive until SPE bit is set.
   - No clock or data transfer happens before this.

3. Assert CS (Chip Select) LOW (if using GPIO CS)
   - Selects the target slave device.
   - Tells slave that communication is about to start.
   - Required for real external SPI devices.

4. Wait until TXE flag is set
   - TXE = Transmit buffer empty.
   - Ensures SPI is ready to accept new data.
   - Prevents overwriting the data register.

5. Write data to SPI Data Register (DR)
   - Writing DR starts the SPI clock automatically (master mode).
   - Data is loaded into transmit shift register.
   - At the same time, reception also begins (full-duplex).

6. Wait until transmission completes
   - Either wait for RXNE (receive buffer not empty)
   - Or wait until BSY flag becomes 0 (SPI not busy).
   - Ensures last clock pulse is finished.

7. Read DR (mandatory, even if data is dummy)
   - Clears RXNE flag.
   - Prevents overrun (OVR) error.
   - SPI always receives data during transmission.

8. Deassert CS HIGH (if using GPIO CS)
   - Marks end of SPI transaction.
   - Allows slave to process received data.

===============================================================================
KEY IDEA:
SPI is full-duplex: every transmit generates a receive.
Clock is generated automatically by master when DR is written.
Skipping RX read or BSY check can cause deadlock or OVR error.
===============================================================================
*/

/*
====================== SPI RECEIVE FLOW (WHY EACH STEP EXISTS) ======================

1. Configure SPI peripheral (ONCE)
   - Set MASTER or SLAVE mode.
   - Configure CPOL and CPHA (clock idle level and sampling edge).
   - Set data frame size (8-bit / 16-bit).
   - Configure NSS handling (SSM/SSI or hardware NSS).

2. Enable SPI peripheral
   - SPI hardware is inactive until SPE bit is set.
   - No reception can occur before this.

3. Assert CS (Chip Select) LOW (if using GPIO CS)
   - Selects the slave device.
   - Required for the slave to start driving MISO.

4. Wait until TXE flag is set
   - Even for RECEIVE, master must transmit something.
   - TXE ensures DR is ready for dummy data.

5. Write DUMMY data to SPI Data Register (DR)
   - SPI is full-duplex: reception happens ONLY during transmission.
   - Writing dummy data generates clock pulses.
   - Slave shifts real data onto MISO.

6. Wait until RXNE flag is set
   - RXNE = Receive buffer not empty.
   - Indicates a full data frame has been received.

7. Read data from SPI Data Register (DR)
   - Retrieves the received data.
   - Clears RXNE flag.
   - Mandatory to avoid overrun (OVR) error.

8. Wait until BSY flag becomes 0
   - Ensures last clock pulse is completed.
   - Prevents premature CS deassertion.

9. Deassert CS HIGH (if using GPIO CS)
   - Ends the SPI transaction.
   - Slave stops driving MISO.

===============================================================================
KEY IDEA:
SPI has no true "receive-only" mode.
Master must transmit dummy data to generate clock,
and every receive operation always includes a transmit.
===============================================================================
*/
