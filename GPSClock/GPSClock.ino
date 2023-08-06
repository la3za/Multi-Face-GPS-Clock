// Set version and date manually for code status display
const char codeVersion[]   = "v2.0.0    02.08.2023";

// or set date automatically to compilation date (US format) - nice to use during development - while version number is set manually
//const char codeVersion[] = "v2.0.0   "__DATE__;  

/*
  LA3ZA Multi Face GPS Clock

  Copyright 2015 - 2023 Sverre Holm, LA3ZA
  All trademarks referred to in source code and documentation are copyright their respective owners.


    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

  If you offer a hardware kit using this software, show your appreciation by sending the author a complimentary kit ;-)

  Full documentation can be found at https://github.com/la3za .  Please read it before requesting help.


  Features:
           GPS clock on 20x4 I2C LCD
           Controlled by a GPS module outputting data over a serial interface, typ. QRPLabs QLG1 GPS Receiver kit, or the QLG2
           GPS is handled with the TinyGPS++ library
           Shows raw GPS data such as UTC time and date, position, altitude, and number of satellitess
           Also with various forms of binary, BCD, hex and octal displays
           Shows derived GPS data such as 6-digit locator
           Finds local time and handles daylight saving automatically using the Timezone library
           Finds local sunset and sunrise, either actual value, or civil, nautical, or astronomical using the Sunrise library
           The clock also gives local solar height based on the Sunpos library from the K3NG rotator controller.
           The clock also provides the lunar phase as well as predicts next rise/set time for the moon


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

/* Functions in this file - one per screen type:

            setup
            loop
            screenSelect

One routine per clock face, i.e. one per menu item:
            LocalUTC
            UTCLocator

            LocalSun
            LocalSunAzEl [added 12.11.2021]
            LocalSunMoon
            LocalMoon
            MoonRiseSet
            TimeZones

            Binary
            Bar
            MengenLehrUhr
            LinearUhr
            Hex
            Octal

            InternalTime
            CodeStatus

            UTCPosition

            NCDXFBeacons
            WSPRsequence

            HexOctalClock 
            EasterDates   
            MathClock     
            LunarEclipse   
                                   
            Roman         
            Morse         
            WordClock    
            Sidereal      
            DemoClock 

            PlanetVisibility
            ISOHebIslam
            GPSInfo     
 
            ChemicalElements

            screenSelect
                        
  In development: 
 
            Reminder           

*/

/*

  Revisions:

 2.0.0   02.08.2023: 
                - New menu system for setup by pushing rotary control, instead of editing clock_options.h
                - Setup menu gives choice of:
                -- Choose: Clock subset, Backlight, Date/time format, Time zone, Local language, Secondary menu
                ---- Secondary menu: GPS baud rate, FancyClock help, Dwell time demo, Time math quiz
                -- Chosen values saved and retrieved from EEPROM. Timeout from menu system after nominally 30 sec if no activity
                - Reset clock for long push on rotary control
                - Several small formatting changes/improvements in several functions:
                -- UTCPosition, NCDXFBeacons, ChemicalElements, MathClock, Sidereal, ISOHebIslam, DemoClock
                - Removed options FEATURE_PUSH_FAVORITE, FEATURE_ROTARY_ENCODER - now in secondary menu
                - Removed SECONDS_CLOCK_HELP, DWELL_TIME_DEMO. Are now secondary menu items (secondsClockHelp, dwellTimeDemo)
                - Removed almost all warnings when using Arduino IDE 2.1.0 (mostly to do with char arrays)
                - More character strings in PROGMEM -> 5717 bytes (69%) of dynamic memory

 1.6.0   14.04.2023:
                - ChemicalElements: new display
                - Saved 764 bytes of dynamic memory: replaced lcd-print(".. with lcd.print(F(".. and char by int/float
                - Changed > to >= so DWELL_TIME in demo mode is correctly implemented 
                - Fixed small formatting error in week number for single-digit week #
                - GPSInfo() now indicates 0 satellites etc when GPS signal is lost
                - UTCLocator() and UTCPosition(): Replaced gps.satellites.isValid with gps.satellites.isUpdated 
                  in order to see loss of GPS signal immediately in the no of satellites
                - LocalUTC(): ISOWeek changes to Norwegian "Uke" rather than "Week" if Norwegian language if set
                - Small formatting changes in LocalSunMoon(), localMoon() 

 1.5.0  03.11.2022:
                - PlanetVisibility
                - ISOHebIslam
                - GPSInfo
                - Option for showing local week number (ISO) rather than locator in local time display 
                - cycleTime in UTCPosition for cycling between different formats for position increased from 4 to 10 sec
                - fixed bug in menu system for menu # 0, now initialized to menuOrder[iiii] = -1, rather than to 0.

 1.4.0  25.07.2022:
                - Multiple language support: 'no', 'se', 'dk', 'is', 'de', 'fr', 'es' day names for local time in addition to English
                - Cleaned display from LcdSolarRiseSet for Northern positions in case sun never goes below Civil, Nautical, Astronomical limit in summer
                - Follows this naming conventions better:
                -- function:  CapitalLetterFirst      
                -- variable:  smallLetterFirst
                -- defines:   CAPITAL_LETTERS
                
 1.3.0 05.04.2022:
                - WordClock - displays time in words
                - DemoClock - cycles through all the other selected modes
                - Sidereal  - Local Sidereal and Solar Time
                - Morse     - Morse display on lcd
                - Roman     - Roman number clock 
    
1.2.1 07.03.2022:
                - Removed unused code WordClock
                - Fixed display of short day names in Native Language (affects Norwegian only)                                

1.2.0 21.01.2022
                - MathClock - 4 ways to present an arithmetic computation to find hour and minute at regular intervals
                - LunarEclipse - predict lunar eclipses 2-3 years into the future
                - MoonRiseSet now only shows future events
                - New LocalSunAzEl 
                - New LocalSun(1) with a simpler look (LocalSun(0) is the old look)
                - HexOctalClock(3) now always shows normal decimal clock also
                - Menu system simplified by using descriptive names for screens not just numbers (see definitions in clock_defines.h)

1.1.1 14.11.2021
                - Fixed bug in selection of favorite menu item
                
1.1.0 25.10.2021
                - Implemented rotary encoder for control of screen number as an alternative (or in addition to) up/down buttons
                - FEATURE_PUSH_FAVORITE - push on rotary encoder: jump to favorite screen. If not set: adjust backlight
                - New defines FEATURE_POTENTIOMETER, FEATURE_BUTTONS, FEATURE_ROTARY_ENCODER, FEATURE_PUSH_FAVORITE

1.0.4  18.10.2021
                - Fixed small formatting bug in LcdShortDayDateTimeLocal which affected display of date on line 0 in several screens
                - Added screen 22, EasterDates, with dates for Gregorian and Julian Easter Sunday three years ahead

1.0.3  11.10.2021
                - Added missing 6. bit in minutes, seconds in binary clocks
		            - Added screen 21 with simultaneous binary, octal, and hex clocks
                - Removed FEATURE_CLOCK_SOME_SECONDS, replaced by SECONDS_CLOCK_HELP = (0...60) for additional "normal" clock in binary, octal, BCD, etc

1.0.2  06.10.2021
                - UTC and position screen: Changed layout. Now handles Western longitudes and Southern latitudes also. Thanks Mitch W4OA
                - Corrected bug in Maidenhead routine, only appeared if letter 5 was beyond a certain letter in the alphabet. Thanks Ross VA1KAY

1.0.1  29.09.2021
                - Fixed  small layout bug on screen
                - New variable in clock_options.h: SECONDS_CLOCK_HELP - no of seconds to show normal clock in binary, BCD, hex, octal etc clocks.
                - 2 new screens: no 19 and 20: hex and octal clock

1.0.0  24.09.2021
                - First public release
                - 18 different screens


*/

// turn on/off experimental options
//#define EXP_Tide_Sidereal  // Experimental option - unfinished
#define SNR_GPSInfo          // Unstable 

///// load user-defined setups //////
#include "clock_defines.h"
#include "clock_debug.h"            // debugging options via serial port

#define noOfEntries 50              // 50, must be large enough to hold all possible screens in menu!!
#define maxNumTimeZones 25          // 25, must have fewer than these time zone definitions in clock_zone.h

char textBuffer[21];                // 1 line on lcd, 20 characters was: buffer[80]; For display strings

const int lengthOfMenuIn = noOfEntries;
int menuOrder[noOfEntries];         // chosen submenu is here
int noOfStates = 0;                 // no of actual entries in chosen submenu
int numTimeZones = 5;               // no of time zones defined in clock_zone.h
                                    // guessing a small number, real value is set in clock_zone.h

long utcOffset = 0;                 // value set automatically by means of Timezone library

#include "clock_options.h"          // customization of order and number of menu items
#include "clock_pin_settings.h"     // hardware pins 

//////////////////////////////////////

// libraries need to be installed according to how the Arduino environment expects them. 
// See https://docs.arduino.cc/software/ide-v1/tutorials/installing-libraries
// See project wiki for source for alle the libraries

#include <TimeLib.h>                // https://github.com/PaulStoffregen/Time - timekeeping functionality
#include <Timezone_Generic.h>       // https://github.com/khoih-prog/Timezone_Generic
//#include "clock_zone.h"           // user-defined setup for local time zone and daylight saving

#include <TinyGPS++.h>              // http://arduiniana.org/libraries/tinygpsplus/
#include "clock_z_planets.h"
#include "clock_z_calendar.h"

static tmElements_t curr_time;

#if defined(FEATURE_LCD_I2C)
  #include <Wire.h>                 // For I2C. Comes with Arduino IDE
  #include <LiquidCrystal_I2C.h>    // Install NewliquidCrystal_1.3.4.zip https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads/
#endif

#if defined(FEATURE_LCD_4BIT)
  #include <LiquidCrystal.h>
#endif

#include <rotary.h>                 // rotary handler https://bitbucket.org/Dershum/rotary_button/src/master/

#include <Sunrise.h>                // https://github.com/chaeplin/Sunrise, http://www.andregoncalves.info/ag_blog/?p=47
// Now in AVR-Libc version 1.8.1, Aug. 2014 (not in Arduino official release)

// K3NG https://blog.radioartisan.com/yaesu-rotator-computer-serial-interface/
//      https://github.com/k3ng/k3ng_rotator_controller
#include <sunpos.h>      // http://www.psa.es/sdg/archive/SunPos.cpp (via https://github.com/k3ng/k3ng_rotator_controller/tree/master/libraries)
#include <moon2.h>       // via https://github.com/k3ng/k3ng_rotator_controller/tree/master/libraries

#if defined(FEATURE_LCD_I2C)
//            set the LCD address to 0x27 and set the pins on the I2C chip used for LCD connections:
//                     addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
  LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address
#endif

#if defined(FEATURE_LCD_4BIT)
  LiquidCrystal lcd(lcd_rs, lcd_enable, lcd_d4, lcd_d5, lcd_d6, lcd_d7);
#endif

Rotary r = Rotary(PIN_A, PIN_B, PUSHB); // Initialize the Rotary object

#define RAD                 (PI/180.0)
#define SMALL_FLOAT         (1e-12)

// LCD characters:
#define DASHED_UP_ARROW        1 // user defined character
#define DASHED_DOWN_ARROW      2 // user defined character
#define UP_ARROW               3 // user defined character
#define DOWN_ARROW             4 // user defined character
#define APOSTROPHE            34 // in LCD character set
#define BIG_DOT              165 // in LCD character set
#define THREE_LINES          208 // in LCD character set
#define SQUARE               219 // in LCD character set
#define DEGREE               223 // in LCD character set
#define ALL_OFF              254 // in LCD character set
#define ALL_ON               255 // in LCD character set

static uint32_t gpsBaud;        // stores baud rate for GSP, read from gspBaud1 - array
int     dispState ;             // depends on button, decides what to display
int     demoDispState;          // decides what to display in Demo Mode

char    today[15];              // was [12]
char    todayFormatted[15];     // was [12]: for storing string with day name up to 12 characters long
double  latitude, lon, alt;
int     Year;
byte    Month, Day, Hour, Minute, Seconds;
u32     noSats;


TimeChangeRule *tcr[maxNumTimeZones];  //pointer to the time change rule, use to get TZ abbrev
time_t  utc, local[maxNumTimeZones]; 
time_t  oldNow = 0;          // keeps time from now(), for counting how long a screen has been held in Demo mode
time_t  prevDisplay = 0;     // keeps time from now(), to find out last when the digital clock was displayed

int     packedRise;
double  moon_azimuth = 0;
double  moon_elevation = 0;
double  moon_dist = 0;

int     iiii;  // general loop counter
int     oldMinute = -1;  // compared to minuteGPS in order to get immediate display of some info

int     yearGPS;
uint8_t monthGPS, dayGPS, hourGPS, minuteGPS, secondGPS, weekdayGPS;

/*
  Uses Serial1 for GPS input
  4800; // OK for EM-406A and ADS-GM1
  9600; // OK for NEO-6M
  Serial1 <=> pin 19 on Mega
*/
TinyGPSPlus gps; // The TinyGPS++ object

#include "clock_z_lunarCycle.h"

// Initial values for primary menu parameters
int     subsetMenu      = 0;  // variable for choosing subset of possible clock faces    
byte    backlightVal    = 10; // (0...255) initial backlight value
int     dateFormat      = 0;  // variable for choosing date/time format
int     languageNumber  = 0;  // variable for choosing language for local time day names
int     timeZoneNumber  = 0;  // variable for choosing time zone

// Initial values for secondary menu parameter:
int     baudRateNumber  = 2;  // points to entry in array of possible baudrates
byte    secondsClockHelp= 6;  // no of seconds per minute of normal clock display for fancy clocks
byte    dwellTimeDemo   = 10; // no of seconds per screen as DemoClock cycles through all screen
byte    mathSecondPeriod= 10; // 1...6 per minute, i.e. 10-60 seconds in AlbertClock app

#include "clock_language.h"         //user customable functions for local language, was "clock_custom_routines.h"
#include "clock_helper_routines.h"  // library of functions


#include "clock_z_moon_eclipse.h"
#include "clock_z_equatio.h"

#ifdef FEATURE_DATE_PER_SECOND // for stepping date quickly and check calender function
  int   dateIteration;
#endif

 // builds on the example program SatelliteTracker from the TinyGPS++ library
  // https://www.arduino.cc/reference/en/libraries/tinygps/
  /*
    From http://aprs.gids.nl/nmea/:
   
  $GPGSV
  
  GPS Satellites in view (SV - Satellite in View)
  
  eg. $GPGSV,3,1,11,03,03,111,00,04,15,270,00,06,01,010,00,13,06,292,00*74
      $GPGSV,3,2,11,14,25,170,00,16,57,208,39,18,67,296,40,19,40,246,00*74
      $GPGSV,3,3,11,22,42,067,42,24,14,311,43,27,05,244,00,,,,*4D

  1    = Total number of messages of this type in this cycle
  2    = Message number
  3    = Total number of SVs in view
  4    = SV PRN number
  5    = Elevation in degrees, 90 maximum
  6    = Azimuth, degrees from true north, 000 to 359
  7    = SNR, 00-99 dB (null when not tracking)
  8-11 = Information about second SV, same as field 4-7
  12-15= Information about third SV, same as field 4-7
  16-19= Information about fourth SV, same as field 4-7
*/
  static const int MAX_SATELLITES = 40;

  TinyGPSCustom totalGPGSVMessages(gps, "GPGSV", 1); // $GPGSV sentence, first element
  TinyGPSCustom messageNumber(gps, "GPGSV", 2);      // $GPGSV sentence, second element
  TinyGPSCustom satsInView(gps, "GPGSV", 3);         // $GPGSV sentence, third element
  TinyGPSCustom GPSMode(gps, "GPGSA", 2);            // $GPGSA sentence, 2nd element 1-none, 2=2D, 3=3D
  TinyGPSCustom posStatus(gps, "GPRMC", 2);          // $GPRMC sentence: Position status (A = data valid, V = data invalid)
  TinyGPSCustom satNumber[4]; // to be initialized later
  TinyGPSCustom elevation[4];
  TinyGPSCustom azimuth[4];
  TinyGPSCustom snr[4];

  struct
{
  bool  active;
  int   elevation;
  int   azimuth;
  int   snr;
} sats[MAX_SATELLITES];


//#include "clock_development.h"

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
  lcd.createChar(DASHED_UP_ARROW, upDashedArray);
  lcd.createChar(DASHED_DOWN_ARROW, downDashedArray);
  lcd.createChar(UP_ARROW, upArray);
  lcd.createChar(DOWN_ARROW, downArray);

#include "clock_native.h"                      // character sets for multiple languages

  lcd.clear(); // in order to set the LCD back to the proper memory mode after custom characters have been created

  pinMode(LCD_PWM, OUTPUT);                    // for backlight control

