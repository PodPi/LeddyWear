/*
LeddyWear
---------
Project:  Ultra Low Power LED jewels
Platform: ATTiny45 @ 1Mhz
Author:   Stephane Come stephane@podpi.com
Date:     04/24/2017
http://www.leddywear.com
*/
 
// Include various libraries
#include <avr/sleep.h>     // for sleep times
#include <avr/wdt.h>       // for WatchDog Timer and waking up
#include <avr/interrupt.h> // for Interrupts support

// Routines to set and clear bits
// Clear
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
// Set
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif
 
// Variables for the Sleep/power down modes:
volatile boolean f_wdt = 1;       // watchdog mode, 1=wake up, 0=sleep
volatile uint8_t currPattern = 0; // current pattern to display

// Microcontroller Setup
// ---------------------
// This routine runs once on startup only
void setup()
{
    // Enable interupts on INT0 - button press
    MCUCR |= B00000000;    // Watch for low level on INT0
    GIMSK |= B01000000;    // Enable external interrupt
    SREG  |= B10000000;    // Global interrupt enable
  
    // Sleep for 4 seconds
    setup_watchdog(8); // approximately 4 seconds sleep
}
 
void loop()
{ 
   if (f_wdt==1) {  // wait for timed out watchdog / flag is set when a watchdog timeout occurs
      f_wdt=0;       // reset flag
      // your logic here
   }
   system_sleep();
}
 
// set system into the sleep state
// system wakes up when wtchdog times out
void system_sleep() 
{
   cbi(ADCSRA,ADEN);                    // switch Analog to Digitalconverter OFF
   set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
   sleep_enable();
   sleep_mode();                        // System sleeps here
   sleep_disable();                     // System continues execution here when watchdog timed out
}
 
// 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
// 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec
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
 
ISR(INT0_vect) {
  cli();
  f_wdt = 2;  
}
  
// Watchdog Interrupt Service / is executed when watchdog timed out
ISR(WDT_vect) {
  f_wdt=1;  // set global flag
}

