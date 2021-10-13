/*
 * Copyright (c) 2019-2020, University of Southampton and Contributors.
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* ------ Includes ----------------------------------------------------------*/
#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>

#include "hal_spi_rf.h"
#include "msp_nrf24.h"
#include "nRF24L01.h"


/* ------ Global Variables --------------------------------------------------*/
uint8_t buf[32];

uint8_t addr[5] = "latte";

/* ------ Function Prototypes -----------------------------------------------*/

void clk_init(void) {
    CSCTL0_H = 0xA5;  // Unlock register
    CSCTL1 |= DCOFSEL_6;  // DCO 8MHz

    // Set ACLK = VLO; SMCLK = DCO; MCLK = DCO;
    CSCTL2 = SELA__VLOCLK + SELS__DCOCLK + SELM__DCOCLK;

    // ACLK: Source/1; SMCLK: Source/1; MCLK: Source/1;
    CSCTL3 = DIVA_0 + DIVS_0 + DIVM_0;  // SMCLK = MCLK = 8 MHz

    CSCTL0_H = 0x01;  // Lock Register
}

void gpio_init(void) {
    // Initialize all pins to output low to reduce power consumption
    P1OUT = 0;
    P1DIR = 0xff;
    P2OUT = 0;
    P2DIR = 0xff;
    P3OUT = 0;
    P3DIR = 0xff;
    P4OUT = 0;
    P4DIR = 0xff;
    P5OUT = 0;
    P5DIR = 0xff;
    P6OUT = 0;
    P6DIR = 0xff;
    P7OUT = 0;
    P7DIR = 0xff;
    P8OUT = 0;
    P8DIR = 0xff;
    PJOUT = 0;
    PJDIR = 0xff;
}

void uart_init(void) {
    // P2.0 UCA0TXD
    // P2.1 UCA0RXD
    P2SEL0 &= ~(BIT0 | BIT1);
    P2SEL1 |=   BIT0 | BIT1;

    /* 115200 bps on 8MHz SMCLK */
    UCA0CTL1 |= UCSWRST;                        // Reset State
    UCA0CTL1 |= UCSSEL__SMCLK;                  // SMCLK

    // 115200 baud rate
    // 8M / 115200 = 69.444444...
    // 69.444 = 16 * 4 + 5 + 0.444444....
    UCA0BR0 = 4;
    UCA0BR1 = 0;
    UCA0MCTLW = UCOS16 | UCBRF0 | UCBRF2 | 0x5500;  // UCBRF = 5, UCBRS = 0x55

    UCA0CTL1 &= ~UCSWRST;
}

void uart_send_str_sz(char* str, unsigned sz) {
    UCA0IFG &= ~UCTXIFG;
    while (sz--) {
        UCA0TXBUF = *str++;
        while (!(UCA0IFG & UCTXIFG)) {}
        UCA0IFG &= ~UCTXIFG;
    }
}

int main(void) {
    /*** Initialization stack ***/

    WDTCTL = WDTPW | WDTHOLD;
    clk_init();
    gpio_init();
    // Blink P1.0 for indicating packet received
    P1DIR |= BIT0;
    P1OUT &= BIT0;
    uart_init();
    nrf24_spi_init();
    nrf24_ce_irq_pins_init();
    PM5CTL0 &= ~LOCKLPM5;

    nrf24_init();
    // nrf24_w_tx_addr(addr);
    nrf24_open_pipe(0);
    nrf24_w_rx_addr_p0(addr);

    // Flush RX FIFO
    nrf24_cmd(RF24_FLUSH_RX);
    // Clear all IRQ flags
    nrf24_wr_reg(RF24_STATUS, RF24_RX_DR | RF24_TX_DS | RF24_MAX_RT);
    // Power up in PRX mode
    nrf24_wr_reg(RF24_CONFIG, RF24_PWR_UP | RF24_PRIM_RX);
    __delay_cycles(12000);  // Should be in Standby-I in 1.5ms
    nrf24_enable_irq();
	RF_TRX_BEGIN();

    

    /* ------ Test: Power Down -> Standby-I -> Tx Mode -> Standby-I -> Power Down ------ */
    for (;;) {
        __bis_SR_register(LPM0_bits | GIE);

        nrf24_r_rx_payload(buf, 32);
        nrf24_wr_reg(RF24_STATUS, RF24_RX_DR);
        P1OUT ^= BIT0;
        uart_send_str_sz(buf, 32);
    }
}
