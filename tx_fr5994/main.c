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

// #define BLINK
// #define PROFILE


/* ------ Global Variables --------------------------------------------------*/
// uint8_t dummy_payload[32] = {0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
//                             0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
//                             0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
//                             0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55};
uint8_t dummy_payload[32] = "Hiya Baobeie, how are you today?";

uint8_t tx_addr[5] = "latte";

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

void timer_init(void) {
    TA0CCR0 = 10000;             // 7150 = ~1s
    TA0CTL = TASSEL__ACLK;      // ACLK (VLO ~9.4kHz), divided by 1
    TA0CCTL0 = CCIE;            // TACCR0 interrupt enabled
}

// Timer0_A0 interrupt service routine
void __attribute__((interrupt(TIMER0_A0_VECTOR))) Timer0_A0_ISR(void) {
    TA0CTL &= ~MC;  // Halt timer
    __bic_SR_register_on_exit(LPM3_bits | GIE);
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

int main(void) {
    /*** Initialization stack ***/

    WDTCTL = WDTPW | WDTHOLD;
    clk_init();
    gpio_init();
    timer_init();
#ifdef BLINK
    // Blink P1.0 for indicating packet received
    P1DIR |= BIT0;
    P1OUT &= BIT0;
#endif
#ifdef PROFILE
    P3DIR |= BIT0;
    P3OUT &= ~BIT0;
#endif
    nrf24_spi_init();
    nrf24_ce_irq_pins_init();
    PM5CTL0 &= ~LOCKLPM5;

    nrf24_init();
    nrf24_w_tx_addr(tx_addr);
    nrf24_enable_irq();

    /* ------ Test: Power Down -> Standby-I -> Tx Mode -> Standby-I -> Power Down ------ */
    for (;;) {
        /* Sleep in LPM3 */
        TA0CTL |= MC__UP;
        __bis_SR_register(LPM3_bits | GIE);

#ifdef BLINK
        P1OUT ^= BIT0;
#endif
#ifdef PROFILE
        P3OUT |= BIT0;
#endif

        /*** Write payload to Tx Fifo ***/
        // 1 level
        nrf24_w_payload(dummy_payload, 32);
        // 2 levels
        // nrf24_wr_payload(dummy_payload, 32);
        // 3 levels
        // nrf24_wr_payload(dummy_payload, 32);



        /*** Activate Tx ***/
        nrf24_power_up();
        // Set CE high for at least 10us (too short on this clone) to trigger Tx
        // Goes back to Standby-1 when everything sent
        RF_TRX_BEGIN();
        // If opearting at 8MHz...
        __delay_cycles(80);  // ~10us for 1 level (32B)
        // __delay_cycles(2400);  // ~300us for 2 levels (64B)
        // __delay_cycles(3600);  // ~450us for 3 levels (96B)
        RF_TRX_END();


        /*** Wait for Tx done ***/
        nrf24_wait_tx_done();
        // Clear power-up data sent flag
        nrf24_wr_reg(RF24_STATUS, RF24_TX_DS);
        // nrf24_wr_reg(RF24_STATUS, RF24_RX_DR | RF24_TX_DS | RF24_MAX_RT);

        /*** Power down ***/
        nrf24_wr_reg(RF24_CONFIG, RF24_EN_CRC);

#ifdef PROFILE
        P3OUT &= ~BIT0;
#endif
    }
}
