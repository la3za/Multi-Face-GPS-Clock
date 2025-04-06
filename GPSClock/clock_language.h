// Routines that define letters and language dependent routines

/*
dayName
nativeDayLong
loadNativeCharacters
WordClockNorwegian
 */

// user defined characters, #1-4 are already used in main code (for arrows etc), so numbers start with 5:

#define SV_DE_oe_SMALL   239  // ö, already exists in LCD memory

// The following def's must match those of LCDchar0_3 etc in *.ino file:
#define IS_eth_SMALL     4    // ð 
#define NO_DA_oe_SMALL   5    // ø
#define IS_THORN_CAPITAL 5    // þ  
#define ES_e_ACCENT      5    // é  
#define ES_IS_a_ACCENT   6    // á

#define SCAND_aa_SMALL   6    // å
#define SCAND_AA_CAPITAL 7    // Å

#define NORSE_o_ACCENT   7    // ó, 10.10.2024, also Faroese
//#define NORSE_O_ACCENT   7    // Ó, 13.10.2024

#define FO_i_ACCENT      4    // í, 29.10.2024
#define FO_y_ACCENT      5    // ý, 29.10.2024

#define AM_PM            7    // for 12 hr clock 31.03.2025 


/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void dayName(int dayAddr)   // return day name in correct language in "today", given dayAddr=1...7 (Sunday ... Saturday)
{
    memcpy_P(today,myDays[languageNumber][dayAddr], 12);    // put today's day / language into "today"
 
// fix odd letters requiring special characters depending on language chosen:

    if (strcmp(languages[languageNumber],"nb ") == 0 || strcmp(languages[languageNumber],"da ") == 0
     || strcmp(languages[languageNumber],"nn ") == 0)  
      if (dayAddr==0) today[1] = char(NO_DA_oe_SMALL); // Søndag - Norwegian, Danish, Nynorsk

    if (strcmp(languages[languageNumber],"nb ") == 0 || strcmp(languages[languageNumber],"da ") == 0)
      if (dayAddr==6) today[1] = char(NO_DA_oe_SMALL); // Lørdag - Norwegian, Danish

    if (strcmp(languages[languageNumber],"es ") == 0)
    {
      if (dayAddr==3) today[2] = char(ES_e_ACCENT);    // Miércoles - Spanish
      if (dayAddr==6) today[1] = char(ES_IS_a_ACCENT); // Sábado - Spanish
    }

    if (strcmp(languages[languageNumber],"sv ") == 0)
      if (dayAddr==0 || dayAddr==6) today[1] = char(SV_DE_oe_SMALL); // Söndag, Lördag - Swedish

    if (strcmp(languages[languageNumber],"sv ") == 0 || strcmp(languages[languageNumber],"nn ") == 0)
      if (dayAddr==1)               today[1] = char(SCAND_aa_SMALL); // Måndag - Swedish, Nynorsk

    if (strcmp(languages[languageNumber],"is ") == 0)
    {
      if (dayAddr==1) today[1] = char(ES_IS_a_ACCENT);   // Mánudagur - Icelandic
      if (dayAddr==2)                                    // þriðjudagur - Icelandic
      {
          today[0] = char(IS_THORN_CAPITAL); 
          today[3] = char(IS_eth_SMALL);     
      }
      if (dayAddr==3) today[2] = char(IS_eth_SMALL);     // Miðvikudagur - Icelandic
      if (dayAddr==5) today[1] = char(SV_DE_oe_SMALL);   // Föstudagu - Icelandic
    }

    if (strcmp(languages[languageNumber],"non") == 0) // Old Norse, 10.10.2024
    {
      if (dayAddr==1) today[1] = char(ES_IS_a_ACCENT);   // Mánudagr - Old Norse 
      if (dayAddr==3) 
      {
       // today[0] = char(NORSE_O_ACCENT);   // Óðinsdagr - If not capital Ó, then O or ó must do
        today[1] = char(IS_eth_SMALL);     // Óðinsdagr - Old Norse 
      }
      if (dayAddr==4)                         // Þórsdagr - Old Norse 
      {
        today[0] = char(IS_THORN_CAPITAL); 
        today[1] = char(NORSE_o_ACCENT); 
      }
      if (dayAddr==5) today[3] = char(ES_IS_a_ACCENT);   // Frjádagr - Old Norse 
    }

    if (strcmp(languages[languageNumber],"fo ") == 0) // Faroese, 29.10.2024
    {
      if (dayAddr==1) today[1] = char(ES_IS_a_ACCENT);  // Mánudagur - Faroese
      if (dayAddr==2) today[1] = char(FO_y_ACCENT);     // Týsdagur - Faroese
      if (dayAddr==4) today[1] = char(NORSE_o_ACCENT);  // Hósdagur - Faroese
      if (dayAddr==5) today[2] = char(FO_i_ACCENT);     // Fríggjadagur - Faroese
    }
}

