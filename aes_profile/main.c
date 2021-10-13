#include <stdint.h>
#include <string.h>
#include <msp430fr5994.h>

#include "aesa.h"

unsigned char key[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                      0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
unsigned char iv[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

unsigned char input[2048] =
    "Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Aenean commodo "
    "ligula eget dolor. Aenean massa. Cum sociis natoque penatibus et magnis "
    "dis parturient montes, nascetur ridiculus mus. Donec quam felis, "
    "ultricies nec, pellentesque eu, pretium quis, sem. Nulla consequat massa "
    "quis enim. Donec pede justo, fringilla vel, aliquet nec, vulputate eget, "
    "arcu. In enim justo, rhoncus ut, imperdiet a, venenatis vitae, justo. "
    "Nullam dictum felis eu pede mollis pretium. Integer tincidunt. Cras "
    "dapibus. Vivamus elementum semper nisi. Aenean vulputate eleifend tellus. "
    "Aenean leo ligula, porttitor eu, consequat vitae, eleifend ac, enim. "
    "Aliquam lorem ante, dapibus in, viverra quis, feugiat a, tellus. "
    "Phasellus viverra nulla ut metus varius laoreet. Quisque rutrum. Aenean "
    "imperdiet. Etiam ultricies nisi vel augue. Curabitur ullamcorper "
    "ultricies nisi. Nam eget dui. Lorem ipsum dolor sit amet, consectetuer "
    "adipiscing elit. Aenean commodo ligula eget dolor. Aenean massa. Cum "
    "sociis natoque penatibus et magnis dis parturient montes, nascetur "
    "ridiculus mus. Donec quam felis, ultricies nec, pellentesque eu, pretium "
    "quis, sem. Nulla consequat massa quis enim. Donec pede justo, fringilla "
    "vel, aliquet nec, vulputate eget, arcu. In enim justo, rhoncus ut, "
    "imperdiet a, venenatis vitae, justo. Nullam dictum felis eu pede mollis "
    "pretium. Integer tincidunt. Cras dapibus. Vivamus elementum semper nisi. "
    "Aenean vulputate eleifend tellus. Aenean leo ligula, porttitor eu, "
    "consequat vitae, eleifend ac, enim. Aliquam lorem ante, dapibus in, "
    "viverra quis, feugiat a, tellus. Phasellus viverra nulla ut metus varius "
    "laoreet. Quisque rutrum. Aenean imperdiet. Etiam ultricies nisi vel "
    "augue. Curabitur ullamcorper ultricies nisi. Nam eget dui. Lorem ipsum "
    "dolor sit amet, consectetuer adipiscing elit. Aenean commodo ligula eget "
    "dolor. Aenean massa. Cum sociis natoque penatibus et magnis dis "
    "parturient montes, nascetur ridiculus mus. Donec quam felis, ultricies "
    "nec, pellentesque eu, pretium quis, sem. Nulla consequat massa quis enim. "
    "Donec pede justo";

void clk_init(void) {
    CSCTL0_H = CSKEY_H;     // Unlock register
    CSCTL1 |= DCOFSEL_6;    // DCO 8MHz
    // CSCTL1 |= DCOFSEL_3;    // DCO 4MHz
    // CSCTL1 |= DCOFSEL_0;    // DCO 1MHz

    // Set ACLK = VLO; SMCLK = DCO; MCLK = DCO;
    CSCTL2 = SELA_1 + SELS_3 + SELM_3;

    // ACLK: Source/1; SMCLK: Source/1; MCLK: Source/1;
    CSCTL3 = DIVA_0 + DIVS_0 + DIVM_0;  // SMCLK = MCLK = 8 MHz

    CSCTL0_H = 0x01;  // Lock Register
}

// void timer_init(void) {
//     TA0CCR0 = 7150;             // 7150 = ~1s
//     TA0CTL = TASSEL__ACLK;      // ACLK (VLO ~9.4kHz), divided by 1
//     TA0CCTL0 = CCIE;            // TACCR0 interrupt enabled
// }

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
    // timer_init();
    gpio_init();
    PM5CTL0 &= ~LOCKLPM5;
}

int main(void) {
    sys_init();

    for (;;) {
        /* AES encryption */
        // P1OUT |= BIT0;  // For profiling
        // aes_128_enc(key, iv, input, input, 1);   // 16 bytes
        // aes_128_enc(key, iv, input, input, 4);   // 64 bytes
        // aes_128_enc(key, iv, input, input, 16);   // 256 bytes
        // aes_128_enc(key, iv, input, input, 32);   // 512 bytes
        // aes_128_enc(key, iv, input, input, 64);   // 1024 bytes
        aes_128_enc(key, iv, input, input, 128);  // 2048 bytes
        // P1OUT &= ~BIT0;  // For profiling
        P1OUT ^= BIT0;

        /* Sleep in LPM3 */
        // TA0CTL |= MC__UP;
        // __bis_SR_register(LPM3_bits | GIE);
    }
}

// Timer0_A0 interrupt service routine
// void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) Timer0_A0_ISR (void) {
//     TA0CTL &= ~MC; // halt timer
//     __bic_SR_register_on_exit(LPM3_bits | GIE);
// }

