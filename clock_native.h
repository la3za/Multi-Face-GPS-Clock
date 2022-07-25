#if FEATURE_NATIVE_LANGUAGE == 'no' | FEATURE_NATIVE_LANGUAGE == 'dk'
//  Norwegian letters used in native day names and in WordClock
//  https://forum.arduino.cc/t/error-lcd-16x2/211977/6

    // ø: "lørdag, søndag"
    byte OE_small[8] = {B00000, B00001, B01110, B10101, B10101, B01110, B10000, B00000}; // ø, for søndag, lørdag (Sunday, Saturday)
    lcd.createChar(NO_DK_oe_SMALL, OE_small); //   Danish, Norwegian

#elif  FEATURE_NATIVE_LANGUAGE == 'se' | FEATURE_NATIVE_LANGUAGE == 'is'

    // ö exists as char(B11101111) = char(239), no need to create it separately
    // ä, ü, ñ also
//    byte OE_small[8] = {B01010, B00000, B01110, B10001, B10001, B10001, B01110, B00000};
//    lcd.createChar(SE_DE_OE_small, OE_small); // Swedish, German, ...

#elif FEATURE_NATIVE_LANGUAGE == 'es' 
//  Spanish: é, á

    byte a_accent[] = {B00010, B00100, B01110, B00001, B01111, B10001, B01111, B00000};
    lcd.createChar(ES_IS_a_ACCENT, a_accent);
    
    byte e_accent[] = {B00010, B00100, B01110, B10001, B11111, B10000, B01110, B00000};
    lcd.createChar(ES_e_ACCENT, e_accent);
#endif

    
#if  FEATURE_NATIVE_LANGUAGE == 'no' | FEATURE_NATIVE_LANGUAGE == 'se' | FEATURE_NATIVE_LANGUAGE == 'dk'
    // å: "åtte"
    byte AA_small[8] = {B00100, B00000, B01110, B00001, B01111, B10001, B01111, B00000}; 
    lcd.createChar(SCAND_aa_SMALL, AA_small); //   Danish, Norwegian, Swedish
    
    // Å: "Åtte"
    byte AA_capital[8] = {B00100,B00000,B01110,B10001,B11111,B10001,B10001,B00000}; 
    lcd.createChar(SCAND_AA_CAPITAL, AA_capital); //   Danish, Norwegian, Swedish
#endif

#if FEATURE_NATIVE_LANGUAGE == 'is'
    // Icelandic, á, ð, Þ
    byte a_accent[] = {B00010, B00100, B01110, B00001, B01111, B10001, B01111, B00000};
    lcd.createChar(ES_IS_a_ACCENT, a_accent);

//   https://einhugur.com/blog/index.php/xojo-gpio/hd44780-based-lcd-display/
    byte Thorn[] =  {B01000, B01110, B01001, B01001, B01110, B01000, B01000, B00000};
    lcd.createChar(IS_THORN_CAPITAL, Thorn);

      byte eth[] = {B01000, B00100, B01110, B10001, B10001, B10001, B01110, B00000};;
//    byte eth[] = {B00100, B00010, B01110, B10001, B10001, B10001, B01110, B00000};
    lcd.createChar(IS_eth_SMALL, eth);

#endif