/////////////////////////////////////////////////////////////////////////

void nativeDayLong(time_t unixTime) {  // unix time in sec, output through global char array today. Was float, now time_t. Fix 12.10.2024
 //  Full weekday name in native (= non-English language)
 //  Max 12 characters in name
 //  

int dayAddr = weekday(unixTime) - 1; // weekday() = day of the week (1-7), Sunday is day 1

#ifdef FEATURE_DAY_PER_SECOND     //    fake the day -- for testing only
          dayAddr = second(localTime/2)%7; // change every 2 seconds
#endif

dayName(dayAddr);
}

///////////////////////////////////////////////

const byte OE_small[8]   PROGMEM = {B00000, B00001, B01110, B10101, B10101, B01110, B10000, B00000}; // ø, for søndag, lørdag
const byte a_accent[8]   PROGMEM = {B00010, B00100, B01110, B00001, B01111, B10001, B01111, B00000};
const byte o_accent[8]   PROGMEM = {B00010, B00100, B01110, B10001, B10001, B10001, B01110, B00000}; // ó, old norse, 10.10.2024
const byte O_accent[8]   PROGMEM = {B00010, B01110, B10001, B10001, B10001, B10001, B01110, B00000}; // Ó, old norse, 10.10.2024
const byte e_accent[8]   PROGMEM = {B00010, B00100, B01110, B10001, B11111, B10000, B01110, B00000};
//const byte AA_capital[8] PROGMEM = {B00100, B00000, B01110, B10001, B11111, B10001, B10001, B00000}; // not so nice-looking
const byte AA_capital[8] PROGMEM = {B00100, B01010, B00100, B01010, B10001, B11111, B10001, B00000};  // Å, like European/English LCD 9.4.2024

const byte AA_small[8]   PROGMEM = {B00100, B00000, B01110, B00001, B01111, B10001, B01111, B00000}; // Scandinavian å, small ring
//const byte AA_small[8]   PROGMEM = {B00100, B01010, B00100, B01110, B00001, B01111, B10001, B01111}; // Scandinavian å, too low, like European/English LCD
const byte Thorn[8]      PROGMEM = {B01000, B01110, B01001, B01001, B01110, B01000, B01000, B00000}; // Icelandic
const byte eth[8]        PROGMEM = {B01000, B00100, B01110, B10001, B10001, B10001, B01110, B00000}; // Icelandic
const byte y_accent[8]   PROGMEM = {B00010, B00100, B10001, B10001, B01111, B00001, B01110, B00000}; // Faroese 
const byte i_accent[8]   PROGMEM = {B00100, B01000, B01100, B00100, B00100, B00100, B01110, B00000}; // Faroese Fríggjadagur
//const byte Cedila[8]     PROGMEM = {B00000, B01110, B10000, B10001, B01110, B00100, B01100, B00000}; // for Portugese

//const byte am[8]         PROGMEM = {B01100, B10010, B11110, B10010, B00000, B11011, B10101, B10001};  // AM for 12 hr clock in a single one character
//const byte pm[8]         PROGMEM = {B11100, B10010, B11100, B10000, B00000, B11011, B10101, B10001};  // PM for 12 hr clock in a single character

const byte am[8]         PROGMEM = {B00110, B01001, B01111, B01001, B00000, B11011, B10101, B10001};  // AM for 12 hr clock in a single one character
const byte pm[8]         PROGMEM = {B01110, B01001, B01110, B01000, B00000, B11011, B10101, B10001};  // PM for 12 hr clock in a single character
///////////////////////////////////////////////

