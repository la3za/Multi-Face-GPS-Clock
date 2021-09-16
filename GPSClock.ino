#define CODE_VERSION "1.0 2021-09-16"/*
    GPS clock on 20x4 I2C LCD
    Sverre Holm, LA3ZA Nov 2015 - September 2020

Features:
           Controlled by a GPS module outputting data over an RS232 serial interface,
           and handled with the TinyGPS++ library
           Shows raw GPS data such as UTC time and date, position, altitude, and number of satellitess
           Also with various forms of binary, BCD, digit-5, digit-10 displays
           Shows derived GPS data such as 6-digit locator
           Finds local time and handles daylight saving automatically using the Timezone library
           Finds local sunset and sunrise, either actual value, or civil, nautical, or astronomical.
           The library is Sunrise.
           The clock also gives local solar height based on the Sunpos library from the K3NG rotator controller.
           The clock also provides the lunar phase based on ideas found here
           as well as predict necxt rise/set time for the moon


           Input   from GPS
           Output:
           UTC time with day
           Local time with day
           Automatic daylight saving for local time
           Longitude, latitude, altitude
           Locator, e.g. JO59fu
           Number of satellites used for position
             Moon phase, rise/set times/azimuth
           Solar data:
             Actual, Civil rise and set times
             Time of solar noon
             Solar elevation
             PWM control of LCD backlight via potentiometer on analog input
               */


               ////// load user-defined setups //////
#include "clock_pin_settings.h"     // hardware pins 
#include "clock_debug.h"            // debugging options via serial port
#include "clock_options.h"        // customization of order and number of menu items
//////////////////////////////////////

// libraries

#include <TimeLib.h>            // https://github.com/PaulStoffregen/Time - timekeepng functionality
#include <Timezone_Generic.h>   // https://github.com/khoih-prog/Timezone_Generic

#include "clock_zone.h"         // user-defined setup for local time zone and daylight saving

#include <TinyGPS++.h>          // http://arduiniana.org/libraries/tinygpsplus/
#include <Wire.h>               // For I2C. Comes with Arduino IDE
#include <LiquidCrystal_I2C.h>  // Install NewliquidCrystal_1.3.4.zip https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads/

#include <Sunrise.h>            // https://github.com/chaeplin/Sunrise, http://www.andregoncalves.info/ag_blog/?p=47
// Now in AVR-Libc version 1.8.1, Aug. 2014 (not in Arduino official release)

               // K3NG https://blog.radioartisan.com/yaesu-rotator-computer-serial-interface/
               //      https://github.com/k3ng/k3ng_rotator_controller
#include <sunpos.h>      // http://www.psa.es/sdg/archive/SunPos.cpp (via https://github.com/k3ng/k3ng_rotator_controller/tree/master/libraries)
#include <moon2.h>        // via https://github.com/k3ng/k3ng_rotator_controller/tree/master/libraries


// set the LCD address to 0x27 and set the pins on the I2C chip used for LCD connections:
//                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

#define RAD                 (PI/180.0)
#define SMALL_FLOAT         (1e-12)

#define DashedUpArrow       1
#define DashedDownArrow     2
#define UpArrow             3
#define DownArrow           4
#define DegreeSymbol        223


int dispState ;  // depends on button, decides what to display
byte wkday;
char today[10];
double latitude, lon, alt;
int Year;
byte Month, Day, Hour, Minute, Seconds;
u32 noSats;

TimeChangeRule *tcr;        //pointer to the time change rule, use to get TZ abbrev
time_t utc, local;
time_t prevDisplay = 0;     // when the digital clock was displayed

int packedRise;
double moon_azimuth = 0;
double moon_elevation = 0;
double moon_dist = 0;
float ag; // age of moon in days

int iiii;
int oldminute = -1;

int yearGPS;
uint8_t monthGPS, dayGPS, hourGPS, minuteGPS, secondGPS, weekdayGPS;

int val; // pot value which controls backlight brighness
int menuOrder[30]; //menuOrder[noOfStates];

/*
  Uses Serial1 for GPS input
  4800; // OK for EM-406A and ADS-GM1
  9600; // OK for NEO-6M
  Serial1 <=> pin 19 on Mega
*/
TinyGPSPlus gps; // The TinyGPS++ object

#include "clock_lunarCycle.h"
#include "clock_custom_routines.h"  // user customable functions for local language
#include "clock_helper_routines.h"  // library of functions

void LcdShortDayDateTimeLocal(int lineno=0, int moveLeft=0);

