// Routines that define letters and language dependent routines

/*
nativeDayLong
WordClockNorwegian
 */

// user defined characters, #1-4 are already used in main code (for arrows etc), so numbers start with 5:
#define NO_DK_oe_SMALL   5    // ø
#define SE_DE_oe_SMALL   239  // ö, already exists in LCD memory

// 5.5.2023 changed storage cell numbers
#define SCAND_AA_CAPITAL 6    // Å
#define IS_eth_SMALL     6    // ð 
#define SCAND_aa_SMALL   7    // å
#define ES_IS_a_ACCENT   7    // á
#define ES_e_ACCENT      4    // é   // 26.12.2023: from 8 to 4 as 8 is invalid address
#define IS_THORN_CAPITAL 4    // þ   // 26.12.2023: from 8 to 4 as 8 is invalid address

// Language is selected according to variable in clock_options.h
// character definitions are in clock_native.h

/////////////////////////////////////////////////////////////////////////
// Native language support for names of days of week when local time is displayed:
// additional customizable characters for LCD in clock_native.h
// native weekday names are defined in function nativeDayLong()
// not enough customable characters to support all languages in one group, therefore divided
//


//#if FEATURE_LANGUAGE_GROUP2 == no_es
#if FEATURE_LANGUAGE_GROUP == 0                   
    char languages[][4] = {"fr", "de", "no", "es"};  // my default
//#elif FEATURE_LANGUAGE_GROUP2 == no_se 
#elif FEATURE_LANGUAGE_GROUP == 1
    char languages[][4] = {"fr", "de", "no", "se"};
//#elif FEATURE_LANGUAGE_GROUP2 == dk_se_es
#elif FEATURE_LANGUAGE_GROUP == 2
    char languages[][5] = {"fr", "de", "dk", "se", "es"};
//#elif FEATURE_LANGUAGE_GROUP2 == dk_is
#elif FEATURE_LANGUAGE_GROUP == 3
    char languages[][4]=  {"fr", "de" , "dk", "is"};
#endif


void nativeDayLong(float j) {      // output through global char array today
 //  Full weekday name in native (= non-English language)
 //  Max 11 (12) characters in name

int addr = weekday(j) - 1;

#ifdef FEATURE_DAY_PER_SECOND     //    fake the day -- for testing only
          addr = second(localTime/2)%7; // change every 2 seconds
#endif

  const uint8_t numCol = 7;

// myDays[language][day][character]:
   char myDays[][numCol][11] = {
            {"dimanche", "lundi", "mardi", "mercredi", "jeudi", "vendredi", "samedi"},          // French, fr
            {"Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"},  // German, de  
            {"SXndag", "Mandag", "Tirsdag", "Onsdag", "Torsdag", "Fredag", "LXrdag"},           // Norwegian/Danish, no/dk                  
//  #if   (FEATURE_LANGUAGE_GROUP == 'no_es') 
    #if FEATURE_LANGUAGE_GROUP == 0
            {"Domingo", "Lunes", "Martes", "MiXrcoles", "Jueve", "Viernes", "SXbado"}};         // Spanish, es
//  #elif (FEATURE_LANGUAGE_GROUP == 'no_se')
    #elif FEATURE_LANGUAGE_GROUP == 1
            {"SYndag", "MYndag", "Tisdag", "Onsdag", "Torsdag", "Fredag", "LYrdag"}};           // Swedish, se
//  #elif (FEATURE_LANGUAGE_GROUP == 'dk_se_es')
    #elif FEATURE_LANGUAGE_GROUP == 2
            {"SYndag", "MYndag", "Tisdag", "Onsdag", "Torsdag", "Fredag", "LYrdag"},            // Swedish, se
            {"Domingo", "Lunes", "Martes", "MiXrcoles", "Jueve", "Viernes", "SXbado"}};         // Spanish, es
//  #elif (FEATURE_LANGUAGE_GROUP == 'dk_is') 
    #elif FEATURE_LANGUAGE_GROUP == 3     
            {"Sunnudagur", "MXnudagur", "XriXjudagur", "MiXvikudag", "Fimmtudagur", "FXstudagur", "Laugardagur"}}; // Icelandic, is
    #endif

// Native letters [language][day][character]. no=dk always:
          myDays[2][0][1] = char(NO_DK_oe_SMALL); // Søndag
          myDays[2][6][1] = char(NO_DK_oe_SMALL); // Lørdag

#if FEATURE_LANGUAGE_GROUP == 1 || FEATURE_LANGUAGE_GROUP == 2
         myDays[3][0][1] = char(SE_DE_oe_SMALL); // Söndag
         myDays[3][6][1] = char(SE_DE_oe_SMALL); // Lördag
         myDays[3][1][1] = char(SCAND_aa_SMALL); // Måndag
#endif 

#if FEATURE_LANGUAGE_GROUP == 0
          myDays[3][3][2] = char(ES_e_ACCENT);    // Miércoles
          myDays[3][6][1] = char(ES_IS_a_ACCENT); // Sábado