void loadNativeCharacters(int8_t languageNumber)
{
//  LCD display definitions of special native characters for languages
//  03.03.2024: Using PROGMEM, only loads the chosen language's character set, no longer for a group of languages

//  Norwegian/Danish letters are also used in WordClock, ø: "lørdag, søndag", and Chemical Elements "sølv"
//  https://forum.arduino.cc/t/error-lcd-16x2/211977/6

 if (LCDchar4_5 != LCDNATIVE  || languageNumber != presentLanguageNumber)  //
 {
    if (strcmp(languages[languageNumber],"nb ") == 0 || strcmp(languages[languageNumber],"da ") == 0 
                                                     || strcmp(languages[languageNumber],"nn ") == 0)
    {
        memcpy_P(buffer,OE_small, 8);
        lcd.createChar(NO_DA_oe_SMALL, buffer);       // ø: "Lørdag", "Søndag"   
        LCDchar4_5 = LCDNATIVE;
        presentLanguageNumber = languageNumber;
        #ifdef FEATURE_SERIAL_LOAD_CHARACTERS
              Serial.print(F("loadNativeCharacters: ")); Serial.print(languageNumber); 
              Serial.print(" ");Serial.println(languages[languageNumber]);
        #endif
    }
 }

 if (LCDchar4_5 != LCDNATIVE || LCDchar6_7 != LCDNATIVE || languageNumber != presentLanguageNumber)  // LCDchar6_7 also 5.11.2024
 {

    if (strcmp(languages[languageNumber],"es ") == 0 || strcmp(languages[languageNumber],"is ") == 0 
     || strcmp(languages[languageNumber],"non") == 0 || strcmp(languages[languageNumber],"fo ") == 0)
    {
        memcpy_P(buffer,a_accent, 8);
        lcd.createChar(ES_IS_a_ACCENT, buffer);       // á: "Sábado"
        LCDchar6_7 = LCDNATIVE; // since it fills more than locations 4 and 5 - 30.11.2024
        #ifdef FEATURE_SERIAL_LOAD_CHARACTERS
          Serial.print(F("loadNativeCharacters: ")); Serial.print(languageNumber); 
          Serial.print(" ");Serial.println(languages[languageNumber]);
        #endif 
    }

    if (strcmp(languages[languageNumber],"sv ") == 0 || strcmp(languages[languageNumber],"nn ") == 0)
    {
        // ö exists as char(B11101111) = char(239), no need to create it separately, ä, ü, ñ also
        memcpy_P(buffer,AA_small, 8);
        lcd.createChar(SCAND_aa_SMALL, buffer);       //  å: "måndag", Swedish/nynorsk
        LCDchar6_7 = LCDNATIVE; // since it fills more than locations 4 and 5 - 30.11.2024
        #ifdef FEATURE_SERIAL_LOAD_CHARACTERS
          Serial.print(F("loadNativeCharacters: ")); Serial.print(languageNumber); 
          Serial.print(" ");Serial.println(languages[languageNumber]);
        #endif         
    }

    if (strcmp(languages[languageNumber],"es ") == 0)
    {
        memcpy_P(buffer,e_accent, 8);                 // é, "Miércoles"
        lcd.createChar(ES_e_ACCENT, buffer);
        #ifdef FEATURE_SERIAL_LOAD_CHARACTERS
          Serial.print(F("loadNativeCharacters: ")); Serial.print(languageNumber); 
          Serial.print(" ");Serial.println(languages[languageNumber]);
        #endif         
    }

    if (strcmp(languages[languageNumber],"is ") == 0 || strcmp(languages[languageNumber],"non") == 0)
    {
    //  Icelandic, á, ð, Þ, also Old Norse
    //  https://einhugur.com/blog/index.php/xojo-gpio/hd44780-based-lcd-display/
        memcpy_P(buffer,Thorn, 8);
        lcd.createChar(IS_THORN_CAPITAL, buffer);
        memcpy_P(buffer,eth, 8);
        lcd.createChar(IS_eth_SMALL, buffer);
  //      LCDchar6_7 = LCDNATIVE; // since it fills more than locations 4 and 5
        #ifdef FEATURE_SERIAL_LOAD_CHARACTERS
          Serial.print(F("loadNativeCharacters: ")); Serial.print(languageNumber); 
          Serial.print(" ");Serial.println(languages[languageNumber]);
        #endif 
    }
    if (strcmp(languages[languageNumber],"non") == 0 || strcmp(languages[languageNumber],"fo ") == 0)  // Old Norse, 10.10.2024; Faroese 29.10.2024
    {   
        //if (day == Wednesday ... hard to find a suitable variable for weekday which also knows if it is local time or UTC
        //     memcpy_P(buffer,O_accent, 8);
        memcpy_P(buffer,o_accent, 8);
        lcd.createChar(NORSE_o_ACCENT, buffer);
        LCDchar6_7 = LCDNATIVE; // since it fills more than locations 4 and 5 - 30.11.2024
        #ifdef FEATURE_SERIAL_LOAD_CHARACTERS
          Serial.print(F("loadNativeCharacters: ")); Serial.print(languageNumber); 
          Serial.print(" ");Serial.println(languages[languageNumber]);
        #endif 
    }   

    if (strcmp(languages[languageNumber],"fo ") == 0)  // Faroese 29.10.2024 
    {
        memcpy_P(buffer,y_accent, 8);
        lcd.createChar(FO_y_ACCENT, buffer);

        memcpy_P(buffer,i_accent, 8);
        lcd.createChar(FO_i_ACCENT, buffer);
       // LCDchar6_7 = LCDNATIVE; // since it fills more than locations 4 and 5  
       #ifdef FEATURE_SERIAL_LOAD_CHARACTERS
          Serial.print(F("loadNativeCharacters: ")); Serial.print(languageNumber); 
          Serial.print(" ");Serial.println(languages[languageNumber]);
       #endif 
    }

    LCDchar4_5 = LCDNATIVE;
    presentLanguageNumber = languageNumber;

 }
}

