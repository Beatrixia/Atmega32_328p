/** This code use in my C++ project (its notsafe if you just copie and paste)
to connect to the real pin please use this setup
- button 1-3 connect with PORTD0,3,5 and gnd (in this case I use PORTD[1,3,5,7] to be gnd)
- 7Segment (PORTC[0-a,1-b,2-c,3-d,4-e,5-f,6-g,7-dp] PORTA[4-en_digit1,5-en_digit2,6-en_digit3,7-en_digit4]
**/

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//defind 16Mhz clk and libraries

	#define F_CPU 16000000UL
	#include <avr/io.h>
	#include <avr/interrupt.h>
	#include <util/delay.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// difine variable for code part

	// volatile for ISR (maybe its unnecessary)
	volatile uint8_t interrupt_count = 0;
	volatile uint16_t time_count = 0000; // start 7segment with 0000

	// limitation of 7-segment, 4 digits
	#define DISP_LIMIT 9999

	// interupt count untill 100ms
	// compare with compare_value (I use 1024clk per interupt)
	// 1024clk[time(sec)]*(78+1)[compare value with interupt]*(19+1)[interrupt count till 100ms] = 101.12 ms err 1.12%
	#define interrupt_set 19
	#define compare_value 78

	//this code i just copied from teacher > u < (set the How 7_segment bit should be)
	const unsigned char num[]= {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};
	unsigned char dig1,dig2,dig3,dig4; // define variable

	// button zone (pervious status,enable counting)
	unsigned char but1_s = 1; // button status 1 is not pressed yet
	unsigned char but2_s = 1;
	unsigned char but3_s = 1;
	unsigned char enable = 0; // status to enable counter

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// difine function part

	void limitter(void) { // function to limitthe display
		if (time_count > DISP_LIMIT) time_count = DISP_LIMIT;
	}

	void counting(void) { //function that timing main time .can use this fungtion anywhere 
		if (interrupt_count > interrupt_set) { 
			interrupt_count = 0;
			if (time_count) {
				time_count--;
			}
		}
	}

	void scan(void) { // function scan to scan the 7 segment
		static unsigned char digit = 0x80; // static use to create variable only once that don't disappear but cant use outside function
		switch(digit) {
			case 0x80: //1st digit.
			PORTC = dig1;
			break;
			case 0x40: //2nd digit.
			PORTC = dig2;
			break;
			case 0x20: //3rd digit.
			PORTC = dig3;
			break;
			case 0x10: //4th digit.
			PORTC = dig4;
			break;
			default:
			break;
		}
		PORTA = ~digit;
		digit >>= 1;
		if(0x10 > digit) digit=0x80;
	}

	void number(uint16_t cnt) { //use this function to define each digit
		dig1 =~num[cnt%10]; //active low, use ~
		cnt /=10;
		dig2 =~num[cnt%10]; 
		cnt /=10;
		dig3 =~num[cnt%10]; 
		cnt /=10;
		dig4 =~num[cnt%10];
	}
	
	void init_timer0(void) { // this function use to declare all of the value to interrupt
		// you can just paste this in main(void) but i don't do that XD
		// timing from 06-ch3-HW page 51
		TCCR0 |= ( 1 << WGM01 ); // CTC auto set 0 when compare match (Clear Timer on Compare)
		TCCR0 |= ( 1 << CS02 ) | ( 1 << CS00 ); // use timing per 1024clk
		TIMSK |= ( 1 << OCIE0 ); // enable compare match (int)
	
		TIFR |= ( 1 << TOV0 ); // clear overflow flag
		TCNT0 = 0x00; // set counter start at 0
		OCR0 = compare_value; // set the compare value
	}
	
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// ISR (interrupt zone) 
	
	ISR(TIMER0_COMP_vect) { //TIMER0_COMP_vect is function(NAME!) in "avr/interrupt.h" that declared! HOW THE DUCK THAT I KNOW?!
		if (enable == 1) interrupt_count++;
		scan();
	}
	
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// main code (- v - )
	
	int main(void) {
	
		// define port zone
		DDRA = 0xFF; // output to enable digit
		DDRC = 0xFF; // output to set the 7_segment number
		DDRD = 0xAA; // set output and input for button
		DDRB = 0xFF; // output for PB5
		PORTB = 0xFF; // send high for portB
		PORTD = 0x55; // set output to low and input to pull_up
		
		init_timer0(); //setting up interrupt 100ms
		sei(); // enable global interrupt	
	
	    while (1) {
			// looping setup zone
			counting();
			number(time_count);
	
			// button zone
			if (!(PIND & (1 << 0))) {
				if (but1_s == 1) {
					time_count = time_count + 50;
					but1_s = 0; 
					limitter();
					_delay_ms(20);
				}
			}
			else if (!(PIND & (1<<2))) {
				if (but2_s == 1) {
					time_count = time_count + 300;
					but2_s = 0; 
					limitter();
					_delay_ms(20);
				}
			}
			else if (!(PIND & (1<<4))) {
				if (but3_s == 1) {
						if (enable == 0) {
							enable = 1;
						}
						else {
							time_count = 0;
							enable = 0;
						}
					but3_s = 0; 
					_delay_ms(20);
				}
			}
			else if (but1_s == 0 || but2_s == 0 || but3_s == 0) {
				but1_s = 1; but2_s = 1; but3_s = 1; _delay_ms(20);
			}
	
			// LED PB5 zone
			if (time_count && enable) {
				PORTB &= ~(1<<5);
			}
			else if (!(time_count)) {
				PORTB |= (1<<5);
				enable = 0;
			}
	    }
	}
	
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	
