/*
LeddyWear
---------
Project:  Low Power LED jewels
Platform: ATTiny25/45/85 @ 1Mhz
Author:   Stephane Come stephane@podpi.com
Date:     04/24/2017
 
http://www.leddywear.com
*/
 
// Include various libraries
#include <avr/sleep.h>     // for Sleep modes
#include <avr/wdt.h>       // for WatchDog Timer and waking up
#include <avr/interrupt.h> // for Interrupts support

// Routines to set and clear bits
// Clear bit
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
// Set bit
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif
 
// Variables for the Sleep/power down modes:
volatile boolean f_wdt = 1;       // watchdog mode, 1=wake up, 0=sleep
volatile uint8_t currPattern = 0; // current pattern to display

int pin0 = 0;  // Led on PB0
int pin1 = 1;  // Led on PB1
int pin3 = 3;  // Led on PB3
int pin4 = 4;  // N/C
int pin2 = 2;  // Button

// Microcontroller Setup
// ---------------------
// This routine runs nly once on startup (power up)
void setup()
{
    // Enable interupts on INT0 - button press
    MCUCR |= B00000000;    // Watch for low level on INT0
    GIMSK |= B01000000;    // Enable external interrupt
    SREG  |= B10000000;    // Global interrupt enable
  
    // Set the LED pins as OUTPUTs
    DDRB |= _BV(PB0) | _BV(PB1) | _BV(PB3);
    // Set the LED pins on HIGH (or LEDs off)
    PORTB |= _BV(PB0)|_BV(PB1)|_BV(PB3);
   
    // short burst of blinking when power up   
    for (int i=0; i<16; i++) {
      PORTB &= ~_BV(PB0);
      delay(10);
      PORTB |= _BV(PB0);
      PORTB &= ~_BV(PB1);
      delay(10);
      PORTB |= _BV(PB1);
      PORTB &= ~_BV(PB3);
      delay(10);
      PORTB |= _BV(PB3);
      delay(100);
    }
   
   // The WatchDog wakes up the device when in sleep mode.
   // The sleep duration can be set by setting the appropriate number 
   // in the setup_watchdog call.  Here are the available sleep times
   // 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
   // 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec
   
   setup_watchdog(8); // 8 will be approximately 4 seconds between cycles
}
 
// This method will be run in a continuous loop
void loop()
{  
   // wait for timed out watchdog / flag is set to 1 when a watchdog timeout occurs
   if (f_wdt==1) {  
      
      f_wdt=0;       // reset the watchdog flag

      if ( currPattern == 1 ) {
      
        // First pattern
        digitalWrite(pin0,LOW);    // Turn on LED on PB0 (right)
        delay(150);                // Wait for 150 milliseconds
        digitalWrite(pin3,LOW);    // Turn on LED on PB3 (middle)
        delay(150);                // Wait for 150 milliseconds
        digitalWrite(pin1,LOW);    // Turn on LED on PB1 (left)
        delay(1000);               // Wait for 1 second (1000 milliseconds)
        digitalWrite(pin0,HIGH);   // Turn off LED on PB0 (right)
        delay(150);                // Wait for 150 milliseconds
        digitalWrite(pin3,HIGH);   // Turn off LED on PB3 (middle)
        delay(150);                // Wait for 150 milliseconds
        digitalWrite(pin1,HIGH);   // Turn off LED on PB1 (left)
        
      } else if ( currPattern == 2 ) {
      
        // Second pattern
        digitalWrite(pin3,LOW);    // Turn on LED on PB3 (middle)
        delay(150);                // Wait for 150 milliseconds
        digitalWrite(pin0,LOW);    // Turn on LED on PB0 (right)
        digitalWrite(pin1,LOW);    // Turn on LED on PB1 (left)
        delay(750);                // Wait for 750 milliseconds
        digitalWrite(pin0,HIGH);   // Turn off LED on PB0 (right)
        digitalWrite(pin1,HIGH);   // Turn off LED on PB1 (left)
        delay(150);                // Wait for 150 milliseconds
        digitalWrite(pin3,HIGH);   // Turn off LED on PB3 (middle)
      
      } else if ( currPattern == 3 ) {
      
          // Third pattern
          // This pattern uses the PORTB notation and 
          // the _BV() methods to set the bits
          PORTB &= ~(_BV(PB3));        // Turn on the LED on PB3
          PORTB |= _BV(PB0)|_BV(PB1);  // Turn off the LEDs on PB0 and PB1
          delay(125);                  // Wait for 125 milliseconds
          PORTB |= _BV(PB0)|_BV(PB1)|_BV(PB3);  // Turn off all LEDs
          
      } else { }
      
   } else if ( f_wdt == 2 ) {
   
        // The button was pressed
        currPattern++;  // Increment the current pattern
        
        if ( currPattern == 1 ) { 
          // Show pattern 1
          PORTB &= ~(_BV(PB1));
          PORTB |= _BV(PB0)|_BV(PB3);
          
        } else if ( currPattern == 2 ) { 
          // Show pattern 2
          PORTB &= ~(_BV(PB3));
          PORTB |= _BV(PB0)|_BV(PB1);
          
        } else if ( currPattern == 3 ) { 
          // Show pattern 3
          PORTB &= ~(_BV(PB0));
          PORTB |= _BV(PB3)|_BV(PB1);

        } else if ( currPattern > 3 ) {
          // Turn off all LEDs
          PORTB |= (_BV(PB0)|_BV(PB1)|_BV(PB3));
          currPattern = 0;  // no pattern (sleep)
        }
        delay(500);  // Wait 500 milliseconds (for button debounce)
        f_wdt = 0;
        sei();  // enable interrupts
   }

   system_sleep();  // Put the device to sleep

}
 
// set system into the sleep state
// system wakes up when wtchdog is timed out
void system_sleep() 
{
   // Turn off some features to prevent current leakage
   // Turning off some features will increase the battery life
   cbi(ADCSRA,ADEN);      // switch Analog to Digitalconverter OFF
   pinMode(pin0,INPUT);   // Set pin PB0 as input
   pinMode(pin1,INPUT);   // Set pin PB1 as input
   pinMode(pin3,INPUT);   // Set pin PB3 as input

   set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
   sleep_enable();
   sleep_mode();                        // System sleeps here
 
   sleep_disable();                     // System continues execution here when watchdog timed out
   pinMode(pin0,OUTPUT);     // Set pin PB0 as output
   digitalWrite(pin0,HIGH);  // Turn off LED on PB0
   pinMode(pin1,OUTPUT);     // Set pin PB1 as output
   digitalWrite(pin1,HIGH);  // Turn off LED on PB1
   pinMode(pin3,OUTPUT);     // Set pin PB3 as output
   digitalWrite(pin3,HIGH);  // Turn off LED on PB3
}
 
void setup_watchdog(int ii) 
{
   byte bb;
   int ww;
   if (ii > 9 ) ii=9;
   bb=ii & 7;
   if (ii > 7) bb|= (1<<5);
   bb|= (1<<WDCE);
   ww=bb;
 
   MCUSR &= ~(1<<WDRF);
   // start timed sequence
   WDTCR |= (1<<WDCE) | (1<<WDE);
   // set new watchdog timeout value
   WDTCR = bb;
   WDTCR |= _BV(WDIE);
}
 
// Interrupt when button is pressed 
ISR(INT0_vect) {
  cli();      // disbale any other interrupts
  f_wdt = 2;  // set the watchdog mode to 2
}
  
// Watchdog Interrupt Service / is executed when watchdog timed out
ISR(WDT_vect) {
  f_wdt=1;  // set global flag
}