////////////////////////////////////////////////////////////////////////////////
void setup() {
  dispState = 0;
  lcd.begin(20, 4);

  // Store bit maps, designed using editor at https://maxpromer.github.io/LCD-Character-Creator/
  byte upDashedArray[8] = {0x4, 0xa, 0x15, 0x0, 0x4, 0x0, 0x4, 0x0};
  byte downDashedArray[8] = {0x4, 0x0, 0x4, 0x0, 0x15, 0xa, 0x4, 0x0};
  byte upArray[8] = {0x4, 0xe, 0x15, 0x4, 0x4, 0x4, 0x4, 0x0};
  byte downArray[8] = {0x4, 0x4, 0x4, 0x4, 0x15, 0xe, 0x4, 0x0};
  
  // upload characters to the lcd
  lcd.createChar(DashedUpArrow, upDashedArray);
  lcd.createChar(DashedDownArrow, downDashedArray);
  lcd.createChar(UpArrow, upArray);
  lcd.createChar(DownArrow, downArray);
  lcd.clear(); // in order to set the LCD back to the proper memory mode

  #ifdef FEATURE_LCD_NATIVE
    #include "clock_native.h"  // user customable character set
  #endif

  pinMode(LCD_pwm, OUTPUT);
  digitalWrite(LCD_pwm, HIGH);   // sets the backlight LED to full

  // unroll menu system order
  for (iiii = 0; iiii < noOfStates; iiii += 1) menuOrder[menuIn[iiii]] = iiii;

  code_Status();  // start screen
  lcd.setCursor(10, 2); lcd.print("......"); // ... waiting for GPS
  lcd.setCursor(0, 3); lcd.print("        ");
    delay(1000);

  Serial1.begin(GPSBaud);

// Serial output is only used for debugging:

  #ifdef FEATURE_SERIAL_SOLAR 
    Serial.begin(115200);
    Serial.println(F("Solar debug"));
  #endif  

  #ifdef FEATURE_SERIAL_GPS 
  // set the data rate for the Serial port
    Serial.begin(115200);
    Serial.print(F("Testing TinyGPS++ library v. ")); Serial.println(TinyGPSPlus::libraryVersion());
    Serial.println(F("by Mikal Hart"));
    Serial.println();
  #endif

  #ifdef FEATURE_SERIAL_MOON
    Serial.begin(115200);
    Serial.println(F("Moon debug"));
  #endif
  
  #ifdef FEATURE_SERIAL_MENU
    Serial.begin(115200);
    Serial.println(F("menuOrder: "));
    for (iiii = 0; iiii < noOfStates; iiii += 1) Serial.println(menuOrder[iiii]);
  #endif

  #ifdef FEATURE_SERIAL_TIME
    Serial.begin(115200);
    Serial.println(F("Time debug"));
  #endif
}

  ////////////////////////////////////// L O O P //////////////////////////////////////////////////////////////////
  void loop() {
    val = analogRead(potentiometer);   // read the input pin for control of backlight
    // analogRead values go from 0 to 1023, analogWrite values from 0 to 255
    // compress using sqrt to get smoother characteristics for the eyes
    analogWrite(LCD_pwm, (int)(255 * sqrt((float)val / 1023))); //

    byte button = analogbuttonread(0); // using K3NG function
    if (button == 2) { // increase menu # by one
      dispState = (dispState + 1) % noOfStates;
      lcd.clear();
      oldminute = -1; // to get immediate display of some info
      lcd.setCursor(18, 3); lcd.print(dispState); // lower left-hand corner
      delay(200); // was 300
    }
    else if (button == 1) { // decrease menu # by one
      dispState = (dispState - 1) % noOfStates;;
      lcd.clear();
      oldminute = -1; // to get immediate display of some info
      if (dispState < 0) dispState += noOfStates;
      lcd.setCursor(18, 3); lcd.print(dispState);
      delay(200);
    }

    else {
      while (Serial1.available()) {
        if (gps.encode(Serial1.read())) { // process gps messages
          // when GPS reports new data...
          unsigned long age;

          hourGPS = gps.time.hour();
          minuteGPS = gps.time.minute();
          secondGPS = gps.time.second();
          dayGPS = gps.date.day() ;
          monthGPS = gps.date.month() ;
          yearGPS = gps.date.year() ;
          age = gps.location.age();

          //int utc, int local; // must be time_t and global
          utc = now(); //+(long)86400*150;

          #ifdef AUTO_UTC_OFFSET       
            #include "clock_zone2.h" // this file contains the concrete time zone call
            UTCoffset = local/long(60) - utc/long(60); // order of calculation is important          
          #else 
            local = utc + UTCoffset * 60; // UTCoffset is set in clock_zone.h
          #endif

         #ifdef FEATURE_SERIAL_TIME
            Serial.print(F("utc     "));Serial.println(utc);
            Serial.print(F("local   "));Serial.println(local);
            Serial.print(F("diff,s  "));Serial.println(long(local-utc));
            Serial.print(F("diff,m1 "));Serial.println(long(local-utc)/long(60));
            Serial.print(F("diff,m  "));Serial.println(long(local)/long(60)-long(utc)/long(60));
            
            Serial.print(F("UTCoffset: "));
            Serial.println(UTCoffset);
            Serial.print("The time zone is: "); Serial.println(tcr -> abbrev);
         #endif

          if (age < 500) {
            // set the Time to the latest GPS reading
            setTime(hourGPS, minuteGPS, secondGPS, dayGPS, monthGPS, yearGPS);
            weekdayGPS = weekday();
            // Versions from 17.04.2020 has Arduino time = utc
   
          }
        }
      }
      if (timeStatus() != timeNotSet) {
        if (now() != prevDisplay) { //update the display only if the time has changed. i.e. every second
          prevDisplay = now();

/////////////////////////////////////////////// USER INTERFACE /////////////////////////////////////////////////////////
        #ifdef FEATURE_SERIAL_MENU
          Serial.println(F("menuOrder: "));
          for (iiii = 0; iiii < noOfStates; iiii += 1) Serial.println(menuOrder[iiii]);
          Serial.print(F("dispState ")); Serial.println(dispState);
          Serial.println((dispState % noOfStates));
          Serial.println(menuOrder[dispState % noOfStates]);
        #endif

////////////// Menu system ////////////////////////////////////////////////////////////////////////////////
////////////// This is the order of the menu system unless menuOrder[] contains information to the contrary

          if      ((dispState) == menuOrder[0])  LocalUTC();      // local time, date; UTC, locator
          else if ((dispState) == menuOrder[1])  UTCLocator();    // UTC, locator, # sats

// Sun, moon:  
          else if ((dispState) == menuOrder[2])  LocalSun();      // local time, sun x 3 
          else if ((dispState) == menuOrder[3])  LocalSunMoon();  // local time, sun, moon 
          else if ((dispState) == menuOrder[4])  LocalMoon();     // local time, moon size and elevation
          else if ((dispState) == menuOrder[5])  MoonRiseSet();   // Moon rises and sets at these times

// Nice to have
          else if ((dispState) == menuOrder[6]) TimeZones();     // Other time zones           
 
// Fancy, near unreadable displays, fun to program:
          else if ((dispState) == menuOrder[7])  Binary(2);       // Binary, horizontal, display of time
          else if ((dispState) == menuOrder[8])  Binary(1);       // BCD, horizontal, display of time
          else if ((dispState) == menuOrder[9])  Binary(0);       // BCD vertical display of time
          else if ((dispState) == menuOrder[10]) Bar();           // horizontal bar
          else if ((dispState) == menuOrder[11]) MengenLehrUhr(); // set theory clock
          else if ((dispState) == menuOrder[12]) LinearUhr();     // Linear clock
// debugging:
          else if ((dispState) == menuOrder[13]) InternalTime();  // Internal time - for debugging
          else if ((dispState) == menuOrder[14]) code_Status();   //

// Nice to have:           
          else if ((dispState) == menuOrder[15]) UTCPosition();   // position
          
// WSPR and beacons:
          else if ((dispState) == menuOrder[16]) NCDXFBeacons(2); // UTC + NCDXF beacons, 18-28 MHz
          else if ((dispState) == menuOrder[17]) NCDXFBeacons(1); // UTC + NCDXF beacons, 14-21 MHz
          else if ((dispState) == menuOrder[18]) WSPRsequence();  // UTC + Coordinated WSPR band/frequency (20 min cycle)
        }
      }
    }
  }  // end loop

////////////////////////////////////// END LOOP //////////////////////////////////////////////////////////////////


// The rest of this file consists of one routine per menu item:

