/*
 * Copyright (c) 2019-2020, University of Southampton and Contributors.
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* ------ Includes ----------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include <msp430.h>

#include "hal_spi_rf.h"
#include "nRF24L01.h"

// #define ASSERT
#define BLINK
#define PROFILE


/* ------ Global Variables --------------------------------------------------*/
uint8_t dummy_payload[32] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                            0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                            0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                            0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};

/* ------ Function Prototypes -----------------------------------------------*/

#ifdef ASSERT
void init_indicate_pin() {
    P1DIR |= BIT1;
    P1OUT &= ~BIT1;
}

void indicate_test_fail() { P1OUT |= BIT1; }

// Assert c==true, otherwise indicate test fail and stall.
void assert(bool c) {
    if (!c) {
        indicate_test_fail();
        while (1) {}  // Stall
    }
}
#endif

void clk_init(void) {
    CSCTL0_H = 0xA5;  // Unlock register
    CSCTL1 |= DCOFSEL_6;  // DCO 8MHz

    // Set ACLK = VLO; SMCLK = DCO; MCLK = DCO;
    CSCTL2 = SELA_0 + SELS_3 + SELM_3;

    // ACLK: Source/1; SMCLK: Source/1; MCLK: Source/1;
    CSCTL3 = DIVA_0 + DIVS_0 + DIVM_0;  // SMCLK = MCLK = 8 MHz

    CSCTL0_H = 0x01;  // Lock Register
}

