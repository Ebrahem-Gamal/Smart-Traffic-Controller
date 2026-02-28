#include <avr/io.h>
#include <util/delay.h>

#define F_CPU 8000000UL // Define clock speed

// LCD control pins connected to PORTD
#define RS PD0
#define RW PD1
#define E  PD2
#define D4 PD4
#define D5 PD5
#define D6 PD6
#define D7 PD7

// Function to send command to LCD
void lcd_command(uint8_t command) {
	PORTD = (PORTD & 0x0F) | (command & 0xF0);  // Send the high nibble
	PORTD &= ~(1 << RS);                        // RS = 0 for command
	PORTD &= ~(1 << RW);                        // RW = 0 for write
	PORTD |= (1 << E);                          // Enable high
	_delay_us(1);
	PORTD &= ~(1 << E);                         // Enable low
	_delay_us(200);

	PORTD = (PORTD & 0x0F) | (command << 4);    // Send the low nibble
	PORTD |= (1 << E);                          // Enable high
	_delay_us(1);
	PORTD &= ~(1 << E);                         // Enable low
	_delay_us(200);
}

// Function to send data to LCD
void lcd_data(uint8_t data) {
	PORTD = (PORTD & 0x0F) | (data & 0xF0);     // Send the high nibble
	PORTD |= (1 << RS);                         // RS = 1 for data
	PORTD &= ~(1 << RW);                        // RW = 0 for write
	PORTD |= (1 << E);                          // Enable high
	_delay_us(1);
	PORTD &= ~(1 << E);                         // Enable low
	_delay_us(200);

	PORTD = (PORTD & 0x0F) | (data << 4);       // Send the low nibble
	PORTD |= (1 << E);                          // Enable high
	_delay_us(1);
	PORTD &= ~(1 << E);                         // Enable low
	_delay_us(200);
}

// Function to initialize LCD
void lcd_init() {
	// Set PORTD pins as output
	DDRD |= (1 << RS) | (1 << RW) | (1 << E) | (1 << D4) | (1 << D5) | (1 << D6) | (1 << D7);

	// Initialize LCD in 4-bit mode
	_delay_ms(15);          // Wait for more than 15 ms after Vcc is applied
	lcd_command(0x33);      // Initialize LCD in 4-bit mode
	lcd_command(0x32);      // Initialize LCD in 4-bit mode
	lcd_command(0x28);      // 4-bit mode, 2-line, 5x7 matrix
	lcd_command(0x0C);      // Display ON, Cursor OFF
	lcd_command(0x06);      // Increment cursor (shift to right)
	lcd_command(0x01);      // Clear screen
	_delay_ms(2);           // Wait for the command to process
}

// Function to set the cursor position on LCD
void lcd_gotoxy(uint8_t x, uint8_t y) {
	uint8_t pos;
	if (y == 0) {
		pos = 0x80 + x;  // First row
		} else {
		pos = 0xC0 + x;  // Second row
	}
	lcd_command(pos);    // Send position command
}

// Function to display string on LCD
void lcd_puts(const char *str) {
	while (*str) {
		lcd_data(*str);   // Display each character
		str++;
	}
}

// Function to display a number on two 7-segment displays
void display_bcd_number(uint8_t number) {
		uint8_t units = number % 10; // Extract units place

	uint8_t tens = number / 10;  // Extract tens place
//	uint8_t units = number % 10; // Extract units place

	// Display tens place on first seven-segment
	PORTC = (tens << 4) | (units & 0x0F); // High nibble for tens, low nibble for units
	_delay_ms(5);
}

