/*
 * 
nativeDay
nativeDayLong
TimeZones
WordClockNorwegian

 */

// user defined characters 1-4 used in main code:
#define NO_DK_OE_small   5
#define SCAND_AA_small   6
#define SCAND_AA_capital 7

// The user may wish to customize the routines in this file according to 
// desired language for day names in local time display

/////////////////////////////////////////////////////////////////////////
// Native language support for names of days of week when local time is displayed:
// additional characters for Norwegian defined in clock_Norwegian.h
// native weekday names are defined in functions nativeDay(), nativeDayLong()
//
// if FEATURE_NATIVE_LANGUAGE is set, day names from antiveDay() and nativeDayLong() are used

/*
 From here on follows routines that the user may want to configure
  */
 
/////////// Change for native language day names: ////////////////////////////

                    void nativeDay(float j) {
                    /*
                       Print abbreviated weekday in native (= non-English language)
                    */
                    const char* myDays[] = {"Son", "Man", "Tir", "Ons", "Tor", "Fre", "Lor"};
                    int addr = weekday(j) - 1;
                    strncpy(today, myDays[addr], 4);

                    if (addr == 0 | addr == 6) { // søndag, lørdag
                      lcd.print(today[0]);
                      lcd.write(NO_DK_OE_small); // ø
                      lcd.print(today[2]);
                    }
                    else  lcd.print(today);

#ifdef FEATURE_SERIAL_MENU
                      Serial.println("nativeDay: ");
                      Serial.println("  today");
                      Serial.println(today);
                  #endif

                    lcd.print(" ");
                  }

/////////// Change for native language day names ////////////////////////////

                  void nativeDayLong(float j) {
                    /*
                       Print full weekday in native (= non-English language)
                    */
                    const char* myDays[] = {"Sondag ", "Mandag ", "Tirsdag", "Onsdag ", "Torsdag", "Fredag ", "Lordag "};
                    int addr = weekday(j) - 1;
                    if (addr == 2 | addr == 4) {
                      strncpy(today, myDays[addr], 8);
                    }
                    else {
                      strncpy(today, myDays[addr], 7);
                      lcd.print(" ");
                    }

                    if (addr == 0 | addr == 6) { // søndag, lørdag
                      lcd.print(today[0]);
                      lcd.write(NO_DK_OE_small); // ø
                      lcd.print(&today[2]);
                    }
                    else  lcd.print(today);

                    lcd.print(" ");
                  }
 // Menu item ///////////////////////////////////////////////////////////////////////////////////////////

                    void TimeZones() { // local time, UTC, + many more time zones
                      
                      TimeChangeRule *tcr1, *tcr2, *tcr3, *tcr4;        //pointer to local time change rules, use to get TZ abbrev
                      time_t local1, local2, local3, local4;
                  
                      char textbuffer[9];
                      // get global local time
                  
                      local = now() + UTCoffset * 60;
                      Hour = hour(local);
                      Minute = minute(local);
                      Seconds = second(local);
                  
                      lcd.setCursor(0, 0); // 1. line *********
                      sprintf(textbuffer, "%02d%c%02d", Hour, HOUR_SEP, Minute);
                      lcd.print(textbuffer);
                      lcd.print(" ");lcd.print(tcr -> abbrev);
                      
                      lcd.setCursor(18,0); // end of line 1
                      sprintf(textbuffer, "%02d", Seconds);
                      lcd.print(textbuffer);
                      
                       
                      lcd.setCursor(0, 1); // 2. line *********
                  
                      if (gps.time.isValid()) {
                        lcd.setCursor(0, 1);
                        sprintf(textbuffer, "%02d%c%02d UTC  ", hourGPS, HOUR_SEP,minuteGPS);
                        lcd.print(textbuffer);
                        //   lcd.setCursor(14, 3);
                      }
                  
                      lcd.setCursor(0, 2); // ******** 3. line 
                  
                   // Indian Standard Time
                      TimeChangeRule inIST = {"IND", Second, Sun, Mar, 2, 330};  // Indian Standard Time = UTC - 5 hours 30 min
                   
                      Timezone India(inIST, inIST);
                      local2 = India.toLocal(utc, &tcr2);
                      Hour = hour(local2);
                      Minute = minute(local2);
                      sprintf(textbuffer, "%02d%c%02d ", Hour, HOUR_SEP, Minute);
                      lcd.print(textbuffer);
                      lcd.print(tcr2 -> abbrev);

//                     China Standard Time
                       lcd.setCursor(10, 2);
                       TimeChangeRule CN = {"CHN", Second, Sun, Mar, 2, 480};  // China  Time = UTC + 8 hours
                       Timezone China(CN, CN);
                       local3 = China.toLocal(utc, &tcr3);
                       Hour = hour(local3);
                       Minute = minute(local3);
                       sprintf(textbuffer, "%02d%c%02d ", Hour, HOUR_SEP, Minute);
                       lcd.print(textbuffer);
                       lcd.print(tcr3 -> abbrev);
                  
                       // Turkey  Time
//                      lcd.setCursor(10, 2);
//                      TimeChangeRule TT = {"TUR", Second, Sun, Mar, 2, 180};  // Turkey  Time = UTC + 3 hours
//                      Timezone Turkey(TT, TT);
//                      local3 = Turkey.toLocal(utc, &tcr3);
//                      Hour = hour(local3);
//                      Minute = minute(local3);
//                      sprintf(textbuffer, "%02d%c%02d ", Hour, HOUR_SEP, Minute);
//                      lcd.print(textbuffer);
//                      lcd.print(tcr3 -> abbrev);
                      
                      lcd.setCursor(0, 3); //////// line 4
                       
                      // US Eastern Time Zone (New York, Detroit)
                      TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, -240};  // Eastern Daylight Time = UTC - 4 hours
                      TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, -300};   // Eastern Standard Time = UTC - 5 hours
                      Timezone usET(usEDT, usEST);
                      local1 = usET.toLocal(utc, &tcr1);
                      Hour = hour(local1);
                      Minute = minute(local1);
                      sprintf(textbuffer, "%02d%c%02d ", Hour, HOUR_SEP, Minute);
                      lcd.print(textbuffer);
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
                      sprintf(textbuffer, "%02d%c%02d ", Hour, HOUR_SEP, Minute);
                      lcd.print(textbuffer);
                      lcd.print(tcr4 -> abbrev);
                           
                      oldminute = minuteGPS;
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
  WordTens[4][2] = (char)NO_DK_OE_small;
  WordOnes[8][0] = (char)SCAND_AA_small;
  CapiOnes[8][0] = (char)SCAND_AA_capital;

  /* The longest symbol
   *  Hours:            xx: ?? 
   *  Minutes, seconds: 37: Thirty-seven
   *  Longest symbol is 5+1+7+1+7 = 21 letters long, so it doesn't fit a single line on a 20 line LCD
   */
 
  //  get local time
  local = now() + UTCoffset * 60;
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
