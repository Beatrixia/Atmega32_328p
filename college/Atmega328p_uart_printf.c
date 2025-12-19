#define F_CPU 16000000UL
#include <avr/io.h>
#include <stdio.h>

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	setting uart zone
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

  void uart_init(unsigned long baud) {
    // Set baud rate
    uint16_t ubrr = (F_CPU / 16 / baud) - 1;                //<<this math is from datasheet its "ubrr = (f_cpu / (16*baud)) - 1" if doublespeed 16 is 8
    UBRR0H = (ubrr >> 8);                                   //ubrr is 16 bit long but UBRR0 is only 8 bit so we have to set 8 high and 8 low each
    UBRR0L = ubrr;
    // Enable transmitter and receiver
    UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);;  //rxen0 is enable rx , txen0 is enable tx , RXCIE0 is interrupt mode for rx
    // Set frame format: 8 data bits, 1 stop bit, no parity
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);                 //Setbits from data sheet
  }
  void uart_transmit(uint8_t UartTxData) {
    // Wait for empty transmit buffer
    while (!(UCSR0A & (1 << UDRE0)));                       //UDRE0 will be 1 when its ready to send data (use to check is the wire have traffic)
    // Put data into buffer, sends the data
    UDR0 = UartTxData;
  }
  int uart_putchar(char c, FILE *stream) {
		if (c == '\n') {
    	uart_transmit('\r');
    }
    uart_transmit(c);
    return 0;
  }

  FILE uart_output = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

  volatile uint8_t UartRxData;                              //UartRx interrupt Data_in;
  volatile uint8_t UartRxFlag = 0;                          //UartRx Flag when interupt(true)

  ISR(USART_RX_vect) {
    UartRxData = UDR0;                                      //Read data from UartRx
    rx_flag = 1;                                            //UartRx Flag
  }

	// UartRxData ,UartRxFlag Use this variable For UartRx
	// printf(""); ,For send UartTx
