#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	setting uart zone
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

  void UART_Init(unsigned long baud) {
    // Set baud rate
    uint16_t ubrr = (F_CPU / 16 / baud) - 1;                //<<this math is from datasheet its "ubrr = (f_cpu / (16*baud)) - 1" if doublespeed 16 is 8
    UBRR0H = (ubrr >> 8);                                   //ubrr is 16 bit long but UBRR0 is only 8 bit so we have to set 8 high and 8 low each
    UBRR0L = ubrr;
    // Enable transmitter and receiver
    UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);;  //rxen0 is enable rx , txen0 is enable tx , RXCIE0 is interrupt mode for rx
    // Set frame format: 8 data bits, 1 stop bit, no parity
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);                 //Setbits from data sheet
  }
  void UART_Transmit(uint8_t UartTxData) {
    // Wait for empty transmit buffer
    while (!(UCSR0A & (1 << UDRE0)));                       //UDRE0 will be 1 when its ready to send data (use to check is the wire have traffic)
    // Put data into buffer, sends the data
    UDR0 = UartTxData;
  }
  int UART_Putchar(char c, FILE *stream) {
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
    UartRxFlag = 1;																					//UartRx Flag
  }

	// UartRxData ,UartRxFlag Use this variable For UartRx

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	setting I2C
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/
	
	// SCL = F_CPU / (16 + 2*TWBR*Prescaler)
	
	void TWI_Init(void) {
		TWSR = 0x00;											//Prescaler = 1
		TWBR = 72;												//~100kHz Clk at SCL @ 16MHz
		TWCR = (1 << TWEN);
	}

	void TWI_Start(void) {
		TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);		//
		while (!(TWCR & (1 << TWINT)));
	}
    
	void TWI_Stop(void) {
		TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);		//
		_delay_us(10);
	}
    
	void TWI_Write(uint8_t data) {
		TWDR = data;																				//
		TWCR = (1 << TWINT) | (1 << TWEN);									//
		while (!(TWCR & (1 << TWINT)));											//
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	setting I2C_LCD_1602
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

	#define LCD_ADDR (0x27 << 1)					//LCD_I2C_Address Shift left For TWI
	
	void LCD_Write(uint8_t data) {
		TWI_Start();
		TWI_Write(LCD_ADDR);
		TWI_Write(data);
		TWI_Stop();
	}
    
	#define LCD_EN  (1 << 2)
	#define LCD_RS  (1 << 0)
	#define LCD_BL  (1 << 3)

	void LCD_Pulse(uint8_t data) {
		LCD_Write(data | LCD_EN);
		_delay_us(1);												//If I2C Start not stable ise 5 ms
		LCD_Write(data & ~LCD_EN);
		_delay_us(50);											//If I2C Start not stable ise 100 ms
	}
	
	void LCD_Send(uint8_t data, uint8_t rs) {
		uint8_t high = data & 0xF0;
		uint8_t low  = (data << 4) & 0xF0;

		LCD_Pulse(high | rs | LCD_BL);
		LCD_Pulse(low  | rs | LCD_BL);
	}

	void LCD_Command(uint8_t cmd) {
		LCD_Send(cmd, 0);
	}

	void LCD_Data(uint8_t data) {
		LCD_Send(data, LCD_RS);
	}

	void LCD_Init(void) {
		_delay_ms(50);
		LCD_Pulse(0x30);
		_delay_ms(5);
		LCD_Pulse(0x30);
		_delay_us(150);
		LCD_Pulse(0x30);
		LCD_Pulse(0x20);   // 4-bit mode
		LCD_Command(0x28); // 4-bit, 2 line
		LCD_Command(0x0C); // Display ON, Cursor OFF
		LCD_Command(0x06); // Entry mode
		LCD_Command(0x01); // Clear
		_delay_ms(2);
	}

	//Set Cursor
	void LCD_SetCursor(uint8_t col, uint8_t row) {
		uint8_t row_addr[] = {0x00, 0x40};
		LCD_Command(0x80 | (col + row_addr[row]));
	}

	void LCD_Print(const char *str) {
		while (*str) {
			LCD_Data(*str++);
		}
	}
    
/* =-=-=-= Main() =-=-=-=
		UART_Init(9600);
    TWI_Init();
		LCD_Init();
		sei();
		
		LCD_SetCursor(0, 0);
		LCD_Print("ATmega328P");

		while(1) {
			if ( UartRxFlag=1 ) {
				LCD_SetCursor(0, 0);
				LCD_Print(UartRxData);
			}
		}
    =-=-=-= END_Example =-=-=-= */
