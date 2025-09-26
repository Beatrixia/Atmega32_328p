#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	setting ADC zone
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

volatile uint16_t adc_value = 0;

void ADC_init(void) {
	// Reference = AVcc
	ADMUX = (1 << REFS0);

	// choose ADC1
	ADMUX |= (1 << MUX0);

	ADCSRA = (1 << ADEN)  // enable ADC
			 | (1 << ADATE) // Auto Trigger Enable
			 | (1 << ADIE)  // Interrupt Enable
			 | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Prescaler=128
	
	// Trigger Source = Free Running
	ADCSRB = 0x00;
	
	// first time ADC
	ADCSRA |= (1 << ADSC);
}

ISR(ADC_vect) {
	// read ADC (16bit value from <avr/io.h>)
	adc_value = ADC;
}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	1ms interupt
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

volatile uint16_t __millis = 0;

void timer2_init(void) {
	TCCR2A |= (1 << WGM21);  // CTC mode
	TCCR2A &= ~(1 << WGM20);
	
	OCR2A = 249;
	
	// Prescaler = 64
	TCCR2B |= (1 << CS22) | (1 << CS21);
	TCCR2B &= ~(1 << CS20);
	
	TIMSK2 |= (1 << OCIE2A);
}

ISR(TIMER2_COMPA_vect) {
    __millis++;  // นับเวลา ms
}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	setting pwm zone
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

void PWM_init() {
	TCCR0A |= (1 << WGM00) | (1 << WGM01); // fast pwm
	TCCR0A |= (1 << COM0B1);	//non-inverting
	TCCR0B |= (1 << CS02);	//prescaler 256

	DDRD |= (1 << PD5); // output pwm
}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	setting uart zone
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

void uart_init(unsigned long baud) {
	// Set baud rate
	uint16_t ubrr = (F_CPU / 16 / baud) - 1; //<<this math is from datasheet its "ubrr = (f_cpu / (16*baud)) - 1" if doublespeed 16 is 8
	UBRR0H = (ubrr >> 8); // ubrr is 16 bit long but UBRR0 is only 8 bit so we have to set 8 high and 8 low each
	UBRR0L = ubrr;
	// Enable transmitter and receiver
	UCSR0B = (1 << RXEN0) | (1 << TXEN0); // rxen0 is enable rx , txen0 is enable tx
	// Set frame format: 8 data bits, 1 stop bit, no parity
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // just set bit can read from data sheet
}
void uart_transmit(uint8_t data) {
	// Wait for empty transmit buffer
	while (!(UCSR0A & (1 << UDRE0))); //UDRE0 will be 1 when its ready to send data (use to check is the wire have traffic)
	// Put data into buffer, sends the data
	UDR0 = data;
}
uint8_t uart_receive(void) {
	// Wait for data to be received
	while (!(UCSR0A & (1 << RXC0))); //RXC0 is flag that mean we have data came to us and finish tranfer
	// Get and return received data from buffer
	return UDR0;
}
int uart_putchar(char c, FILE *stream) { //WHAT IS THIS???
	if (c == '\n') {  // if c have \n sent \r too
		uart_transmit('\r');
	}
	uart_transmit(c);
	return 0;
}

FILE uart_output = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE); //WHAT IS THIS???

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	main
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#define LP PC4
#define RP PC5

void motor_cw(void) {
	PORTC |= (1<<LP);
	PORTC &= ~(1<<RP);
}

void motor_ccw(void) {
	PORTC |= (1<<RP);
	PORTC &= ~(1<<LP);
}

void motor_stop(void) {
	PORTC &= ~(1<<LP);
	PORTC &= ~(1<<RP);
}

int main(void)
{
	DDRC |= (1 << LP) | (1<<RP);
	PORTC &= ~((1 << LP) | (1 << RP));
	
	stdout = &uart_output;
	
	timer2_init();
	ADC_init(); //ADC 1
	PWM_init(); //OCR0B
	uart_init(9600);
	
	int16_t ref, motorPWM;
	float error, pControl, gain=5.0;
	ref = 512;
	
	sei();

	while (1)
	{
		error = (float) (ref - adc_value)/1024;
		pControl = gain * error;
		motorPWM = (int16_t) (pControl * 255);
		
		if (0< motorPWM) motor_ccw();
		else if (0> motorPWM) motor_cw();
		else motor_stop();
		
		if (255<abs(motorPWM)) OCR0B = 255;
		else OCR0B =abs(motorPWM);
		
		printf("ref=%d,error=%d,gain=%d,adc=%d\n", (int)ref, (int)ref-adc_value, (int)gain, adc_value);
	}
	
	/* 14.
	char data =0;
	data = uart_receive();
	if (data == 'f')
	*/
	
}

// acd to pwm : adc_8bit = adc_value >> 2;
//printf("%d\n",adc_8bit);
//OCR0B = adc_8bit;
