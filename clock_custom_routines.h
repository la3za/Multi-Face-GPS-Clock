#define NO_DK_OE_small  5

// The user may wish to customize the routines in this file according to 
// desired language for day names in local time display

/////////////////////////////////////////////////////////////////////////
// Native language support for names of days of week when local time is displayed:
// additional characters for Norwegian defined in clock_Norwegian.h
// native weekday names are defined in functions nativeDay(), nativeDayLong()
//
// if LCD_NATIVE is set, day names from antiveDay() and nativeDayLong() are used

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
                  
                  
                       // Turkey  Time
//                      lcd.setCursor(10, 2);
//                      TimeChangeRule TT = {"TUR", Second, Sun, Mar, 2, 180};  // Turkey  Time = UTC - 3 hours
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


// THE END /////
