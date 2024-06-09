// Choose only one of these display interface options:
#define FEATURE_LCD_I2C                 // serial interface to 20x4 LCD on 0x27 I2C address
//#define FEATURE_LCD_4BIT              // parallel interface to 20x4 LCD

/*  Version 2.0.0+:
       Rotary alone:      New menu system, also supports buttons for up/down
*/
//#define FEATURE_BUTTONS       // two push buttons increase/decrease screen number 
                              // in addition to rotary encoder with push button

// Hardware pins for backlight and rotary encoder, GPS baudrate, LCD display

//#define ARDUINO_SAMD_VARIANT_COMPLIANCE  // tested for Adafreuit METRO Express M0

#ifndef ARDUINO_SAMD_VARIANT_COMPLIANCE          // default setup for Arduino Mega

    #define analog_buttons_pin  A0
    #define LCD_PWM             45 // PWM on Mega pins 2 - 13 or 44 - 46

    // Rotary encoder: 
      #define PIN_A             33 
      #define PIN_B             35 
      #define PUSHB             31  

    // Pulse per second from GPS
    #define GPS_PPS             3  //   GPS 1PPS signal to hardware interrupt pin 
    
    #ifdef FEATURE_POTENTIOMETER
      #define potentiometer A1
    #endif

  #else     //*** Metro Express M0 ***

    #define analog_buttons_pin  A0  // unused
    #define LCD_PWM             12
    #define PIN_A               8 
    #define PIN_B               7 
    #define PUSHB               6
    #define GPS_PPS             3  //13 //2 //3 //11 (not interrupt pin 4) 
    //#define INT_GPS             14 //15 // 14 //   corresponding INT number
    // https://forums.adafruit.com/viewtopic.php?t=155506
    // https://forums.adafruit.com/viewtopic.php?t=156152
    
    #ifdef FEATURE_INTERRUPTTEST
      volatile byte state = HIGH;
      pinMode(LED_BUILTIN, OUTPUT);  // for checking interrupt
    #endif

    #include "dtostrf.h"

    // the following from K3NG keyer #if defined(ARDUINO_SAMD_VARIANT_COMPLIANCE):
    #include <FlashAsEEPROM_SAMD.h>  // https://groups.io/g/radioartisan/topic/104997720#18735
  
    // see https://github.com/k3ng/k3ng_cw_keyer/blob/master/k3ng_keyer/k3ng_keyer.ino (search for: ARDUINO_SAMD_VARIANT_COMPLIANCE)
    extern uint32_t __get_MSP(void);   // Read the MSP register. 
    #define SP __get_MSP()


    // added 14.2.2024 - for Metro M0: Need to add a new line, ca. line 100, to sunrise.cpp:  return 0; 
    //                   later replaced, so no problem

  #endif

// Uses Serial1, pin 19, for GPS input
// Arduino Mega: Serial1 on pins 19 (RX) and 18 (TX)
/*
  4800; // OK for EM-406A and ADS-GM1
  9600; // OK for QLG1, QRPLabs
*/
// set of baud rates to choose from for GPS input:
static const uint32_t gpsBaud1[] = {4800, 9600, 19200};

//lcd pins
#if defined(FEATURE_LCD_4BIT) 
  #define lcd_rs 8
  #define lcd_enable 9 
  #define lcd_d4 4
  #define lcd_d5 5
  #define lcd_d6 6
  #define lcd_d7 7
#endif 
