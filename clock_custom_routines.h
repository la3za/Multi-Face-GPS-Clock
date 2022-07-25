/*
userNames
nativeDayLong
TimeZones
WordClockNorwegian
 */

// user defined characters, #1-4 are already used in main code (for arrows), so numbers start with #5:
#define NO_DK_oe_SMALL   5
#define SE_DE_oe_SMALL   239 
#define SCAND_aa_SMALL   6
#define SCAND_AA_CAPITAL 7
#define ES_IS_a_ACCENT   6
#define ES_e_ACCENT      5
#define IS_eth_SMALL     7
#define IS_THORN_CAPITAL 8

// character definitions are in clock_native.h

// The user may wish to customize the routines in this file according to 
// desired language for day names in local time display



/////////////////////////////////////////////////////////////////////////
// Native language support for names of days of week when local time is displayed:
// additional customizable characters for LCD in clock_native.h
// native weekday names are defined in function nativeDayLong()
//
// if FEATURE_NATIVE_LANGUAGE is set, day names from nativeDayLong() are used

void nativeDayLong(float j) {
 //  Full weekday name in native (= non-English language)
 //  Max 11 (12) characters in name

  #if FEATURE_NATIVE_LANGUAGE == 'no' | FEATURE_NATIVE_LANGUAGE == 'dk'                    
          // Norwegian, no - Danish, dk:
          char* myDays[] = {"SXndag", "Mandag", "Tirsdag", "Onsdag", "Torsdag", "Fredag", "LXrdag"}; // 7
          myDays[0][1] = char(NO_DK_oe_SMALL); // søndag
          myDays[6][1] = char(NO_DK_oe_SMALL); // lørdag
          
  #elif FEATURE_NATIVE_LANGUAGE == 'se'                    
          // Swedish, se:
          char* myDays[] = {"SXndag", "MXndag", "Tisdag", "Onsdag", "Torsdag", "Fredag", "LXrdag"}; // 7
          myDays[0][1] = char(SE_DE_oe_SMALL); // söndag
          myDays[6][1] = char(SE_DE_oe_SMALL); // lördag
          myDays[1][1] = char(SCAND_aa_SMALL); // måndag

  #elif FEATURE_NATIVE_LANGUAGE == 'is'   
        // Icelandic, is. Longest day is "Miðvikudagur" - 12 characters. There is space, but it looks better if truncated to "Miðvikudag"
 //         char* myDays[] = {"Sunnudagur", "MXnudagur", "XriXjudagur", "MiXvikudagur", "Fimmtudagur", "FXstudagur", "Laugardagur"}; // 12
          char* myDays[] = {"Sunnudagur", "MXnudagur", "XriXjudagur", "MiXvikudag", "Fimmtudagur", "FXstudagur", "Laugardagur"}; // 11
          myDays[1][1] = char(ES_IS_a_ACCENT); // mánudagur
          myDays[5][1] = char(SE_DE_oe_SMALL); // föstudagu
          myDays[2][3] = char(IS_eth_SMALL); // þriðjudagur
          myDays[3][2] = char(IS_eth_SMALL); // miðvikudagur
          myDays[2][0] = char(IS_THORN_CAPITAL); // þriðjudagur
          
          
  #elif FEATURE_NATIVE_LANGUAGE == 'de' 
          // German, de:
          char* myDays[] = {"Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"};  // 10

  #elif FEATURE_NATIVE_LANGUAGE == 'fr'  
          // French, fr:
          char* myDays[] = {"Dimanche", "Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi"}; // 8
          
  #elif FEATURE_NATIVE_LANGUAGE == 'es' 
          // Spanish, es:
          char* myDays[] = {"Domingo", "Lunes", "Martes", "MiXrcoles", "Jueve", "Viernes", "SXbado"};  // 9 
          myDays[3][2] = char(ES_e_ACCENT); // miércoles
          myDays[6][1] = char(ES_IS_a_ACCENT); // sábado
                    
  #else
          // English (not used, included only in order that myDays always will be defined
          char* myDays[] = {"Sunday", "Monday", "Tueday", "Wednesday", "Thursday", "Friday", "Saturday"};
  #endif    
							
      int addr = weekday(j) - 1;

#ifdef FEATURE_DAY_PER_SECOND
//    fake the day -- for testing only
          addr = second(local/2)%7; // change every second
#endif

strncpy(today, myDays[addr], 12);
}

 // Menu item ///////////////////////////////////////////////////////////////////////////////////////////