// Menu item ///////////////////////////////////////////////////////////////////////////////////////////
void LocalUTC() { // local time, UTC,  locator

    char textbuffer[9];
    // get local time

    local = now() + UTCoffset * 60;
    Hour = hour(local);
    Minute = minute(local);
    Seconds = second(local);

    lcd.setCursor(0, 0); // 1. line *********
    sprintf(textbuffer, "%02d%c%02d%c%02d", Hour, HOUR_SEP, Minute, MIN_SEP, Seconds);
    lcd.print(textbuffer);
    lcd.print("      ");
    // local date
    Day = day(local);
    Month = month(local);
    Year = year(local);
    if (dayGPS != 0)
    {
      char today[4];

      #ifdef FEATURE_LCD_NATIVE
        lcd.setCursor(13, 0);
        nativeDayLong(local);
      #else // English
        lcd.setCursor(11, 0);
        sprintf(today, "%09s", dayStr(weekday(local)));
        lcd.print(today); lcd.print(" ");
      #endif
    
    lcd.setCursor(0, 1); //////// line 2


        lcd.print("          ");
        lcd.setCursor(10, 1);

       if (DATEORDER=='B')
          {
            printFixedWidth(lcd, Year, 4); lcd.print(DATE_SEP);
            printFixedWidth(lcd, Month, 2,'0'); lcd.print(DATE_SEP); 
            printFixedWidth(lcd, Day, 2,'0'); 
            }
       else if (DATEORDER=='M')
            {
            printFixedWidth(lcd, Month, 2,'0'); lcd.print(DATE_SEP);
            printFixedWidth(lcd, Day, 2,'0'); lcd.print(DATE_SEP);
            printFixedWidth(lcd, Year, 4);
            }
          {
            printFixedWidth(lcd, Day, 2,'0'); lcd.print(DATE_SEP);
            printFixedWidth(lcd, Month, 2,'0'); lcd.print(DATE_SEP);
            printFixedWidth(lcd, Year, 4);
          }
    }

    lcd.setCursor(0, 2); lcd.print("                    ");
    LcdUTCTimeLocator(3); // / 4. line *********
    
    oldminute = minuteGPS;
  }

 
// Menu item //////////////////////////////////////////////////////////////////////////////////////////

void UTCLocator() {     // UTC, locator, # satellites
    char textbuffer[20];

    lcd.setCursor(0, 0); // 1. line *********
    if (gps.time.isValid()) {
      sprintf(textbuffer, "%02d%c%02d%c%02d         UTC", hourGPS, HOUR_SEP, minuteGPS, MIN_SEP, secondGPS);
      lcd.print(textbuffer);
    }

    // UTC date

    if (dayGPS != 0)
    {
      lcd.setCursor(0, 1); // 2. line
      char today[4];
      lcd.print(dayStr(weekdayGPS)); lcd.print("   "); // two more spaces 14.04.2018

      lcd.setCursor(10, 1); 
      if (DATEORDER=='B')
          {
            printFixedWidth(lcd, yearGPS, 4); lcd.print(DATE_SEP);
            printFixedWidth(lcd, monthGPS, 2,'0'); lcd.print(DATE_SEP); 
            printFixedWidth(lcd, dayGPS,2, '0'); 
            }
       else if (DATEORDER=='M')
            {
            printFixedWidth(lcd, monthGPS, 2,'0'); lcd.print(DATE_SEP); 
            printFixedWidth(lcd, dayGPS,2, '0');  lcd.print(DATE_SEP);
            printFixedWidth(lcd, yearGPS, 4);  
            }
          {
            printFixedWidth(lcd, dayGPS,2, '0');  lcd.print(DATE_SEP);
            printFixedWidth(lcd, monthGPS, 2,'0'); lcd.print(DATE_SEP);
            printFixedWidth(lcd, yearGPS, 4);  
          }
      
    }
    if (gps.location.isValid()) {
      
      #ifndef MANUAL_POSITION 
        latitude = gps.location.lat();
        lon = gps.location.lng();
      #else
        latitude = latitude_manual;
        lon      = longitude_manual;
      #endif
      
      char locator[7];
      Maidenhead(lon, latitude, locator);
      lcd.setCursor(0, 3); // 4. line *********
      lcd.print(locator);
    }
    if (gps.satellites.isValid()) {
      noSats = gps.satellites.value();
      if (noSats < 10) lcd.setCursor(14, 3);
      else lcd.setCursor(13, 3); lcd.print(noSats); lcd.print(" Sats");
    }
  }

// Menu item //////////////////////////////////////////////////////////////////////////////////
void LocalSunMoon() { // local time, sun, moon
    //
    // shows Actual (0 deg) and Civil (-6 deg) sun rise/set
    //

//LcdShortDayDateTimeLocal;
    LcdShortDayDateTimeLocal(0, 2);  // line 0, time offset 2 to the left

    if (gps.location.isValid()) {
      if (minuteGPS != oldminute) {
        
      #ifndef MANUAL_POSITION 
        latitude = gps.location.lat();
        lon = gps.location.lng();
      #else
        latitude = latitude_manual;
        lon      = longitude_manual;
      #endif

      LcdSolarRiseSet(1,' ');  // 2. line
      LcdSolarRiseSet(2,'C'); // 3. line
//      LcdSolarRiseElevationSet(1); 
//      LcdSolarRiseNoonSetCivil(2); 
     
// MOON 
        
        float Phase, PercentPhase;     

        lcd.setCursor(0, 3);  // last line
        lcd.print("M ");  
                
        update_moon_position();
        lcd.setCursor(16, 3);
        printFixedWidth(lcd, (int)float(moon_elevation), 3);
        lcd.write(DegreeSymbol); 
        
        MoonPhase(Phase, PercentPhase);
             
        lcd.setCursor(9, 3);
        MoonNyNe(Phase); // (, O, ) symbol
        MoonWaxWane(Phase); //arrow up/down
        
        printFixedWidth(lcd, (int)round(PercentPhase), 3);
        lcd.print("%");               

      /////////// next rise / set
      short pRise, pSet, pLocal, pTime;
      double rAz, sAz;
      int order;
     
      GetNextRiseSet(&pRise, &rAz, &pSet, &sAz, &order); 
   
      local = now() + UTCoffset * 60;
      Hour = hour(local);
      Minute = minute(local);

      int packedTime = Hour*100 + Minute;

      lcd.setCursor(2, 3); // 4. line
      
      // find next event
      if (order == 1)
      // Rise 
      {
        pTime = pRise;  lcd.write(UpArrow); 
      }
      else
      {
       pTime = pSet; lcd.write(DownArrow); 
      }
     
        if (pTime > -1)
        {
          int pHr  = pTime / 100;
          int pMin = pTime - 100 * pHr;           
          printFixedWidth(lcd, pHr, 2);lcd.print(HOUR_SEP);printFixedWidth(lcd, pMin,2, '0');lcd.print(" ");    
        }
        else lcd.print(" - ");     
      }
    }
    oldminute = minuteGPS;
  }


