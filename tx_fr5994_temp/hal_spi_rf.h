/******************************************************************************
*  Filename: hal_spi_rf.h
*
*  Description: Common header file for spi access to the different tranceiver
*               radios. Supports CC1101/CC112X radios
*				 
*  Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
*
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*    Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*******************************************************************************/

#ifndef HAL_SPI_RF_H
#define HAL_SPI_RF_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * INCLUDES
 */
#include <msp430.h>
#include <stdint.h>

/******************************************************************************
 * CONSTANTS
 */

/* Pins defined for msp430fr5994 */

/* Pin map
 * nrf24     - msp430fr5994
 * Pin1 GND  - GND
 * Pin2 VCC  - VCC
 * Pin3 CE   - P3.6
 * Pin4 CS_N - P3.5
 * Pin5 SCLK - P5.2
 * Pin6 MOSI - P5.0
 * Pin7 MISO - P5.1
 * Pin8 IRQ  - P2.5
 */

/* Transceiver SPI signal */
// P5.0 UCB1MOSI
// P5.1 UCB1MISO
#define     RF_PORT_SEL0           P5SEL0
#define     RF_PORT_SEL1           P5SEL1
#define     RF_PORT_OUT            P5OUT
#define     RF_PORT_DIR            P5DIR
#define     RF_PORT_IN             P5IN
#define     RF_PORT_REN            P5REN
#define     RF_MOSI_PIN            BIT0
#define     RF_MISO_PIN            BIT1

// P5.2 UCB1CLK
#define     RF_SCLK_PORT_SEL0      P5SEL0
#define     RF_SCLK_PORT_SEL1      P5SEL1
#define     RF_SCLK_PIN            BIT2

// P3.5 CS_N
#define     RF_CS_N_PORT_SEL0      P3SEL0
#define     RF_CS_N_PORT_SEL1      P3SEL1
#define     RF_CS_N_PORT_OUT       P3OUT
#define     RF_CS_N_PORT_DIR       P3DIR
#define     RF_CS_N_PIN            BIT5


/* Transceiver interrupt configuration */
#define     RF_IRQ_PORT_VECTOR     PORT2_VECTOR
#define     RF_IRQ_OUT             P2OUT
#define     RF_IRQ_DIR             P2DIR
#define     RF_IRQ_IN              P2IN
#define     RF_IRQ_REN             P2REN
#define     RF_IRQ_SEL0            P2SEL0
#define     RF_IRQ_SEL1            P2SEL1
#define     RF_IRQ_PxIES           P2IES
#define     RF_IRQ_PxIFG           P2IFG
#define     RF_IRQ_PxIE            P2IE
#define     RF_IRQ_PIN             BIT5

/* Transceiver Tx/Rx enable */
#define     RF_CE_OUT              P3OUT
#define     RF_CE_DIR              P3DIR
#define     RF_CE_SEL0             P3SEL0
#define     RF_CE_SEL1             P3SEL1
#define     RF_CE_PIN              BIT6



/******************************************************************************
 *  Macros for Tranceivers(TRX)
 */
#define st(x)      do { x } while (0)
#define RF_SPI_BEGIN()              st(RF_CS_N_PORT_OUT &= ~RF_CS_N_PIN;)
#define RF_SPI_END()                st(RF_CS_N_PORT_OUT |= RF_CS_N_PIN;)
#define RF_SPI_TX(x)                st(UCB1IFG  &= ~UCRXIFG; \
                                       UCB1TXBUF = (x);)
#define RF_SPI_WAIT_DONE()          st(while (!(UCB1IFG & UCRXIFG));)
#define RF_SPI_WAIT_TX_DONE()       st(while (!(UCB1IFG & UCTXIFG));)
#define RF_SPI_RX()                 UCB1RXBUF
#define RF_TRX_BEGIN()              st(RF_CE_OUT |= RF_CE_PIN;)
#define RF_TRX_END()                st(RF_CE_OUT &= ~RF_CE_PIN;)



/******************************************************************************
 * PROTOTYPES
 */

void nrf24SpiInit(void);
uint8_t nrf24SpiTransaction(uint8_t data);
uint8_t nrf24RegWrite(uint8_t address, uint8_t data);
uint8_t nrf24RegRead(uint8_t address, uint8_t* data);
uint8_t nrf24Cmd(uint8_t cmd);
uint8_t nrf24WritePayload(uint8_t* payload, uint8_t size);
void nrf24_wait_tx_done(void);

#ifdef  __cplusplus
}
#endif

#endif  // HAL_SPI_RF_H