//////////////////////////

#ifdef MORELANGUAGES // unfinished ... dayAddr not declared
void loadNorseO(int8_t languageNumber)      // make sure correct Ó (Ódinsdagr) and ó (Thórsdagr) is loaded depending on day
{
    int dayAddr; // faked in order for code to compile - needs to come from outside
    // special case for Old Norse as not enough room for both Ó (Ódinsdagr) and ó (Thórsdagr))
        if (strcmp(languages[languageNumber],"non") == 0)
          //      Serial.println(dayAddr);
        {
        // if (weekday(localTime) == 4)        // Wednesday: Ó 
        if (dayAddr == 3)        // Wednesday: Ó 
            {
              //  Serial.println("Wednesday OOOO");
                // memcpy_P(buffer, O_accent, 8);
                // lcd.createChar(NORSE_O_ACCENT, buffer);
            }
        // else if (weekday(localTime) == 5)  // Thursday: ó 
        else if (dayAddr == 4)  // Thursday: ó 
            {
              //  Serial.println("Thursday  oooo");
                memcpy_P(buffer, o_accent, 8);
                lcd.createChar(NORSE_o_ACCENT, buffer);
            }
        }
    }
#endif
   

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
 // char textbuf[21];

// replace some characters - the X's - with native Norwegian ones:
  WordTens[4][2] = (char)NO_DA_oe_SMALL;          // Førti, 'ø' also used in "lørdag", "søndag"
  WordOnes[8][0] = (char)SCAND_aa_SMALL;          // åtte,  'å'
  CapiOnes[8][0] = (char)SCAND_AA_CAPITAL;        // Åtte,  'Å'

  /* The longest symbol
   *  Hours:            xx: ?? 
   *  Minutes, seconds: 37: Thirty-seven
   *  Longest symbol is 5+1+7+1+7 = 21 letters long, so it doesn't fit a single line on a 20 line LCD
   */
 
  //  get local time
  #ifdef FEATURE_DATE_PER_SECOND                                 // for stepping date quickly and check calender function
    localTime = now() + utcOffset * 60 + dateIteration * SPEED_UP_FACTOR;  // fake local time by stepping up to 1 sec/day
    dateIteration = dateIteration + 1;
  #else 
    localTime = now() + utcOffset * 60;  // the default!
  #endif
  if (Twelve24Local > 12) Hour = hour(localTime);
  else                    Hour = hourFormat12(localTime);
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
   
  if (Twelve24Local <= 12) {
    lcd.setCursor(18, 0);
    if (isAM(localTime))      lcd.print(F("AM"));
    else if (isPM(localTime)) lcd.print(F("PM"));
  }
  lcd.setCursor(18, 3); lcd.print(F("  ")); // blank out number in lower right-hand corner 
}

// THE END /////
