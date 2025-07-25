#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

int main(void) {
  DDRC = 0xff;
  PORTC = 0x00;
  //_delay_ms(500);
  while(1) {
    PORTC ^= 0xff;
    _delay_ms(500);
  }
}