// Menu item //////////////////////////////////////////////////////////////////////////////////////////
void LocalSun() { // local time, sun x 3
    //
    // shows Actual (0 deg), Civil (-6 deg), and Nautical (-12 deg) sun rise/set
    //
    LcdShortDayDateTimeLocal(0, 2);  // line 1
    if (gps.location.isValid()) {
      if (minuteGPS != oldminute) {      
        #ifndef MANUAL_POSITION 
        latitude = gps.location.lat();
        lon = gps.location.lng();
      #else
        latitude = latitude_manual;
        lon      = longitude_manual;
      #endif
   
     LcdSolarRiseSet(1,' ');  
     LcdSolarRiseSet(2,'C');
     LcdSolarRiseSet(3,'N');
     }
    }
    oldminute = minuteGPS;
  }


// Menu item /////////////////////////////////////////////////////////////////////////////////////////////
void LocalMoon() { // local time, moon phase, elevation, next rise/set

    String textbuf;
    float percentage;

    LcdShortDayDateTimeLocal(0);  // line 0
    
    if (gps.location.isValid()) {
      if (minuteGPS != oldminute) {  // update display every minute

        // days since last new moon
        float Phase, PercentPhase;
       
        update_moon_position();
        
        lcd.setCursor(0, 2); // 3. line
        
        lcd.print("  ");
        textbuf = String(moon_dist, 0);
        lcd.print(textbuf);lcd.print(" km");
        
        MoonPhase(Phase, PercentPhase);

        lcd.setCursor(13, 2);            
        MoonNyNe(Phase);  // (,0,)
        MoonWaxWane(Phase); // arrow
        
        lcd.setCursor(16, 2); 
        printFixedWidth(lcd, (int)(abs(round(PercentPhase))), 3);
        lcd.print("%");
        
        #ifndef MANUAL_POSITION 
          latitude = gps.location.lat();
          lon = gps.location.lng();
        #else
          latitude = latitude_manual;
          lon      = longitude_manual;
        #endif

        lcd.setCursor(0, 1); // 2. line
     
        lcd.print("M El ");
        lcd.setCursor(4, 1);
        printFixedWidth(lcd, (int)round(moon_elevation), 4);
        lcd.write(DegreeSymbol); 
        
        lcd.setCursor(13, 1);
        lcd.print("Az ");
        printFixedWidth(lcd, (int)round(moon_azimuth), 3);
        lcd.write(DegreeSymbol); 
        
     // Moon rise or set time:
      short pRise, pSet, order;
      double rAz, sAz;
      float Az;
      int pTime, Symb; 
 
      GetNextRiseSet(&pRise, &rAz, &pSet, &sAz, &order); 
      /* 
          pRise = pSet = -2;  // the moon never sets
          pRise = pSet = -1;  // the moon never rises
          pRise = -1;               // no MoonRise and the moon sets
          pSet = -1;                // the moon rises and never sets
      */
      local = now() + UTCoffset * 60;
      Hour = hour(local);
      Minute = minute(local);

      int packedTime = Hour*100 + Minute;

      // find next event
      if (order == 1)
      // Rise 
      {
        pTime = pRise;  Symb = UpArrow; Az = rAz;
      }
      else
      {
       pTime = pSet; Symb = DownArrow; Az = sAz;  
      }
     
 
//      if (pRise < pSet) // Moon rises before it sets
//      {
//        if (packedTime < pRise) 
//        {
//          pTime = pRise;  Symb = UpArrow; Az = rAz;
//        }
//        else 
//        {
//          pTime = pSet; Symb = DownArrow; Az = sAz; 
//        }
//      }
//      else
//      {
//        if (packedTime < pSet) 
//        {
//        pTime = pSet; Symb = DownArrow; Az = sAz; 
//        }
//        else 
//        {
//          pTime = pRise; Symb = UpArrow; Az = rAz;
//        }
//       }

        lcd.setCursor(2, 3); // 4. line
 
        if (pTime > -1)
        {
          int pHr  = pTime / 100;
          int pMin = pTime - 100 * pHr;   
           
          lcd.write(Symb);lcd.print("   ");
          printFixedWidth(lcd, pHr, 2);lcd.print(HOUR_SEP);printFixedWidth(lcd, pMin,2, '0');lcd.print(" ");
          lcd.setCursor(13, 3);
          lcd.print("Az "); 
          printFixedWidth(lcd, (int)round(Az), 3);
          lcd.write(DegreeSymbol); 
        }
        else lcd.print("  No Rise/Set       ");   
   
        oldminute = minuteGPS;
      }
    }
  }


// Menu item //////////////////////////////////////////////////////////////////////////////////
void WSPRsequence() {     // UTC, + WSPR band/frequency for coordinated WSPR
    // https://physics.princeton.edu//pulsar/K1JT/doc/wspr/wspr-main.html#BANDHOPPING
    // 20 min cycle over 10 bands from 160m to 10m

    int ii, iii, iiii, offset, km;
    char textbuffer[20];
    char* band[10] = {"160", "80", "60", "40", "30", "20", "17", "15", "12", "10"};
    char* qrg[10] = {"1838.100", "3570.100", "5366.200", "7040.100", "10140.200", "14097.100", "18106.100", "21096.100", "24926.100", "28126.100"};

    LcdUTCTimeLocator(0); // 1. line *********  
    /*
       Each WSPR frequency is transmitted every 20 minutes in 2 min intervals.
    */
    lcd.setCursor(0, 2); lcd.print("WSPR band hopping:  ");

    ii = (minuteGPS % 20) / 2; // ii from 0 to 9

    // WSPR transmission starts 1 second into even minute and lasts for 110.6 = 60 + 50.6 seconds
    if ((minuteGPS % 2 == 0 && secondGPS < 1) || (minuteGPS % 2 == 1 && secondGPS > 52))
    {
      lcd.setCursor(0, 3); lcd.print("                    ");
    }
    else
    {
      lcd.setCursor(0, 3); lcd.print(band[ii]); lcd.print(" m "); lcd.print(qrg[ii]);  lcd.print(" kHz  ");
    }
    lcd.setCursor(19, 3); lcd.print(" "); // blank out menu number
  }

