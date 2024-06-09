// Routines that define letters and language dependent routines

/*
nativeDayLong
loadNativeCharacters
WordClockNorwegian
 */

// user defined characters, #1-4 are already used in main code (for arrows etc), so numbers start with 5:

#define SE_DE_oe_SMALL   239  // ö, already exists in LCD memory

// The following def's must match those of LCDchar0_3 etc in *.ino file:
#define IS_eth_SMALL     4    // ð 
#define NO_DK_oe_SMALL   5    // ø
#define IS_THORN_CAPITAL 5    // þ  
#define ES_e_ACCENT      5    // é  
#define ES_IS_a_ACCENT   6    // á

#define SCAND_aa_SMALL   6    // å
#define SCAND_AA_CAPITAL 7    // Å

/////////////////////////////////////////////////////////////////////////

void nativeDayLong(float unixTime) {      // unix time in sec, output through global char array today
 //  Full weekday name in native (= non-English language)
 //  Max 12 characters in name
int dayAddr = weekday(unixTime) - 1; // day of the week (1-7), Sunday is day 1

#ifdef FEATURE_DAY_PER_SECOND     //    fake the day -- for testing only
          dayAddr = second(localTime/2)%7; // change every 2 seconds
#endif

    memcpy_P(today,myDays[languageNumber][dayAddr], 12);    // put today's day / language into "today"
 
// fix odd letters requiring special characters depending on language chosen:

    if (strcmp(languages[languageNumber],"no") == 0 || strcmp(languages[languageNumber],"dk") == 0
     || strcmp(languages[languageNumber],"ny") == 0)  
      if (dayAddr==0) today[1] = char(NO_DK_oe_SMALL); // Søndag - Norwegian, Danish, nynorsk

    if (strcmp(languages[languageNumber],"no") == 0 || strcmp(languages[languageNumber],"dk") == 0)
      if (dayAddr==6) today[1] = char(NO_DK_oe_SMALL); // Lørdag - Norwegian, Danish

    if (strcmp(languages[languageNumber],"es") == 0)
    {
      if (dayAddr==3) today[2] = char(ES_e_ACCENT);    // Miércoles
      if (dayAddr==6) today[1] = char(ES_IS_a_ACCENT); // Sábado
    }

    if (strcmp(languages[languageNumber],"se") == 0)
      if (dayAddr==0 || dayAddr==6) today[1] = char(SE_DE_oe_SMALL); // Söndag, Lördag

    if (strcmp(languages[languageNumber],"se") == 0 || strcmp(languages[languageNumber],"ny") == 0)
      if (dayAddr==1)               today[1] = char(SCAND_aa_SMALL); // Måndag

    if (strcmp(languages[languageNumber],"is") == 0)
    {
      if (dayAddr==1) today[1] = char(ES_IS_a_ACCENT);   // Mánudagur  
      if (dayAddr==2)
      {
          today[0] = char(IS_THORN_CAPITAL); // þriðjudagur
          today[3] = char(IS_eth_SMALL);     // þriðjudagur
      }
      if (dayAddr==3) today[2] = char(IS_eth_SMALL);     // Miðvikudagur
      if (dayAddr==5) today[1] = char(SE_DE_oe_SMALL);   // Föstudagu
    }

}
///////////////////////////////////////////////

const byte OE_small[8]   PROGMEM = {B00000, B00001, B01110, B10101, B10101, B01110, B10000, B00000}; // ø, for søndag, lørdag
const byte a_accent[8]   PROGMEM = {B00010, B00100, B01110, B00001, B01111, B10001, B01111, B00000};
const byte AA_small[8]   PROGMEM = {B00100, B00000, B01110, B00001, B01111, B10001, B01111, B00000}; 
const byte e_accent[8]   PROGMEM = {B00010, B00100, B01110, B10001, B11111, B10000, B01110, B00000};
//const byte AA_capital[8] PROGMEM = {B00100, B00000, B01110, B10001, B11111, B10001, B10001, B00000};
const byte AA_capital[8] PROGMEM = 
{B00100, 
 B01010, 
 B00100,
 B01010, 
 B10001, 
 B11111, 
 B10001, 
 B00000};  // like European/English LCD 4x20 9.4.2024