#endif 

#if FEATURE_LANGUAGE_GROUP == 2
          myDays[4][3][2] = char(ES_e_ACCENT);    // Miércoles
          myDays[4][6][1] = char(ES_IS_a_ACCENT); // Sábado
#endif 

#if FEATURE_LANGUAGE_GROUP == 3
          myDays[3][1][1] = char(ES_IS_a_ACCENT);   // Mánudagur
          myDays[3][5][1] = char(SE_DE_oe_SMALL);   // Föstudagu
          myDays[3][2][3] = char(IS_eth_SMALL);     // þriðjudagur
          myDays[3][3][2] = char(IS_eth_SMALL);     // Miðvikudagur
          myDays[3][2][0] = char(IS_THORN_CAPITAL); // þriðjudagur
#endif 
          strncpy(today, myDays[languageNumber][addr], 12);
}


///////////////////////////////////////////////

void loadNativeCharacters()
{
//  LCD display definitions of special native characters for languages
// included in setup()

// Norwegian/Danish letters used in native day names and in WordClock, ø: "lørdag, søndag"
//  https://forum.arduino.cc/t/error-lcd-16x2/211977/6
    
    byte OE_small[8] = {B00000, B00001, B01110, B10101, B10101, B01110, B10000, B00000}; // ø, for søndag, lørdag (Sunday, Saturday)
    lcd.createChar(NO_DK_oe_SMALL, OE_small); //   Danish, Norwegian
    
    // ö exists as char(B11101111) = char(239), no need to create it separately, ä, ü, ñ also

#if FEATURE_LANGUAGE_GROUP == 0 || FEATURE_LANGUAGE_GROUP == 2 // es
    byte a_accent[] = {B00010, B00100, B01110, B00001, B01111, B10001, B01111, B00000};
    lcd.createChar(ES_IS_a_ACCENT, a_accent);
    
    byte e_accent[] = {B00010, B00100, B01110, B10001, B11111, B10000, B01110, B00000};
    lcd.createChar(ES_e_ACCENT, e_accent);
#endif

#if FEATURE_LANGUAGE_GROUP == 0 || FEATURE_LANGUAGE_GROUP == 1 || FEATURE_LANGUAGE_GROUP == 2 // no, se
    // å: "åtte"
    byte AA_small[8] = {B00100, B00000, B01110, B00001, B01111, B10001, B01111, B00000}; 
    lcd.createChar(SCAND_aa_SMALL, AA_small); //   Norwegian, Swedish, (Danish)
#endif

#if FEATURE_LANGUAGE_GROUP == 0 || FEATURE_LANGUAGE_GROUP == 1  // no  
    // Å: "Åtte"
    byte AA_capital[8] = {B00100,B00000,B01110,B10001,B11111,B10001,B10001,B00000}; 
    lcd.createChar(SCAND_AA_CAPITAL, AA_capital); //   Norwegian, (Danish, Swedish)
#endif

#if FEATURE_LANGUAGE_GROUP == 3 // 'is'
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
    lcd.clear();  // in order to set the LCD back to the proper memory mode after custom characters have been created
}


///////////////////////////////////////////////

void WordClockNorwegian()
{
  char WordOnes[10][5] = {{"null"}, {"en  "}, {"to  "}, {"tre "}, {"fire"}, {"fem "}, {"seks"}, {"sju "}, {"Xtte"}, {"ni  "}}; // left justified
  char CapiOnes[10][5] = {{"Null"}, {"Ett "}, {"To  "}, {"Tre "}, {"Fire"}, {"Fem "}, {"Seks"}, {"Sju "}, {"Xtte"}, {"Ni  "}}; // left justified
  char WordTens[6][7]  = {{"  Null"}, {"    Ti"}, {"  Tjue"}, {"Tretti"}, {" FXrti"}, {" Femti"}}; 
  char Teens[10][8]    = {{"       "},{"Elleve "},{"Tolv   "},{"Tretten"}, {"Fjorten"}, {"Femten "}, {"Seksten"}, {"Sytten "}, {"Atten  "}, {"Nitten "}};
  int ones, tens;
  char textbuf[21];

// replace some characters - the X's - with native Norwegian ones:
  WordTens[4][2] = (char)NO_DK_oe_SMALL;
  WordOnes[8][0] = (char)SCAND_aa_SMALL;
  CapiOnes[8][0] = (char)SCAND_AA_CAPITAL;

  /* The longest symbol
   *  Hours:            xx: ?? 
   *  Minutes, seconds: 37: Thirty-seven
   *  Longest symbol is 5+1+7+1+7 = 21 letters long, so it doesn't fit a single line on a 20 line LCD
   */
 
  //  get local time
  localTime = now() + utcOffset * 60;
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
   lcd.setCursor(18, 3); lcd.print("  "); // blank out number in lower right-hand corner 
}




// THE END /////