// Menu items //////////////////////////////////////////////////////////////////////////////////
void NCDXFBeacons(int option) {     // UTC + info about NCDXF beacons
    // option=1: 14-21 MHz beacons on lines 1-3
    // option=2: 21-28 MHz beacons on lines 1-3
    // Inspired by OE3GOD: https://www.hamspirit.de/7757/eine-stationsuhr-mit-ncdxf-bakenanzeige/
	
    int ii, iii, iiii, offset, km;
    char textbuffer[20];
    double lati, longi;
    char* callsign[18] = {
      " 4U1UN", " VE8AT", "  W6WX", " KH6RS", "  ZL6B", "VK6RBP", "JA2IGY", "  RR9O", "  VR2B", "  4S7B", " ZS6DN",
      "  5Z4B", " 4X6TU", "  OH2B", "  CS3B", " LU4AA", "  OA4B", "  YV5B"
    };
    char* location[18] = {
      "FN30as", "EQ79ax", "CM97bd", "BL10ts", "RE78tw", "OF87av", "PM84jk", "NO14kx", "OL72bg", "MJ96wv", "KG44dc",
      "KI88ks", "KM72jb", "KP20dh", "IM12or", "GF05tj", "FH17mw", "FJ69cc"
    };
    // OH2B @ KP20dh and not just KP20: https://automatic.sral.fi/?stype=beacon&language=en
    char* qth[18] = {
      "N York ", "Nunavut", "Califor", "Hawaii ", "N Zeala", "Austral", "Japan  ", "Siberia", "H Kong ", "Sri Lan", "S Afric",
      "Kenya  ", "Israel ", "Finland", "Madeira", "Argenti", "Peru   ", "Venezue"
    };
    char* qrg[5] = {"14100", "18110", "21150", "24930", "28200"};

    LcdUTCTimeLocator(0); // 1. line *********  
    /*
       Each beacon transmits once on each band once every three minutes, 24 hours a day.
       At the end of each 10 second transmission, the beacon steps to the next higher band
       and the next beacon in the sequence begins transmitting.
    */
    ii = (60 * (minuteGPS % 3) + secondGPS) / 10; // ii from 0 to 17

    if (option <= 1) offset = 0; // 14-18 MHz
    else             offset = 2; // 18-28 MHz

    for (iiii = 1; iiii < 4; iiii += 1) { // step over lines 1,2,3
      lcd.setCursor(0, iiii);
      //
      // modulo for negative numbers: https://twitter.com/parkerboundy/status/326924215833985024
      iii = ((ii - iiii + 1 - offset % 18) + 18) % 18;
      lcd.print(qrg[iiii - 1 + offset]); lcd.print(" "); lcd.print(callsign[iii]);
      if (secondGPS % 10 < 5) {
        lcd.print(" ");  // first half of cycle: location
        lcd.print(qth[iii]);
      }
      else                                                            // second half of cycle: distance
      {
        locator_to_latlong(location[iii], lati, longi);// position of beacon
        km = distance(lati, longi, latitude, lon);    // distance beacon - GPS
        printFixedWidth(lcd, (int)float(km),6);
        lcd.print("km");
      }
    }
  }


// Menu item //////////////////////////////////////////////////////////////////////////////////////////
void UTCPosition() {     // position, altitude, locator, # satellites
    char textbuffer[20];
    String textbuf;

    LcdUTCTimeLocator(0); // 1. line *********
    // UTC date
    if (gps.location.isValid()) {
      
      #ifndef MANUAL_POSITION 
        latitude = gps.location.lat();
        lon = gps.location.lng();
        alt = gps.altitude.meters();
      #else
        latitude = latitude_manual;
        lon      = longitude_manual;
        alt = 0.0;
      #endif
           
      lcd.setCursor(0, 3);
      lcd.print(alt); lcd.print("m ");

      // Only works N & E:
      lcd.setCursor(0, 2);
      if ((now() / 4) % 3 == 0) { // change every 4 seconds
        textbuf = String(latitude, 4);
        lcd.print(textbuf); lcd.print("N,   ");

        textbuf = String(lon, 4);
        lcd.print(textbuf); lcd.print("E");
      }
      else if ((now() / 4) % 3 == 1) { // degrees, minutes, seconds

        double mins;
        String textbuf = String(floor(latitude), 0); // rounds!
        lcd.print(textbuf); lcd.write(DegreeSymbol); 
        mins = 60 * (latitude - floor(latitude));
        textbuf = String(floor(mins), 0); // round down = floor
        lcd.print(textbuf); lcd.write("'");
        textbuf = String(floor(0.5 + 60 * (mins - floor(mins))), 0); // round
        lcd.print(textbuf); lcd.write(34); lcd.print(", ");

        textbuf = String(floor(lon), 0);
        lcd.print(textbuf);
        lcd.write(DegreeSymbol); 
        mins = 60 * (lon - floor(lon));
        textbuf = String(floor(mins), 0);
        lcd.print(textbuf); lcd.write("'");
        textbuf = String(floor(0.5 + 60 * (mins - floor(mins))), 0);
        lcd.print(textbuf); lcd.write(34); // symbol for "
        //lcd.print(" ");
      }

      else  { // degrees, decimal minutes
        double mins;
        String textbuf = String(floor(latitude), 0); // rounds!
        lcd.print(textbuf); lcd.write(DegreeSymbol); 
        mins = 60 * (latitude - floor(latitude));
        textbuf = String(mins, 2); // round down = floor
        lcd.print(textbuf); lcd.write("' ");

        textbuf = String(floor(lon), 0);
        lcd.print(textbuf);
        lcd.write(DegreeSymbol); 
        mins = 60 * (lon - floor(lon));
        textbuf = String(mins, 2);
        lcd.print(textbuf); lcd.write("' ");
      }    
    }
    if (gps.satellites.isValid()) {
      noSats = gps.satellites.value();
      if (noSats < 10) lcd.setCursor(14, 3);
      else lcd.setCursor(13, 3); lcd.print(noSats); lcd.print(" Sats");
    }
  }