// *** EEPROM read/default setup:  

  backlightVal = max(10, EEPROM.read(0)); // minimum 10 to ensure that display always is readable on very first startup
  analogWrite(LCD_PWM, backlightVal);

  int noOfMenuIn = sizeof(menuStruct)/sizeof(menuStruct[0]);
  subsetMenu = readIntFromEEPROM(1); // read int from EEPROM addresses 1 and 2
  if ((subsetMenu < 0) || (subsetMenu >= noOfMenuIn)) // if EEPROM stores invalid value
  {
    subsetMenu = 0; //  set to default value on very first startup
    updateIntIntoEEPROM(1, subsetMenu); // make sure EEPROM has a valid value
  }

  noOfMenuIn = sizeof(dateTimeFormat)/sizeof(dateTimeFormat[0]);
  dateFormat = readIntFromEEPROM(3); // read int from EEPROM addresses 3 and 4
  if ((dateFormat < 0) || (dateFormat >= noOfMenuIn)) // if EEPROM stores invalid value
  {
    dateFormat = 0; //  set to default value on very first startup
    updateIntIntoEEPROM(3, dateFormat); // make sure EEPROM has a valid value
  }

  uint8_t numLanguages = sizeof(languages) / sizeof(languages[0]);
  languageNumber = readIntFromEEPROM(5); // read int from EEPROM addresses 5 and 6
  if ((languageNumber < -1) || (languageNumber >= numLanguages)) // if EEPROM stores invalid value
  {
    languageNumber = -1; // set to default value (-1 = English) on very first startup
    updateIntIntoEEPROM(5, languageNumber); // make sure EEPROM has a valid value
  }
 
  timeZoneNumber = readIntFromEEPROM(7); // read int from EEPROM addresses 7 and 8
  if ((timeZoneNumber < 0) || (timeZoneNumber > numTimeZones))
  {
    timeZoneNumber = 0; 
    updateIntIntoEEPROM(7, timeZoneNumber);
  }

  baudRateNumber = readIntFromEEPROM(9); // addresses 9 and 10
  if ((baudRateNumber < 0) || (baudRateNumber > sizeof(gpsBaud1)/sizeof(gpsBaud1[0])))
  {
    baudRateNumber = 0; 
    updateIntIntoEEPROM(9, baudRateNumber);
  }

  secondsClockHelp = EEPROM.read(11);
  dwellTimeDemo    = min(max(EEPROM.read(12), 4), 60);
  mathSecondPeriod = min(max(EEPROM.read(13), 1), 60);
  
// *** end EEPROM read/default setup 

  InitScreenSelect();            // Initalize screen selection order

  digitalWrite (PIN_A, HIGH);     // enable pull-ups for rotary encoder and button
  digitalWrite (PIN_B, HIGH);
  digitalWrite (PUSHB, HIGH);

  CodeStatus();  // start screen
  lcd.setCursor(10, 2); lcd.print(F("......")); // ... waiting for GPS
  lcd.setCursor(0, 3); lcd.print(F("                   ")); // timezone info not yet set, so blank it out
  delay(1000);

  gpsBaud = gpsBaud1[baudRateNumber];
#ifndef FEATURE_FAKE_SERIAL_GPS_IN
  Serial1.begin(gpsBaud);
#else
  Serial.begin(gpsBaud);  // for faking GPS data from software simulator
#endif

  oldNow = now();  

// from SatelliteTracker (TinyGPS++ example)
// Initialize all the uninitialized TinyGPSCustom objects
    for (int i=0; i<4; ++i)
    {
      satNumber[i].begin(gps, "GPGSV", 4 + 4 * i); // offsets 4, 8, 12, 16
      elevation[i].begin(gps, "GPGSV", 5 + 4 * i); // offsets 5, 9, 13, 17
      azimuth[i].begin(  gps, "GPGSV", 6 + 4 * i); // offsets 6, 10, 14, 18
      snr[i].begin(      gps, "GPGSV", 7 + 4 * i); // offsets 7, 11, 15, 19
    }
  

// Serial output is only used for debugging:

#ifdef FEATURE_SERIAL_PLANETARY
  Serial.begin(115200);
  Serial.println(F("Planet debug"));
#endif

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
  Serial.println(F("menu "));
#endif

#ifdef FEATURE_SERIAL_TIME
  Serial.begin(115200);
  Serial.println(F("Time debug"));
#endif

#ifdef FEATURE_SERIAL_MATH
  Serial.begin(115200);
  Serial.println(F("Math debug"));
#endif

#ifdef FEATURE_SERIAL_LUNARECLIPSE
  Serial.begin(115200);
  Serial.println(F("Moon eclipse debug"));
#endif

#ifdef FEATURE_SERIAL_EQUATIO
  Serial.begin(115200);
  Serial.println(F("Equation of Time debug"));
#endif

#ifdef FEATURE_SERIAL_MENU
    for(int i=0; i < sizeof(languages)/2 ; i++)   Serial.println(languages[i]);
#endif

#ifdef FEATURE_DATE_PER_SECOND // for stepping date quickly and check calender function
  dateIteration = 0;
#endif

}

