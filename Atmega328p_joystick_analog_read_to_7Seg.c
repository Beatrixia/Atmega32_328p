
#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>

uint8_t analog = 0x00;
const unsigned char num[]= {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};
unsigned char dig1,dig2,dig3,dig4;

void number(uint16_t cnt) { //use this function to define each digit
	dig1 =~num[cnt%10]; //active low, use ~
	cnt /=10;
	dig2 =~num[cnt%10];
	cnt /=10;
	dig3 =~num[cnt%10];
	cnt /=10;
	dig4 =~num[cnt%10];
}

void scan(void) { // function scan to scan the 7 segment
	static unsigned char digit = 0x08; // static use to create variable only once that don't disappear but cant use outside function
	switch(digit) {
		case 0x08: //1st digit.
		PORTD = dig1;
		break;
		case 0x04: //2nd digit.
		PORTD = dig2;
		break;
		case 0x02: //3rd digit.
		PORTD = dig3;
		break;
		case 0x01: //4th digit.
		PORTD = dig4;
		break;
		default:
		break;
	}
	PORTB = ~digit;
	digit >>= 1;
	if(0x01 > digit) digit=0x08;
}

void init_timer0(void) {
	// Set Timer0 to CTC Mode
	TCCR0A |= (1 << WGM01);  // CTC auto reset when compare match
	TCCR0B |= (1 << CS02) | (1 << CS00);  // Prescaler 1024

	TIMSK0 |= (1 << OCIE0A);  // Enable Compare Match A Interrupt

	TIFR0 |= (1 << TOV0);  // Clear overflow flag
	TCNT0 = 0x00;  // Counter start at 0
	OCR0A = 78;  // Set compare value
}

void adc_init(void) {
	ADMUX = (1 << REFS0); //Use AVcc(5v) to be reference.
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); //for prescaler for ADC Hz (128clk = 125kHz)
	ADMUX |= (1 << ADLAR); // use to setup yo use lsb or msb
	
	ADCSRA |= (1 << ADEN); // enable ADC
	
}

uint8_t abc_read(uint8_t port) {
	
	ADMUX = (ADMUX & 0xF8) | (port & 0x07);
	// "(ADMUX & 0xF8)" use to read the default setting of ADMUX by &
	// "(port & 0x07)" use to set 1 to the port we want
	
	ADCSRA |= (1 << ADSC);
	while (ADCSRA & (1 << ADSC));
	
	return ADCH;
}

ISR(TIMER0_COMPA_vect) {  // Correct vector name for ATmega328P
	scan();
}

int main(void) {
	
	DDRB = 0xFF;
	DDRD = 0xFF;
	PORTD = 0x00;
	
	sei();
	init_timer0();
	adc_init();
	
    while (1) {
		analog = abc_read(0);
		number(analog);
    }
}
