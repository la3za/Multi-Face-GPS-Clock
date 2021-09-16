// Norwegian letters used in native day names


//  byte AA_small[8] = {B00100, B00000, B01110, B00001, B01111, B10001, B01111, B00000}; // å, for måne (moon)
    byte OE_small[8] = {B00000, B00001, B01110, B10101, B10101, B01110, B10000, B00000}; // ø, for søndag, lørdag (Sunday, Saturday)


//  lcd.createChar(6, AA_small); //   Danish, Norwegian, Swedish
  lcd.createChar(NO_DK_OE_small, OE_small); //   Danish, Norwegian
  lcd.clear(); // you have to, in order to set the LCD back to the proper memory mode