////////////////////////////////////// L O O P //////////////////////////////////////////////////////////////////
void loop() {

#ifdef FEATURE_POTENTIOMETER
  int potVal = analogRead(potentiometer);   // read the input pin for control of backlight
  backlightVal = max(backlightVal, 1); // to ensure there is some visibility
  #ifdef FEATURE_SERIAL_MENU
      Serial.print(F("backlightVal (backlight) ")); Serial.println(backlightVal);
  #endif
  backlightVal = potVal/4; // backlightVal values go from 0 to 1023 (from analogRead), analogWrite values from 0 to 255
  analogWrite(LCD_PWM, backlightVal);
#endif

#ifdef FEATURE_BUTTONS // may be in addition to rotary encoder
// these are separate buttons, not the encoder button
  byte button = AnalogButtonRead(0); // using K3NG function
  if (button == 2) { // increase menu # by one
    dispState = (dispState + 1) % noOfStates;
    lcd.clear();
    oldMinute = -1; // to get immediate display of some info
    lcd.setCursor(18, 3); PrintFixedWidth(lcd,dispState,2);  // lower left-hand corner
    delay(300); // was 300
  }
  else if (button == 1) { // decrease menu # by one
    dispState = (dispState - 1) % noOfStates;;
    lcd.clear();
    oldMinute = -1; // to get immediate display of some info
    if (dispState < 0) dispState += noOfStates;
    lcd.setCursor(18, 3); lcd.print(dispState);
    delay(300);
  }
#endif // FEATURE_BUTTONS 
    
  volatile unsigned char rotaryResult = r.process(); 
  if (rotaryResult) {                         // change clock face number by rotation
    
  //        Serial.print("Rotary engaged, dispState ");Serial.println(dispState);
  
    {
      if (rotaryResult == DIR_CCW) {                // Counter clockwise: decrease screen number
        dispState = (dispState - 1) % noOfStates;
        lcd.clear();
        oldMinute = -1; // to get immediate display of some info
        if (dispState < 0) dispState += noOfStates;
        lcd.setCursor(18, 3); PrintFixedWidth(lcd,dispState,2); // lower left-hand corner
        
        if (dispState == menuOrder[ScreenDemoClock]) 
        {
            demoDispState = dispState;    // start demo
            oldNow = now();               // reset timer for time between screens in demo mode
        }
        #ifdef FEATURE_SERIAL_MENU
              Serial.println(F("CCW"));
        #endif
        //delay(50);
      }
      else                                          // Clockwise: increase screen number
      {
        dispState = (dispState + 1) % noOfStates;
        lcd.clear();
        oldMinute = -1; // to get immediate display of some info
        lcd.setCursor(18, 3); PrintFixedWidth(lcd,dispState,2); // lower left-hand corner
        
        if (dispState == menuOrder[ScreenDemoClock]) 
        {
            demoDispState = dispState;    // start demo
            oldNow = now();               // reset timer for time between screens in demo mode
        }
        
        #ifdef FEATURE_SERIAL_MENU
              Serial.println(F("CW"));
        #endif
        //delay(50);
      }
    }
  }

  if (r.buttonPressedReleased(500)) {        // 500 ms = long press for reset of processor
      lcd.clear();
      lcd.print(" *** R E S E T *** ");
      delay(1000);
      resetFunc();  // call reset
  }      
       
  if (r.buttonPressedReleased(25)) RotarySetup(); // call setup, 25ms = debounce_delay

else {  //  no button/rotary touched, -> second last } of void loop() 

// main code for updating time

#ifndef FEATURE_FAKE_SERIAL_GPS_IN
   while (Serial1.available()) {
      if (gps.encode(Serial1.read())) { // process gps messages from hw GPS
#else
    while (Serial.available()) {
      if (gps.encode(Serial.read())) { // process gps messages from sw GPS emulator
#endif

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// Necessary for initializing GPSInfo() properly 22.09.2022:

  if (totalGPGSVMessages.isUpdated())
    {
      for (int i=0; i<4; ++i)
      {
        int no = atoi(satNumber[i].value());

        #ifdef FEATURE_SERIAL_GPS
              Serial.print(F("SatNumber is ")); Serial.print(no);
        #endif
              
        if (no >= 1 && no <= MAX_SATELLITES)
        {
          sats[no-1].elevation = atoi(elevation[i].value());
          sats[no-1].azimuth = atoi(azimuth[i].value());
          sats[no-1].snr = atoi(snr[i].value());
                sats[no-1].snr = min(sats[no-1].snr, 99);    // to limit wild values 30.05.2023
          sats[no-1].active = true;
        }
        #ifdef FEATURE_SERIAL_GPS
              Serial.print(F(", SNR ")); Serial.println(sats[no-1].snr);
        #endif
      }
    }
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

   // when GPS reports new data...
    unsigned long age;
  
    hourGPS = gps.time.hour();
    minuteGPS = gps.time.minute();
    secondGPS = gps.time.second();
    dayGPS = gps.date.day() ;
    monthGPS = gps.date.month() ;
    yearGPS = gps.date.year() ;
    age = gps.location.age();            // age in millisecs 

        
#ifdef AUTO_UTC_OFFSET  // the usual mode
    utcOffset = local[timeZoneNumber] / long(60) - utc / long(60); // min, order of calculation is important
#else
    local[timeZoneNumber] = utc + utcOffset * 60; // utcOffset in minutes is set manually in clock_options.h (was in clock_zone.h)
#endif


#ifdef FEATURE_SERIAL_TIME
        Serial.print(F("utc         ")); Serial.println(utc);
        Serial.print(F("local       ")); Serial.println(local[timeZoneNumber]);
        Serial.print(F("diff [sec]  ")); Serial.println(long(local[timeZoneNumber] - utc));
//        Serial.print(F("diff,m1 ")); Serial.println(long(local[timeZoneNumber] - utc) / long(60));
        Serial.print(F("diff [min]  ")); Serial.println(long(local[timeZoneNumber]) / long(60) - long(utc) / long(60));

        Serial.print(F("utcOffset: "));
        Serial.println(utcOffset);
        Serial.print(F("The time zone is: ")); Serial.print(timeZoneNumber); Serial.print(F(" ")); Serial.println(tcr[timeZoneNumber] -> abbrev);
#endif

    if (age < 500) {                                // younger than 0.5 sec
      // set the Time to the latest GPS reading
      setTime(hourGPS, minuteGPS, secondGPS, dayGPS, monthGPS, yearGPS); // Versions from 17.04.2020+: Arduino time = utc
      weekdayGPS = weekday();

      // only executed whenever GPS is updated (can probably be done more seldom, e.g. every 10 sec)
      #include "clock_timezone.h"      // setup for multiple time zones with daylight saving rules, was clock_zone.h
                                       // must have correct date and time from GPS, also sets numTimeZones 
                                       // moved from just before if (age < 500), 7.6.2023, avoids overwriting tcr->abbrev in a few cases
   
    }
    } // (gps.encode(Serial.read()))
  }   // while (Serial.available())
            
    utc = now();  // updated even if GPS data stream stops in order to avoid frozen UTC display
    
    if (timeStatus() != timeNotSet) {
      if (now() != prevDisplay) { //update the display only if the time has changed. i.e. every second
        prevDisplay = now();

// this is for jumping from screen to screen in demo Mode:
        if (now() >= oldNow + dwellTimeDemo)  // demo mode: increment screen number
        {
          #ifdef FEATURE_SERIAL_MENU
              Serial.print(F("demoDispState ")); Serial.println(demoDispState);
              Serial.print(F("dispState ")); Serial.print(dispState);Serial.print(" ");Serial.println(menuOrder[ScreenDemoClock]);
          #endif
          demoDispState += 1;                                     // increment screen number in demo mode
          if (demoDispState < 0) demoDispState += noOfStates;     // but not below the lowest number
          demoDispState = demoDispState%noOfStates;               // or above the largest number
          oldNow = now(); 
         // oldMinute = -1; // to get immediate display of some info
          if (dispState == menuOrder[ScreenDemoClock])            // check that we are in demo mode
          {
            lcd.clear();  // clear if demo just started ?
            oldMinute = -1; // to get immediate display of some info. Moved here 27.6.2023
          }
        }  

    /////////////////////////////////////////////// USER INTERFACE /////////////////////////////////////////////////////////
#ifdef FEATURE_SERIAL_MENU
 //       Serial.println(F("menuOrder: "));
//        for (iiii = 0; iiii < noOfStates; iiii += 1) Serial.println(menuOrder[iiii]);
        Serial.print(F("dispState ")); Serial.println(dispState);
        Serial.println((dispState % noOfStates));
        Serial.println(menuOrder[dispState % noOfStates]);
#endif

    ////////////// Menu system ////////////////////////////////////////////////////////////////////////////////
    ////////////// This is the order of the menu system unless menuOrder[] contains information to the contrary

        screenSelect(dispState, 0);             // select right routine for chosen screen, 0: not demo mode

      }   // if (now() != prevDisplay) 
    }     // if (timeStatus() != timeNotSet)
    
  }       // else // >100 lines up

}         // end loop


////////////////////////////////////// END LOOP //////////////////////////////////////////////////////////////////

void screenSelect(int dispState, int DemoMode) // menu System - called at end of loop and from DemoClock
{
        if      ((dispState) == menuOrder[ScreenLocalUTC ])     LocalUTC(0);     // local time, date; UTC, locator
        else if ((dispState) == menuOrder[ScreenLocalUTCWeek])  LocalUTC(1);     // local time, date; UTC, week #
        else if ((dispState) == menuOrder[ScreenUTCLocator])    UTCLocator();    // UTC, locator, # sats

        // Sun, moon:
        else if ((dispState) == menuOrder[ScreenLocalSun])      LocalSun(0);      // local time, sun x 3
        else if ((dispState) == menuOrder[ScreenLocalSunMoon])  LocalSunMoon();  // local time, sun, moon
        else if ((dispState) == menuOrder[ScreenLocalMoon])     LocalMoon();     // local time, moon size and elevation
        else if ((dispState) == menuOrder[ScreenMoonRiseSet])   MoonRiseSet();   // Moon rises and sets at these times

        // Nice to have
        else if ((dispState) == menuOrder[ScreenTimeZones])     TimeZones();     // Other time zones

        // Fancy, sometimes near unreadable displays, fun to program, and fun to look at:
        else if ((dispState) == menuOrder[ScreenBinary])        Binary(2);       // Binary, horizontal, display of time
        else if ((dispState) == menuOrder[ScreenBinaryHorBCD])  Binary(1);       // BCD, horizontal, display of time
        else if ((dispState) == menuOrder[ScreenBinaryVertBCD]) Binary(0);       // BCD vertical display of time
        else if ((dispState) == menuOrder[ScreenBar])           Bar();           // horizontal bar
        else if ((dispState) == menuOrder[ScreenMengenLehrUhr]) MengenLehrUhr(); // set theory clock
        else if ((dispState) == menuOrder[ScreenLinearUhr])     LinearUhr();     // Linear clock

        // debugging:
        else if ((dispState) == menuOrder[ScreenInternalTime])  InternalTime();  // Internal time - for debugging
        else if ((dispState) == menuOrder[ScreenCodeStatus])    CodeStatus();   //

        // GPS Location
        else if ((dispState) == menuOrder[ScreenUTCPosition])   UTCPosition();   // position

        // WSPR and beacons:
        else if ((dispState) == menuOrder[ScreenNCDXFBeacons2]) NCDXFBeacons(2); // UTC + NCDXF beacons, 18-28 MHz
        else if ((dispState) == menuOrder[ScreenNCDXFBeacons1]) NCDXFBeacons(1); // UTC + NCDXF beacons, 14-21 MHz
        else if ((dispState) == menuOrder[ScreenWSPRsequence])  WSPRsequence();  // UTC + Coordinated WSPR band/frequency (20 min cycle)

        else if ((dispState) == menuOrder[ScreenHex])           HexOctalClock(0); // Hex clock
        else if ((dispState) == menuOrder[ScreenOctal])         HexOctalClock(1); // Octal clock
        else if ((dispState) == menuOrder[ScreenHexOctalClock]) HexOctalClock(3); // 3-in-1: Hex-Octal-Binary clock

        else if ((dispState) == menuOrder[ScreenEasterDates])   EasterDates(yearGPS); // Gregorian and Julian Easter Sunday

        else if ((dispState) == menuOrder[ScreenLocalSunSimpler]) LocalSun(1);    // local time, sun x 3 - simpler layout
        else if ((dispState) == menuOrder[ScreenLocalSunAzEl]) LocalSunAzEl();    // local time, sun az, el
        
        else if ((dispState) == menuOrder[ScreenRoman])         Roman();          // Local time in Roman numerals
        else if ((dispState) == menuOrder[ScreenMathClockAdd])  MathClock(0);     // Math clock: add
        else if ((dispState) == menuOrder[ScreenMathClockSubtract]) MathClock(1); // Math clock: subtract/add
        else if ((dispState) == menuOrder[ScreenMathClockMultiply]) MathClock(2); // Math clock: multiply/subtract/add
        else if ((dispState) == menuOrder[ScreenMathClockDivide]) MathClock(3);   // Math clock: divide/multiply/subtract
        
        else if ((dispState) == menuOrder[ScreenLunarEclipse])  LunarEclipse();   // time for lunar eclipse
        else if ((dispState) == menuOrder[ScreenSidereal])      Sidereal();       // sidereal and solar time
        else if ((dispState) == menuOrder[ScreenMorse])         Morse();          // morse time
        else if ((dispState) == menuOrder[ScreenWordClock])     WordClock();      // time in clear text

        else if ((dispState) == menuOrder[ScreenGPSInfo])       GPSInfo();        // Show technical GPS Info
        else if ((dispState) == menuOrder[ScreenISOHebIslam])   ISOHebIslam();    // ISO, Hebrew, Islamic calendar
        else if ((dispState) == menuOrder[ScreenPlanetsInner])  PlanetVisibility(1); // Inner planet data
        else if ((dispState) == menuOrder[ScreenPlanetsOuter])  PlanetVisibility(0); // Inner planet data
        else if ((dispState) == menuOrder[ScreenChemical])      LocalUTC(2);      // local time + chemical element
 //       else if ((dispState) == menuOrder[ScreenReminder])      Reminder();     // Show dates to rememember and elapsed time
      
        else if ((dispState) == menuOrder[ScreenDemoClock])                       // last menu item
        {
           if (!DemoMode) DemoClock(0);       // Start demo of all clock functions if not already in DemoMode
           else           DemoClock(1);
        }
        else    //added 12.7.2023
        {
            lcd.setCursor(0,0); lcd.print(F("Warning:"));
            lcd.setCursor(0,1); lcd.print(F("  Invalid screen #")); // due to repeated entry in menuStruct[]
        }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////

// Menu item ///////////////////////////////////////////////////////////////////////////////////////////
void LocalUTC(           // local time, UTC, locator, option: ISO week #
                int mode // 0 for original version
                         // 1 for added week # (new 3.9.2022)
                         // 2 for Chemical element on last two lines (27.3.2023)
) { // 
      
#ifndef FEATURE_DATE_PER_SECOND 
  local[timeZoneNumber] = now() + utcOffset * 60;

#else                             // for stepping date quickly and check calender function
  local[timeZoneNumber] = now() + utcOffset * 60 + dateIteration*86400; //int(86400.0/5.0); // fake local time by stepping per day
  dateIteration = dateIteration + 1;
//  Serial.print(dateIteration); Serial.print(": ");
//  Serial.println(local);
#endif
  
  Hour = hour(local[timeZoneNumber]);
  Minute = minute(local[timeZoneNumber]);
  Seconds = second(local[timeZoneNumber]);

  lcd.setCursor(0, 0); // top line *********
  sprintf(textBuffer, "%02d%c%02d%c%02d", Hour, dateTimeFormat[dateFormat].hourSep, Minute, dateTimeFormat[dateFormat].minSep, Seconds);
  lcd.print(textBuffer);
  lcd.print(F("      "));
  // local date
  Day = day(local[timeZoneNumber]);
  Month = month(local[timeZoneNumber]);
  Year = year(local[timeZoneNumber]);
  //if (dayGPS != 0)        // was this (26.05.2023)
  //if (gps.date.isValid()) // same slow response as old test (26.05.2023)
  {

// right-adjusted long day name:

    if (languageNumber >=0)
    {
          nativeDayLong(local[timeZoneNumber]);  // output in "today"
          sprintf(todayFormatted,"%12s", today);
    }
    else // English
    {

    #ifdef FEATURE_DAY_PER_SECOND  //      fake the day -- for testing only
          sprintf(todayFormatted, "%12s", dayStr( 1+(local[timeZoneNumber]/2)%7 )); // change every two seconds
    #else
          sprintf(todayFormatted, "%12s", dayStr(weekday(local[timeZoneNumber])));  // normal
    #endif
    }

    lcd.setCursor(8, 0);
    lcd.print(todayFormatted);

    lcd.setCursor(0, 1); //////// line 2
    if (mode==1)
  {
    // option added 3.9.2022 - ISO week # on second line
        GregorianDate a(month(local[timeZoneNumber]), day(local[timeZoneNumber]), year(local[timeZoneNumber]));      
        IsoDate ISO(a);
        if (!strcmp(languages[languageNumber],"no")) lcd.print(F("Uke "));
        else                                         lcd.print(F("Week "));
    
        lcd.print(ISO.GetWeek()); lcd.print(" "); // added space 15.01.2023 - needed for 1-digit week numbers
  }

  lcd.setCursor(9, 1); lcd.print(" "); LcdDate(Day, Month, Year); 
  
  if (mode == 2) // Chemical elements
  {
      // Inspired by https://www.instructables.com/Periodic-Table-Clock/
      // added 4.4.2023

      int Hr, Min, Sec;
      Hr = (int)Hour;
      Min = (int)Minute;
      Sec = (int)Seconds;
      
      lcd.setCursor(0, 2); //////// line 3
      LCDChemicalElement(Hr, Min, Sec);       // Display chemical 1- or 2-letter element abbreviation       
      LCDChemicalGroupPeriod(Sec);            // Display chemical group (column) and period (row)
        
      lcd.setCursor(0,3); lcd.print(F("      ")); // line 4
      LCDChemicalElementName(Sec);            // Display full name of chemical element
         
      lcd.setCursor(18,3); lcd.print("  ");
  }
      
if (mode != 2)  // 30.05.2023 from LCDUTCTimeLocator -> UTC time + No sats
    {   
          lcd.setCursor(0, 2); lcd.print(F("                    "));
          
          lcd.setCursor(0, 3);
          sprintf(textBuffer, "%02d%c%02d%c%02d UTC ", hourGPS, dateTimeFormat[dateFormat].hourSep, minuteGPS, dateTimeFormat[dateFormat].minSep, secondGPS);
          lcd.print(textBuffer);

          #ifdef FEATURE_SERIAL_GPS
              Serial.println(textBuffer);
          #endif

          if (gps.satellites.isUpdated()) {
            noSats = gps.satellites.value();
            lcd.setCursor(13, 3);   
            }
          else noSats = 0;       
          PrintFixedWidth(lcd, noSats, 2);
          lcd.print(F(" Sats"));
    }
  }
}


// Menu item //////////////////////////////////////////////////////////////////////////////////////////

void UTCLocator() {     // UTC, locator, # satellites

#ifdef FEATURE_FAKE_SERIAL_GPS_IN
      hourGPS = hour(now());
      minuteGPS = minute(now());
      secondGPS = second(now());
#endif

  lcd.setCursor(0, 0); // top line *********
//  if (gps.time.isValid()) {
    sprintf(textBuffer, "%02d%c%02d%c%02d         UTC", hourGPS, dateTimeFormat[dateFormat].hourSep, minuteGPS, dateTimeFormat[dateFormat].minSep, secondGPS);
    lcd.print(textBuffer);
//  }

  // UTC date

  //if (dayGPS != 0)
  {
    lcd.setCursor(0, 1); // line 1
    lcd.print(dayStr(weekdayGPS)); lcd.print(F("   ")); // two more spaces 14.04.2018

    lcd.setCursor(10, 1);
    LcdDate(dayGPS, monthGPS, yearGPS); 
  }
  if (gps.location.isValid()) {

#ifndef DEBUG_MANUAL_POSITION
    latitude = gps.location.lat();
    lon = gps.location.lng();
#else
    latitude = latitude_manual;
    lon      = longitude_manual;
#endif

    //char locator[7];
    Maidenhead(lon, latitude, textBuffer);
    lcd.setCursor(0, 3); // last line *********
    lcd.print(textBuffer); lcd.print(F("       "));
  }
  //  if (gps.satellites.()) { // 16.11.2022
  if (gps.satellites.isUpdated()) {
    noSats = gps.satellites.value();
    lcd.setCursor(13, 3);   
  }
    else noSats = 0;

    PrintFixedWidth(lcd, noSats, 2);
    lcd.print(F(" Sats"));
  }

// Menu item //////////////////////////////////////////////////////////////////////////////////////////
void LocalSun(           // local time, sun x 3
              int mode   // 0 for ScreenLocal
                         // 1 for ScreenLocalSunSimpler 
) {
  //
  // shows Actual (0 deg), Civil (-6 deg), and Nautical (-12 deg) sun rise/set
  //
  if (mode == 0)  LcdShortDayDateTimeLocal(0, 2);  // line 0
  else            LcdShortDayDateTimeLocal(0, 0);

  
  if (gps.location.isValid()) {
    if (minuteGPS != oldMinute) {
#ifndef DEBUG_MANUAL_POSITION
      latitude = gps.location.lat();
      lon = gps.location.lng();
#else
      latitude = latitude_manual;
      lon      = longitude_manual;
#endif

if (mode == 0)
{
      LcdSolarRiseSet(1, ' ',ScreenLocalSun);
      LcdSolarRiseSet(2, 'C',ScreenLocalSun);
      LcdSolarRiseSet(3, 'N',ScreenLocalSun);
}
else
{
      LcdSolarRiseSet(1, ' ',ScreenLocalSunSimpler);
      LcdSolarRiseSet(2, 'C',ScreenLocalSunSimpler);
      LcdSolarRiseSet(3, 'N',ScreenLocalSunSimpler);
}
    }
  }
  oldMinute = minuteGPS;
}


/*****
Purpose: Menu item
Finds local time, and three different sun rise/set times

Argument List: Global variables 

Return value: Displays on LCD
*****/

void LocalSunAzEl() { // local time, sun x 3
  //
  // shows Actual (0 deg), Civil (-6 deg), and Nautical (-12 deg) sun rise/set
  //
  LcdShortDayDateTimeLocal(0, 0);  // line 0
  if (gps.location.isValid()) {
    if (minuteGPS != oldMinute) {
    #ifndef DEBUG_MANUAL_POSITION
          latitude = gps.location.lat();
          lon = gps.location.lng();
    #else
          latitude = latitude_manual;
          lon      = longitude_manual;
    #endif

    LcdSolarRiseSet(1, ' ', ScreenLocalSunAzEl); // Actual Rise, Set times<
    LcdSolarRiseSet(2, 'O', ScreenLocalSunAzEl); //Noon info 
    LcdSolarRiseSet(3, 'Z', ScreenLocalSunAzEl); //Current Az El info

    }
  }
  oldMinute = minuteGPS;
}

/*****
Purpose: Menu item
Finds local time, Actual solar rise/set time and azimuth; lunar rise, set, illumination and azimuth

Argument List: Global variables 

Return value: Displays on LCD
*****/

void LocalSunMoon() { // local time, sun, moon
  //
  // shows solar rise/set in a chosen definition (Actual, Civil, ...)
  //

  LcdShortDayDateTimeLocal(0, 0);  // line 0, (was time offset 2) to the left

  if (gps.location.isValid()) {
    if (minuteGPS != oldMinute) {

#ifndef DEBUG_MANUAL_POSITION
      latitude = gps.location.lat();
      lon = gps.location.lng();
#else
      latitude = latitude_manual;
      lon      = longitude_manual;
#endif

      LcdSolarRiseSet(1, ' ', ScreenLocalSunMoon); // line 1, Actual rise time
//      LcdSolarRiseSet(2, 'C', 0); // line 2

      // MOON

      lcd.setCursor(0, 3);  // last line
      lcd.print(F("M "));

      // next rise / set
      short pRise, pSet, pTime;
      double rAz, sAz;
      int order;   

      GetNextRiseSet(&pRise, &rAz, &pSet, &sAz, &order);
#ifdef FEATURE_SERIAL_MOON
      Serial.print(F("LocalSunMoon: order: ")); Serial.println(order);
#endif

      local[timeZoneNumber] = now() + utcOffset * 60;
      Hour = hour(local[timeZoneNumber]);
      Minute = minute(local[timeZoneNumber]);

      int packedTime = Hour * 100 + Minute;

      lcd.setCursor(2, 3); // last line

      // find next event
      if (order == 1)  // Rise
      {
        pTime = pRise;  lcd.write(UP_ARROW);
      }
      else // Set (or order not initialized correctly)
      {
        pTime = pSet; lcd.write(DOWN_ARROW);
      }

      if (pTime > -1)
      {
        int pHr  = pTime / 100;
        int pMin = pTime - 100 * pHr;
        PrintFixedWidth(lcd, pHr, 2); lcd.print(dateTimeFormat[dateFormat].hourSep); PrintFixedWidth(lcd, pMin, 2, '0'); lcd.print(" ");
      }
      else lcd.print(F(" - "));

      float PhaseM, PercentPhaseM;
      MoonPhaseAccurate(PhaseM, PercentPhaseM);
      //MoonPhase(PhaseM, PercentPhaseM);

#ifdef FEATURE_SERIAL_MOON
      Serial.println(F("LocalSunMoon: "));
      Serial.print(F(" PhaseM, PercentPhaseM "));
      Serial.print(PhaseM); Serial.print(F(", ")); Serial.println(PercentPhaseM);
#endif

      lcd.setCursor(9, 3);
     
      MoonWaxWane(PhaseM); //arrow up/down or ' ' (space)
      MoonSymbol(PhaseM); // (, O, ), symbol

      PrintFixedWidth(lcd, (int)round(PercentPhaseM), 3);
      lcd.print("%");

      UpdateMoonPosition();
      lcd.setCursor(16, 3);
      PrintFixedWidth(lcd, (int)round(moon_elevation), 3);
      lcd.write(DEGREE);
    }
  }
  oldMinute = minuteGPS;
}

/*****
Purpose: Menu item
Finds local time, and lots of lunar info

Argument List: Global variables 

Return value: Displays on LCD
*****/

void LocalMoon() { // local time, moon phase, elevation, next rise/set

//  String textbuf;
  float percentage;

  LcdShortDayDateTimeLocal(0, 0); // line 0, (was 1 position left) to line up with next lines

  if (gps.location.isValid()) {
    if (minuteGPS != oldMinute) {  // update display every minute

      // days since last new moon
      float Phase, PercentPhase;

      UpdateMoonPosition();

      lcd.setCursor(0, 3); // line 3
      //
      //        lcd.print("  ");
      //        textbuf = String(moon_dist, 0);
      //        lcd.print(textbuf); lcd.print(" km");

      lcd.print(" ");
      PrintFixedWidth(lcd, int(round(moon_dist / 4067.0)), 3);
      lcd.print("% ");

      PrintFixedWidth(lcd, int(round(moon_dist / 1000.0)), 3);
      lcd.print(F("'km "));


      MoonPhase(Phase, PercentPhase);

      lcd.setCursor(14, 3);
      MoonWaxWane(Phase); // arrow
      MoonSymbol(Phase);  // (,0,)

      lcd.setCursor(16, 3);
      PrintFixedWidth(lcd, (int)(abs(round(PercentPhase))), 3);
      lcd.print("%");

#ifndef DEBUG_MANUAL_POSITION
      latitude = gps.location.lat();
      lon = gps.location.lng();
#else
      latitude = latitude_manual;
      lon      = longitude_manual;
#endif

      lcd.setCursor(0, 1); // line 1

      lcd.print(F("M El "));
      lcd.setCursor(4, 1);
      PrintFixedWidth(lcd, (int)round(moon_elevation), 4);
      lcd.write(DEGREE);

      lcd.setCursor(13, 1);
      lcd.print(F("Az "));
      PrintFixedWidth(lcd, (int)round(moon_azimuth), 3);
      lcd.write(DEGREE);

      // Moon rise or set time:
      short pRise, pSet;
      int order;
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
      local[timeZoneNumber] = now() + utcOffset * 60;
      Hour = hour(local[timeZoneNumber]);
      Minute = minute(local[timeZoneNumber]);

      int packedTime = Hour * 100 + Minute;

      // find next event
      if (order == 1)
        // Rise
      {
        pTime = pRise;  Symb = UP_ARROW; Az = rAz;
      }
      else
      {
        pTime = pSet; Symb = DOWN_ARROW; Az = sAz;
      }

      lcd.setCursor(2, 2); // line 2

      if (pTime > -1)
      {
        int pHr  = pTime / 100;
        int pMin = pTime - 100 * pHr;

        lcd.write(Symb); lcd.print(F("   "));
        PrintFixedWidth(lcd, pHr, 2); lcd.print(dateTimeFormat[dateFormat].hourSep); PrintFixedWidth(lcd, pMin, 2, '0'); lcd.print(" ");
        lcd.setCursor(13, 2);
        lcd.print(F("Az "));
        PrintFixedWidth(lcd, (int)round(Az), 3);
        lcd.write(DEGREE);
      }
      else lcd.print(F("  No Rise/Set       "));

      oldMinute = minuteGPS;
    }
  }
}

/*****
Purpose: Menu item
Finds lunar rise/set times for the next couple of days

Argument List: Global variables 

Return value: Displays on LCD

Issues: follows UTC day/night - may get incorrect sorting of rise/set times if timezone is very different from UTC?
*****/

void MoonRiseSet(void) {

  if (gps.location.isValid()) {

#ifndef DEBUG_MANUAL_POSITION
    latitude = gps.location.lat();
    lon = gps.location.lng();
#else
    latitude = latitude_manual;
    lon      = longitude_manual;
#endif

    if (minuteGPS != oldMinute) {

      short pRise, pSet, pRise2, pSet2, packedTime; // time in compact format '100*hr + min'
      double rAz, sAz, rAz2, sAz2;

      local[timeZoneNumber] = now() + utcOffset * 60;
      Hour = hour(local[timeZoneNumber]);
      Minute = minute(local[timeZoneNumber]);

      packedTime = Hour * 100 + Minute; // local time 19.11.2021

      // ***** rise/set for this UTC day:
      
      GetMoonRiseSetTimes(float(utcOffset) / 60.0, latitude, lon, &pRise, &rAz, &pSet, &sAz);

      lcd.setCursor(0, 0); // top line
      lcd.print(F("M "));

      int lineNo = 0;
      int lineUsed = 0;

      int MoonRiseHr  = pRise / 100;
      int MoonRiseMin = pRise - 100 * MoonRiseHr;
      int MoonSetHr  = pSet / 100;
      int MoonSetMin = pSet - 100 * MoonSetHr;   

      lcd.setCursor(2, lineNo);     // row no 0

 // Determine if there is two, one or no rise/set events on the present date and which are in the future

      int NoOfEvents = 0;
      if (packedTime < pRise | packedTime < pSet) NoOfEvents = 1;
      if (packedTime < pRise & packedTime < pSet) NoOfEvents = 2;

#ifdef FEATURE_SERIAL_MOON
  Serial.println(NoOfEvents);
#endif

      if (NoOfEvents == 2)
      {
          if (pRise < pSet) lcd.setCursor(2, lineNo);     // row no 0
          else              lcd.setCursor(2, lineNo + 1); // row no 1  
      }
      
      if (pRise > -1 & pRise > packedTime) // only show a future event
      {        
        lcd.write(UP_ARROW); lcd.print(" ");
        PrintFixedWidth(lcd, MoonRiseHr, 2, '0');   lcd.print(dateTimeFormat[dateFormat].hourSep);
        PrintFixedWidth(lcd, MoonRiseMin, 2, '0');  lcd.print(F("  "));
        PrintFixedWidth(lcd, (int)round(rAz), 4);
        lcd.write(DEGREE); lcd.print(F("  "));
        lineUsed = lineUsed + 1;
      }
//      else if (pRise < 0)
//      {
//        lcd.print(pRise); lcd.print(F("              "));
//      }
#ifdef FEATURE_SERIAL_MOON
  Serial.println(lineUsed);
#endif

      if (NoOfEvents == 2)
      {
        if (pRise < pSet) lcd.setCursor(2, lineNo + 1); // row no 1  
        else              lcd.setCursor(2, lineNo);     // row no 0
      }
      
      if (pSet > -1 & pSet > packedTime) // only show a future event
      {      
        lcd.write(DOWN_ARROW);  lcd.print(" ");
        PrintFixedWidth(lcd, MoonSetHr, 2, '0');   lcd.print(dateTimeFormat[dateFormat].hourSep); // doesn't handle 00:48 well with ' ' as separator
        PrintFixedWidth(lcd, MoonSetMin, 2, '0');  lcd.print(F("  "));
        PrintFixedWidth(lcd, (int)round(sAz), 4); lcd.write(DEGREE); lcd.print(F("  "));
        lineUsed = lineUsed + 1;
      }
//      else if (pSet < 0)
//      {
//        lcd.print(pSet); lcd.print(F("              "));
//      }

#ifdef FEATURE_SERIAL_MOON
  Serial.println(lineUsed);
#endif

      lineNo = lineUsed;

      // ****** rise/set for next UTC day:
      
      GetMoonRiseSetTimes(float(utcOffset) / 60.0 - 24.0, latitude, lon, &pRise2, &rAz2, &pSet2, &sAz2);

      // Rise and set times for moon:

      MoonRiseHr  = pRise2 / 100;
      MoonRiseMin = pRise2 - 100 * MoonRiseHr;
      MoonSetHr  = pSet2 / 100;
      MoonSetMin = pSet2 - 100 * MoonSetHr;

      if (pRise2 < pSet2) lcd.setCursor(2, lineNo);
      else              lcd.setCursor(2, lineNo + 1);

      lcd.write(UP_ARROW); lcd.print(" ");

      if (pRise2 > -1) {
        PrintFixedWidth(lcd, MoonRiseHr, 2, '0');   lcd.print(dateTimeFormat[dateFormat].hourSep);
        PrintFixedWidth(lcd, MoonRiseMin, 2, '0');  lcd.print(F("  "));
        PrintFixedWidth(lcd, (int)round(rAz2), 4);
        lcd.write(DEGREE); lcd.print(F("  "));
         lineUsed = lineUsed + 1;
      }
//      else
//      {
//        lcd.print(pRise2); lcd.print("              ");
//      }

      if (pRise2 < pSet2) lcd.setCursor(2, lineNo + 1);
      else              lcd.setCursor(2, lineNo);

      lcd.write(DOWN_ARROW);  lcd.print(" ");
      if (pSet2 > -1) {
        PrintFixedWidth(lcd, MoonSetHr, 2, '0');   lcd.print(dateTimeFormat[dateFormat].hourSep); // doesn't handle 00:48 well with ' ' as separator
        PrintFixedWidth(lcd, MoonSetMin, 2, '0');   lcd.print("  ");
        PrintFixedWidth(lcd, (int)round(sAz2), 4); lcd.write(DEGREE); lcd.print("  ");
         lineUsed = lineUsed + 1;
      }
//      else
//      {
//        lcd.print(pSet2); lcd.print("              ");
//      }

    
      lineNo = lineUsed;

      if (lineNo <= 3)
      // **** if there is room add a line or two more
      // rise/set for next UTC day:
      {     
        GetMoonRiseSetTimes(float(utcOffset) / 60.0 - 48.0, latitude, lon, &pRise2, &rAz2, &pSet2, &sAz2);
  
        // Rise and set times for moon:
  
        MoonRiseHr  = pRise2 / 100;
        MoonRiseMin = pRise2 - 100 * MoonRiseHr;
        MoonSetHr  = pSet2 / 100;
        MoonSetMin = pSet2 - 100 * MoonSetHr;
  
        if (pRise2 < pSet2) 
        {
          lcd.setCursor(2, lineNo);
          lineUsed = lineNo;
        }
        else
        {
          lcd.setCursor(2, lineNo + 1);
          lineUsed = lineNo + 1;
        }

        if (lineUsed <= 3)
        {
          lcd.write(UP_ARROW); lcd.print(" ");
    
          if (pRise2 > -1) {
            PrintFixedWidth(lcd, MoonRiseHr, 2, '0');   lcd.print(dateTimeFormat[dateFormat].hourSep);
            PrintFixedWidth(lcd, MoonRiseMin, 2, '0');  lcd.print("  ");
            PrintFixedWidth(lcd, (int)round(rAz2), 4);
            lcd.write(DEGREE); lcd.print("  ");
          }
          else
          {
            lcd.print(pRise2); lcd.print(F("              "));
          }
        }
  
        if (pRise2 < pSet2) 
        {
          lcd.setCursor(2, lineNo + 1);
          lineUsed = lineNo + 1;
        }
        else
        {
          lcd.setCursor(2, lineNo);
          lineUsed = lineNo;
        }

        if (lineUsed <=3)
        {
          lcd.write(DOWN_ARROW);  lcd.print(" ");
          if (pSet2 > -1) {
            PrintFixedWidth(lcd, MoonSetHr, 2, '0');   lcd.print(dateTimeFormat[dateFormat].hourSep); // doesn't handle 00:48 well with ' ' as separator
            PrintFixedWidth(lcd, MoonSetMin, 2, '0');   lcd.print("  ");
            PrintFixedWidth(lcd, (int)round(sAz2), 4); lcd.write(DEGREE); lcd.print("  ");
          }
          else
          {
            lcd.print(pSet2); lcd.print(F("              "));
          }
        }

      }
      
      lcd.setCursor(18, 3); lcd.print("  ");
    }
  }
  oldMinute = minuteGPS;
}

 // Menu item ///////////////////////////////////////////////////////////////////////////////////////////

/*****
Purpose: Display time in 6 time zones on LCD, 4 user selectable

Argument List: none

Return value: Display on LCD
*****/

void TimeZones() { // local time, UTC, + 4 more time zones (user selectable)
//  https://github.com/khoih-prog/Timezone_Generic

  // get local time in many locations

  local[timeZoneNumber] = now() + utcOffset * 60;
  Hour = hour(local[timeZoneNumber]);
  Minute = minute(local[timeZoneNumber]);
  Seconds = second(local[timeZoneNumber]);

  lcd.setCursor(0, 0); // 1. line ********* always time zone set for clock
  sprintf(textBuffer, "%02d%c%02d", Hour, dateTimeFormat[dateFormat].hourSep, Minute);
  lcd.print(textBuffer);
  lcd.print(" ");lcd.print(tcr[timeZoneNumber] -> abbrev);
  
  lcd.setCursor(18,0); // end of line 1 shows seconds
  sprintf(textBuffer, "%02d", Seconds);
  lcd.print(textBuffer);
  
   
  lcd.setCursor(0, 1); // 2. line  always UTC *********

  if (gps.time.isValid()) {
    lcd.setCursor(0, 1);
    sprintf(textBuffer, "%02d%c%02d UTC  ", hourGPS, dateTimeFormat[dateFormat].hourSep,minuteGPS);
    lcd.print(textBuffer);
  }

// First user selectable (e.g. China Standard Time)
  lcd.setCursor(0, 2); // ******** 3. line 
  Hour = hour(local[userTimeZones[0]]);
  Minute = minute(local[userTimeZones[0]]);
  sprintf(textBuffer, "%02d%c%02d ", Hour, dateTimeFormat[dateFormat].hourSep, Minute);
  lcd.print(textBuffer);
  lcd.print(tcr[userTimeZones[0]] -> abbrev);
 

// Second user selectable (e.g. Indian Standard Time)
   lcd.setCursor(10, 2);
   Hour = hour(local[userTimeZones[1]]);
   Minute = minute(local[userTimeZones[1]]);
   sprintf(textBuffer, "%02d%c%02d ", Hour, dateTimeFormat[dateFormat].hourSep, Minute);
   lcd.print(textBuffer);
   lcd.print(tcr[userTimeZones[1]] -> abbrev);
   

// Third user selectable (e.g. US Eastern Time Zone)
  lcd.setCursor(0, 3); //////// line 4
  Hour = hour(local[userTimeZones[2]]);
  Minute = minute(local[userTimeZones[2]]);
  sprintf(textBuffer, "%02d%c%02d ", Hour, dateTimeFormat[dateFormat].hourSep, Minute);
  lcd.print(textBuffer);
  lcd.print(tcr[userTimeZones[2]] -> abbrev);

  lcd.setCursor(19, 3); lcd.print(" "); // blank out rest of menu number in lower right-hand corner

// Fourth user selectable (e.g. Pacific US)
  lcd.setCursor(10, 3);
  Hour = hour(local[userTimeZones[3]]);
  Minute = minute(local[userTimeZones[3]]);
  sprintf(textBuffer, "%02d%c%02d ", Hour, dateTimeFormat[dateFormat].hourSep, Minute);
  lcd.print(textBuffer);
  lcd.print(tcr[userTimeZones[3]] -> abbrev);
       
  oldMinute = minuteGPS;
}

// Menu item ////////////////////////////////////////

/*****
Purpose: Menu item
Finds local time in binary formats hour, minute, second

Argument List: int mode = 0 - vertical BCD
                   mode = 1 - horizontal BCD
                   mode = 2 - horisontal binary

Return value: Displays on LCD
*****/

void Binary(int mode) { // binary local time

  int tens, ones;

  int BinaryTensHour[6], BinaryHour[6], BinaryTensMinute[6], BinaryMinute[6], BinaryTensSeconds[6], BinarySeconds[6];

  // get local time
  local[timeZoneNumber] = now() + utcOffset * 60;
  Hour = hour(local[timeZoneNumber]);
  Minute = minute(local[timeZoneNumber]);
  Seconds = second(local[timeZoneNumber]);

  // convert to BCD

  // must send a variable, not an equation, to DecToBinary as it does in-place arithmetic on input variable
  ones = Hour % 10; tens = (Hour - ones) / 10;
  DecToBinary(ones, BinaryHour);
  DecToBinary(tens, BinaryTensHour);

  ones = Minute % 10; tens = (Minute - ones) / 10;
  DecToBinary(tens, BinaryTensMinute); DecToBinary(ones, BinaryMinute);

  ones = Seconds % 10;   tens = (Seconds - ones) / 10;
  DecToBinary(tens, BinaryTensSeconds); DecToBinary(ones, BinarySeconds);


  if (mode == 0) // vertical digits:
  {
    lcd.setCursor(0, 0); lcd.print(F("BCD"));

    lcd.setCursor(9, 0);
    sprintf(textBuffer, " %1d  %1d  %1d", BinaryHour[2], BinaryMinute[2], BinarySeconds[2]);
    lcd.print(textBuffer);
    lcd.setCursor(19, 0); lcd.print("8");

    //   lcd.setCursor(0,1); lcd.print("hh mm ss");
    lcd.setCursor(9, 1);
    sprintf(textBuffer, " %1d %1d%1d %1d%1d", BinaryHour[3], BinaryTensMinute[3], BinaryMinute[3], BinaryTensSeconds[3], BinarySeconds[3]);
    lcd.print(textBuffer);
    lcd.setCursor(19, 1); lcd.print("4");

    lcd.setCursor(9, 2);
    sprintf(textBuffer, "%1d%1d %1d%1d %1d%1d", BinaryTensHour[4], BinaryHour[4], BinaryTensMinute[4], BinaryMinute[4], BinaryTensSeconds[4], BinarySeconds[4]);
    lcd.print(textBuffer);
    lcd.setCursor(19, 2); lcd.print("2");


    lcd.setCursor(9, 3); //LSB
    sprintf(textBuffer, "%1d%1d %1d%1d %1d%1d  ", BinaryTensHour[5], BinaryHour[5], BinaryTensMinute[5], BinaryMinute[5], BinaryTensSeconds[5], BinarySeconds[5]);
    lcd.print(textBuffer);
    lcd.setCursor(19, 3); lcd.print("1");
  }
  else if (mode == 1)
  {
    //// horizontal BCD digits:

    lcd.setCursor(0, 0); lcd.print(F("BCD"));

    lcd.setCursor(9, 1); sprintf(textBuffer, "  %1d%1d ", BinaryTensHour[4], BinaryTensHour[5] );
    lcd.print(textBuffer);
    sprintf(textBuffer, "%1d%1d%1d%1d H", BinaryHour[2], BinaryHour[3], BinaryHour[4], BinaryHour[5]);
    lcd.print(textBuffer);

    lcd.setCursor(9, 2);  sprintf(textBuffer, " %1d%1d%1d ", BinaryTensMinute[3], BinaryTensMinute[4], BinaryTensMinute[5] );
    lcd.print(textBuffer);
    sprintf(textBuffer, "%1d%1d%1d%1d M", BinaryMinute[2], BinaryMinute[3], BinaryMinute[4], BinaryMinute[5] );
    lcd.print(textBuffer);

    lcd.setCursor(9, 3);  sprintf(textBuffer, " %1d%1d%1d ", BinaryTensSeconds[3], BinaryTensSeconds[4], BinaryTensSeconds[5] );
    lcd.print(textBuffer);
    sprintf(textBuffer, "%1d%1d%1d%1d S", BinarySeconds[2], BinarySeconds[3], BinarySeconds[4], BinarySeconds[5] );
    lcd.print(textBuffer);




    if (Seconds < secondsClockHelp)  // show help: weighting
    {
      lcd.setCursor(9, 0); lcd.print(F(" 421 8421"));
    }
    else
    {
      lcd.setCursor(9, 0); lcd.print(F("         "));
    }
  }
  else
    // horisontal 5/6-bit binary
  {
    // convert to binary:
    DecToBinary(Hour, BinaryHour);
    DecToBinary(Minute, BinaryMinute);
    DecToBinary(Seconds, BinarySeconds);

    lcd.setCursor(13, 1); sprintf(textBuffer, "%1d%1d%1d%1d%1d H", BinaryHour[1], BinaryHour[2], BinaryHour[3], BinaryHour[4], BinaryHour[5]);
    lcd.print(textBuffer);

    lcd.setCursor(12, 2); sprintf(textBuffer, "%1d%1d%1d%1d%1d%1d M", BinaryMinute[0], BinaryMinute[1], BinaryMinute[2], BinaryMinute[3], BinaryMinute[4], BinaryMinute[5]);
    lcd.print(textBuffer);

    lcd.setCursor(12, 3); sprintf(textBuffer, "%1d%1d%1d%1d%1d%1d S", BinarySeconds[0], BinarySeconds[1], BinarySeconds[2], BinarySeconds[3], BinarySeconds[4], BinarySeconds[5] );
    lcd.print(textBuffer);

    lcd.setCursor(0, 0); lcd.print(F("Binary"));

    if (Seconds < secondsClockHelp)  // show help: weighting
    {
      lcd.setCursor(13, 0); lcd.print(F(" 8421"));
    }
    else
    {
      lcd.setCursor(13, 0); lcd.print(F("     "));
    }
  }

  // Common for all modes:

  if (Seconds < secondsClockHelp)  // show time in normal numbers
  {
    sprintf(textBuffer, "%02d%c%02d%c%02d", Hour, dateTimeFormat[dateFormat].hourSep, Minute, dateTimeFormat[dateFormat].minSep, Seconds);
  }
  else
  {
    sprintf(textBuffer, "        ");
  }
  lcd.setCursor(0, 3); // last line *********
  lcd.print(textBuffer);
}

/*****
Purpose: Menu item
Finds local time as a bar display

Argument List: None

Return value: Displays on LCD
*****/

void Bar(void) {
  // get local time

  local[timeZoneNumber] = now() + utcOffset * 60;
  Hour = hour(local[timeZoneNumber]);
  Minute = minute(local[timeZoneNumber]);
  Seconds = second(local[timeZoneNumber]);

  // use a 12 character bar  just like a 12 hour clock with ticks every hour
  // for second ticks use ' " % #
  lcd.setCursor(0, 0);
  int imax = Hour;

  if (Hour == 13 & Minute == 0 & Seconds == 0)
  {
    lcd.print(F("                   "));
    lcd.setCursor(0, 0);
  }

  if (Hour > 12) imax = Hour - 12;

  if (Hour == 0) lcd.print(F("                "));
  for (int i = 0; i < imax; i++) {
    lcd.write(ALL_ON); // fills square
    if (i == 2 | i == 5 | i == 8) lcd.write(ALL_OFF); // empty
  }

  // could have used |, ||, |||, |||| for intermediate symbols by creating new characters
  // like here https://forum.arduino.cc/index.php?topic=180678.0
  // but easier to use something standard

  //  if (Minute/12 == 1) {lcd.write(BIG_DOT);}
  //  else if (Minute/12 == 2) {lcd.print('"');}
  //  else if (Minute/12 == 3) {lcd.write(THREE_LINES);}
  //  else if (Minute/12 == 4) {lcd.write(SQUARE);}

  //  lcd.print(" ");lcd.print(Hour);

  lcd.setCursor(18, 0); lcd.print("1h");

  lcd.setCursor(0, 1);
  imax = Minute / 5;
  if (Minute == 0) lcd.print(F("                "));
  for (int i = 0; i < imax; i++) {
    lcd.write(ALL_ON); // fills square
    if (i == 2 | i == 5 | i == 8) lcd.write(ALL_OFF); // empty
  }
  if (Minute % 5 == 1) {
    lcd.write(BIG_DOT);
  }
  else if (Minute % 5 == 2) {
    lcd.print('"');
  }
  else if (Minute % 5 == 3) {
    lcd.write(THREE_LINES);
  }
  else if (Minute % 5 == 4) {
    lcd.write(SQUARE);
  }
  // lcd.print(" ");lcd.print(Minute);
  lcd.setCursor(18, 1); lcd.print("5m");


  // seconds in 12 characters, with a break every 3 characters
  lcd.setCursor(0, 2);
  imax = Seconds / 5;
  if (Seconds == 0) lcd.print(F("                "));
  for (int i = 0; i < imax; i++) {
    lcd.write(ALL_ON); // fills square
    if (i == 2 | i == 5 | i == 8) lcd.write(ALL_OFF); // empty
  }
  if (Seconds % 5 == 1) {
    lcd.write(BIG_DOT);
  }
  else if (Seconds % 5 == 2) {
    lcd.print('"');
  }
  else if (Seconds % 5 == 3) {
    lcd.write(THREE_LINES);
  }  //("%");}
  else if (Seconds % 5 == 4) {
    lcd.write(SQUARE);
  }  //("#");}
  lcd.setCursor(18, 2); lcd.print("5s");
  //        lcd.setCursor(18, 3); lcd.print("  ");

  lcd.setCursor(18, 3);
  if (Hour > 12) lcd.print(F("PM"));
  else lcd.print(F("AM"));

  lcd.setCursor(9, 3);
  if (Seconds < secondsClockHelp)  // show time in normal numbers
  {
    //    lcd.print("Bar");
    sprintf(textBuffer, "%02d%c%02d%c%02d", Hour % 12, dateTimeFormat[dateFormat].hourSep, Minute, dateTimeFormat[dateFormat].minSep, Seconds);

    lcd.print(textBuffer);

  }
  else
  {
    lcd.print(F("         "));
  }
}

/*****
Purpose: Menu item
Finds local time as Set theory clock of https://en.wikipedia.org/wiki/Mengenlehreuhr in Berlin

Argument List: None

Return value: Displays on LCD
*****/

void MengenLehrUhr(void) {
  //
  
  int imax;
  //  lcd.clear(); // makes it blink

  // get local time
  local[timeZoneNumber] = now() + utcOffset * 60;
  Hour = hour(local[timeZoneNumber]);
  Minute = minute(local[timeZoneNumber]);
  Seconds = second(local[timeZoneNumber]);

  lcd.setCursor(0, 0);
  // top line has 5 hour resolution
  if (Hour > 4)
  {
    lcd.print("-"); lcd.write(ALL_ON); lcd.print("-"); lcd.print(" ");// fills square
  }
  else lcd.print(F("    "));
  if (Hour > 9)
  {
    lcd.print("-"); lcd.write(ALL_ON); lcd.print("-"); lcd.print(" ");// fills square
  }
  else lcd.print(F("    "));

  if (Hour > 14)
  {
    lcd.print("-"); lcd.write(ALL_ON); lcd.print("-"); lcd.print(" ");// fills square
  }
  else lcd.print(F("    "));
  if (Hour > 19)
  {
    lcd.print("-"); lcd.write(ALL_ON); lcd.print("-"); lcd.print(" ");// fills square
  }
  else lcd.print(F("    "));

  lcd.setCursor(18, 0); lcd.print("5h");

  // second line shows remainder and has 1 hour resolution

  lcd.setCursor(0, 1);
  imax = Hour % 5;
  if (imax > 0)
  {
    lcd.print("-"); lcd.write(ALL_ON); lcd.print("-"); lcd.print(" ");// fills square
  }
  else lcd.print(F("    "));
  if (imax > 1)
  {
    lcd.print("-"); lcd.write(ALL_ON); lcd.print("-"); lcd.print(" ");// fills square
  }
  else lcd.print(F("    "));
  if (imax > 2)
  {
    lcd.print("-"); lcd.write(ALL_ON); lcd.print("-"); lcd.print(" ");// fills square
  }
  else lcd.print(F("    "));
  if (imax > 3)
  {
    lcd.print("-"); lcd.write(ALL_ON); lcd.print("-"); lcd.print(" ");// fills square
  }
  else lcd.print(F("    "));

  lcd.setCursor(18, 1); lcd.print("1h");

  // third line shows a bar for every 5 minutes
  int ii;
  lcd.setCursor(0, 2);

  // only overwrite old characters when needed, to avoid flicker

  imax = Minute / 5;
  if (imax == 0)
  {
    lcd.print(F("                  "));
    lcd.setCursor(0, 2);
  }

  for (ii = 0; ii < imax; ii++)
  {
    lcd.write(ALL_ON);
    if (ii == 2 || ii == 5 || ii == 8) lcd.print(" ");
  }

  lcd.setCursor(18, 2); lcd.print("5m");

  // fourth line shows remainder and has 1 minute resolution
  lcd.setCursor(0, 3);

  imax = Minute % 5;
  if (imax > 0)
  {
    lcd.print("-"); lcd.write(ALL_ON); lcd.print("-"); lcd.print(" ");// fills square
  }
  else lcd.print(F("    "));

  if (imax > 1)
  {
    lcd.print("-"); lcd.write(ALL_ON); lcd.print("-"); lcd.print(" ");// fills square
  }
  else lcd.print(F("    "));

  if (imax > 2)
  {
    lcd.print("-"); lcd.write(ALL_ON); lcd.print("-"); lcd.print(" ");// fills square
  }
  else lcd.print(F("    "));

  if (imax > 3)
  {
    lcd.print("-"); lcd.write(ALL_ON); lcd.print("-"); lcd.print(" ");// fills square
  }
  else lcd.print(F("    "));

  lcd.setCursor(18, 3); lcd.print("1m");
}

/*****
Purpose: Menu item
Finds local time as Linear Clock, https://de.wikipedia.org/wiki/Linear-Uhr in Kassel

Argument List: None

Return value: Displays on LCD
*****/

void LinearUhr(void) {
  

  int imax;
  int ii;

  // get local time
  local[timeZoneNumber] = now() + utcOffset * 60;
  Hour = hour(local[timeZoneNumber]);
  Minute = minute(local[timeZoneNumber]);
  Seconds = second(local[timeZoneNumber]);

  lcd.setCursor(0, 0);
  // top line has 10 hour resolution
  imax = Hour / 10;
  lcd.print(F("             "));
  lcd.setCursor(0, 0);
  for (ii = 0; ii < imax; ii++)
  {
    lcd.write(ALL_ON);
  }
  lcd.setCursor(17, 0); lcd.print("10h");

  // second line shows remainder and has 1 hour resolution
  lcd.setCursor(0, 1);
  imax = Hour % 10;

  // only overwrite old characters when needed, to avoid flicker
  if (imax == 0)
  {
    lcd.print(F("             "));
    lcd.setCursor(0, 1);
  }
  for (ii = 0; ii < imax; ii++)
  {
    lcd.write(ALL_ON);
    if (ii == 4) lcd.print(" ");
  }
  lcd.setCursor(18, 1); lcd.print("1h");

  // third line shows a bar for every 10 minutes
  lcd.setCursor(0, 2);
  imax = Minute / 10;

  // only overwrite old characters when needed, to avoid flicker
  if (imax == 0)
  {
    lcd.print(F("                  "));
    lcd.setCursor(0, 2);
  }

  for (ii = 0; ii < imax; ii++)
  {
    lcd.write(ALL_ON);
    if (ii == 4) lcd.print(" ");
  }
  lcd.setCursor(17, 2); lcd.print("10m");

  // last line shows remainder and has 1 minute resolution
  lcd.setCursor(0, 3);

  // only overwrite old characters when needed, to avoid flicker

  imax = Minute % 10;
  if (imax == 0)
  {
    lcd.print(F("                  "));
    lcd.setCursor(0, 3);
  }

  for (ii = 0; ii < imax; ii++)
  {
    lcd.write(ALL_ON);
    if (ii == 4) lcd.print(" ");
  }
  lcd.setCursor(18, 3); lcd.print("1m");
}

/*****
Purpose: Menu item
Finds internal time: jd, j2000, etc

Argument List: None

Return value: Displays on LCD
*****/

void InternalTime() {     // UTC, Unix time, J2000, etc

  lcd.setCursor(0, 0); // top line *********
  if (gps.time.isValid()) {

    float jd = now() / 86400.0; // cdn(now()); // now/86400, i.e. no of days since 1970
    float j2000 = jd - 10957.5; // 1- line
    lcd.print("j2k ");
    lcd.print(j2000);

    #ifdef FEATURE_FAKE_SERIAL_GPS_IN
        hourGPS = hour(now());
        minuteGPS = minute(now());
        secondGPS = second(now());
    #endif

    lcd.setCursor(12, 0);
    sprintf(textBuffer, "%02d%c%02d%c%02d UTC ", hourGPS, dateTimeFormat[dateFormat].hourSep, minuteGPS, dateTimeFormat[dateFormat].minSep, secondGPS);
    lcd.print(textBuffer);

    lcd.setCursor(0, 1);
    lcd.print(F("jd1970 "));
    lcd.print(jd);

    // utc = now(); // UNIX time, seconds ref to 1970
    lcd.setCursor(0, 2);
    lcd.print(F("now    "));
    lcd.print(now());

    lcd.setCursor(0, 3);
    lcd.print(F("local  "));
    local[timeZoneNumber] = now() + utcOffset * 60;
    lcd.print(local[timeZoneNumber]);
  }
}
/*****
Purpose: Menu item
Gives code status

Argument List: None

Return value: Displays on LCD
*****/

void CodeStatus(void) {
  lcd.setCursor(0, 0); lcd.print(F("* LA3ZA GPS clock *"));
  lcd.setCursor(0, 1); lcd.print(codeVersion);
  //lcd.setCursor(0, 2); lcd.print(F("GPS  ")); lcd.print(gpsBaud); lcd.print(" bps");
  lcd.setCursor(0, 2); lcd.print(F("GPS  ")); lcd.print(gpsBaud1[baudRateNumber]); lcd.print(" bps");
  lcd.setCursor(0, 3); lcd.print(tcr[timeZoneNumber] -> abbrev); lcd.setCursor(5, 3); PrintFixedWidth(lcd, utcOffset, 4); lcd.print(" min   ");//timezone name and offset (min)
    lcd.print(languages[languageNumber]);lcd.print(F("  "));
}

/*****
Purpose: Menu item
Gives UTC time, locator, latitude/longitude, altitude and no of satellites

Argument List: None

Return value: Displays on LCD
*****/

void UTCPosition() {     // position, altitude, locator, # satellites
  String textBuffer;

  LcdUTCTimeLocator(0, 1); // top line ********* start 1 position right - in order to line up with latitude/longitude
  // UTC date
  if (gps.location.isValid()) {

#ifndef DEBUG_MANUAL_POSITION
    latitude = gps.location.lat();
    lon = gps.location.lng();
    alt = gps.altitude.meters();
#else
    latitude = latitude_manual;
    lon      = longitude_manual;
    alt = 0.0;
#endif



    int cycleTime = 10; // 4.10.2022: was 4 seconds

    lcd.setCursor(0, 2);
    if ((now() / cycleTime) % 3 == 0) { // change every cycleTime seconds

// fixed formatting to handle 3-digit E-W, single-digit degree, and single-digit minute; 17.7.2023

      //  decimal degrees
      lcd.setCursor(1, 2);
      if (abs(latitude) < 10) lcd.print(" ");
      textBuffer = String(abs(latitude), 4);
      lcd.print(textBuffer); lcd.write(DEGREE);
      if (latitude < 0) lcd.print(F(" S   "));
      else lcd.print(F(" N   "));

      lcd.setCursor(0, 3);
     // textBuffer = String(abs(lon), 4);
      //int strLength = textBuffer.length();
      //lcd.print(textBuffer); 
      if (abs(lon) < 100) lcd.print(" ");
      if (abs(lon) <  10) lcd.print(" ");
      lcd.print(abs(lon),4);
      lcd.write(DEGREE);
      if (lon < 0) lcd.print(F(" W    "));
      else lcd.print(F(" E    "));
    }
    else if ((now() / cycleTime) % 3 == 1) {

      // degrees, minutes, seconds
      lcd.setCursor(0, 2);
      float mins;
      //textBuffer = String((int)abs(latitude));
      //lcd.print(textBuffer); 
      PrintFixedWidth(lcd, (int)abs(latitude), 3); lcd.write(DEGREE);
      mins = abs(60 * (latitude - (int)latitude));  // minutes
      //textBuffer = String((int)mins);
      //lcd.print(textBuffer); 
      PrintFixedWidth(lcd, (int)mins, 2,'0'); lcd.write("'");
      // textBuffer = String((int)(abs(60 * (mins - (int)mins)))); // seconds
      // lcd.print(textBuffer); 
      PrintFixedWidth(lcd, (int)(abs(60 * (mins - (int)mins))), 2,'0'); lcd.write(34);
      if (latitude < 0) lcd.print(F(" S  "));
      else lcd.print(F(" N  "));


      lcd.setCursor(0, 3);
      //textBuffer = String((int)abs(lon));
      //lcd.print(textBuffer);
      PrintFixedWidth(lcd, (int)abs(lon), 3);
      lcd.write(DEGREE);
      mins = abs(60 * (lon - (int)lon));
      //textBuffer = String((int)mins);
      //lcd.print(textBuffer); 
      PrintFixedWidth(lcd, (int)mins, 2,'0'); lcd.write("'");
      // textBuffer = String((int)(abs(60 * (mins - (int)(mins)))));
      // lcd.print(textBuffer); 
      PrintFixedWidth(lcd, (int)(abs(60 * (mins - (int)mins))), 2,'0'); lcd.write(34); // symbol for "
      if (lon < 0) lcd.print(F(" W "));
      else lcd.print(F(" E "));
    }

    else  {

      // degrees, decimal minutes
      lcd.setCursor(0, 2);
      float mins;
      // textBuffer = String(int(abs(latitude)));
      // lcd.print(textBuffer); 
      PrintFixedWidth(lcd, (int)abs(latitude), 3); lcd.write(DEGREE);
      mins = abs(60 * (latitude - (int)latitude));
      if (mins <10) lcd.print('0');
      textBuffer = String(abs(mins), 2);
      lcd.print(textBuffer);
      if (latitude < 0) lcd.print(F("' S "));
      else lcd.print(F("' N "));

      lcd.setCursor(0, 3);
      //textBuffer = String(int(abs(lon)));
      //lcd.print(textBuffer);
      PrintFixedWidth(lcd, (int)abs(lon), 3);
      lcd.write(DEGREE);
      mins = abs(60 * (lon - (int)lon));
      if (mins <10) lcd.print('0');
      textBuffer = String(abs(mins), 2);  // double abs() to avoid negative number for x.00 degrees
      lcd.print(textBuffer);
      if (lon < 0) lcd.print(F("' W  "));
      else lcd.print(F("' E  "));
    }
  }
  // enough space on display for 2469 m
  lcd.setCursor(14, 2);
  PrintFixedWidth(lcd, (int)round(alt), 4, ' '); lcd.print(" m");

//  if (gps.satellites.isValid()) { // 16.11.2022
   if (gps.satellites.isUpdated()) {
    noSats = gps.satellites.value();
    if (noSats < 10) lcd.setCursor(14, 3);
    else lcd.setCursor(13, 3);    
  }
    else {
      noSats = 0;
      lcd.setCursor(14, 3);
    }
    lcd.print(noSats); 
    lcd.print(F(" Sats"));
}


/*****
Purpose: Menu item
Finds data for beacons of NCDXF  Northern California DX Foundation

Argument List: int option - option=1: 14-21 MHz beacons on lines 1-3,  option=2: 21-28 MHz beacons on lines 1-3

Return value: Displays on LCD
*****/
// up to date rel to https://www.ncdxf.org/beacon/beaconlocations.html 22.07.2023
const char callsign[19][7] PROGMEM = {
  " 4U1UN", " VE8AT", "  W6WX", " KH6RS", "  ZL6B", "VK6RBP", "JA2IGY", "  RR9O", "  VR2B", "  4S7B", " ZS6DN",
  "  5Z4B", " 4X6TU", "  OH2B", "  CS3B", " LU4AA", "  OA4B", "  YV5B"
};
const char locationP[19][7] PROGMEM = {
  "FN30as", "CP38gh", "CM97bd", "BL10ts", "RE78tw", "OF87av", "PM84jk", "NO14kx", "OL72bg", "MJ96wv", "KG33xi",
  "KI88hr", "KM72jb", "KP20eh", "IM12jt", "GF05tj", "FH17mw", "FJ69cc"
};
const char qth[19][8] PROGMEM = {
  "N York ", "Inuvik ", "Califor", "Hawaii ", "N Zeala", "Austral", "Japan  ", "Nsibirs", "H Kong ", "Colombo", "Pretori",
  "Kenya  ", "Tel Avi", "Finland", "Madeira", "B Aires", "Lima   ", "Caracas"
};


void NCDXFBeacons(int option) {
  // option=1: 14-21 MHz beacons on lines 1-3,  option=2: 21-28 MHz beacons on lines 1-3

  // Inspired by OE3GOD: https://www.hamspirit.de/7757/eine-stationsuhr-mit-ncdxf-bakenanzeige/

  int ii, iii, iiii, km, bandOffset;
  double lati, longi;
  char location[7]; // to hold 6 char + terminating '\0'
  int qrg[6] = {14100, 18110, 21150, 24930, 28200}; // 10.4.2023: from char to int to save RAM
  
  LcdUTCTimeLocator(0); // top line *********

  ii = (60 * (minuteGPS % 3) + secondGPS) / 10; // ii from 0 to 17

  if (option <= 1) bandOffset = 0; // 14-18 MHz
  else             bandOffset = 2; // 18-28 MHz

  for (iiii = 1; iiii < 4; iiii += 1) { // step over lines 1,2,3
    lcd.setCursor(0, iiii);

    // modulo for negative numbers: https://twitter.com/parkerboundy/status/326924215833985024
    iii = ((ii - iiii + 1 - bandOffset % 18) + 18) % 18;
    lcd.print(qrg[iiii - 1 + bandOffset]); lcd.print(" ");
    lcd.print(reinterpret_cast<const __FlashStringHelper*>(callsign[iii]));
    
if (secondGPS % 10 < 5) {
      lcd.print(" ");
      lcd.print(reinterpret_cast<const __FlashStringHelper *>((qth[iii])));
    }
    else { 
      // copy from PROGMEM to local memory:
      strlcpy_P(location, locationP[iii], sizeof(location)); // does not overflow, just truncates

      LocatorToLatLong(location, lati, longi);
      km = Distance(lati, longi, latitude, lon);
      PrintFixedWidth(lcd, (int)float(km), 6);
      lcd.print(F("km"));
    }
  }
}

/*****
Purpose: Menu item
Shows data for WSPR (Weak Signal Propagation Reporter) coordinated mode, frequency

Argument List: none

Return value: Displays on LCD
*****/

void WSPRsequence() {     // UTC, + WSPR band/frequency for coordinated WSPR
  // https://physics.princeton.edu//pulsar/K1JT/doc/wspr/wspr-main.html#BANDHOPPING
  // 20 min cycle over 10 bands from 160m to 10m

  int ii;
  // 10.4.2023: changed from char to int and float storage to save RAM:
  int band[11] = {160, 80, 60, 40, 30, 20, 17, 15, 12, 10};
  float qrg[11] = {1838.100, 3570.100, 5366.200, 7040.100, 10140.200, 14097.100, 18106.100, 21096.100, 24926.100, 28126.100};
  
  LcdUTCTimeLocator(0); // top line *********
  /*
     Each WSPR frequency is transmitted every 20 minutes in 2 min intervals.
  */
  lcd.setCursor(0, 2);
  lcd.print(F("WSPR band hopping:  "));

  ii = (minuteGPS % 20) / 2; // ii from 0 to 9, adresses band

  // WSPR transmission starts 1 second into even minute and lasts for 110.6 = 60 + 50.6 seconds
  if ((minuteGPS % 2 == 0 && secondGPS < 1) || (minuteGPS % 2 == 1 && secondGPS > 52))
  {
    lcd.setCursor(0, 3); lcd.print(F("                    "));
  }
  else
  {
    int isec = secondGPS-1;
    if (minuteGPS % 2 == 1) isec = isec + 60;    
    lcd.setCursor(0, 3); lcd.print(band[ii]); lcd.print(F(" m ")); lcd.print(qrg[ii],1);  lcd.print(F(" kHz  "));
    lcd.setCursor(17,3); PrintFixedWidth(lcd, isec, 3); // seconds into transmission
  }
  // lcd.setCursor(18, 3); lcd.print("  "); // blank out menu number
}

/*****
Purpose: Menu item
Shows clock in hex, octal, or binary format

Argument List: int val = 0 - hex, 1 - octal, 3 - hex, octal, and binary

Return value: Displays on LCD
*****/

void HexOctalClock(int val)   
{
  int  BinaryHour[6],  BinaryMinute[6],  BinarySeconds[6];

  //  get local time
  local[timeZoneNumber] = now() + utcOffset * 60;
  Hour = hour(local[timeZoneNumber]);
  Minute = minute(local[timeZoneNumber]);
  Seconds = second(local[timeZoneNumber]);

  lcd.setCursor(0, 0);
  if (val == 0)      lcd.print(F("Hex   "));
  else if (val == 1) lcd.print(F("Oct   "));

  if (val == 0)     // Hex
  {
    lcd.setCursor(7, 0);    // one line up 18.6.2023
    sprintf(textBuffer, "%02X%c%02X%c%02X", Hour, dateTimeFormat[dateFormat].hourSep, Minute, dateTimeFormat[dateFormat].minSep, Seconds);
    lcd.print(textBuffer);
    lcd.setCursor(17,3); lcd.print(F("   "));
  }
  else if (val == 1) // Oct
  {
    lcd.setCursor(7, 0);    // one line up 18.6.2023
    sprintf(textBuffer, "%02o%c%02o%c%02o", Hour, dateTimeFormat[dateFormat].hourSep, Minute, dateTimeFormat[dateFormat].minSep, Seconds);
    lcd.print(textBuffer);
    lcd.setCursor(17,3); lcd.print(F("   "));
  }
  else // Binary, oct, dec, hex
  {
    // convert to binary:
    DecToBinary(Hour, BinaryHour);
    DecToBinary(Minute, BinaryMinute);
    DecToBinary(Seconds, BinarySeconds);

    lcd.setCursor(0, 0); sprintf(textBuffer, "%1d%1d%1d%1d%1d", BinaryHour[1], BinaryHour[2], BinaryHour[3], BinaryHour[4], BinaryHour[5]);
    lcd.print(textBuffer); lcd.print(dateTimeFormat[dateFormat].hourSep);

    sprintf(textBuffer, "%1d%1d%1d%1d%1d%1d", BinaryMinute[0], BinaryMinute[1], BinaryMinute[2], BinaryMinute[3], BinaryMinute[4], BinaryMinute[5] );
    lcd.print(textBuffer); lcd.print(dateTimeFormat[dateFormat].minSep);

    sprintf(textBuffer, "%1d%1d%1d%1d%1d%1dB", BinarySeconds[0], BinarySeconds[1], BinarySeconds[2], BinarySeconds[3], BinarySeconds[4], BinarySeconds[5] );
    lcd.print(textBuffer);
    
    lcd.setCursor(19, 1); lcd.print("O");
    lcd.setCursor(7, 1);
    sprintf(textBuffer, "%02o%c%02o%c%02o", Hour, dateTimeFormat[dateFormat].hourSep, Minute, dateTimeFormat[dateFormat].minSep, Seconds); // octal
    lcd.print(textBuffer);
    
    lcd.setCursor(18, 3); lcd.print(" H");
    lcd.setCursor(7, 3);
    sprintf(textBuffer, "%02X%c%02X%c%02X", Hour, dateTimeFormat[dateFormat].hourSep, Minute, dateTimeFormat[dateFormat].minSep, Seconds); // hex
    lcd.print(textBuffer);
  }


  //  help screen with normal clock for 0...secondsClockHelp seconds per minute
  if (Seconds < secondsClockHelp | val==3)  // show time in normal numbers - always if simultaneous Bin, Oct, Hex
  {
    if (val == 3) {
      lcd.setCursor(19, 2); lcd.print("D");
    }
    else
    {
      lcd.setCursor(18, 3); lcd.print(F("  ")); // clear number in lower left corner
    }
    lcd.setCursor(7, 2);   
    sprintf(textBuffer, "%02d%c%02d%c%02d", Hour, dateTimeFormat[dateFormat].hourSep, Minute, dateTimeFormat[dateFormat].minSep, Seconds);
    lcd.print(textBuffer);
  }
  else
  {
    lcd.setCursor(0, 2);
    lcd.print(F("                    "));
  }

}


/*****
Purpose: Menu item
Shows date for Easter for the next years after  Gregorian (Western) aad Julian (Eastern) calendars
Output dates are in Gregorian dates

Argument List: int yr -  input value for year

Return value: Displays on LCD
*****/
void EasterDates(int yr)
{
  /* Parameters K, E:
      Julian calendar:
      K=-3 E=-1

      Gregorian calendar:
      1583-1693 K= l E=-8
      1700-1799 K= 0 E=-9
      1800-1899 K=-l E=-9
      1900-2099 K=-2 E=-10
      2100-2199 K=-3 E=-10
  */

  int K, E;
  int ii = 1;
  int PaschalFullMoon, EasterDate, EasterMonth;
  lcd.setCursor(0, 0); lcd.print(F("Easter  Greg. Julian"));

if (minuteGPS != oldMinute) {

  for (int yer = yr; yer < yr + 3; yer++)
  {
    lcd.setCursor(2, ii); lcd.print(yer); lcd.print(F(": "));
    // Gregorian (West):
    K = -2; E = -10;
    ComputeEasterDate(yer, K, E, &PaschalFullMoon, &EasterDate, &EasterMonth);
    lcd.setCursor(8, ii); LcdDate(EasterDate, EasterMonth);
    
    // Julian (East)
    K = -3; E = -1;
    ComputeEasterDate(yer, K, E, &PaschalFullMoon, &EasterDate, &EasterMonth);
    JulianToGregorian(&EasterDate, &EasterMonth); // add 13 days
    lcd.setCursor(14, ii); LcdDate(EasterDate, EasterMonth);
    lcd.print(" "); // empty space at the end
    ii++;
  }
}
oldMinute = minuteGPS;
}


/*****
Purpose: Menu item
Shows local time as an arithmetic computation
Inspired by https://www.albertclock.com/, named after Albert Einstein. Website says:
"This digital clock keeps your brain active and helps to improve 
the mathematical skills of you and your kids in a playful way. Simply by reading the time."

Argument List: int LevelMath  0 for +
                              1 for - or +
                              2 for *, -, or +
                              3 for /,*,-, or +

Return value: Displays on LCD
*****/

void MathClock(int LevelMath )
{
  int Term1Hr, Term2Hr, Term1Min, Term2Min;
  int rnd, HrMult, MinMult;
  
  //  get local time
  local[timeZoneNumber] = now() + utcOffset * 60;
  Hour = hour(local[timeZoneNumber]); 
  Minute = minute(local[timeZoneNumber]);
  Seconds = second(local[timeZoneNumber]);


  if (now()%mathSecondPeriod == 0 | oldMinute == -1)  // ever so often + immediate start
  {

    #ifdef FEATURE_SERIAL_MATH
        Serial.print(F("oldMinute, Seconds, mathSecondPeriod: ")); Serial.print(oldMinute); Serial.print(":"); 
        Serial.print(Seconds); Serial.print(", "); Serial.println(mathSecondPeriod); 
    #endif
  
  
    HrMult = 0;
    MinMult = 0;
  
    if (LevelMath == 0)      // +
    {
        MathPlus(Hour, &Term1Hr, &Term2Hr);    
        MathPlus(Minute, &Term1Min, &Term2Min);
    } 
   
    else if (LevelMath == 1) //  - and +
// was - : 4/5 probability, + : 1/5 probability. Now 50/50:
    {
      // was 0,1: +, 2,3,4,5: - now 0,1,2 or 3,4,5:
      if (random(0,6) <= 2) MathPlus(Hour, &Term1Hr, &Term2Hr);    
      else                  MathMinus(Hour, &Term1Hr, &Term2Hr);    
      
      if (random(0,6) <= 2) MathPlus(Minute, &Term1Min, &Term2Min);
      else                  MathMinus(Minute, &Term1Min, &Term2Min);
    }
    
    else if (LevelMath == 2) // x - +
    // was x: 50% prob, -: 3/8 prob, +: 1/8 prob. Now 1/3, 1/3, 1/3:
    
    {
      rnd = random(0,3); // 0, 1, 2 was [0,...,7] - Hour
      if (rnd == 0)       
      {
        MathMultiply(Hour, &Term1Hr, &Term2Hr);
        HrMult = 1;
      }
      else if (rnd == 1)  MathPlus(Hour, &Term1Hr, &Term2Hr); 
      else                MathMinus(Hour, &Term1Hr, &Term2Hr); 

      rnd = random(0,3); // Minute
      if (rnd == 0)      
      {
        MathMultiply(Minute, &Term1Min, &Term2Min);
        MinMult = 1;
      }
      else if (rnd == 1) MathPlus(Minute, &Term1Min, &Term2Min); 
      else               MathMinus(Minute, &Term1Min, &Term2Min);       
    }
    
    else if (LevelMath == 3) 
    //was  /: 50% prob; x: 30% prob; -: 20% prob, no +, now 3 x 1/3 prob
    {
      rnd = random(0,3); //  Hour
      if (rnd == 0)       
      {
        MathDivide(Hour, &Term1Hr, &Term2Hr);
        HrMult = 2;  // means divide
      }
      else if (rnd == 1)
      {
        MathMultiply(Hour, &Term1Hr, &Term2Hr);
        HrMult = 1;
      }
      else  MathMinus(Hour, &Term1Hr, &Term2Hr); 


      rnd = random(0,3); // Minute
      if (rnd == 0)      
      {
        MathDivide(Minute, &Term1Min, &Term2Min);
        MinMult = 2;
      }
      else if (rnd == 1)
      {
        MathMultiply(Minute, &Term1Min, &Term2Min);
        MinMult = 1;
      }
      else  MathMinus(Minute, &Term1Min, &Term2Min);  
    }


    // display result on lcd

    // Hour:  
      lcd.setCursor(5, 0); PrintFixedWidth(lcd, Term1Hr, 2); 
      if (HrMult == 2)  // divide
      {
        lcd.print(" ");lcd.print(MATH_CLOCK_DIVIDE);lcd.print(" ");
      }
      else if (HrMult == 1) 
      {
        lcd.print(" ");lcd.print(MATH_CLOCK_MULTIPLY);lcd.print(" ");
      }
      else
      { 
        if (Term2Hr < 0) lcd.print(F(" - ")); else lcd.print(F(" + ")); 
      }
      lcd.print(abs(Term2Hr)); lcd.print(F("   ")); 
      lcd.setCursor(19, 0);lcd.print("h");     

    // Minute:
      lcd.setCursor(8, 2); PrintFixedWidth(lcd, Term1Min, 2);  
      if (MinMult == 2)  // divide
      {
        lcd.print(" ");lcd.print(MATH_CLOCK_DIVIDE);lcd.print(" ");
      }
      else if (MinMult == 1) 
      {
        lcd.print(" ");lcd.print(MATH_CLOCK_MULTIPLY);lcd.print(" ");
      }
      else
      { 
        if (Term2Min < 0) lcd.print(F(" - ")); else lcd.print(F(" + ")); 
      }
      lcd.print(abs(Term2Min)); lcd.print(F("   "));
      lcd.setCursor(19, 2);lcd.print("m");  
   }

  // lower line 
  // left corner:
  lcd.setCursor(0, 3); 
  // show remaining time (13.7.2023):
  PrintFixedWidth(lcd, mathSecondPeriod-now()%mathSecondPeriod, 2);//lcd.print(F("  "));
  
  // then show symbols to the right:
  lcd.setCursor(17,3);

    if (LevelMath == 3)
  {
    lcd.print(MATH_CLOCK_DIVIDE);lcd.print(MATH_CLOCK_MULTIPLY);lcd.print("-");
  }
  else if (LevelMath == 2) 
  {
    lcd.print(MATH_CLOCK_MULTIPLY);lcd.print(F("+-"));
  }
  else if (LevelMath == 1) lcd.print(F(" +-"));  
  else                     lcd.print(F("  +"));  


  //lcd.setCursor(18, 3); lcd.print(F("  "));

  oldMinute = minuteGPS;

  

}

/*****
Purpose: Menu item
Finds the Lunar eclipses for the next years

Argument List: none

Return value: Displays on LCD

*****/

void LunarEclipse()
{
  int pDate[10]; // max 5 per day: packed date = 100*month + day, i.e. 1209 = 9 December
  int eYear[10];
  int pday, pmonth, yy;
  int i;

if (minuteGPS != oldMinute) {

  
  lcd.setCursor(0, 0); lcd.print(F("Lunar Eclipses "));

  // Good up to and including 2021, but misses the almost eclipse 18-19 July 2027
  // Test: try 2028 with 3 eclipses, see https://www.timeanddate.com/eclipse/list-lunar.html
  yy = yearGPS;//
   
  #ifdef FEATURE_SERIAL_LUNARECLIPSE
    Serial.print((int)yy); Serial.println(F(" ****************"));
  #endif

  for (i=0; i<10; i++) pDate[i]=0;
  
  MoonEclipse(yy, pDate, eYear); 
  int lineNo = 1;
  lcd.setCursor(2,lineNo);lcd.print(yy);lcd.print(":");
  int col = 8;
   
  for (i=0; (i<10 & pDate[i] != 0) ; i++)
  {   
      if (eYear[i] == yy)
      {
        if (col>14)
        {
          col = 2; lineNo = lineNo + 1; // start another line if more than 3 eclipses this year (first time in 2028!)
        } 
        lcd.setCursor(col,lineNo);
        pmonth  = pDate[i] / 100;
        pday = pDate[i] - 100 * pmonth;
        LcdDate(pday, pmonth); lcd.print(" ");  
        col = col+6;
      }
  }
  
  for (i=0; i<10; i++) pDate[i]=0;
  yy = yy + 1;
  MoonEclipse(yy, pDate, eYear);

  //pDate[2] = 729; //pDate[3] = 1209; pDate[4] = 101; // artificial data  for testing
   
  lineNo = lineNo + 1;
  lcd.setCursor(2,lineNo);lcd.print(yy);lcd.print(":");
  col = 8;
  for (i=0; (i < 10 & pDate[i] != 0 & lineNo < 4) ; i++)
  {   
      if (eYear[i] == yy)
      {
        if (col>14)
        {
          col = 2; lineNo = lineNo+1;
        } 
        lcd.setCursor(col,lineNo);
        pmonth  = pDate[i] / 100;
        pday = pDate[i] - 100 * pmonth;
        LcdDate(pday, pmonth); lcd.print(" ");
        col = col+6; 
      }   
  }

  if (lineNo < 3)
  { 
    for (i=0; i<10; i++) pDate[i]=0;

    yy = yy + 1;
    MoonEclipse(yy, pDate, eYear);
   
    lineNo = lineNo + 1;
    lcd.setCursor(2,lineNo);lcd.print(yy);lcd.print(":");
    col = 8;
      
    for (i=0; (i < 2 & pDate[i] != 0 & lineNo < 4) ; i++) // never room for more than two
    {   
        if (eYear[i] == yy)
        {    
          lcd.setCursor(col,lineNo);
          pmonth  = pDate[i] / 100;
          pday = pDate[i] - 100 * pmonth;
          LcdDate(pday, pmonth); lcd.print(" ");
          col = col+6; 
        }   
    }
  }
  if (col < 14) 
  {
    lcd.setCursor(18,3); lcd.print(F("  "));  // erase lower right-hand corner if not already done
  }
  oldMinute = Minute;
}
}


/*****
Purpose: Menu item
Finds local time with Roman numerals (letters)

Argument List: none

Return value: Displays on LCD
*****/

const char RomanOnes[10][6] PROGMEM = {{"     "}, {"I    "}, {"II   "}, {"III  "}, {"IV   "}, {"V    "}, {"VI   "}, {"VII  "}, {"VIII "}, {"IX   "}}; // left justified
const char RomanTens[6][4]  PROGMEM = {{""}, {"X"}, {"XX"}, {"XXX"}, {"XL"}, {"L"}}; 

void Roman()
{
  int ones, tens;

  /* The longest symbol
   *  Hours:            18: XVIII, 23: XXIII
   *  Minutes, seconds: 38: XXXVIII
   *  Longest symbol is 5+1+7+1+7 = 21 letters long, so it doesn't fit a single line on a 20 line LCD
   */
 
  //  get local time
  local[timeZoneNumber] = now() + utcOffset * 60;
  Hour = hour(local[timeZoneNumber]);
  Minute = minute(local[timeZoneNumber]);
  Seconds = second(local[timeZoneNumber]);

  ones = Hour % 10; tens = (Hour - ones) / 10;

  lcd.setCursor(0, 0); lcd.print(reinterpret_cast<const __FlashStringHelper*>(RomanTens[tens])); 
  lcd.print(reinterpret_cast<const __FlashStringHelper*>(RomanOnes[ones]));
  //lcd.setCursor(5, 0);lcd.print(dateTimeFormat[dateFormat].hourSep);

  ones = Minute % 10; tens = (Minute - ones) / 10;
  lcd.setCursor(6, 1); 
  lcd.print(reinterpret_cast<const __FlashStringHelper*>(RomanTens[tens])); 
  lcd.print(reinterpret_cast<const __FlashStringHelper*>(RomanOnes[ones]));
  //lcd.setCursor(13, 0); lcd.print(dateTimeFormat[dateFormat].minSep);

  ones = Seconds % 10; tens = (Seconds - ones) / 10;
  lcd.setCursor(12, 2); 
  lcd.print(reinterpret_cast<const __FlashStringHelper*>(RomanTens[tens])); 
  lcd.print(reinterpret_cast<const __FlashStringHelper*>(RomanOnes[ones]));

//  help screen with normal clock for 0...secondsClockHelp seconds per minute
  if (Seconds < secondsClockHelp)  // show time in normal numbers
  {
    lcd.setCursor(0, 3);
    sprintf(textBuffer, "%02d%c%02d%c%02d", Hour, dateTimeFormat[dateFormat].hourSep, Minute, dateTimeFormat[dateFormat].minSep, Seconds);
    lcd.print(textBuffer);
  }
  else
  {
    lcd.setCursor(0, 3);lcd.print(F("        "));
  }
  lcd.setCursor(18, 3); lcd.print(F("  ")); // blank out number in lower right-hand corner 
}



/*****
Purpose: Menu item
Calculates local sidereal time based on this calculation, http://www.stargazing.net/kepler/altaz.html 
code pieces from https://hackaday.io/project/163103-freeform-astronomical-clock
also computes local solar time

Argument List: none

Return value: Displays on LCD

Issues:  Is there enough precision? since Mega doesn't have double???
         seems so: accurate within a second or so with http://www.jgiesen.de/astro/astroJS/siderealClock/
*****/

void Sidereal()  // LST - Local Sidereal Time
{
     // 

    double LST_hours,LST_degrees;

    float jd = now() / 86400.0; // cdn(now()); // now/86400, i.e. no of days since 1970
    float j2000 = jd - 10957.5; // 1- line

    double decimal_time = hourGPS + (minuteGPS/60.0) + (secondGPS/3600.0) ;
    double LST = 100.46 + 0.985647 * j2000 + gps.location.lng() + 15*decimal_time; 
    LST_degrees = (LST-(floor(LST/360)*360));
    LST_hours = LST_degrees/15;
    
    int rHours =(int) LST_hours;
    int rMinutes = ((int)floor((LST_hours-rHours)*60));
  
    // compute local solar time based on Equation Of Time
    // EQUATIO: Sidereal & Solar Clock, by Wooduino
    // Routines from http://woodsgood.ca/projects/2015/06/14/equatio-sidereal-solar-clock/

    double tv; // time variable offset in minutes
    doEoTCalc(&tv);

    // time correction factor: https://www.pveducation.org/pvcdrom/properties-of-sunlight/solar-time
    // note 4 minutes = 1 degree of rotation of earth

    // 2021-11-25 test with average of the three
    // rel https://fate.windada.com/cgi-bin/SolarTime_en: 35 sec too fast
    // rel to http://www.jgiesen.de/astro/astroJS/siderealClock/: 51 sec too slow

    // new test with Wikipedia method:
    // rel https://fate.windada.com/cgi-bin/SolarTime_en: 35 sec too fast: 2 sec 3-4 sec faster
    
    
    double tc = 4.0*lon + tv; // correction in minutes: Deviation from center of time zone + Equation of Time
 
   // display results:

    LcdShortDayDateTimeLocal(0, 0); // line 3 local time 
 
    
    lcd.setCursor(0,1);lcd.print(F("Solar time"));

    lcd.setCursor(11,1);

    time_t solar;
    solar = now() + (int)(tc*60);
    Hour = hour(solar); 
    Minute = minute(solar);
    Seconds = second(solar);
    
    //sprintf(textBuffer, " %02d%c%02d%c%02d", Hour, dateTimeFormat[dateFormat].hourSep, Minute, dateTimeFormat[dateFormat].minSep, Seconds); 
    // drop seconds:
    sprintf(textBuffer, " %02d%c%02d%   ", Hour, dateTimeFormat[dateFormat].hourSep, Minute); 
    lcd.print(textBuffer);

    lcd.setCursor(0,2);lcd.print(F("Sidereal"));
    lcd.setCursor(12,2);
    PrintFixedWidth(lcd, rHours,   2,'0'); lcd.print(dateTimeFormat[dateFormat].hourSep);
    PrintFixedWidth(lcd, rMinutes, 2,'0');

// sidereal calc # 2

//    double localsidereal = localSiderealTime(lon, jd, float(utcOffset) / 60.0); // in radians
//    localsidereal = 24.0 * localsidereal/(2.0*PI); // in hours
//
//    rHours =(int) localsidereal;
//    rMinutes = ((int)floor((localsidereal-rHours)*60));
//    lcd.setCursor(0,3);
//    PrintFixedWidth(lcd, rHours, 2, '0'); lcd.print(dateTimeFormat[dateFormat].hourSep);
//    PrintFixedWidth(lcd, rMinutes, 2, '0');
//    shows 04:25 when the first one shows 09:36 ???
//    
    
    lcd.setCursor(18,3);lcd.print(F("  "));   

    #ifdef EXP_Tide_Sidereal
// tidal calculation
     int luniTidal = 359; // minutes, lunitidal interval: how long after meridian high tide occurs, Asker (1 July 2023)
                          // lunitidal interval further varies within about +/-30 minutes according to the lunar phase. https://en.wikipedia.org/wiki/Lunitidal_interval

     
     float comp = - 934;// minutes, offset for meridian on July 1, 2023 in Asker: local fudge factor!!!
     local[timeZoneNumber] = now() + utcOffset * 60; // local time in sec
     
     long secondsMeridian = 86400 + 86400./(CYCLELENGTH/86400.-1); // CYCLELENGTH in sec = 29.53 days for moon phase
     // no double for Arduino
    long nextMeridian = secondsMeridian - local[timeZoneNumber]  % secondsMeridian; // no of seconds to next meridian

     //double fract = (local[timeZoneNumber]/float(secondsMeridian)) - int((local[timeZoneNumber]/float(secondsMeridian)));   // where are we in meridian cycle 0...1
     time_t next = local[timeZoneNumber] + nextMeridian + comp*60.; // time of meridian

     Hour = hour(next); 
     Minute = minute(next);
     lcd.setCursor(0,3);
     lcd.print(F("Mer ")); // meridian - time of closest approach of moon
     PrintFixedWidth(lcd,Hour,2,'0'); lcd.print(dateTimeFormat[dateFormat].hourSep);PrintFixedWidth(lcd,Minute,2,'0');
     
     Hour = hour(next + 60.*luniTidal);     // High tide
     Minute = minute(next + 60.*luniTidal);
     lcd.print(F(" T "));  // tide
     PrintFixedWidth(lcd,Hour,2,'0'); lcd.print(dateTimeFormat[dateFormat].hourSep);PrintFixedWidth(lcd,Minute,2,'0');
     //lcd.print(F(" Hi")); 
     
     #ifdef FEATURE_SERIAL_MOON
        Serial.print(F("secondsMeridian ")); Serial.println(secondsMeridian);
        Serial.print(F("nextMeridian    ")); Serial.println(nextMeridian);
     #endif
    #endif 
}

/*****
Purpose: Menu item
Finds local time in Morse code

Argument List: none

Return value: Displays on LCD
*****/

void Morse()   // time in Morse code on LCD
{ 
  int ones, tens;
  
  //  get local time
  local[timeZoneNumber] = now() + utcOffset * 60;
  Hour = hour(local[timeZoneNumber]);
  Minute = minute(local[timeZoneNumber]);
  Seconds = second(local[timeZoneNumber]);

  ones = Hour % 10; tens = (Hour - ones) / 10;
  lcd.setCursor(0, 0); 
  LcdMorse(tens);
//  if (tens != 0) LcdMorse(tens);
//  else lcd.print("     ");
  lcd.print(" ");
  LcdMorse(ones);
  
  ones = Minute % 10; tens = (Minute - ones) / 10;
  lcd.setCursor(4, 1); 
  LcdMorse(tens);
  lcd.print(" ");
  LcdMorse(ones);
  
  ones = Seconds % 10; tens = (Seconds - ones) / 10;
  lcd.setCursor(8, 2); 
  LcdMorse(tens);
  lcd.print(" ");
  LcdMorse(ones);


//  help screen with normal clock for 0...secondsClockHelp seconds per minute
  if (Seconds < secondsClockHelp)  // show time in normal numbers
  {
    lcd.setCursor(0, 3);
    sprintf(textBuffer, "%02d%c%02d%c%02d", Hour, dateTimeFormat[dateFormat].hourSep, Minute, dateTimeFormat[dateFormat].minSep, Seconds);
    lcd.print(textBuffer);
  }
  else
  {
    lcd.setCursor(0, 3);lcd.print(F("        "));
  }
  lcd.setCursor(18, 3); lcd.print(F("  ")); // blank out number in lower right-hand corner 
}

/*****
Purpose: Menu item
Finds local time written out with words

Argument List: none

Return value: Displays on LCD
*****/

const char WordOnes[10][6] PROGMEM = {{" oh  "}, {"one  "}, {"two  "}, {"three"}, {"four "}, {"five "}, {"six  "}, {"seven"}, {"eight"}, {"nine "}}; // left justified
const char CapiOnes[10][6] PROGMEM = {{"Oh   "}, {"One  "}, {"Two  "}, {"Three"}, {"Four "}, {"Five "}, {"Six  "}, {"Seven"}, {"Eight"}, {"Nine "}}; // left justified
const char WordTens[6][7]  PROGMEM  = {{"    Oh"}, {"   Ten"}, {"Twenty"}, {"Thirty"}, {" Forty"}, {" Fifty"}}; 
const char Teens[10][10]   PROGMEM   = {{"         "},{"Eleven   "},{"Twelve   "},{"Thirteen "}, {"Fourteen "}, {"Fifteen  "}, {"Sixteen  "}, {"Seventeen"}, {"Eighteen "}, {"Nineteen "}};

void WordClockEnglish()
{
int ones, tens;

/* The longest symbol
 *  Hours:            xx: ?? 
 *  Minutes, seconds: 37: Thirty-seven
 *  Longest symbol is 5+1+7+1+7 = 21 letters long, so it doesn't fit a single line on a 20 line LCD
 */

//  get local time
local[timeZoneNumber] = now() + utcOffset * 60;
Hour = hour(local[timeZoneNumber]);
Minute = minute(local[timeZoneNumber]);
Seconds = second(local[timeZoneNumber]);

lcd.setCursor(0, 0); 
if (Hour < 10) 
{
  lcd.print(reinterpret_cast<const __FlashStringHelper*>(CapiOnes[int(Hour)])); lcd.print(F("      "));
}
else if (Hour > 10 && Hour < 20) lcd.print(reinterpret_cast<const __FlashStringHelper*>(Teens[int(Hour)-10]));
else
{
  ones = Hour % 10; tens = (Hour - ones) / 10;
  lcd.print(reinterpret_cast<const __FlashStringHelper*>(WordTens[tens])); 
  if (ones != 0) 
  {
    if (tens!=0) lcd.print("-");
    else lcd.print(" ");
    lcd.print(reinterpret_cast<const __FlashStringHelper*>(WordOnes[ones]));
  } 
  else 
  {
    if (tens == 0) lcd.print(reinterpret_cast<const __FlashStringHelper*>(WordOnes[ones])); 
    lcd.print(F("       ")); // added extra space 14.5.2023
  }
}

lcd.setCursor(2,1); 
if (Minute > 10 && Minute < 20) lcd.print(reinterpret_cast<const __FlashStringHelper*>(Teens[int(Minute)-10]));
else
{
  ones = Minute % 10; tens = (Minute - ones) / 10;
  lcd.print(reinterpret_cast<const __FlashStringHelper*>(WordTens[tens])); 
  if (ones != 0) 
  {
    if (tens!=0) lcd.print("-"); 
    else lcd.print(" "); 
    lcd.print(reinterpret_cast<const __FlashStringHelper*>(WordOnes[ones]));
  }
   else 
  {
    if (tens == 0) lcd.print(reinterpret_cast<const __FlashStringHelper*>(WordOnes[ones])); 
    lcd.print(F("      "));
  }
}


lcd.setCursor(4, 2); 
if (Seconds > 10 && Seconds < 20) lcd.print(reinterpret_cast<const __FlashStringHelper*>(Teens[int(Seconds)-10]));
else
{

// not rewritten due to 2048 character limit:
  ones = Seconds % 10; tens = (Seconds - ones) / 10;
    lcd.print(reinterpret_cast<const __FlashStringHelper*>(WordTens[tens])); 
    if (ones != 0) 
    {
      if (tens!=0) lcd.print("-");
      else lcd.print(" "); 
      lcd.print(reinterpret_cast<const __FlashStringHelper*>(WordOnes[ones]));
    }
    else 
    {
      if (tens == 0) lcd.print(reinterpret_cast<const __FlashStringHelper*>(WordOnes[ones])); 
      lcd.print(F("      "));
    }
  }
   
   lcd.setCursor(0, 3);  lcd.print(F("        "));
   lcd.setCursor(18, 3); lcd.print(F("  ")); // blank out number in lower right-hand corner 
}



/*****
Purpose: Menu item
Run a demo by cycling through all specified memu items

Argument List: int inDemo: 0 or 1
                           1 - first time in demo mode, initialization
                           0 - already in demo mode

Return value: Displays on LCD

Issues: may take up to dwellTimeDemo seconds before first screen is shown (typ 10-15 sec max)
*****/

void DemoClock(int inDemo // 0 or 1
)  
{   
    //int dwell = dwellTimeDemo;
    if (inDemo == 0) // already in demo mode
        screenSelect(demoDispState, 1);
    else             // first time in demo mode
     {
        lcd.setCursor(0,0); lcd.print(F("  *** D E M O ***   ")); 
        lcd.setCursor(0,1); lcd.print(F("Multi Face GPS Clock"));
        lcd.setCursor(0,3); lcd.print(F("github/la3za ")); 
                            PrintFixedWidth(lcd, dwellTimeDemo, 3); lcd.print(F(" sec"));
     }
}


/*****
Purpose: 
Selects WordClock language 

Argument List: none

Return value: none
*****/

void WordClock()
{
    if ((languageNumber >=0) && !(strcmp(languages[languageNumber],"no")))  WordClockNorwegian();
    else                                                                    WordClockEnglish();
}

/*****
Purpose: Menu item
Shows info about 3 outer and 2 inner planets + alternates between solar/lunar info

Argument List: inner = 1 for inner planets, else outer planets

Return value: Displays on LCD
*****/

void PlanetVisibility(int inner // inner = 1 for inner planets, all other values -> outer
) {
//
// Agrees with https://www.heavens-above.com/PlanetSummary.aspx?lat=59.8348&lng=10.4299&loc=Unnamed&alt=0&tz=CET
// except for brightness or magnitude of mercury: this code says 0.2 when web says 0.6
  
  // Julian day ref noon Universal Time (UT) Monday, 1 January 4713 BC in the Julian calendar:
  //jd = get_julian_date (20, 1, 2017, 17, 0, 0);//UTC
  Seconds = second(now());
  Minute = minute(now());
  Hour = hour(now());
  Day = day(now());
  Month = month(now());
  Year = year(now());
   
  jd = get_julian_date (Day, Month, Year, Hour, Minute, Seconds); // local
 
  #ifdef FEATURE_SERIAL_PLANETARY
    Serial.println("JD:" + String(jd, DEC) + "+" + String(jd_frac, DEC)); // jd = 2457761.375000;
  #endif
  
  get_object_position (2, jd, jd_frac);//earth -- must be included always
  
  lcd.setCursor(0, 0); // top line ********* 
  lcd.print(F("    El"));lcd.write(DEGREE);lcd.print(F(" Az"));lcd.write(DEGREE);lcd.print(F("   % Magn"));
  
  if (inner==1){
    get_object_position (0, jd, jd_frac);
    lcd.setCursor(0, 2);
    lcd.print(F("Mer ")); LCDPlanetData(altitudePlanet, azimuthPlanet, phase, magnitude);
    
    lcd.setCursor(0, 3);
    get_object_position (1, jd, jd_frac);
    lcd.print(F("Ven ")); LCDPlanetData(altitudePlanet, azimuthPlanet, phase, magnitude);

    lcd.setCursor(0,1); 
    if ((now() / 10) % 2 == 0)   // change every 10 seconds
      { 
// Moon
      float Phase, PercentPhase;
      lcd.print(F("Lun "));
      UpdateMoonPosition();
      MoonPhase(Phase, PercentPhase);
      LCDPlanetData(moon_elevation, moon_azimuth, PercentPhase/100., -12.7);
    }
    else
    {
// Sun
      lcd.print(F("Sun "));

      /////// Solar elevation //////////////////
      
      cTime c_time;
      cLocation c_loc;
      cSunCoordinates c_sposn;
      double dElevation;
      double dhNoon, dmNoon;
    
      c_time.iYear = yearGPS;
      c_time.iMonth = monthGPS;
      c_time.iDay = dayGPS;
      c_time.dHours = hourGPS;
      c_time.dMinutes = minuteGPS;
      c_time.dSeconds = secondGPS;
    
      c_loc.dLongitude = lon;
      c_loc.dLatitude  = latitude;
    
      c_sposn.dZenithAngle = 0;
      c_sposn.dAzimuth = 0;
    
      float sun_azimuth = 0;
      float sun_elevation = 0;
    
      sunpos(c_time, c_loc, &c_sposn);
    
      // Convert Zenith angle to elevation
      sun_elevation = 90. - c_sposn.dZenithAngle;
      sun_azimuth = c_sposn.dAzimuth;

      LCDPlanetData(round(sun_elevation), round(sun_azimuth), 1., -26.7);

    }
  
  }
  else  // outer planets
  {
    get_object_position (3, jd, jd_frac);
    lcd.setCursor(0, 1);
    lcd.print(F("Mar ")); LCDPlanetData(round(altitudePlanet), round(azimuthPlanet), phase, magnitude);
    
    get_object_position (4, jd, jd_frac);
    lcd.setCursor(0, 2);
    lcd.print(F("Jup ")); LCDPlanetData(round(altitudePlanet), round(azimuthPlanet), phase, magnitude);
    
    get_object_position (5, jd, jd_frac);
    lcd.setCursor(0, 3);
    lcd.print(F("Sat ")); LCDPlanetData(round(altitudePlanet), round(azimuthPlanet), phase, magnitude);

    if (full) get_object_position (6, jd, jd_frac); // Uranus
    if (full) get_object_position (7, jd, jd_frac); // Neptune
  }
}  

/*****
Purpose: Menu item
Shows local time in 4 different calendars: Gregorian (Western), Julian (Eastern), Islamic, Hebrew

Argument List: inner = 1 for inner planets, else outer planets

Return value: Displays on LCD

Issues: Hebrew calendar is quite slow (3+ seconds) on an Arduino Mega
*****/



const char IslamicMonth[12][10] PROGMEM {"Muharram ", "Safar    ", "Rabi I   ", "Rabi II  ", "Jumada I ", "Jumada II","Rajab    ", "Sha'ban  ", "Ramadan  ", "Shawwal  ", "DhuAlQada", "DhuAlHija"}; // left justified
const char HebrewMonth[13][10]  PROGMEM {"Nisan    ", "Iyyar    ", "Sivan    ", "Tammuz   ", "Av       ", "Elul     ", "Tishri   ", "Heshvan  ", "Kislev   ", "Teveth   ", "Shevat   ", "Adar     ", "Adar II  "}; // left justified

void ISOHebIslam() {     // ISOdate, Hebrew, Islamic

#ifdef FEATURE_DATE_PER_SECOND   // for stepping date quickly and check calender function
    local[timeZoneNumber] = now() + utcOffset * 60 + dateIteration*86400; // fake local time by stepping up to 1 sec/day
    dateIteration = dateIteration + 1;
//  Serial.print(dateIteration); Serial.print(": ");
//  Serial.println(local[timeZoneNumber]);
#endif

// algorithms in Nachum Dershowitz and Edward M. Reingold, Calendrical Calculations,
// Software-Practice and Experience 20 (1990), 899-928
// code from https://reingold.co/calendar.C

       lcd.setCursor(0, 0); // top line *********
       // all dates are in local time
       GregorianDate a(month(local[timeZoneNumber]), day(local[timeZoneNumber]), year(local[timeZoneNumber]));      
       LcdDate(a.GetDay(), a.GetMonth(), a.GetYear());

////    Serial.print("Absolute date ");Serial.println(a);

        lcd.setCursor(10, 0);
        IsoDate ISO(a);
        lcd.print(F(" Week   ")); lcd.print(ISO.GetWeek());             
        lcd.setCursor(0, 1);
        JulianDate Jul(a);
        LcdDate(Jul.GetDay(), Jul.GetMonth(), Jul.GetYear());     
        if (now() % 10 < 5)
        {
          lcd.print(F(" Julian   "));
        }
        else
        {
          lcd.setCursor(0, 1);     
          int ByzYear = Jul.GetYear() + 5508;             // Byzantine year = Annus Mundi rel to Sept 1, 5509 BC
          if (Jul.GetMonth()>= 9) ByzYear = ByzYear + 1;  // used by the Eastern Orthodox Church from 
          LcdDate(Jul.GetDay(), Jul.GetMonth(), ByzYear); // c. 691 to 1728 https://en.wikipedia.org/wiki/Byzantine_calendar
          lcd.print(F(" Byzantine"));
        }
        lcd.setCursor(0, 2);
        IslamicDate Isl(a);
        int m;
        m = Isl.GetMonth();
        LcdDate(Isl.GetDay(), m, Isl.GetYear()); 
        lcd.print(" "); 
        lcd.print(reinterpret_cast<const __FlashStringHelper*>(IslamicMonth[m-1]));      
         
         // Hebrew calendar is complicated and *** very *** slow - takes ~3 sec. Therefore it is on the last line and only done occasionally           
        if (now() % 5 == 0)    // only check every 5 or 10 sec. Otherwise no time for the clock to read encoder or buttons
          {
 //           Serial.print(now());Serial.println(" Hebrew ...");
              lcd.setCursor(0, 3);
              HebrewDate Heb(a);  
              m = Heb.GetMonth();
              LcdDate(Heb.GetDay(), m, Heb.GetYear());
              lcd.print(" "); 
              lcd.print(reinterpret_cast<const __FlashStringHelper*>(HebrewMonth[m-1]));
          } 

//      Serial.println("Leaving ISOHebIslam ...");
}


/*****
Purpose: 
Display all kinds of GPS-related info

Argument List: none

Return value: none
*****/ 

 void GPSInfo()
{
  // builds on the example program SatelliteTracker from the TinyGPS++ library
  // https://www.arduino.cc/reference/en/libraries/tinygps/

  float SNRavg = 0;
  int total = 0;
  float hdop;  

    #ifdef FEATURE_SERIAL_GPS 
      Serial.println(F("GPSInfo"));
    #endif
    
    if (totalGPGSVMessages.isUpdated())  
    {

//  https://github.com/mikalhart/TinyGPSPlus/issues/52
   
      int totalMessages = atoi(totalGPGSVMessages.value());
      int currentMessage = atoi(messageNumber.value());    
      
      if (totalMessages == currentMessage)
      {   
      #ifdef FEATURE_SERIAL_GPS 
        Serial.print(F("Sats in use = ")); Serial.print(gps.satellites.value());
        Serial.print(F(" Nums = "));
      
        for (int i=0; i<MAX_SATELLITES; ++i)
        {
          if (sats[i].active)
          {
            Serial.print(i+1);
            Serial.print(F(" "));
          }
        }
      #endif
       
        // int total = 0;
        // float SNRavg = 0;
        #ifdef FEATURE_SERIAL_GPS 
          Serial.print(F(" SNR = "));
        #endif
        
        for (int i=0; i<MAX_SATELLITES; ++i)
        {
          if (sats[i].active)
          {
            #ifdef FEATURE_SERIAL_GPS 
                Serial.print(sats[i].snr);
                Serial.print(F(" "));
            #endif
            if (sats[i].snr >0)          // 0 when not tracking
            {
              total = total + 1; 
              SNRavg = SNRavg + float(sats[i].snr);
            }
          }
        }
        // moved this } from end of routine to here on 19.11.2022:
       }                                 // if (totalMessages == currentMessage)
       
        lcd.setCursor(0,0); lcd.print(F("In view ")); //printFixedWidth(lcd, satsInView.value(), 2);
        if (*satsInView.value() < 10)     lcd.print(" ");
        lcd.print(satsInView.value()); lcd.print(F(" Sats"));
        
        noSats = gps.satellites.value();  // in list of http://arduiniana.org/libraries/tinygpsplus/
        lcd.setCursor(0,1); lcd.print(F("In fix  ")); //printFixedWidth(lcd, noSats, 2); 
        PrintFixedWidth(lcd, noSats, 2);
        //if (noSats<10) lcd.print(" ");
        //lcd.print(noSats);

        if (total>0) SNRavg = SNRavg/total; 
        else         SNRavg = 0;               // 16.11.2022
          
        #ifdef SNR_GPSInfo
          // unreliable SNR number (sometimes shows 0 and other times 99 dB) ?
          lcd.print(F(" SNR "));
          PrintFixedWidth(lcd, (int)SNRavg, 2);
          lcd.print(F(" dB"));
        #endif
        //
        lcd.setCursor(0,2); lcd.print(F("Mode    ")); lcd.print(GPSMode.value());lcd.print(F("D Status  "));
        lcd.print(posStatus.value()); 
        
        hdop   = gps.hdop.hdop();  // in list of http://arduiniana.org/libraries/tinygpsplus/
        lcd.setCursor(0,3); lcd.print(F("Hdop  ")); lcd.print(hdop); 
     
        if      (hdop<1) lcd.print(F(" Ideal    "));// 1-2 Excellent, 2-5 Good https://en.wikipedia.org/wiki/Dilution_of_precision_(navigation)
        else if (hdop<2) lcd.print(F(" Excellent"));
        else if (hdop<5) lcd.print(F(" Good     "));
        else             lcd.print(F(" No good  "));
     
       #ifdef FEATURE_SERIAL_GPS 
            Serial.print(F(" Total=")); Serial.print(total);
            Serial.print(F(" InView=")); Serial.print(satsInView.value());       
                
            Serial.print(F(" SNRavg=")); Serial.print((int)SNRavg);
            Serial.print(F(" Mode = ")); Serial.print(GPSMode.value()); // 1-none, 2=2D, 3=3D
            Serial.print(F(" Status = ")); Serial.print(posStatus.value()); // A-valid, V-invalid
            Serial.println();
        #endif

          for (int i=0; i<MAX_SATELLITES; ++i)
            sats[i].active = false; 
// 
// removed 22.5.2023: made the display blink between valid values and 0 values as update doesn't happen every second
  }                                   // if (totalGPGSVMessages.isUpdated())
  
//  else                              // new 19.11.2022, purpose? show loss of signal?
//  {
//          lcd.setCursor(0,0); lcd.print(F("In view  ")); lcd.print(0); lcd.print(F(" Sats"));         
//          lcd.setCursor(0,1); lcd.print(F("In fix   ")); lcd.print(0);
//          lcd.print(F(" SNR  "));lcd.print(0); lcd.print(F(" dB"));
//        
//          lcd.setCursor(0,2); lcd.print(F("Mode       Status   ")); 
//          lcd.setCursor(0,3); lcd.print(F("Hdop                ")); 
//  }
 
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// End of functions for Menu system ////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/// THE END ///



///// end /////