// Function to control traffic lights and display text on LCD
void control_traffic_lights(uint8_t state) {
	if (state == 0) {
		// Traffic Lights 1 & 2: Green; Traffic Lights 3 & 4: Red
		PORTA = (1 << PA2) | (1 << PA5); // Green for Traffic Lights 1 & 2
		PORTB = (1 << PB0) | (1 << PB3); // Red for Traffic Lights 3 & 4
		lcd_gotoxy(0, 0); // First row, first column
		lcd_puts("MOVE AB STOP CD   "); // Show "MOVE AB STOP CD"
		
		
		} else if (state == 1) {
		// All Yellow Lights for transition
		PORTA = (1 << PA1) | (1 << PA4); // Yellow for Traffic Lights 1 & 2
		PORTB = (1 << PB1) | (1 << PB4); // Yellow for Traffic Lights 3 & 4
		lcd_gotoxy(0, 0);
		lcd_puts("DON'T MOVE AB"); // Show "don't move FOR YELLOW"
		lcd_gotoxy(0, 1);
		lcd_puts("READY CD ");
		} else if (state == 2) {
		// Traffic Lights 3 & 4: Green; Traffic Lights 1 & 2: Red
		PORTA = (1 << PA0) | (1 << PA3); // Red for Traffic Lights 1 & 2
		PORTB = (1 << PB2) | (1 << PB5); // Green for Traffic Lights 3 & 4
		lcd_gotoxy(0, 1);
		lcd_puts("STOP AB MOVE CD   "); // Show "STOP AB MOVE CD"
		
	}
	else if (state == 3) {
		// All Yellow Lights for transition
		PORTA = (1 << PA1) | (1 << PA4); // Yellow for Traffic Lights 1 & 2
		PORTB = (1 << PB1) | (1 << PB4); // Yellow for Traffic Lights 3 & 4
		lcd_gotoxy(0, 0);
		lcd_puts("READY AB"); // FOR YELLOW"
		lcd_gotoxy(0, 1);
		lcd_puts("DON'T MOVE CD ");
	}
}

int main() {
	// Configure ports
	DDRA = 0xFF; // Set PORTA as output (Traffic Lights 1 & 2)
	DDRB = 0xFF; // Set PORTB as output (Traffic Lights 3 & 4)
	DDRC = 0xFF; // Set PORTC as output for BCD signals
	DDRD = 0xFF; // Set PORTD as output for LCD

	// Initialize ports
	PORTA = 0x00; // All lights off
	PORTB = 0x00; // All lights off
	PORTC = 0x00; // Clear BCD output
	PORTD = 0x00; // Clear LCD output

	// Initialize LCD
	lcd_init(); // Initialize LCD in 4-bit mode

	uint8_t state = 0; // Initial state (Traffic Lights 1 & 2 Green, 3 & 4 Red)

	while (1) {
		// Green/Red Phase: Countdown from 20 to 0
		for (uint8_t i = 20; i > 0; i--) {
			control_traffic_lights(state); // Set traffic light states
			display_bcd_number(i);        // Display countdown on 7-segment
			_delay_ms(1000);              // 1-second delay
			
		}
		lcd_command(0x01); // Clear LCD screen after display
		_delay_ms(1000);   // Delay after clearing
		
		// Yellow Phase: Run for 5 seconds
		for (uint8_t i = 4; i > 0; i--) {
			control_traffic_lights(1); // Yellow lights
			display_bcd_number(i);     // Display countdown on 7-segment
			_delay_ms(1000);           // 1-second delay
		          }
		lcd_command(0x01); // Clear LCD screen after display
		_delay_ms(1000);   // Delay after clearing
		// Green/Red Phase: Countdown from 20 to 0
		for (uint8_t i = 20; i > 0; i--) {
			control_traffic_lights(2); // Set traffic light states
			display_bcd_number(i);        // Display countdown on 7-segment
			_delay_ms(1000);              // 1-second delay
			
		}
		lcd_command(0x01); // Clear LCD screen after display
		_delay_ms(1000);   // Delay after clearing
		// Yellow Phase: Run for 5 seconds
		for (uint8_t i = 4; i > 0; i--) {
			control_traffic_lights(3); // Yellow lights
			display_bcd_number(i);     // Display countdown on 7-segment
			_delay_ms(1000);           // 1-second delay
		}
		lcd_command(0x01); // Clear LCD screen after display
		_delay_ms(1000);   // Delay after clearing
		
		
	}
	return 0;
	}