const byte Thorn[8]      PROGMEM = {B01000, B01110, B01001, B01001, B01110, B01000, B01000, B00000}; // Icelandic
const byte eth[8]        PROGMEM = {B01000, B00100, B01110, B10001, B10001, B10001, B01110, B00000}; // Icelandic
//const byte Cedila[8]     PROGMEM = {B00000, B01110, B10000, B10001, B01110, B00100, B01100, B00000}; // for Portugese
///////////////////////////////////////////////

void loadNativeCharacters(int8_t languageNumber)
{
//  LCD display definitions of special native characters for languages
//  03.03.2024: Using PROGMEM, simplified to only 2 rather than 4 options
//              Only loads the chosen language's character set, no longer for a group of languages

//  Norwegian/Danish letters used in native day names and in WordClock, ø: "lørdag, søndag", and Chemical Elements "sølv"
//  https://forum.arduino.cc/t/error-lcd-16x2/211977/6

boolean loadedCharacterSet = false;

if (LCDchar4_5 != LCDNATIVE || languageNumber != presentLanguageNumber)
{
    if (strcmp(languages[languageNumber],"no") == 0 || strcmp(languages[languageNumber],"dk") == 0 
    || strcmp(languages[languageNumber],"ny") == 0)
    {
        memcpy_P(buffer,OE_small, 8);
        lcd.createChar(NO_DK_oe_SMALL, buffer);       // ø: "Lørdag", "Søndag"
        loadedCharacterSet = true;
    }

    if (strcmp(languages[languageNumber],"es") == 0 || strcmp(languages[languageNumber],"is") == 0)
    {
        memcpy_P(buffer,a_accent, 8);
        lcd.createChar(ES_IS_a_ACCENT, buffer);       // á: "Sábado"
        loadedCharacterSet = true;
    }

    if (strcmp(languages[languageNumber],"se") == 0 || strcmp(languages[languageNumber],"ny") == 0)
    {
        // ö exists as char(B11101111) = char(239), no need to create it separately, ä, ü, ñ also
        memcpy_P(buffer,AA_small, 8);
        lcd.createChar(SCAND_aa_SMALL, buffer);       //  å: "måndag", Swedish/nynorsk
        loadedCharacterSet = true;
    }

    if (strcmp(languages[languageNumber],"es") == 0)
    {
        memcpy_P(buffer,e_accent, 8);                 // é, "Miércoles"
        lcd.createChar(ES_e_ACCENT, buffer);
        loadedCharacterSet = true;
    }

    if (strcmp(languages[languageNumber],"is") == 0)
    {
    //  Icelandic, á, ð, Þ
    //  https://einhugur.com/blog/index.php/xojo-gpio/hd44780-based-lcd-display/
    //  ö exists as char(B11101111) = char(239), no need to create it separately 
        memcpy_P(buffer,Thorn, 8);
        lcd.createChar(IS_THORN_CAPITAL, buffer);
        memcpy_P(buffer,eth, 8);
        lcd.createChar(IS_eth_SMALL, buffer);
        LCDchar6_7 = LCDNATIVE; // sine it fills more than locations 4 and 5
        loadedCharacterSet = true;
    }
    LCDchar4_5 = LCDNATIVE;

    //   lcd.clear();  // in order to set the LCD back to the proper memory mode after custom characters have been created
        #ifdef FEATURE_SERIAL_LOAD_CHARACTERS
            if (loadedCharacterSet) 
            {
              Serial.print(F("loadNativeCharacters: ")); Serial.print(languageNumber); 
              Serial.print(" ");Serial.println(languages[languageNumber]);
            }
        #endif
        presentLanguageNumber = languageNumber;
}
}

//////////////////////////
void loadAring()   // for use with WordClockNorwegian()
// New 02.03.2024
{
  if (LCDchar6_7 != LCDARING)
{
   memcpy_P(buffer,AA_capital, 8);
   lcd.createChar(SCAND_AA_CAPITAL, buffer); //  Norwegian, Å: "Åtte"

   memcpy_P(buffer,AA_small, 8);
   lcd.createChar(SCAND_aa_SMALL, buffer);   //  Norwegian, å: "åtte"

   LCDchar6_7 = LCDARING;

   #ifdef FEATURE_SERIAL_LOAD_CHARACTERS
     Serial.println(F("loadAring"));
  #endif

}
}


