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
#include "adc.h"

uint16_t prescalerValue = PRESCALER_1024; // Default prescaler value.
volatile uint16_t overflowCount = 0; // Global variable to store the number of timeroverflows.

// 16-bit timer. Used to adjust the LED-toggle frequency.
void timer1Init() {
    TCCR1A = 0; // Set default values
    TCCR1B = 0;
    TCCR1A |= (1 << WGM11) | (1 << WGM10); // Enable Fast PWM mode, 10-bit.
    TCCR1B |= (1 << WGM12) | (1 << WGM13); // Enable CTC mode (Clear Timer on Compare Match).
    TCCR1B |= (1 << CS12) | (1 << CS10); // Enable CS12 and CS10 for Prescaler 1024.
    OCR1A = 3125; // Timer counter value for 200ms compare match (16Mhz/1024/5Hz).
    TIMSK1 |= (1 << OCIE1A); // Enable compare match interrupt.
};

// 8-bit timer. Used for PWM on the LED.
void timer2Init() {
    TCCR2A |= (1 << WGM21) | (1 << WGM20); // Enable Fast PWM mode.
    TCCR2B |= (1 << CS22); // Set prescaler to 64.
    TCCR2A |= (1 << COM2B1); // The COM2B1 control the PWM of the digital pin 3.
    OCR2A = MAX_POWER_VALUE; // Set the top value for the timer.
    OCR2B = MIN_POWER_VALUE; // Set the duty cycle to 0.
};

// Adjustable LED frequency-delay.
ISR(TIMER1_COMPA_vect) {
    if(ledTimerOn) {
        if(overflowCount > 0) {
            overflowCount--;
        }  
        if (overflowCount == 0) {
            // When overflowCount reaches 0, toggle the LED.
            ledToggle();
        }
    }
};

// Interrupt when ADC-conversion is complete.
ISR(ADC_vect) {
    if(adcToggle){
        adcReadState = true;
    } else {
        adcReadState = false;
    }
}

// Switch the compare match value for the timer to increase/decrease LED toggle frequency.
void switchTimer1Value(uint32_t timerValue) {
    OCR1A = timerValue; // Set the new compare match value.
}; 

void adjustTimerFrequency(float frequency) {
    cli(); // Disable interrupts.
    // Calculate compare value based on desired frequency.
    uint32_t timerTicks = F_CPU / prescalerValue / frequency;
    OCR1A = (uint16_t)timerTicks - 1; // Subtract 1 because Timer1 counts from 0
    sei(); // Re-Enable interrupts.
}

// Switch the prescaler for the timer, if necessary.
void switchPrescaler(uint16_t prescaler) {
    cli(); // Disable interrupts.
    TCCR1B &= ~((1 << CS10) | (1 << CS11) | (1 << CS12)); // Clear the prescaler bits.
    switch(prescaler) {
        case PRESCALER_1:
            TCCR1B |= (1 << CS10);
            prescalerValue = PRESCALER_1;
            break;
        case PRESCALER_8:
            TCCR1B |= (1 << CS11);
            prescalerValue = PRESCALER_8;
            break;
        case PRESCALER_64:
            TCCR1B |= (1 << CS11) | (1 << CS10);
            prescalerValue = PRESCALER_64;
            break;
        case PRESCALER_256:
            TCCR1B |= (1 << CS12);
            prescalerValue = PRESCALER_256;
            break;
        case PRESCALER_1024:
            TCCR1B |= (1 << CS12) | (1 << CS10);
            prescalerValue = PRESCALER_1024;
            break;
        default:
            break;
    }
    sei(); // Re-enable interrupts.
};

// TIMER2 functions for ADC-printout - NOT IN USE FOR "DELUPPGIFT03"

/* 8-bit timer. Used to count seconds for the ADC-printout.
void timer2Init() {
    TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20); // Set prescaler to 1024.
    TIMSK2 |= (1 << TOIE2); // Enable overflow interrupt.
};*/

/* Timer for ADC-printout. Approximately 1 second.
ISR(TIMER2_OVF_vect) {
    // Rough 1 second delay.
    static uint16_t count = 0;
    count++;
    if (count >= 61) { // Approximately 1 second (16MHz/1024/256 = 61.035 Hz).
        adcPrintState = true;
        count = 0;
    }
} */