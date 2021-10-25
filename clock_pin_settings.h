// hardware pins, GPS baudrate

#define analog_buttons_pin A0
#define LCD_pwm 45 // PWM on Mega pins 2 - 13 or 44 - 46

#ifdef FEATURE_POTENTIOMETER
  #define potentiometer A1
#endif

static const uint32_t GPSBaud =  9600; // QRPlabs GPS

// Uses Serial1, pin 19, for GPS input
// Arduino Mega: Serial1 on pins 19 (RX) and 18 (TX)
/*
  4800; // OK for EM-406A and ADS-GM1
  9600; // OK for NEO-6M in QLG1
*/

#ifdef FEATURE_ROTARY_ENCODER
  #define PINA 2 //
  #define PINB 3 //A1
  #define PUSHB 31 // A2
#endif

//lcd pins
#if defined(FEATURE_LCD_4BIT) 
  #define lcd_rs 8
  #define lcd_enable 9 
  #define lcd_d4 4
  #define lcd_d5 5
  #define lcd_d6 6
  #define lcd_d7 7
#endif 
