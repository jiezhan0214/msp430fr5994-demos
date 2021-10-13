#include <stdint.h>
#include <string.h>
#include <msp430fr5994.h>

#include "aesa.h"
#include "uart.h"

#define CAL_ADC_12T30 0x0a63
#define CAL_ADC_12T85 0x0c2e

uint16_t adc_reading;

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
    CSCTL0_H = CSKEY_H;  // Unlock register
    CSCTL1 |= DCOFSEL_6;  // DCO 8MHz

    // Set ACLK = VLO; SMCLK = DCO; MCLK = DCO;
    CSCTL2 = SELA_1 + SELS_3 + SELM_3;

    // ACLK: Source/1; SMCLK: Source/1; MCLK: Source/1;
    CSCTL3 = DIVA_0 + DIVS_0 + DIVM_0;  // SMCLK = MCLK = 8 MHz

    CSCTL0_H = 0x01;  // Lock Register
}

void adc12_init(void) {
    // Configure ADC12

    // Comment the following line to use 1.2V reference
    // REFCTL0 |= REFVSEL_1;               // 2.0 V reference selected

    ADC12CTL0 = ADC12SHT0_3 |           // 32 cycles sample and hold time
                ADC12ON;                // ADC12 on

    // 1MHz clock source
    ADC12CTL1 = ADC12PDIV_1 |           // Predivide by 4
                ADC12SHP    |           // SAMPCON is the sampling timer
                ADC12DIV_1  |           // Divide by 2
                ADC12SSEL_3;            // SMCLK clock source

    // Default: 12-bit conversion results, 14 cycles conversion time
    // ADC12CTL2 = ADC12RES_2;

    // Use temperature sensor
    ADC12CTL3 = ADC12TCMAP;            // Temperature sensor selected for A30
    ADC12MCTL0 = ADC12INCH_30 |
                 ADC12VRSEL_1;          // VR+ = VREF buffered, VR- = Vss

    ADC12IER0 = ADC12IE0;               // Enable ADC conv complete interrupt
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
    uart_init();
    adc12_init();
    timer_init();
    PM5CTL0 &= ~LOCKLPM5;
}

int main(void) {
    sys_init();

    char str_buffer[20];

    for (;;) {
        /* Temperature reading and send thru UART */
        ADC12CTL0 |= ADC12ENC | ADC12SC;    // Start sampling/conversion
        __bis_SR_register(LPM0_bits | GIE);

        double voltage = (double)adc_reading / 4095.0 * 1.2 * 1000.0;

        double temperature = ((double)adc_reading - CAL_ADC_12T30) /
                             (CAL_ADC_12T85 - CAL_ADC_12T30) * 55.0 + 30.0;

        uart_send_str("ADC reading: ");
        uart_send_str(uitoa_10(adc_reading, str_buffer));
        uart_send_str(" / 4095, Voltage: ");
        uart_send_str(uitoa_10((unsigned)voltage, str_buffer));
        uart_send_str(" mV, Temperature: ");
        uart_send_str(uitoa_10((unsigned)temperature, str_buffer));
        uart_send_str(".");
        uart_send_str(uitoa_10((unsigned)(temperature * 10) % 10, str_buffer));
        uart_send_str(" C\n\r");

        /* Sleep in LPM3 */
        TA0CTL |= MC__UP;
        __bis_SR_register(LPM3_bits);

        /* AES encryption */
        P1OUT |= BIT0;  // For profiling
        // aes_128_enc(key, iv, input, input, 128);
        __delay_cycles(24000);
        P1OUT &= ~BIT0;  // For profiling

        /* Sleep in LPM3 */
        TA0CTL |= MC__UP;
        __bis_SR_register(LPM3_bits);
    }
}

// ADC12 interrupt service routine
void __attribute__ ((interrupt(ADC12_B_VECTOR))) ADC12_ISR(void) {
    switch (__even_in_range(ADC12IV, ADC12IV__ADC12RDYIFG)) {
        case ADC12IV__ADC12IFG0:            // Vector 12:  ADC12MEM0
            adc_reading = ADC12MEM0;
            __bic_SR_register_on_exit(LPM0_bits);
            break;
        default: break;
    }
}

// Timer0_A0 interrupt service routine
void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) Timer0_A0_ISR (void) {
    TA0CTL &= ~MC;  // halt timer
    __bic_SR_register_on_exit(LPM3_bits);
}

