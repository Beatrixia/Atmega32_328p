
#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

void init_timer2(void) {
	TCCR2B = (1<<CS22); //System divided by 64 (CLK/64).
	TCCR2A |= (1<<WGM21); //auto reload, TCNT2 reset to 0 when reaches OCR2A
	TIMSK2 = (1<<OCIE2A); // enable timer interrupt (output compare match A)
	TIFR2 = (1<<OCF2A); //clear flag
	TCNT2 = 0x00; //reset counter2
	OCR2A = 249; //output compare register A (compare value)
	//initialize
}

uint16_t volatile __millis = 0;

ISR (TIMER2_COMPA_vect) { // Timer 2 output compare match ISR.
	__millis += 1;
}

uint16_t compare_time = 500; //time(ms) to compare
uint16_t old_timer = 0;
uint16_t new_timer = 0;

int main(void) {
    /* Replace with your application code */
	init_timer2();
	sei();
	old_timer = __millis;
	new_timer = __millis;
	
	DDRC = 0x01;
	PORTC = 0x00;
	
    while (1) {
		new_timer = __millis;
		if ((new_timer - old_timer) >= compare_time) {
			PORTC ^= 0x01;
			old_timer = new_timer;
		}			
	}
}

