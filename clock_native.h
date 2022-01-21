// Norwegian letters used in native day names
// https://forum.arduino.cc/t/error-lcd-16x2/211977/6

// ø:
byte OE_small[8] = {B00000, B00001, B01110, B10101, B10101, B01110, B10000, B00000}; // ø, for søndag, lørdag (Sunday, Saturday)
lcd.createChar(NO_DK_OE_small, OE_small); //   Danish, Norwegian

// å:
byte AA_small[8] = {B00100, B00000, B01110, B00001, B01111, B10001, B01111, B00000}; 
lcd.createChar(SCAND_AA_small, AA_small); //   Danish, Norwegian, Swedish

// Å:
byte AA_capital[8] = {B00100,B00000,B01110,B10001,B11111,B10001,B10001,B00000}; 
lcd.createChar(SCAND_AA_capital, AA_capital); //   Danish, Norwegian, Swedish
