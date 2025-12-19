#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

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
		TWDR = data;											//
		TWCR = (1 << TWINT) | (1 << TWEN);						//
		while (!(TWCR & (1 << TWINT)));						//
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	setting I2C_LCD_1602
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

	#define LCD_ADDR (0x27 << 1)								//LCD_I2C_Address Shift left For TWI
	
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
		_delay_us(1);											//If I2C Start not stable ise 5 ms
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
    	TWI_Init();
		LCD_Init();

		LCD_SetCursor(0, 0);
		LCD_Print("ATmega328P");

		LCD_SetCursor(0, 1);
		LCD_Print("I2C LCD 1602");

	}
    =-=-=-= END_Example =-=-=-= */