void TimeZones() { // local time, UTC, + many more time zones
  
  TimeChangeRule *tcr1, *tcr2, *tcr3, *tcr4;        //pointer to local time change rules, use to get TZ abbrev
  time_t local1, local2, local3, local4;

  char textBuffer[9];
  // get global local time

  local = now() + utcOffset * 60;
  Hour = hour(local);
  Minute = minute(local);
  Seconds = second(local);

  lcd.setCursor(0, 0); // 1. line *********
  sprintf(textBuffer, "%02d%c%02d", Hour, HOUR_SEP, Minute);
  lcd.print(textBuffer);
  lcd.print(" ");lcd.print(tcr -> abbrev);
  
  lcd.setCursor(18,0); // end of line 1
  sprintf(textBuffer, "%02d", Seconds);
  lcd.print(textBuffer);
  
   
  lcd.setCursor(0, 1); // 2. line *********

  if (gps.time.isValid()) {
    lcd.setCursor(0, 1);
    sprintf(textBuffer, "%02d%c%02d UTC  ", hourGPS, HOUR_SEP,minuteGPS);
    lcd.print(textBuffer);
    //   lcd.setCursor(14, 3);
  }

  lcd.setCursor(0, 2); // ******** 3. line 

// Indian Standard Time
  TimeChangeRule inIST = {"IND", Second, Sun, Mar, 2, 330};  // Indian Standard Time = UTC - 5 hours 30 min

  Timezone India(inIST, inIST);
  local2 = India.toLocal(utc, &tcr2);
  Hour = hour(local2);
  Minute = minute(local2);
  sprintf(textBuffer, "%02d%c%02d ", Hour, HOUR_SEP, Minute);
  lcd.print(textBuffer);
  lcd.print(tcr2 -> abbrev);

//                     China Standard Time
   lcd.setCursor(10, 2);
   TimeChangeRule CN = {"CHN", Second, Sun, Mar, 2, 480};  // China  Time = UTC + 8 hours
   Timezone China(CN, CN);
   local3 = China.toLocal(utc, &tcr3);
   Hour = hour(local3);
   Minute = minute(local3);
   sprintf(textBuffer, "%02d%c%02d ", Hour, HOUR_SEP, Minute);
   lcd.print(textBuffer);
   lcd.print(tcr3 -> abbrev);

   // Turkey  Time
//                      lcd.setCursor(10, 2);
//                      TimeChangeRule TT = {"TUR", Second, Sun, Mar, 2, 180};  // Turkey  Time = UTC + 3 hours
//                      Timezone Turkey(TT, TT);
//                      local3 = Turkey.toLocal(utc, &tcr3);
//                      Hour = hour(local3);
//                      Minute = minute(local3);
//                      sprintf(textBuffer, "%02d%c%02d ", Hour, HOUR_SEP, Minute);
//                      lcd.print(textBuffer);
//                      lcd.print(tcr3 -> abbrev);
  
  lcd.setCursor(0, 3); //////// line 4
   
  // US Eastern Time Zone (New York, Detroit)
  TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, -240};  // Eastern Daylight Time = UTC - 4 hours
  TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, -300};   // Eastern Standard Time = UTC - 5 hours
  Timezone usET(usEDT, usEST);
  local1 = usET.toLocal(utc, &tcr1);
  Hour = hour(local1);
  Minute = minute(local1);
  sprintf(textBuffer, "%02d%c%02d ", Hour, HOUR_SEP, Minute);
  lcd.print(textBuffer);
  lcd.print(tcr1 -> abbrev);

lcd.setCursor(19, 3); lcd.print(" "); // blank out rest of menu number in lower right-hand corner

  // Pacific US 
  lcd.setCursor(10, 3);
  TimeChangeRule usPDT = {"PDT", Second, Sun, Mar, 2, -420};
  TimeChangeRule usPST = {"PST", First, Sun, Nov, 2, -480};
  Timezone usPacific(usPDT, usPST);
  local4 = usPacific.toLocal(utc, &tcr4);
  Hour = hour(local4);
  Minute = minute(local4);
  sprintf(textBuffer, "%02d%c%02d ", Hour, HOUR_SEP, Minute);
  lcd.print(textBuffer);
  lcd.print(tcr4 -> abbrev);
       
  oldMinute = minuteGPS;
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
  local = now() + utcOffset * 60;
  Hour = hour(local);
  Minute = minute(local);
  Seconds = second(local);

  lcd.setCursor(0, 0); 
  if (Hour < 10) 
  {
    lcd.print(CapiOnes[int(Hour)]); lcd.print("      ");
  }
  else if (Hour > 10 & Hour < 20) lcd.print(Teens[int(Hour)-10]);
  else
  {
    ones = Hour % 10; tens = (Hour - ones) / 10;
    lcd.print(WordTens[tens]); 
  if (tens == 0) 
    {
      lcd.print(" ");
      lcd.print(WordOnes[ones]);
    }
    else if (ones == 0) lcd.print("      ");
    else lcd.print(WordOnes[ones]);
  }
  
  lcd.setCursor(4,1); 
  if (Minute > 10 & Minute < 20) lcd.print(Teens[int(Minute)-10]);
  else
  {
    ones = Minute % 10; tens = (Minute - ones) / 10;
    lcd.print(WordTens[tens]); 
   if (tens == 0) 
    {
      lcd.print(" ");
      lcd.print(WordOnes[ones]);
    }
    else if (ones == 0) lcd.print("      ");
    else lcd.print(WordOnes[ones]);
  }
  
  
  lcd.setCursor(8, 2); 
  if (Seconds > 10 & Seconds < 20) lcd.print(Teens[int(Seconds)-10]);
  else
  {
    ones = Seconds % 10; tens = (Seconds - ones) / 10;
    lcd.print(WordTens[tens]);
    if (tens == 0) 
    {
      lcd.print(" ");
      lcd.print(WordOnes[ones]);
    }
    else if (ones == 0) lcd.print("      ");
    else lcd.print(WordOnes[ones]);
  }
  
   lcd.setCursor(0, 3);lcd.print("        ");
   lcd.setCursor(18, 3); lcd.print("  "); // blank out number in lower right-hand corner 
}


// THE END /////
