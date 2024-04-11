#include <stdio.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer.h"
#include "led.h"
#include "serial.h"
#include "command.h"
#include "menu.h"
#include "device.h"

// 16-bit timer.
void timer1Init() {
    TCCR1A = 0; // Set default values
    TCCR1B = 0;
    TCCR1B |= (1 << WGM12); // Enable CTC mode (Clear Timer on Compare Match).
    TCCR1B |= (1 << CS11) | (1 << CS10); // Enable CS11 and CS10 for Prescaler 64.
    OCR1A = 50000; // Timer counter value for 200ms compare match (16Mhz/64/5Hz).
    TIMSK1 |= (1 << OCIE1A); // Enable compare match interrupt.
};

void timer2Init() {
    TCCR2A = 0; // Set default values
    TCCR2B = 0;
    // Enable Fast PWM mode
    TCCR2A |= (1 << WGM20) | (1 << WGM21);
    // Set non-inverting mode on OC2B
    TCCR2A |= (1 << COM2B1);
    TCCR2B |= (1 << CS22); // Enable CS22 for Prescaler 64.
    OCR2B = 0; // Set the duty cycle to 0.
};

ISR(TIMER1_COMPA_vect) {
    if(ledTimer) {
        ledToggle();
    }
}