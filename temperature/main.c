#include <stdint.h>
#include <string.h>
#include <msp430fr5994.h>

#define CAL_ADC_12T30 0x0a63
// #define CAL_ADC_12T85 0x0c2e  // Old board
#define CAL_ADC_12T85 0x0c2c  // New board

uint16_t adc_reading;

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

void uart_send_str(char* str) {
    UCA0IFG &= ~UCTXIFG;
    while (*str != '\0') {
        UCA0TXBUF = *str++;
        while (!(UCA0IFG & UCTXIFG)) {}
        UCA0IFG &= ~UCTXIFG;
    }
}

/* unsigned int to string, decimal format */
char* uitoa_10(unsigned num, char* const str) {
    // calculate decimal
    char* ptr = str;
    unsigned modulo;
    do {
        modulo = num % 10;
        num /= 10;
        *ptr++ = '0' + modulo;
    } while (num);

    // reverse string
    *ptr-- = '\0';
    char* ptr1 = str;
    char tmp_char;
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }

    return str;
}

void clk_init(void) {
    CSCTL0_H = CSKEY_H;  // Unlock register
    CSCTL1 |= DCOFSEL_6;  // DCO 8MHz

    // Set ACLK = VLO; SMCLK = DCO; MCLK = DCO;
    CSCTL2 = SELA_1 + SELS_3 + SELM_3;

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
    TA0CCR0 = 7150;             // 7150 = ~1s
    TA0CTL = TASSEL__ACLK;      // ACLK (VLO ~9.4kHz), divided by 1
    TA0CCTL0 = CCIE;            // TACCR0 interrupt enabled
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
        ADC12CTL0 |= ADC12ENC | ADC12SC;    // Start sampling/conversion
        __bis_SR_register(LPM0_bits | GIE);
        // itoa(adc_reading, adc_reading_str, 10);

        double voltage = (double)adc_reading / 4095.0 * 1.2 * 1000.0;
        // itoa((int) voltage, voltage_str, 10);

        double temperature = ((double)adc_reading - CAL_ADC_12T30) /
                             (CAL_ADC_12T85 - CAL_ADC_12T30) * 55.0 + 30.0;
        // itoa((int) temperature, temperature_str, 10);

        uart_send_str("ADC reading: ");
        uart_send_str(uitoa_10(adc_reading, str_buffer));
        uart_send_str(" / 4095, Voltage: ");
        uart_send_str(uitoa_10((unsigned)voltage, str_buffer));
        uart_send_str(" mV, Temperature: ");
        uart_send_str(uitoa_10((unsigned)temperature, str_buffer));
        uart_send_str(".");
        uart_send_str(uitoa_10((unsigned)(temperature * 10) % 10, str_buffer));
        uart_send_str(" C\n\r");

        TA0CTL |= MC__UP;
        __bis_SR_register(LPM3_bits | GIE);
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
    TA0CTL &= ~MC; // halt timer
    __bic_SR_register_on_exit(LPM3_bits);
}