void timer_init(void) {
    TA0CCR0 = 3000;             // 7150 = ~1s
    TA0CTL = TASSEL__ACLK;      // ACLK (VLO ~9.4kHz), divided by 1
    TA0CCTL0 = CCIE;            // TACCR0 interrupt enabled
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

void sys_init(void) {
    WDTCTL = WDTPW | WDTHOLD;

    clk_init();
    gpio_init();
    timer_init();
    nrf24_spi_init();

#ifdef ASSERT
    init_indicate_pin();
#endif

#ifdef BLINK
    // Blink P1.0 for indicating packet received
    P1DIR |= BIT0;
    P1OUT &= BIT0;
#endif

#ifdef PROFILE
    P3DIR |= BIT0;
    P3OUT &= ~BIT0;
#endif

    PM5CTL0 &= ~LOCKLPM5;
}


int main(void) {
    sys_init();

    uint8_t status;
    uint8_t read_byte;








    /* ------ Test nrf24: Power Down -> Standby-I ------ */
    // Radio should be in Power Down in 10.3ms after VDD >= 1.9V
    // Wait for 10.3ms (10.3 / 1K * 8M)
    __delay_cycles(82400);

    // Flush TX FIFO
    status = nrf24_cmd(RF24_FLUSH_TX);

    // clear power-up data sent flag, write 1 to clear
    // 0x0E default
    status = nrf24_wr_reg(RF24_STATUS, RF24_TX_DS);

    // clear PWR_UP, 0x08 default CONFIG state
    status = nrf24_wr_reg(RF24_CONFIG, 0x08);

    // Read the STATUS and CONFIG registers
    status = nrf24_rd_reg(RF24_CONFIG, &read_byte);
#ifdef ASSERT
    assert(status == 0b00001110);    // STATUS should be 0b00001110
    assert(read_byte == 0b00001000);  // CONFIG should be 0b00001000
#endif

    // Power Down -> Standby-I
    status = nrf24_wr_reg(RF24_CONFIG, RF24_EN_CRC | RF24_PWR_UP);
#ifdef ASSERT
    assert(status == 0b00001110);
#endif

    // Disable Enchanced Shockburst auto acknowledge;
    // Enable Rx data pipe 0 only
    // Disable retransmit;
    status = nrf24_wr_reg(RF24_EN_AA, 0x00);  // Clear all auto acks
#ifdef ASSERT
    assert(status == 0b00001110);
#endif

    status = nrf24_wr_reg(RF24_EN_RXADDR, 0x01);  // Enable Rx data pipe 0 only
#ifdef ASSERT
    assert(status == 0b00001110);
#endif

    status = nrf24_wr_reg(RF24_SETUP_RETR, 0x00);  // Disable retransmit
#ifdef ASSERT
    assert(status == 0b00001110);
#endif




    // Should be in Standby-I in 1.5ms
    __delay_cycles(1500);
    // Power Down current draw is 900nA. (x100V/A = 90uV = 0.09 mV)
    // PD -> SI transient 285 uA (x100V/A = 28.5mV)
    // Standy-I current draw is 22 uA. (x100V/A = 2.2mV)
    // Read STATUS and CONFIG registers
    // Read the STATUS and CONFIG registers
    status = nrf24_rd_reg(RF24_CONFIG, &read_byte);
#ifdef ASSERT
    assert(status == 0b00001110);    // STATUS should be 0b00001110
    assert(read_byte == 0b00001010);  // CONFIG should be 0b00001010
#endif
    status = nrf24_rd_reg(RF24_RF_CH, &read_byte);
#ifdef ASSERT
    assert(status == 0x0E);
    assert(read_byte == 0x02);
#endif











    /* ------ Test: Standby-I -> Tx Mode -> Standby-I ------ */
    for (;;) {
#ifdef BLINK
        P1OUT ^= BIT0;
#endif
#ifdef PROFILE
        P3OUT |= BIT0;
#endif

        // Clear power-up data sent flag
        status = nrf24_wr_reg(RF24_STATUS, RF24_TX_DS);

#ifdef ASSERT
        status = nrf24_rd_reg(RF24_FIFO_STATUS, &read_byte);
        assert(status == 0x0E);
        assert(read_byte == 0x11);
#endif

        // Power up
        status = nrf24_wr_reg(RF24_CONFIG, RF24_EN_CRC | RF24_PWR_UP);
#ifdef ASSERT
        assert(status == 0b00001110);
#endif

        // Write payload to Tx Fifo
        // 1 level
        status = nrf24_wr_payload(dummy_payload, 32);
#ifdef ASSERT
        assert(status == 0b00001110);
        status = nrf24_rd_reg(RF24_FIFO_STATUS, &read_byte);
        assert(status == 0x0E);
        assert(read_byte == 0x01);
#endif

//         // 2 levels
//         status = nrf24_wr_payload(dummy_payload, 32);
// #ifdef ASSERT
//         assert(status == 0b00001110);
//         status = nrf24_rd_reg(RF24_FIFO_STATUS, &read_byte);
//         assert(status == 0x0E);
//         assert(read_byte == 0x01);
// #endif

//         // 3 levels
//         status = nrf24_wr_payload(dummy_payload, 32);
// #ifdef ASSERT
//         assert(status == 0b00001110);
//         status = nrf24_rd_reg(RF24_FIFO_STATUS, &read_byte);
//         assert(status == 0x0F);
//         assert(read_byte == 0x21);
// #endif

        // Set CE high for at least 10us (too short on this clone) to trigger Tx
        // Goes back to Standby-1 when everything sent
        RF_TRX_BEGIN();

        // If operating at 1MHz...
        // __delay_cycles(10);  // ~20us for 1 level (32B)
        // __delay_cycles(290);  // ~300us for 2 levels (64B)
        // __delay_cycles(440);  // ~450us for 3 levels (96B)

        // If opearting at 8MHz...
        __delay_cycles(80);  // ~20us for 1 level (32B)
        // __delay_cycles(2400);  // ~300us for 2 levels (64B)
        // __delay_cycles(3600);  // ~450us for 3 levels (96B)

        RF_TRX_END();

        // Air time:
        // Preamble 1 byte, Address 5 byte, Payload 1 byte, CRC 1 byte
        // 8 bytes * 8 bits/byte @ 1 Mbps = 64 us

        nrf24_wait_tx_done();

        // Power down
        // status = nrf24_wr_reg(RF24_CONFIG, RF24_EN_CRC);

#ifdef PROFILE
        P3OUT &= ~BIT0;
#endif

        /* Sleep in LPM3 */
        TA0CTL |= MC__UP;
        __bis_SR_register(LPM3_bits | GIE);
    }
}


// Timer0_A0 interrupt service routine
void __attribute__((interrupt(TIMER0_A0_VECTOR))) Timer0_A0_ISR(void) {
    TA0CTL &= ~MC;  // halt timer
    __bic_SR_register_on_exit(LPM3_bits | GIE);
}

