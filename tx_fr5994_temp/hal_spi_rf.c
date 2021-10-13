#include "hal_spi_rf.h"
#include "nRF24L01.h"

void nrf24SpiInit(void) {
    /* Keep peripheral in reset state*/
    UCB1CTL1 |= UCSWRST;

    /* Configuration
     * -  8-bit
     * -  Master Mode
     * -  3-pin, active low
     * -  synchronous mode
     * -  MSB first
     * -  Clock phase select = captured on first edge
     * -  SMCLK as clock source
     * -  Spi clk is adjusted corresponding to systemClock as the highest rate
     *    supported by the supported radios: this could be optimized and done
     *    after chip detect.
     */
    UCB1CTL0  = (UCMST | UCSYNC | UCMSB | UCCKPH) >> 8;
    UCB1CTL1 |= UCSSEL_2;

    // f_BitClock = f_BRClock / prescalerValue
    UCB1BR1 = 0x00;
    UCB1BR0 = 0x08;

    // configure RF_MISO_PIN, RF_MOSI_PIN
    RF_PORT_SEL0 |=   RF_MISO_PIN | RF_MOSI_PIN;
    RF_PORT_SEL1 &= ~(RF_MISO_PIN | RF_MOSI_PIN);

    // configure RF_SCLK_PIN
    RF_SCLK_PORT_SEL0 |=  RF_SCLK_PIN;
    RF_SCLK_PORT_SEL1 &= ~RF_SCLK_PIN;

    // configure RF_CS_N_PIN
    RF_CS_N_PORT_SEL0 &= ~RF_CS_N_PIN;
    RF_CS_N_PORT_SEL1 &= ~RF_CS_N_PIN;
    RF_CS_N_PORT_DIR  |=  RF_CS_N_PIN;
    RF_CS_N_PORT_OUT  |=  RF_CS_N_PIN;


    // configure IRQ as GPIO style input on the MSP430
    RF_IRQ_SEL0 &= ~RF_IRQ_PIN;
    RF_IRQ_SEL1 &= ~RF_IRQ_PIN;
    RF_IRQ_DIR  &= ~RF_IRQ_PIN;
    RF_IRQ_REN  |=  RF_IRQ_PIN;
    RF_IRQ_OUT  |=  RF_IRQ_PIN;
    RF_IRQ_PxIES|=  RF_IRQ_PIN;
    RF_IRQ_PxIE |=  RF_IRQ_PIN;

    // configure CE
    RF_CE_SEL0  &= ~RF_CE_PIN;
    RF_CE_SEL1  &= ~RF_CE_PIN;
    RF_CE_DIR   |=  RF_CE_PIN;
    RF_CE_OUT   &= ~RF_CE_PIN;


    /* Release for operation */
    UCB1CTL1 &= ~UCSWRST;
    return;
}


void nrf24_wait_tx_done(void) {
    __bis_SR_register(LPM3_bits | GIE);
    // Wait for interrupt from RF_IRQ_PIN...
}

void __attribute__((interrupt(RF_IRQ_PORT_VECTOR))) RF_IRQ_PORT_ISR(void) {
    if (RF_IRQ_PxIFG & RF_IRQ_PIN) {
        // RF_IRQ_PxIFG &= ~RF_IRQ_PIN;  // Clear interrupt flags
        RF_IRQ_PxIFG = 0;
        __bic_SR_register_on_exit(LPM3_bits | GIE);
    }
}


uint8_t nrf24SpiTransaction(uint8_t data) {
    RF_SPI_TX(data);
    RF_SPI_WAIT_DONE();
    return RF_SPI_RX();
}

uint8_t nrf24RegWrite(uint8_t address, uint8_t data) {
    /* Send register address byte */
    RF_SPI_BEGIN();
    RF_SPI_TX(RF24_W_REGISTER | address);
    RF_SPI_WAIT_DONE();

    /* Storing chip status */
    uint8_t status = RF_SPI_RX();

    /* Send data */
    RF_SPI_TX(data);
    RF_SPI_WAIT_DONE();

    RF_SPI_END();

    /* return the status byte value */
    return status;
}

uint8_t nrf24RegRead(uint8_t address, uint8_t* data) {
    RF_SPI_BEGIN();

    RF_SPI_TX(RF24_R_REGISTER | address);
    RF_SPI_WAIT_DONE();
    uint8_t status = RF_SPI_RX();

    RF_SPI_TX(RF24_NOP);
    RF_SPI_WAIT_DONE();
    *data = RF_SPI_RX();

    RF_SPI_END();
    return status;
}

uint8_t nrf24Cmd(uint8_t cmd) {
    RF_SPI_BEGIN();
    RF_SPI_TX(cmd);
    RF_SPI_WAIT_DONE();
    uint8_t status = RF_SPI_RX();
    RF_SPI_END();
    return status;
}

uint8_t nrf24WritePayload(uint8_t* payload, uint8_t size) {
    RF_SPI_BEGIN();
    RF_SPI_TX(RF24_W_TX_PAYLOAD);
    RF_SPI_WAIT_DONE();
    uint8_t status = RF_SPI_RX();
    while (size--) {
        RF_SPI_TX(*payload++);
        RF_SPI_WAIT_DONE();
    }
    RF_SPI_END();
    return status;
}
