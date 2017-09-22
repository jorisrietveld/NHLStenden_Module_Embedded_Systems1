/**
 * Author: Joris Rietveld <jorisrietveld@gmail.com>
 * Created: 15-09-2017 02:44
 * License: GPLv3 - General Public License version 3
 */
#ifndef F_CPU // Setup the clock speed so we delay loops work correct.
#define F_CPU 16000000UL // 16 MHz clock speed
#endif

#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>

volatile static uint8_t counterMode = 0; //  The mode of the pulse counter
volatile static uint16_t countedPulses = 9995; // The counter
static uint8_t numberBase = 10; // The base the use when counting.

// Encoded numbers for 7 segment display, where the indexes correspond to the value being displayed. numbers 1 to 16 (hexadecimal) 
static uint8_t encodedNumbers[16] = { 0x82, 0xbb, 0x85, 0x91, 0b10111000, 0xd0, 0xc0, 0x9B, 0x80, 0x90 ,0x88,	0xe0, 0xc6, 0xa1, 0xc4, 0xcc, };

/**
 * This function gets executed when the external interrupt 0 is triggered. It will track the state of the leds
 * and update there values accordingly. The pattern used:
 */
ISR (INT0_vect)
{
    countedPulses++; // Increase the pulse count by one.
	countedPulses %= 10000;
}

/**
 * Write encoded number to the display using multiplexing.
 *
 * @param segmentByte The encoded number.
 * @param displayIndex The display to print it on.
 */
static void writeSegmentSequence( uint8_t segmentByte, uint8_t displayIndex)
{
    PORTA = ~( 1 << displayIndex ); // Set the correct display.
    for ( int i = 0; i < 7; ++i ) // Loop though every bit in an the segment display number encodings.
    {
        if( !( segmentByte & ( 1 << i ) ) ) // If the bit is 0 at the n'th position of the byte.
        {
            PORTC = ~( 1 << i ); // Turn on the corresponding segment (remember that 0 means on)
			_delay_us(30); 
        }    
    }
	PORTC = 0xff;
}

/**
 * Displays an number on the seven segment display.
 *
 * @param number The number to be printed on the display.
 */
static void writeNumbToDisplay(uint16_t number)
{
	uint8_t index = 0; // Tracks the display that is being written on. 
	
	while( number ) // While there is something left of the number to display.
	{
		uint8_t numberIndex = number % numberBase; //  Get the lowest  digit like: 123, get  3 by dividing by the number base and getting the remainder.
		writeSegmentSequence( encodedNumbers[ numberIndex ], index ); // Write the digit to the screen by getting the encoded value.
		number /= numberBase; // Throw away the lowest digit and move on the the next.
		index++; // Increase the display index.
	}
}

/**
 * This is the main
 * @return
 */
int main(void)
{
    DDRA = DDRC = 0xff; // Initiate ports A and C as output ports.
	// Enable interrupt 0 in the general interrupt control register.
	GICR = 1 << INT0;
	// Set the interrupt sens control bits in the MCU control register to listen for an raising edge event.
	MCUCR = 1 << ISC01 | 1 << ISC00;
	// Set enable interrupt to the control register so interrupts are enabled globally.
	sei();
	
    while (1) 
    {
		writeNumbToDisplay(countedPulses);
    }
}