// Menu item ////////////////////////////////////////////////////////
void MoonRiseSet(void) {

      if (gps.location.isValid()) {
      
      #ifndef MANUAL_POSITION 
        latitude = gps.location.lat();
        lon = gps.location.lng();
      #else
        latitude = latitude_manual;
        lon      = longitude_manual;
      #endif
      
      if (minuteGPS != oldminute) {  
  
        short pRise, pSet, pRise2, pSet2, pLocal;
        double rAz, sAz, rAz2, sAz2;
  
        // rise/set for this UTC day:
         GetMoonRiseSetTimes(float(UTCoffset)/60.0, latitude, lon, &pRise, &rAz, &pSet, &sAz);
  
        lcd.setCursor(0, 0); // 1. line
        lcd.print("M "); 
        
        int MoonRiseHr  = pRise / 100;
        int MoonRiseMin = pRise - 100 * MoonRiseHr;
        int MoonSetHr  = pSet / 100;
        int MoonSetMin = pSet - 100 * MoonSetHr;

        if (pRise < pSet) lcd.setCursor(2,0); 
        else              lcd.setCursor(2,1);
  
        lcd.write(UpArrow); lcd.print(" "); 
          
         if (pRise >-1){
          printFixedWidth(lcd, MoonRiseHr, 2,'0');   lcd.print(HOUR_SEP); 
          printFixedWidth(lcd, MoonRiseMin, 2,'0');  lcd.print("  ");
          printFixedWidth(lcd, (int)round(rAz), 4);
          lcd.write(DegreeSymbol); lcd.print("  ");
        }
        else
        {
          lcd.print(pRise);lcd.print("              ");
        }

        if (pRise < pSet) lcd.setCursor(2,1);
        else              lcd.setCursor(2,0); 
        lcd.write(DownArrow);  lcd.print(" "); 
        if (pSet >-1){
          printFixedWidth(lcd, MoonSetHr, 2,'0');   lcd.print(HOUR_SEP); // doesn't handle 00:48 well with ' ' as separator
          printFixedWidth(lcd, MoonSetMin, 2,'0');  lcd.print("  ");
          printFixedWidth(lcd, (int)round(sAz), 4); lcd.write(DegreeSymbol); lcd.print("  ");
        }
        else
        {
          lcd.print(pSet);lcd.print("              ");
        }

       // rise/set for next UTC day:
         GetMoonRiseSetTimes(float(UTCoffset)/60.0 - 24.0, latitude, lon, &pRise2, &rAz2, &pSet2, &sAz2);

// Rise and set times for moon:

        MoonRiseHr  = pRise2 / 100;
        MoonRiseMin = pRise2 - 100 * MoonRiseHr;
        MoonSetHr  = pSet2 / 100;
        MoonSetMin = pSet2 - 100 * MoonSetHr;

        if (pRise2 < pSet2) lcd.setCursor(2,2);
        else              lcd.setCursor(2,3);
  
        lcd.write(UpArrow); lcd.print(" "); 
          
         if (pRise2 >-1){
          printFixedWidth(lcd, MoonRiseHr, 2,'0');   lcd.print(HOUR_SEP); 
          printFixedWidth(lcd, MoonRiseMin, 2,'0');  lcd.print("  ");
          printFixedWidth(lcd, (int)round(rAz2), 4);
          lcd.write(DegreeSymbol); lcd.print("  ");
        }
        else
        {
          lcd.print(pRise2);lcd.print("              ");
        }
  
        if (pRise2 < pSet2) lcd.setCursor(2,3);
        else              lcd.setCursor(2,2);
  
        lcd.write(DownArrow);  lcd.print(" "); 
        if (pSet2 >-1){
          printFixedWidth(lcd, MoonSetHr, 2, '0');   lcd.print(HOUR_SEP); // doesn't handle 00:48 well with ' ' as separator
          printFixedWidth(lcd, MoonSetMin, 2,'0');   lcd.print("  ");
          printFixedWidth(lcd, (int)round(sAz2), 4); lcd.write(DegreeSymbol); lcd.print("  ");
        }
        else
        {
          lcd.print(pSet2);lcd.print("              ");
        }
        lcd.setCursor(18,3);lcd.print("  ");
      }
    }
    oldminute = minuteGPS;
   }

// Menu item ////////////////////////////////////////////
void code_Status(void) {
    lcd.setCursor(0, 0); lcd.print("* LA3ZA GPS clock *");
    lcd.setCursor(0, 1); lcd.print("Ver "); lcd.print(CODE_VERSION);
    lcd.setCursor(0, 2); lcd.print("GPS "); lcd.print(GPSBaud); //lcd.print(" bps");
    lcd.setCursor(0, 3); lcd.print(tcr -> abbrev);lcd.print(" "); lcd.print(UTCoffset);  //timezone name and offset (min)   
  }
 

// Menu items ///////////////////////////////////////////////////////////////////////////////////////////
void Binary(int mode) { // binary local time
    // mode = 0 - vertical BCD
    // mode = 1 - horizontal BCD
    // mode = 2 - horisontal binary

    char textbuffer[9];
    int tens, ones;

    int BinaryTensHour[5], BinaryHour[5], BinaryTensMinute[5], BinaryMinute[5], BinaryTensSeconds[5], BinarySeconds[5];

    // get local time
    local = now() + UTCoffset * 60;
    Hour = hour(local);
    Minute = minute(local);
    Seconds = second(local);

    // convert to BCD

    // must send a variable, not an equation, to decToBinary as it does in-place arithmetic on input variable
    ones = Hour % 10; tens = (Hour - ones) / 10;
    decToBinary(tens, BinaryTensHour); decToBinary(ones, BinaryHour);

    ones = Minute % 10; tens = (Minute - ones) / 10;
    decToBinary(tens, BinaryTensMinute); decToBinary(ones, BinaryMinute);

    ones = Seconds % 10;   tens = (Seconds - ones) / 10;
    decToBinary(tens, BinaryTensSeconds); decToBinary(ones, BinarySeconds);


    if (mode == 0) // vertical digits:
    {
      lcd.setCursor(10, 3); //LSB
      sprintf(textbuffer, "%1d%1d %1d%1d %1d%1d  ", BinaryTensHour[4], BinaryHour[4], BinaryTensMinute[4], BinaryMinute[4], BinaryTensSeconds[4], BinarySeconds[4]);
      lcd.print(textbuffer);

      lcd.setCursor(10, 2);
      sprintf(textbuffer, "%1d%1d %1d%1d %1d%1d", BinaryTensHour[3], BinaryHour[3], BinaryTensMinute[3], BinaryMinute[3], BinaryTensSeconds[3], BinarySeconds[3]);
      lcd.print(textbuffer);

      lcd.setCursor(10, 1);
      sprintf(textbuffer, " %1d %1d%1d %1d%1d", BinaryHour[2], BinaryTensMinute[2], BinaryMinute[2], BinaryTensSeconds[2], BinarySeconds[2]);
      lcd.print(textbuffer);

      lcd.setCursor(10, 0);
      sprintf(textbuffer, " %1d  %1d  %1d", BinaryHour[1], BinaryMinute[1], BinarySeconds[1]);
      lcd.print(textbuffer);

      lcd.setCursor(0, 0); lcd.print("BCD");

    }
    else if (mode == 1)
    {
      //// horizontal BCD digits:

      lcd.setCursor(9, 1); sprintf(textbuffer, "  %1d%1d ", BinaryTensHour[3], BinaryTensHour[4] );
      lcd.print(textbuffer);
      sprintf(textbuffer, "%1d%1d%1d%1d H", BinaryHour[1], BinaryHour[2], BinaryHour[3], BinaryHour[4]);
      lcd.print(textbuffer);

      lcd.setCursor(9, 2);  sprintf(textbuffer, " %1d%1d%1d ", BinaryTensMinute[2], BinaryTensMinute[3], BinaryTensMinute[4] );
      lcd.print(textbuffer);
      sprintf(textbuffer, "%1d%1d%1d%1d M", BinaryMinute[1], BinaryMinute[2], BinaryMinute[3], BinaryMinute[4] );
      lcd.print(textbuffer);

      lcd.setCursor(9, 3);  sprintf(textbuffer, " %1d%1d%1d ", BinaryTensSeconds[2], BinaryTensSeconds[3], BinaryTensSeconds[4] );
      lcd.print(textbuffer);
      sprintf(textbuffer, "%1d%1d%1d%1d S", BinarySeconds[1], BinarySeconds[2], BinarySeconds[3], BinarySeconds[4] );
      lcd.print(textbuffer);

      lcd.setCursor(0, 0); lcd.print("BCD");


      if (Seconds < 11)  // show help: weighting
      {
      #ifdef CLOCK_HELP
        lcd.setCursor(9, 0); lcd.print(" 421 8421");
      #endif
      } 
      else
      {
       lcd.setCursor(9, 0); lcd.print("         ");
      }
      } 
      else
        // horisontal 5-bit binary
    {
      // convert to binary:
      decToBinary(Hour, BinaryHour);
      decToBinary(Minute, BinaryMinute);
      decToBinary(Seconds, BinarySeconds);
  
      lcd.setCursor(13, 1); sprintf(textbuffer, "%1d%1d%1d%1d%1d H", BinaryHour[0], BinaryHour[1], BinaryHour[2], BinaryHour[3], BinaryHour[4]);
      lcd.print(textbuffer);
  
      lcd.setCursor(13, 2); sprintf(textbuffer, "%1d%1d%1d%1d%1d M", BinaryMinute[0], BinaryMinute[1], BinaryMinute[2], BinaryMinute[3], BinaryMinute[4] );
      lcd.print(textbuffer);
  
      lcd.setCursor(13, 3); sprintf(textbuffer, "%1d%1d%1d%1d%1d S", BinarySeconds[0], BinarySeconds[1], BinarySeconds[2], BinarySeconds[3], BinarySeconds[4] );
      lcd.print(textbuffer);
  
      lcd.setCursor(0, 0); lcd.print("Binary");
  
      if (Seconds < 11)  // show help: weighting
      {
      #ifdef CLOCK_HELP
        lcd.setCursor(13, 0); lcd.print(" 8421");
      #endif
      } 
      else
      {
        lcd.setCursor(13, 0); lcd.print("     ");
      }
     }
  
        // Common for all modes:
      #ifdef CLOCK_HELP
          if (Seconds < 11)  // show time in normal numbers
          {
          sprintf(textbuffer, "%02d%c%02d%c%02d", Hour, HOUR_SEP, Minute, MIN_SEP, Seconds);
          } 
          else
          {
          sprintf(textbuffer, "        ");
          }
          lcd.setCursor(0, 3); // last line *********
          lcd.print(textbuffer);
        #endif
 }