/////////////////////////////

void WordClockNorwegian()
{ // could be in PROGMEM
  char WordOnes[10][5] = {{"null"}, {"en  "}, {"to  "}, {"tre "}, {"fire"}, {"fem "}, {"seks"}, {"sju "}, {"xtte"}, {"ni  "}}; // left justified
  char CapiOnes[10][5] = {{"Null"}, {"Ett "}, {"To  "}, {"Tre "}, {"Fire"}, {"Fem "}, {"Seks"}, {"Sju "}, {"Xtte"}, {"Ni  "}}; // left justified
  char WordTens[6][7]  = {{"  Null"}, {"    Ti"}, {"  Tjue"}, {"Tretti"}, {" FXrti"}, {" Femti"}}; 
  char Teens[10][8]    = {{"       "},{"Elleve "},{"Tolv   "},{"Tretten"}, {"Fjorten"}, {"Femten "}, {"Seksten"}, {"Sytten "}, {"Atten  "}, {"Nitten "}};
  int ones, tens;
  char textbuf[21];

// replace some characters - the X's - with native Norwegian ones:
  WordTens[4][2] = (char)NO_DK_oe_SMALL;          // Førti, 'ø' also used in "lørdag", "søndag"
  WordOnes[8][0] = (char)SCAND_aa_SMALL;          // åtte,  'å'
  CapiOnes[8][0] = (char)SCAND_AA_CAPITAL;        // Åtte,  'Å'

  /* The longest symbol
   *  Hours:            xx: ?? 
   *  Minutes, seconds: 37: Thirty-seven
   *  Longest symbol is 5+1+7+1+7 = 21 letters long, so it doesn't fit a single line on a 20 line LCD
   */
 
  //  get local time
  #ifdef FEATURE_DATE_PER_SECOND                                 // for stepping date quickly and check calender function
    localTime = now() + utcOffset * 60 + dateIteration * 3600; //86400;  // fake local time by stepping up to 1 sec/day
    dateIteration = dateIteration + 1;
  #else 
    localTime = now() + utcOffset * 60;  // the default!
  #endif

  Hour = hour(localTime);
  Minute = minute(localTime);
  Seconds = second(localTime);

  lcd.setCursor(0, 0); 
  if (Hour < 10) 
  {
    lcd.print(CapiOnes[int(Hour)]); lcd.print(F("      "));
  }
  else if (Hour > 10 && Hour < 20) lcd.print(Teens[int(Hour)-10]);
  else
  {
    ones = Hour % 10; tens = (Hour - ones) / 10;
    lcd.print(WordTens[tens]); 
  if (tens == 0) 
    {
      lcd.print(" ");
      lcd.print(WordOnes[ones]);
    }
    else if (ones == 0) lcd.print(F("      "));
    else lcd.print(WordOnes[ones]);
  }
  
  lcd.setCursor(4,1); 
  if (Minute > 10 && Minute < 20) lcd.print(Teens[int(Minute)-10]);
  else
  {
    ones = Minute % 10; tens = (Minute - ones) / 10;
    lcd.print(WordTens[tens]); 
   if (tens == 0) 
    {
      lcd.print(" ");
      lcd.print(WordOnes[ones]);
    }
    else if (ones == 0) lcd.print(F("      "));
    else lcd.print(WordOnes[ones]);
  }
   
  lcd.setCursor(8, 2); 
  if (Seconds > 10 && Seconds < 20) lcd.print(Teens[int(Seconds)-10]);
  else
  {
    ones = Seconds % 10; tens = (Seconds - ones) / 10;
    lcd.print(WordTens[tens]);
    if (tens == 0) 
    {
      lcd.print(" ");
      lcd.print(WordOnes[ones]);
    }
    else if (ones == 0) lcd.print(F("      "));
    else lcd.print(WordOnes[ones]);
  }
  
   lcd.setCursor(0, 3);lcd.print(F("        "));
   lcd.setCursor(18, 3); lcd.print(F("  ")); // blank out number in lower right-hand corner 
}

// THE END /////
