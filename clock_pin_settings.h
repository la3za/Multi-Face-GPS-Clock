// hardware pins, GPS baudrate

#define analog_buttons_pin A0

#define LCD_pwm 45 // PWM on Mega pins 2 - 13 or 44 - 46
#define potentiometer A1

// Uses Serial1 for GPS input
// Arduino Mega: Serial1 on pins 19 (RX) and 18 (TX),
/*
  4800; // OK for EM-406A and ADS-GM1
  9600; // OK for NEO-6M
*/
static const uint32_t GPSBaud =  9600; // QRPlabs GPS