// Menu item ////////////////////////////////////////////
void Bar(void) {
          char textbuffer[9];
          // get local time

          local = now() + UTCoffset * 60;
          Hour = hour(local);
          Minute = minute(local);
          Seconds = second(local);

          // use a 12 character bar  just like a 12 hour clock with ticks every hour
          // for second ticks use ' " % #
          lcd.setCursor(0, 0);
          int imax = Hour;

          if (Hour == 13 & Minute == 0 & Seconds == 0)
          {
            lcd.print("                   ");
            lcd.setCursor(0, 0);
          }

          if (Hour > 12) imax = Hour - 12;

          if (Hour == 0) lcd.print("                ");
          for (int i = 0; i < imax; i++) {
            lcd.write(255); // fills square
            if (i == 2 | i == 5 | i == 8) lcd.write(254); // empty
          }

          // could have used |, ||, |||, |||| for intermediate symbols by creating new characters
          // like here https://forum.arduino.cc/index.php?topic=180678.0
          // but easier to use something standard

          //  if (Minute/12 == 1) {lcd.write(165);}
          //  else if (Minute/12 == 2) {lcd.print('"');}
          //  else if (Minute/12 == 3) {lcd.write(208);}
          //  else if (Minute/12 == 4) {lcd.write(219);}

          //  lcd.print(" ");lcd.print(Hour);

          lcd.setCursor(18, 0); lcd.print("1h");

          lcd.setCursor(0, 1);
          imax = Minute / 5;
          if (Minute == 0) lcd.print("                ");
          for (int i = 0; i < imax; i++) {
            lcd.write(255); // fills square
            if (i == 2 | i == 5 | i == 8) lcd.write(254); // empty
          }
          if (Minute % 5 == 1) {
            lcd.write(165);
          }
          else if (Minute % 5 == 2) {
            lcd.print('"');
          }
          else if (Minute % 5 == 3) {
            lcd.write(208);
          }
          else if (Minute % 5 == 4) {
            lcd.write(219);
          }
          // lcd.print(" ");lcd.print(Minute);
          lcd.setCursor(18, 1); lcd.print("5m");


          // seconds in 12 characters, with a break every 3 characters
          lcd.setCursor(0, 2);
          imax = Seconds / 5;
          if (Seconds == 0) lcd.print("                ");
          for (int i = 0; i < imax; i++) {
            lcd.write(255); // fills square
            if (i == 2 | i == 5 | i == 8) lcd.write(254); // empty
          }
          if (Seconds % 5 == 1) {
            lcd.write(165);
          }
          else if (Seconds % 5 == 2) {
            lcd.print('"');
          }
          else if (Seconds % 5 == 3) {
            lcd.write(208);
          }  //("%");}
          else if (Seconds % 5 == 4) {
            lcd.write(219);
          }  //("#");}
          lcd.setCursor(18, 2); lcd.print("5s");
          lcd.setCursor(18, 3); lcd.print("  ");

          lcd.setCursor(18, 3);
          if (Hour > 12) lcd.print("PM");
          else lcd.print("AM");

          #ifdef CLOCK_HELP
            lcd.setCursor(12, 3);
            if (Seconds < 11)  // show time in normal numbers
            {
          //    lcd.print("Bar");
            sprintf(textbuffer, "%02d%c%02d%c%02d", Hour%12, HOUR_SEP, Minute, MIN_SEP, Seconds);

            lcd.print(textbuffer);

            }
            else
            {
              lcd.print("          ");
            }
          #endif
        }

