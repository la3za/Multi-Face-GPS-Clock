// Hardware pins for backlight and rotary encoder, GPS baudrate, LCD display

//#define METRO

#ifndef METRO                    // default setup for Arduino Mega

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

  #else                             // Metro 
    #define analog_buttons_pin  A0  // unused
    #define LCD_PWM             12
    #define PIN_A               8 
    #define PIN_B               7 
    #define PUSHB               6
    #define GPS_PPS             3 

    #include "dtostrf.h"
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