// Menu item ////////////////////////////////////////////
void MengenLehrUhr(void) {
//
// Set theory clock of https://en.wikipedia.org/wiki/Mengenlehreuhr in Berlin
        int imax;
        //  lcd.clear(); // makes it blink

        // get local time
        local = now() + UTCoffset * 60;
        Hour = hour(local);
        Minute = minute(local);
        Seconds = second(local);

        lcd.setCursor(0, 0);
        // first line has 5 hour resolution
        if (Hour > 4)
        {
          lcd.print("-"); lcd.write(255); lcd.print("-"); lcd.print(" ");// fills square
        }
        else lcd.print("    ");
        if (Hour > 9)
        {
          lcd.print("-"); lcd.write(255); lcd.print("-"); lcd.print(" ");// fills square
        }
        else lcd.print("    ");

        if (Hour > 14)
        {
          lcd.print("-"); lcd.write(255); lcd.print("-"); lcd.print(" ");// fills square
        }
        else lcd.print("    ");
        if (Hour > 19)
        {
          lcd.print("-"); lcd.write(255); lcd.print("-"); lcd.print(" ");// fills square
        }
        else lcd.print("    ");

        lcd.setCursor(18, 0); lcd.print("5h");

        // second line shows remainder and has 1 hour resolution

        lcd.setCursor(0, 1);
        imax = Hour % 5;
        if (imax > 0)
        {
          lcd.print("-"); lcd.write(255); lcd.print("-"); lcd.print(" ");// fills square
        }
        else lcd.print("    ");
        if (imax > 1)
        {
          lcd.print("-"); lcd.write(255); lcd.print("-"); lcd.print(" ");// fills square
        }
        else lcd.print("    ");
        if (imax > 2)
        {
          lcd.print("-"); lcd.write(255); lcd.print("-"); lcd.print(" ");// fills square
        }
        else lcd.print("    ");
        if (imax > 3)
        {
          lcd.print("-"); lcd.write(255); lcd.print("-"); lcd.print(" ");// fills square
        }
        else lcd.print("    ");

        lcd.setCursor(18, 1); lcd.print("1h");

        // third line shows a bar for every 5 minutes
        int ii;
        lcd.setCursor(0, 2);

        // only overwrite old characters when needed, to avoid flicker

        imax = Minute / 5;
        if (imax == 0)
        {
          lcd.print("                  ");
          lcd.setCursor(0, 2);
        }

        for (ii = 0; ii < imax; ii++)
        {
          lcd.write(255);
          if (ii == 2 || ii == 5 || ii == 8) lcd.print(" ");
        }

        lcd.setCursor(18, 2); lcd.print("5m");

        // fourth line shows remainder and has 1 minute resolution
        lcd.setCursor(0, 3);

        imax = Minute % 5;
        if (imax > 0)
        {
          lcd.print("-"); lcd.write(255); lcd.print("-"); lcd.print(" ");// fills square
        }
        else lcd.print("    ");

        if (imax > 1)
        {
          lcd.print("-"); lcd.write(255); lcd.print("-"); lcd.print(" ");// fills square
        }
        else lcd.print("    ");

        if (imax > 2)
        {
          lcd.print("-"); lcd.write(255); lcd.print("-"); lcd.print(" ");// fills square
        }
        else lcd.print("    ");

        if (imax > 3)
        {
          lcd.print("-"); lcd.write(255); lcd.print("-"); lcd.print(" ");// fills square
        }
        else lcd.print("    ");

        lcd.setCursor(18, 3); lcd.print("1m");
      }

// Menu item ////////////////////////////////////////////    
void LinearUhr(void) {
// Linear Clock, https://de.wikipedia.org/wiki/Linear-Uhr in Kassel
     
        int imax;
        int ii;
        //  lcd.clear(); // makes it blink

        // get local time
        local = now() + UTCoffset * 60;
        Hour = hour(local);
        Minute = minute(local);
        Seconds = second(local);

        lcd.setCursor(0, 0);
        // first line has 10 hour resolution
        imax = Hour / 10;
        lcd.print("             ");
        lcd.setCursor(0, 0);
        for (ii = 0; ii < imax; ii++)
        {
          lcd.write(255);
        }
        lcd.setCursor(17, 0); lcd.print("10h");

        // second line shows remainder and has 1 hour resolution
        lcd.setCursor(0, 1);
        imax = Hour % 10;

        // only overwrite old characters when needed, to avoid flicker
        if (imax == 0)
        {
          lcd.print("             ");
          lcd.setCursor(0, 1);
        }
        for (ii = 0; ii < imax; ii++)
        {
          lcd.write(255);
          if (ii == 4) lcd.print(" ");
        }
        lcd.setCursor(18, 1); lcd.print("1h");

        // third line shows a bar for every 10 minutes
        lcd.setCursor(0, 2);
        imax = Minute / 10;

        // only overwrite old characters when needed, to avoid flicker
        if (imax == 0)
        {
          lcd.print("                  ");
          lcd.setCursor(0, 2);
        }

        for (ii = 0; ii < imax; ii++)
        {
          lcd.write(255);
          if (ii == 4) lcd.print(" ");
        }
        lcd.setCursor(17, 2); lcd.print("10m");

        // fourth line shows remainder and has 1 minute resolution
        lcd.setCursor(0, 3);

        // only overwrite old characters when needed, to avoid flicker

        imax = Minute % 10;
        if (imax == 0)
        {
          lcd.print("                  ");
          lcd.setCursor(0, 3);
        }

        for (ii = 0; ii < imax; ii++)
        {
          lcd.write(255);
          if (ii == 4) lcd.print(" ");
        }
        lcd.setCursor(18, 3); lcd.print("1m");
      }


// Menu item //////////////////////////////////////////////////////////////////////////////////////////
void InternalTime() {     // UTC, Unix time, J2000, etc
      char textbuffer[20];

      lcd.setCursor(0, 0); // 1. line *********
      if (gps.time.isValid()) {
        sprintf(textbuffer, "%02d%c%02d%c%02d UTC ", hourGPS, HOUR_SEP, minuteGPS, MIN_SEP, secondGPS);
        lcd.print(textbuffer);
        
        float jd = now() / 86400.0; // cdn(now()); // now/86400, i.e. no of days since 1970
        float j2000 = jd - 10957.5; // 1- line
        lcd.setCursor(9, 0);
        lcd.print("j2k ");
        lcd.print(j2000);

        // utc = now(); // UNIX time, seconds ref to 1970
        lcd.setCursor(0, 1);
        lcd.print("now   ");
        lcd.print(now());
       
        lcd.setCursor(0, 2);
        lcd.print("jd1970 ");
        lcd.print(jd);

        lcd.setCursor(0, 3);
        lcd.print("local ");
        local = now() + UTCoffset * 60;
        lcd.print(local);
      }
    }

////////////////////////////////////////////////////////////////////////////////////////////////////
// End of functions for Menu system ////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/// THE END ///
