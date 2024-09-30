// Set version and date manually for code status display
const char codeVersion[] = "v2.2.1    30.09.2024";

// or set date automatically to compilation date (US format) - nice to use during development - while version number is set manually
//const char codeVersion[] = "v2.1.1   "__DATE__;

/*
  LA3ZA Multi Face GPS Clock

  Copyright 2015 - 2024 Sverre Holm, LA3ZA
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

  Full documentation can be found on https://github.com/la3za .  Please read it before requesting help.


  Features:
           GPS clock with 20x4 LCD display
           Controlled by a GPS module outputting data over a serial interface, and optionally using the PPS output for accurate sync
           typ. QRPLabs QLG1 GPS Receiver kit, or the QLG2
           GPS is handled with the TinyGPS++ library
           Shows raw GPS data such as UTC time and date, position, altitude, and number of satellitess
           Also with various forms of binary, BCD, hex and octal displays
           Shows derived GPS data such as 6-digit locator
           Finds local time and handles daylight saving automatically for any location using the Timezone library
           Finds local sunset and sunrise, either actual value, or civil, nautical, or astronomical using the Sunrise library
           The clock also gives local solar height and provides the lunar phase as well as predicts next rise/set time for the moon
           Azimuth and elevation is found for all planets

           Input:   from GPS, data and optionally also PPS (pulse per second) pulse
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
                      PWM control of LCD backlight
                    Planet data, azimuth, elevation
                    ...  
*/

/*
 Revisions:
 2.2.1   30.09.2024: 
                - Bugfix in clock_z_planets.h: now reads GPS position, and not (0.0,0.0) as in v2.2.0 or Oslo as in v2.1.0.

 2.2.0   09.06.2024:
                - NextEvents() - New screen showing Next Equinox/Solstice, Easter, Lunar/Solar Eclipses in sorted order
                - SolarEclipse() - New screen showing solar eclipses for this year and coming years (table-based -> 2030)
                - Equinoxes() - New screen for time of equinoxes and solstices

                - Demo mode: Choice of increase, decrease, or random next screen number in Secondary menu
                - More languages selectable: {"en", "es", "fr", "de", "no", "se", "dk", "is", ... 
                    Any language with max 4 special letters can be implemented
                - InternalTime(): Julian Day - jd - (since 4713 BC) displayed instead of local now()
                --  jd variable renamed to jd1970 in InternalTime() and Sidereal() 
                - UTCLocator(1) now has same layout as LocalUTC(1)
                -- UTCLOCALLANGUAGE if defined (default): UTCLocator() day also in local language, was always English 
                - No longer need to reset processor on GPS baudrate change. 
                -- CodeStatus shown for a brief period after setting GPS baudrate or PPS flag
                - Rewritten to use the more accurate https://github.com/jpb10/SolarCalculator longer instead of 
                     https://github.com/chaeplin/Sunrise and sunpos from https://github.com/k3ng/k3ng_rotator_controller/tree/master/libraries, 
                -- Sunrise library was inaccurate, obsolete, and won't compile for Metro Express without a fix
                - Now runs on Adafruit Metro Express (M0). Much faster Hebrew calendar
                -- Some fixes to code which was OK for Mega, but not Metro Express: RotarySetup(), Sidereal(), [Sunrise.cpp]
                -- Metro Express: Flash as EEPROM OK but Reminder() won't work with flash. Interrupt works, but not software reset 
                - Changed definition of languageNumber: -1 was English, now 0 is English
                - Saved dynamic memory by using lcd.print(F(" ")) and putting MyDays in PROGMEM
                
                - Fixes for ver 2.1.0:
                -- More accurate prediction of sun rise/set times
                -- Nesting error in RotarySetup(): case 4: local language. Only apparent for fast processor like Metro
                -- More accurate formula for Mercury, Venus, Mars, Jupiter magnitudes in clock_z_planets.h, now follows Meeus [thanks Richard]
                -- Islamic, Hebrew clock's second didn't increment immediately. Now less latency as routine is entered and at full minute
                
 2.1.0   21.01.2024:
                - Implemented interrupt-driven time setting from GPS PPS (optional): second changes more accurately (inspired by Bruce E. Hall GPS clock)
                -- New parameters in Secondary menu: PPS enabled/disabled
                - Several sets of user-defined characters for LCD enabled:
                  -- WSPR: Progressbar showing on lower line
                  -- New screens: BigNumbers3 clock with digits w=3 x h=2 squares big: local time or UTC (the best looking ones)
                  -- New screens: BigNumbers2 clock with digits w=2 x h=3 squares big: local time or UTC
                  -- LinearUhr now indicates 5 x 12 second ticks within minute square
                  -- Clock based on Bar() now shows real bar of width 1, 2, 3, 4 by switching user-defined character sets
                - More screens that change every 5/10 seconds. In addition to ScreenUTCPosition, ScreenNCDXFBeacons1, 
                                                                             ScreenNCDXFBeacons2, ScreenPlanetsInner
                  -- also ScreenLocalSunSimpler, ScreenISOHebIslam, ScreenReminder  
                - 4 ArrowCharacters now in PROGMEM, saves 8 bytes per LCD character, ie. 32 for 4 arrows
                - ISOHebIslam now has display of lunar phase with lunar calendars, Islamic, Hebrew
                - Changed several "if (cond1 & cond2)" to "if (cond1 && cond2)" 
                - Removed support for potentiometer for backlight control as it is now done through menu system
                - Fixes for ver 2.0.0:
                -- Completely rewritten and corrected multiple timezone handling with Timezone library
                -- NCDXF beacon switch-over was 1 second too late, now corrected
                -- Changed loop counter for Demo, no longer absolute time as it failed for corrupt GPS time
                -- Better test of GPS quality before estimating SNR
                -- Small formatting fixes for LCD display and updating when timing out from Setup


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

/* Functions in this file:
            syncCheck
            ppsHandler
            readGPS
            timeZones
            getTimeGPS
            updateDisplay
            checkEncoder
            readButtons
            readEEPROM
            ScreenSelect

            setup
            loop
            
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
            BigNumbers3
            BigNumbers2
            Reminder    

*/

// LCD characters: set 0: arrows for solar rise/set (default):
#define DASHED_UP_ARROW 1    // user defined character
#define DASHED_DOWN_ARROW 2  // user defined character
#define UP_ARROW 3           // user defined character
#define DOWN_ARROW 0         // user defined character, 26.12.2023: from 4 to 0
// set 1: bars for Bar()
#define ONE_BAR 1     // user defined character
#define TWO_BARS 2    // user defined character
#define THREE_BARS 3  // user defined character
#define FOUR_BARS 0   // user defined character, 26.12.2023: from 4 to 0
// predefined:
#define DEGREE 223   // in LCD character set
#define ALL_OFF 254  // in LCD character set
#define ALL_ON 255   // in LCD character set
#define DOT 165      // dot for date deliminator, Morse code, and for big letter clock

#define EEPROM_OFFSET1 0    // first address for setup info in EEPROM, adresses used: EEPROM_OFFSET1 ... EEPROM_OFFSET1+9
#define EEPROM_OFFSET2 100  // first address for birthday info for Reminder()

#define noOfScreens 50  // must be large enough to hold all possible screens in menu!!
#define NUMBER_OF_TIME_ZONES 20  // no of time zones defined in clock_timezone.h

#define RAD (PI / 180.0)
//#define SMALL_FLOAT (1e-12)

#include "clock_defines.h"
#include "clock_debug.h"  // debugging options via serial port

char textBuffer[21];  // 1 line on lcd, 20 characters; For display of strings

const int lengthOfMenuIn = noOfScreens;
byte menuOrder[noOfScreens];  // chosen submenu is here, from int --> byte
int noOfStates = 0;           // no of actual entries in chosen submenu_name

long utcOffset = 0;  // value set automatically by means of Timezone library

struct Menu_type
{
  char  descr[10];   // shortened from [15], 10.02.2024
  int8_t order[noOfScreens]; 
};

struct Date_Time
{
  char descr[10];
  char dateOrder;
  char dateSep;
  char hourSep;
  char minSep;
};

#include "clock_pin_settings.h"  // hardware pins
#include "clock_options.h"       // customization of order and number of menu items

//////////////////////////////////////

// libraries need to be installed according to how the Arduino environment expects them.
// See https://docs.arduino.cc/software/ide-v1/tutorials/installing-libraries
// See project wiki for source for alle the libraries

#include <TimeLib.h>           // https://github.com/PaulStoffregen/Time - timekeeping functionality
#include <Timezone_Generic.h>  // https://github.com/khoih-prog/Timezone_Generic
#include <SolarCalculator.h>   // https://www.arduino.cc/reference/en/libraries/solarcalculator/ 19.02.2024


#include <TinyGPS++.h>  // http://arduiniana.org/libraries/tinygpsplus/

#include "clock_z_calendar.h"

Timezone tz;  // holds currently selected timezone

#include "clock_timezone.h"  // timezone definitions, including daylight saving rules

#if defined(FEATURE_LCD_I2C)
  #include <Wire.h>               // For I2C. Comes with Arduino IDE
  #include <LiquidCrystal_I2C.h>  // Install NewliquidCrystal_1.3.4.zip https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads/
#endif

#if defined(FEATURE_LCD_4BIT)
  #include <LiquidCrystal.h>
#endif

#include <rotary.h>  // rotary handler https://bitbucket.org/Dershum/rotary_button/src/master/
#include <moon2.h>   // via https://github.com/k3ng/k3ng_rotator_controller/tree/master/libraries

#if defined(FEATURE_LCD_I2C)
//            set the LCD address to 0x27 and set the pins on the I2C chip used for LCD connections:
//                     addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
  LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address
#endif

#if defined(FEATURE_LCD_4BIT)
  LiquidCrystal lcd(lcd_rs, lcd_enable, lcd_d4, lcd_d5, lcd_d6, lcd_d7);
#endif

Rotary r = Rotary(PIN_A, PIN_B, PUSHB);  // Initialize the Rotary object

byte LCDchar0_3 = 0; // 0 means unknown status
byte LCDchar4_5 = 0;
byte LCDchar6_7 = 0;
#define LCDARROWS  1 // for LCDchar0_3
#define LCDNATIVE  2 // for LCDchar4_5, LCDchar6_7
#define LCD3WIDE   3 // for LCDchar0_3, LCDchar4_5, LCDchar6_7
#define LCD3HIGH   4 // for LCDchar0_3, LCDchar4_5, LCDchar6_7
#define LCDGAPLESS 5 // for LCDchar0_3, LCDchar4_5 
#define LCDBARS    6 // for LCDchar4_5
#define LCDARING   7 // for LCDchar6_7

byte buffer[8];  // temporary storage for PROGMEM characters before writing to LCD

static uint32_t gpsBaud;  // stores baud rate for GPS, read from gpsBaud1 - array
int dispState;            // depends on rotary, decides which screen to display
int demoDispState;        // decides what to display in Demo Mode
int demoDuration = 0;     // counter for time between Demo screens

char today[15];           // for storing string with day name 
char todayFormatted[15];  // for storing string with day name up to 12 characters long
double latitude, lon, alt;
int Year;
byte Month, Day, Hour, Minute, Seconds;
uint32_t noSats; //was u32 

TimeChangeRule *tcr;  //pointer to the time change rule (tcr), use to get TZ abbrev
time_t utc, localTime;
time_t prevDisplay = 0;  // keeps time from now(), to find out last time when the digital clock was displayed

int packedRise;
double moon_azimuth = 0;
double moon_elevation = 0;
double moon_dist = 0;

int iiii;            // general loop counter
int oldMinute = -1;  // compared to minuteGPS in order to get immediate display of some info

int yearGPS;
uint8_t monthGPS, dayGPS, hourGPS, minuteGPS, secondGPS, weekdayGPS;
volatile byte pps = 0;  // GPS one-pulse-per-second flag

/*
  Uses Serial1 for GPS input
  4800; // OK for EM-406A and ADS-GM1
  9600; // OK for NEO-6M
  Serial1 <=> pin 19 on Mega
*/
TinyGPSPlus gps;  // The TinyGPS++ object

#include "clock_z_planets.h"   // moved from line 318 to here 22.09.2024, must be down here to read longitude correct in clock_z_planets.h
#include "clock_z_lunarCycle.h"

// Initial values for primary menu parameters - stored in EEPROM
int8_t subsetMenu = 0;      // variable for choosing subset of possible clock faces
byte backlightVal = 10;     // (0...255) initial backlight value
int8_t dateFormat = 0;      // variable for choosing date/time format
int8_t languageNumber = 0;  // variable for choosing language for local time day names
int8_t presentLanguageNumber = 0; // remembering the present language for detecting need for change
int8_t timeZoneNumber = 0;  // variable for choosing time zone

// Initial values for secondary menu parameter - stored in EEPROM
int8_t baudRateNumber = 1;     // points to entry in array of possible baudrates
int8_t demoStepType = 0;       // step type in demo (increase +, decrease -, random)
int8_t dwellTimeDemo = 10;     // no of seconds per screen as DemoClock cycles through all screen
int8_t secondsClockHelp = 6;   // no of seconds per minute of normal clock display for fancy clocks
int8_t mathSecondPeriod = 10;  // 1...6 per minute, i.e. 10-60 seconds in AlbertClock app
boolean using_PPS = false;     // toggle use of PPS pulse from GPS for interrupt and more accurate timeing

char demoStepTypeText[][7] = {"+", "-", "random"}; // hard-coded index range 0..2 for demoStepType here and there in code

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

TinyGPSCustom totalGPGSVMessages(gps, "GPGSV", 1);  // $GPGSV sentence, first element
TinyGPSCustom messageNumber(gps, "GPGSV", 2);       // $GPGSV sentence, second element
TinyGPSCustom satsInView(gps, "GPGSV", 3);          // $GPGSV sentence, third element
TinyGPSCustom GPSMode(gps, "GPGSA", 2);             // $GPGSA sentence, 2nd element 1-none, 2=2D, 3=3D
TinyGPSCustom posStatus(gps, "GPRMC", 2);           // $GPRMC sentence: Position status (A = data valid, V = data invalid)
TinyGPSCustom satNumber[4];                         // to be initialized later
TinyGPSCustom elevation[4];
TinyGPSCustom azimuth[4];
TinyGPSCustom snr[4];

struct
{
  bool active;
  int elevation;
  int azimuth;
  int snr;
} sats[MAX_SATELLITES];

float SNRAvg = 0.0;
int totalSats = 0;

#ifdef FEATURE_DATE_PER_SECOND  // for stepping date quickly and check calender function
  int dateIteration;
#endif

#include "clock_language.h"         // user customable functions and character sets for multiple local languages, was "clock_custom_routines.h"
#include "clock_helper_routines.h"  // library of functions

#include "clock_z_moon_eclipse.h"
#include "clock_z_equatio.h"

//#include "clock_development.h"  // uncomment if new function is under development

//////////////////////////////////////////////////////////////////////////////////////////////
void readGPS() {
  // ******** start gps time update
  #ifndef FEATURE_FAKE_SERIAL_GPS_IN
    while (Serial1.available()) {
      if (gps.encode(Serial1.read())) {           // process gps messages from hw GPS (default mode)

  #else  // normal mode:
    while (Serial.available()) {
      if (gps.encode(Serial.read())) {            // process gps messages from sw GPS emulator
  #endif 
      }  // (gps.encode(Serial.read()))
    }    // while (Serial.available())
}

///////////////////////////////////////////////////////////////////////////////////////

void syncCheck() {                         // from GPS_Clock_triple.ino by Bruce E. Hall, w8bh.net
  if (pps || (!using_PPS)) syncTimeGPS();  // is it time to sync with GPS?
  pps = 0;                                 // reset flag, regardless
}
//////////////////////////////////////////////////
//                                                    from GPS_Clock_triple.ino by Bruce E. Hall, w8bh.net
void ppsHandler() {  // 1pps interrupt handler:
  pps = 1;           // flag that signal was received
  #ifdef FEATURE_INTERRUPTTEST
    state = !state; // for Built in LED
  #endif
}
////////////////////////////////////////////////////////////////////////////////

void syncTimeGPS() {
  //if (gps.time.isValid())     // 29.08.2023 as gps.time.* was sometimes way off (due to corrupt GPS data?)
  if (gps.time.isValid()) // && gps.location.age() < 500)  // age in millisecs: too strict. W8BH uses "<= 1000"
  {
    // when GPS reports new data...
    hourGPS   = gps.time.hour();
    minuteGPS = gps.time.minute();
    secondGPS = gps.time.second();
    dayGPS    = gps.date.day();
    monthGPS  = gps.date.month();
    yearGPS   = gps.date.year();
                             
    // set the Time to the latest GPS reading
    setTime(hourGPS, minuteGPS, secondGPS, dayGPS, monthGPS, yearGPS);  // Versions from 17.04.2020+: Arduino time = UTC
    weekdayGPS = weekday();
    if (using_PPS) adjustTime(1);  // if using interrupt adjust forward 1 second (only integer seconds)

    utc = now();  // updated even if GPS data stream stops in order to avoid frozen UTC display
                  //if (using_PPS) utc = utc + 1; // if using interrupt adjust forward 1 second
                  //if (!using_PPS) delay(500); // without pps, clock is x00 ms too late (Arduino Mega), but this makes it irregular ...

    tz = *timeZones_arr[timeZoneNumber];  //
    localTime = tz.toLocal(utc, &tcr);

#ifdef AUTO_UTC_OFFSET                                    // the usual mode
    utcOffset = localTime / long(60) - now() / long(60);  // min, order of calculation is important
#else
    localTime = now() + utcOffset * 60;                        // utcOffset in minutes is set manually in clock_options.h (was in clock_zone.h)
#endif
  }  // gps.time.isValid

#ifdef FEATURE_SERIAL_TIME
  Serial.print(F("Utc         "));  Serial.println(now());
  Serial.print(F("Local       "));  Serial.println(localTime);
  //     Serial.print(F("diff [sec]  ")); Serial.println(long(localTime - now()));
  // //  Serial.print(F("diff,m1 ")); Serial.println(long(localTime - now()) / long(60));
  //     Serial.print(F("diff [min]  ")); Serial.println(long(localTime) / long(60) - long(now() / long(60));
  //     Serial.print(F("utcOffset: "));
  //     Serial.println(utcOffset);
  //     Serial.print(F("The time zone is: ")); Serial.print(timeZoneNumber); Serial.print(F(" ")); Serial.println(tcr[timeZoneNumber] -> abbrev);
#endif
}  // ******** end syncTimeGPS()

////////////////////////////////////////////////////////////////////////////////
void updateDisplay() {
  if (timeStatus() != timeNotSet) {
    if (now() != prevDisplay) {  //update the display only if the time has changed. i.e. every second
      prevDisplay = now();
      demoDuration = min(demoDuration + 1, 10000);  // limit it in order not to overflow

      // this is for jumping from screen to screen in demo Mode:
      if ((dispState == menuOrder[ScreenDemoClock]) && (demoDuration >= dwellTimeDemo))  // demo mode: increment screen number, new 30.8.2023
      {
        if (demoStepType == 1)
          demoDispState -= 1;                       // decrement screen number in demo mode                              
        else if (demoStepType == 2)
          demoDispState = random(0, noOfStates-1);  // random choice of screen number in demo mode (noOfStates-1 in order not to recursively select demo Mode itself)
        else
          demoDispState += 1;                     // increment screen number in demo mode

        if (demoDispState < 0) demoDispState += noOfStates;  // but not below the lowest number
        demoDispState = demoDispState % noOfStates;          // or above the largest number
#ifdef FEATURE_SERIAL_MENU
        Serial.print(F("demoDispState "));
        Serial.println(demoDispState);
        Serial.print(F("dispState "));
        Serial.print(dispState);
        Serial.print(", ");
        Serial.print(menuOrder[ScreenDemoClock]);
        Serial.print(F(", oldMinute "));
        Serial.println(oldMinute);
        //            Serial.print(F("now() ")); Serial.println(now());
        //            Serial.print(minute(now())); Serial.print(":");Serial.println(second(now()));
        //Serial.print(F("demoDuration "));
        //Serial.println(demoDuration);
        Serial.println(" ");
#endif
        demoDuration = 0;  // reset counter of seconds between demo screen
        lcd.clear();       // clear if demo just started ? - no, happens every time
        oldMinute = -1;    // to get immediate display of some info. Moved here 27.6.2023
      }

      ////////////////////////////////////////// USER INTERFACE /////////////////////////////////////////////////////////
#ifdef FEATURE_SERIAL_MENU
      //Serial.print(F("dispState ")); Serial.println(dispState);
      //Serial.println((dispState % noOfStates));
      //Serial.println(menuOrder[dispState % noOfStates]);
#endif

      ////////////// This is the order of the menu system unless menuOrder[] contains information to the contrary

      ScreenSelect(dispState, 0);  // select right routine for chosen screen, 0 = ordinary, i.e. not demo mode

    }  // if (now() != prevDisplay)
  }    // if (timeStatus() != timeNotSet)
}


////////////////////////////////////////////////////////////////////////////////

void checkEncoder()  // check and read rotation and button of rotary encoder
{
  volatile unsigned char rotaryResult = r.process();
  if (rotaryResult)  // change clock face number by rotation
  {
    if (rotaryResult == DIR_CCW)  // Counter clockwise: decrease screen number
    {
      dispState = (dispState - 1) % noOfStates;    // decrement screen number
      if (dispState < 0) dispState += noOfStates;  // roll-over if <0
#ifdef FEATURE_SERIAL_MENU
      Serial.println(F("CCW"));
#endif
      //delay(50);
    } else  // Clockwise: increase screen number
    {
      dispState = (dispState + 1) % noOfStates;
#ifdef FEATURE_SERIAL_MENU
      Serial.println(F("CW"));
#endif
      //delay(50);
    }

    lcd.clear();
    oldMinute = -1;  // to get immediate display of some info
    lcd.setCursor(18, 3);
    PrintFixedWidth(lcd, dispState, 2);  // screen number temporarily in lower right-hand corner
    if (dispState == menuOrder[ScreenDemoClock]) {
      demoDispState = dispState;  // start demo
      demoDuration = 0;           // reset timer for time between screens in demo mode
    }
  }

  if (r.buttonPressedReleased(500))  // 500 ms = long press for reset of processor
  {
    #ifndef ARDUINO_SAMD_VARIANT_COMPLIANCE
      lcd.clear();
      lcd.print(F(" *** R E S E T *** "));
      delay(1000);
      resetFunc();  // call reset
    #endif  
    // #else // https://microchip.my.site.com/s/article/Implement-software-reset-on-SAMD21-SAMW25
    //   #define APP_START 0x00000000 
    //   uint32_t app_start_address; //Global variable
    //   app_start_address = *(uint32_t *)(APP_START + 4);
    //   /* Rebase the Stack Pointer */ 
    //   __set_MSP(*(uint32_t *) APP_START); 
    //   /* Rebase the vector table base address */ 
    //   SCB->VTOR = ((uint32_t) APP_START & SCB_VTOR_TBLOFF_Msk); 
    //   /* Jump to application Reset Handler in the application */ 
    //   asm("bx %0"::"r"(app_start_address)); 
    //#endif
  }

  if (r.buttonPressedReleased(25))  // 25 ms debounce_delay = short press to enter setup menu
  {
    // make sure native characters are loaded (for local language display in RotarySetup)
    loadNativeCharacters(languageNumber);

    RotarySetup();   // call setup
    oldMinute = -1;  // to get immediate display of some info.  09.08.2023
  }
}

#ifdef FEATURE_BUTTONS  // may be in addition to rotary encoder
void readButtons()      // read separate buttons (not the one in rotary encoder)
{
  byte button = AnalogButtonRead(0);  // using K3NG function
  if (button == 2) {                  // increase menu # by one
    dispState = (dispState + 1) % noOfStates;
    if (dispState == menuOrder[ScreenDemoClock]) {
      demoDispState = dispState;  // start demo
      demoDuration = 0;           // reset timer for time between screens in demo mode
    }
    lcd.clear();
    oldMinute = -1;  // to get immediate display of some info
    delay(300);      // was 300
    lcd.setCursor(18, 3);
    PrintFixedWidth(lcd, dispState, 2);  // lower left-hand corner

  } else if (button == 1) {  // decrease menu # by one
    dispState = (dispState - 1) % noOfStates;
    ;
    if (dispState < 0) dispState += noOfStates;
    if (dispState == menuOrder[ScreenDemoClock]) {
      demoDispState = dispState;  // start demo
      demoDuration = 0;           // reset timer for time between screens in demo mode
    }
    lcd.clear();
    oldMinute = -1;  // to get immediate display of some info
    lcd.setCursor(18, 3);
    lcd.print(dispState);
    delay(300);
  }
}
#endif  // FEATURE_BUTTONS

////////////////////////////////////////////////////////////////////////////////

void ScreenSelect(int disp, int DemoMode)  // menu System - called from inside loop [from updateTime()] and from DemoClock
{
  if (disp == menuOrder[ScreenLocalUTC])                LocalUTC(0);            // local time, date; UTC, locator
  else if (disp == menuOrder[ScreenLocalUTCWeek])       LocalUTC(1);            // local time, date; UTC, week #
  else if (disp == menuOrder[ScreenUTCLocator])         UTCLocator(1);          // UTC, locator, # sats

  // Sun, moon:
  else if (disp == menuOrder[ScreenLocalSun])           LocalSun(0);            // local time, sun x 3
  else if (disp == menuOrder[ScreenLocalSunMoon])       LocalSunMoon();         // local time, sun, moon
  else if (disp == menuOrder[ScreenLocalMoon])          LocalMoon();            // local time, moon size and elevation
  else if (disp == menuOrder[ScreenMoonRiseSet])        MoonRiseSet();          // Moon rises and sets at these times

  // Nice to have
  else if (disp == menuOrder[ScreenTimeZones])          TimeZones();            // Other time zones

  // Fancy, sometimes near unreadable displays, fun to program, and fun to look at:
  else if (disp == menuOrder[ScreenBinary])             Binary(2);              // Binary, horizontal, display of time
  else if (disp == menuOrder[ScreenBinaryHorBCD])       Binary(1);              // BCD, horizontal, display of time
  else if (disp == menuOrder[ScreenBinaryVertBCD])      Binary(0);              // BCD vertical display of time
  else if (disp == menuOrder[ScreenBar])                Bar();                  // horizontal bar
  else if (disp == menuOrder[ScreenMengenLehrUhr])      MengenLehrUhr();        // set theory clock
  else if (disp == menuOrder[ScreenLinearUhr])          LinearUhr();          // Linear clock
  // debugging:
  else if (disp == menuOrder[ScreenInternalTime])       InternalTime();       // Internal time - for debugging
  else if (disp == menuOrder[ScreenCodeStatus])         CodeStatus();         //

  // GPS Location
  else if (disp == menuOrder[ScreenUTCPosition])        UTCPosition();        // position

  // WSPR and beacons:
  else if (disp == menuOrder[ScreenNCDXFBeacons2])      NCDXFBeacons(2);      // UTC + NCDXF beacons, 18-28 MHz
  else if (disp == menuOrder[ScreenNCDXFBeacons1])      NCDXFBeacons(1);      // UTC + NCDXF beacons, 14-21 MHz
  else if (disp == menuOrder[ScreenWSPRsequence])       WSPRsequence();  // UTC + Coordinated WSPR band/frequency (20 min cycle)

  else if (disp == menuOrder[ScreenHex])                HexOctalClock(0);     // Hex clock
  else if (disp == menuOrder[ScreenOctal])              HexOctalClock(1);     // Octal clock
  else if (disp == menuOrder[ScreenHexOctalClock])      HexOctalClock(3);     // 3-in-1: Hex-Octal-Binary clock

  else if (disp == menuOrder[ScreenEasterDates])        EasterDates(yearGPS); // Gregorian and Julian Easter Sunday

  else if (disp == menuOrder[ScreenLocalSunSimpler])    LocalSun(2);          // local time, sun x 3 - simpler layout
  else if (disp == menuOrder[ScreenLocalSunAzEl])       LocalSunAzEl();       // local time, sun az, el

  else if (disp == menuOrder[ScreenRoman])              Roman();              // Local time in Roman numerals
  else if (disp == menuOrder[ScreenMathClockAdd])       MathClock(0);         // Math clock: add
  else if (disp == menuOrder[ScreenMathClockSubtract])  MathClock(1);         // Math clock: subtract/add
  else if (disp == menuOrder[ScreenMathClockMultiply])  MathClock(2);         // Math clock: multiply/subtract/add
  else if (disp == menuOrder[ScreenMathClockDivide])    MathClock(3);         // Math clock: divide/multiply/subtract

  else if (disp == menuOrder[ScreenLunarEclipse])       LunarEclipse();       // time for lunar eclipse
  else if (disp == menuOrder[ScreenSidereal])           Sidereal();           // sidereal and solar time
  else if (disp == menuOrder[ScreenMorse])              Morse();              // morse time
  else if (disp == menuOrder[ScreenWordClock])          WordClock();          // time in clear text
  else if (disp == menuOrder[ScreenGPSInfo])            GPSInfo();            // Show technical GPS Info
  else if (disp == menuOrder[ScreenISOHebIslam])        ISOHebIslam();        // ISO, Hebrew, Islamic calendar
  else if (disp == menuOrder[ScreenPlanetsInner])       PlanetVisibility(1);  // Inner planet data
  else if (disp == menuOrder[ScreenPlanetsOuter])       PlanetVisibility(0);  // Inner planet data
  else if (disp == menuOrder[ScreenChemical])           LocalUTC(2);          // local time + chemical element

  else if (disp == menuOrder[ScreenBigNumbers2])        BigNumbers2(0);       // local time with big numbers
  else if (disp == menuOrder[ScreenBigNumbers2UTC])     BigNumbers2(1);       // UTC with big numbers
  else if (disp == menuOrder[ScreenBigNumbers3])        BigNumbers3(0);       // local time with big numbers
  else if (disp == menuOrder[ScreenBigNumbers3UTC])     BigNumbers3(1);       // UTC with big numbers
  else if (disp == menuOrder[ScreenReminder])           Reminder();           // Show dates to remember and elapsed time (from EEPROM) 
  else if (disp == menuOrder[ScreenEquinoxes])          Equinoxes();          // Show equinoxes, solstices
  else if (disp == menuOrder[ScreenSolarEclipse])       SolarEclipse();       // time for solar eclipses
  else if (disp == menuOrder[ScreenNextEvents])         NextEvents();         // Show next Easter, eclipse(s), equinox/solstice in sorted order
 
  else if (disp == menuOrder[ScreenDemoClock])  // last menu item
  {
    if (!DemoMode)                                      DemoClock(0);         // Start demo of all clock functions if not already in DemoMode
    else                                                DemoClock(1);         // continue with demo mode
  } else                          // Error handling, added 12.7.2023
  {
    lcd.setCursor(0, 0);
    lcd.print(F("Warning:"));
    lcd.setCursor(0, 1);
    lcd.print(F("  Invalid screen #"));  // due to repeated entry in menuStruct[]
  }
}

////////////////////////////////////////////////////////////////////////////////
void readEEPROM() {
  // *** EEPROM read/default setup. If values are outside of range (as in very first startup), set them to reasonable values

#ifdef FEATURE_SERIAL_EEPROM
  Serial.println("readEEPROM");
#endif

  backlightVal = max(10, EEPROM.read(EEPROM_OFFSET1));  // minimum 10 to ensure that display always is readable on very first startup
  analogWrite(LCD_PWM, backlightVal);

#ifdef FEATURE_SERIAL_EEPROM
  Serial.print("backlightVal ");
  Serial.println(backlightVal);
#endif

  int noOfMenuIn = sizeof(menuStruct) / sizeof(menuStruct[0]);
  subsetMenu = EEPROM.read(EEPROM_OFFSET1 + 1);
  if ((subsetMenu < 0) || (subsetMenu >= noOfMenuIn))  // if EEPROM stores invalid value
  {
    subsetMenu = 0;                                 //  set to default value on very first startup
    EEPROMMyupdate(EEPROM_OFFSET1 + 1, subsetMenu, 1);  // make sure EEPROM has a valid value
  }

  noOfMenuIn = sizeof(dateTimeFormat) / sizeof(dateTimeFormat[0]);
  dateFormat = EEPROM.read(EEPROM_OFFSET1 + 2);        //
  if ((dateFormat < 0) || (dateFormat >= noOfMenuIn))  // if EEPROM stores invalid value
  {
    dateFormat = 0;                                 //  set to default value on very first startup
    EEPROMMyupdate(EEPROM_OFFSET1 + 2, dateFormat, 1);  // make sure EEPROM has a valid value
  }

  uint8_t numLanguages = sizeof(languages) / sizeof(languages[0]);
  languageNumber = EEPROM.read(EEPROM_OFFSET1 + 3);               //
  if ((languageNumber < 0) || (languageNumber >= numLanguages))  // if EEPROM stores invalid value
  {
    languageNumber = 0;                                // set to default value (0 = English) on very first startup
    EEPROMMyupdate(EEPROM_OFFSET1 + 3, languageNumber, 1);  // make sure EEPROM has a valid value
  }

  timeZoneNumber = EEPROM.read(EEPROM_OFFSET1 + 4);
  if ((timeZoneNumber < 0) || (timeZoneNumber > NUMBER_OF_TIME_ZONES - 1)) {
    timeZoneNumber = 0;
    EEPROMMyupdate(EEPROM_OFFSET1 + 4, timeZoneNumber, 1);
  }

  baudRateNumber = EEPROM.read(EEPROM_OFFSET1 + 5);
  if ((baudRateNumber < 0) || (baudRateNumber > sizeof(gpsBaud1) / sizeof(gpsBaud1[0]))) {
    baudRateNumber = 1;
    EEPROMMyupdate(EEPROM_OFFSET1 + 5, baudRateNumber, 1);
  } 

  secondsClockHelp = EEPROM.read(EEPROM_OFFSET1 + 6);
  if (secondsClockHelp < 0 || secondsClockHelp > 60) {
    secondsClockHelp = 12;
    EEPROMMyupdate(EEPROM_OFFSET1 + 6, secondsClockHelp, 1);
  }

  dwellTimeDemo = EEPROM.read(EEPROM_OFFSET1 + 7);
  if (dwellTimeDemo < 4 || dwellTimeDemo > 60) {
    dwellTimeDemo = 8;
    EEPROMMyupdate(EEPROM_OFFSET1 + 7, dwellTimeDemo, 1);
  }
  
  mathSecondPeriod = EEPROM.read(EEPROM_OFFSET1 + 8);
  if (mathSecondPeriod < 4 || mathSecondPeriod > 60) {
    mathSecondPeriod = 10;
    EEPROMMyupdate(EEPROM_OFFSET1 + 8, mathSecondPeriod, 1);
  }

  using_PPS = EEPROM.read(EEPROM_OFFSET1 + 9);
  if (using_PPS !=false & using_PPS != true) {
    using_PPS = false;
    EEPROMMyupdate(EEPROM_OFFSET1 + 9, using_PPS, 1);
  }

  demoStepType = EEPROM.read(EEPROM_OFFSET1 + 10);
  if (demoStepType < 0 || demoStepType > 2) {
    demoStepType = 0;
    EEPROMMyupdate(EEPROM_OFFSET1 + 10, demoStepType, 1);
  }

#ifdef FEATURE_SERIAL_EEPROM
  Serial.print("secondsClockHelp, dwellTimeDemo, mathSecondPeriod ");
  Serial.print(secondsClockHelp),
    Serial.print(" ");
  Serial.print(dwellTimeDemo);
  Serial.print(" ");
  Serial.println(mathSecondPeriod);
#endif

  // *** end EEPROM read/default setup
}

////////////////////////////////////////////////////////////////////////////////

void setup() {

  lcd.begin(20, 4);
  digitalWrite(PIN_A, HIGH);  // enable pull-ups for rotary encoder and button
  digitalWrite(PIN_B, HIGH);
  digitalWrite(PUSHB, HIGH);

  pinMode(LCD_PWM, OUTPUT);  // for backlight control

#ifdef FEATURE_SERIAL_EEPROM
  Serial.begin(115200);
  Serial.println(F("EEPROM debug"));
#endif

#ifdef FEATURE_SERIAL_NEXTEVENTS
  Serial.begin(115200);
  Serial.println(F("NextEvents debug"));
#endif
 
  
  readEEPROM();        // read stored parameter values (settable in menu system)
  #ifndef ARDUINO_SAMD_VARIANT_COMPLIANCE
    readPersonEEPROM();  // read data into variables lengthPersonData, person for Reminder()
   #else
  // //   readDefaults();      // read reasonable startup values (settable in menu system)
   // using_PPS = false;
  #endif 
 
  #ifdef FEATURE_SERIAL_LOAD_CHARACTERS  // check loading of new custom characters to LCD
    Serial.begin(115200);
    Serial.println(F("Character set debug"));
  #endif
  
////////// test code ///
#ifdef FEATURE_INTERRUPTTEST
  int checkPin = digitalPinToInterrupt(GPS_PPS);  
  lcd.setCursor(0,0);
  if (checkPin == -1) {
    lcd.print("Invalid interrupt "); lcd.print(GPS_PPS);
  }
  else {
    lcd.print("Valid interrupt "); lcd.print(GPS_PPS);  // even for pin D4 for Metro??
  }
  delay(1000);
#endif

////
  randomSeed(analogRead(1));  // initalize random number with random noise (for demo order). Pin A1 unconnected
  InitScreenSelect();  // Initalize screen selection order

  gpsBaud = gpsBaud1[baudRateNumber];
    #ifndef FEATURE_FAKE_SERIAL_GPS_IN  // the usual way of reading GPS
      Serial1.begin(gpsBaud);
    #else
      Serial.begin(gpsBaud);            // for faking GPS data from software simulator
  #endif

  attachInterrupt(digitalPinToInterrupt(GPS_PPS), ppsHandler, RISING);  // enable 1pps GPS time sync
 // works here for METRO: https://forum.arduino.cc/t/interrupt-not-being-called-in-arduino-m0-pro/485356 

  CodeStatus();  // show start screen
  lcd.setCursor(0, 3);
  lcd.print(F("..........    "));  // timezone info of start screen not yet set, so blank it out
  delay(1000);

  dispState = 0;     // always start with screen # 0
  demoDuration = 0;  // reset counter for time between demo screens

  // Initialize all the uninitialized TinyGPSCustom objects
  for (int i = 0; i < 4; ++i)  // from SatelliteTracker (TinyGPS++ example)
  {
    satNumber[i].begin(gps, "GPGSV", 4 + 4 * i);  // offsets 4, 8, 12, 16
    elevation[i].begin(gps, "GPGSV", 5 + 4 * i);  // offsets 5, 9, 13, 17
    azimuth[i].begin(gps, "GPGSV", 6 + 4 * i);    // offsets 6, 10, 14, 18
    snr[i].begin(gps, "GPGSV", 7 + 4 * i);        // offsets 7, 11, 15, 19
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
  Serial.print(F("Testing TinyGPS++ library v. "));
  Serial.println(TinyGPSPlus::libraryVersion());
  Serial.println(F("by Mikal Hart"));
  Serial.println();
#endif

#ifdef FEATURE_SERIAL_MOON
  Serial.begin(115200);
  Serial.println(F("Moon debug"));
#endif

#ifdef FEATURE_SERIAL_MENU
  Serial.begin(115200);
  Serial.println(F("Menu debug"));
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
  //   for(uint8_t i=0; i < numLanguages ; i++)   Serial.println(languages[i]);
#endif

#ifdef FEATURE_SERIAL_CHAR_SETS
  Serial.begin(115200);
  Serial.println(F("Character set debug"));
#endif

#ifdef FEATURE_DATE_PER_SECOND  // for stepping date quickly and check calender function
  dateIteration = 0;
#endif
}

////////////////////////////////////// L O O P //////////////////////////////////////////////////////////////////

void loop() {
  readGPS();        // decode incoming GPS
  GPSParse();       // GPS statuscode snippet from TinyGPSParse.ino
  syncCheck();      // set time with interrupt (or without interrupt)
  updateDisplay();  // select function for selected screen
  checkEncoder();   // check and read rotary encoder + its button

  
  #ifdef FEATURE_INTERRUPTTEST
    digitalWrite(LED_BUILTIN, state);
  #endif

  // In support of old user interface with buttons:
#ifdef FEATURE_BUTTONS  // separate buttons which may be in addition to rotary encoder
  readButtons();
#endif  // FEATURE_BUTTONS
}

////////////////////////////////////// END LOOP //////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// The remaining functions in this file correspond to the different clock faces or screens of themenu
//
// Menu item ///////////////////////////////////////////////////////////////////////////////////////////

void LocalUTC(  // local time, UTC, locator, option: ISO week #
  byte mode     // 0 for original version
                // 1 for added week # (new 3.9.2022)
                // 2 for Chemical element on last two lines (27.3.2023)
) {             //

loadNativeCharacters(languageNumber);
#ifndef FEATURE_DATE_PER_SECOND
  localTime = now() + utcOffset * 60;

#else  // for stepping date quickly and check calender function
  localTime = now() + utcOffset * 60 + dateIteration * 3600; //86400;  //int(86400.0/5.0); // fake local time by stepping per day
  dateIteration = dateIteration + 1;
//  Serial.print(dateIteration); Serial.print(": ");
//  Serial.println(local);
#endif

  Hour = hour(localTime);
  Minute = minute(localTime);
  Seconds = second(localTime);

  lcd.setCursor(0, 0);  // top line *********
  sprintf(textBuffer, "%02d%c%02d%c%02d", Hour, dateTimeFormat[dateFormat].hourSep, Minute, dateTimeFormat[dateFormat].minSep, Seconds);
  lcd.print(textBuffer);
  lcd.print(F("      "));
  // local date
  Day = day(localTime);
  Month = month(localTime);
  Year = year(localTime);
  //if (dayGPS != 0)        // was this (26.05.2023)
  //if (gps.date.isValid()) // same slow response as old test (26.05.2023)
  {

    // right-adjusted long day name:
    nativeDayLong(localTime);  // output in "today"
    sprintf(todayFormatted, "%12s", today);
    lcd.setCursor(8, 0);
    lcd.print(todayFormatted);

    lcd.setCursor(0, 1);  //////// line 2
    if (mode == 1) {
      // option added 3.9.2022 - ISO week # on second line
      GregorianDate a(month(localTime), day(localTime), year(localTime));
      IsoDate ISO(a);
      if      (strcmp(languages[languageNumber], "no")==0) lcd.print(F("Uke "));
      else if (strcmp(languages[languageNumber], "dk")==0) lcd.print(F("Uge "));
      else if (strcmp(languages[languageNumber], "ny")==0) lcd.print(F("Veke "));
      else if (strcmp(languages[languageNumber], "se")==0) lcd.print(F("Vecka "));
      else if (strcmp(languages[languageNumber], "is")==0) lcd.print(F("Vika "));
      else if (strcmp(languages[languageNumber], "de")==0) lcd.print(F("Woche "));
      else if (strcmp(languages[languageNumber], "fr")==0) lcd.print(F("Sem. ")); // "Semaine" is too long
      else if (strcmp(languages[languageNumber], "es")==0) lcd.print(F("Semana "));
      else lcd.print(F("Week "));  // also Dutch

      lcd.print(ISO.GetWeek());
      lcd.print(" ");  // added space 15.01.2023 - needed for 1-digit week numbers
    }

    lcd.setCursor(9, 1);
    lcd.print(" ");
    LcdDate(Day, Month, Year);

    if (mode == 2)  // Chemical elements
    {
      // Inspired by https://www.instructables.com/Periodic-Table-Clock/
      // added 4.4.2023

      int Hr, Min, Sec;
      Hr = (int)Hour;
      Min = (int)Minute;
      Sec = (int)Seconds;

      lcd.setCursor(0, 2);               //////// line 3
      LCDChemicalElement(Hr, Min, Sec);  // Display chemical 1- or 2-letter element abbreviation
      LCDChemicalGroupPeriod(Sec);       // Display chemical group (column) and period (row)

      lcd.setCursor(0, 3);
      lcd.print(F("      "));       // line 4
      LCDChemicalElementName(Sec);  // Display full name of chemical element

      lcd.setCursor(18, 3);
      lcd.print(F("  "));
    }

    if (mode != 2)  // 30.05.2023 from LCDUTCTimeLocator -> UTC time + No sats
    {
      lcd.setCursor(0, 2);
      lcd.print(F("                    "));

      lcd.setCursor(0, 3);
      sprintf(textBuffer, "%02d%c%02d%c%02d UTC ", hour(now()), dateTimeFormat[dateFormat].hourSep, minute(now()), dateTimeFormat[dateFormat].minSep, second(now()));
      lcd.print(textBuffer);

#ifdef FEATURE_SERIAL_GPS
//    Serial.println(textBuffer);
#endif

      if (gps.satellites.isUpdated()) {
        noSats = gps.satellites.value();
        lcd.setCursor(13, 3);
      } else noSats = 0;
      PrintFixedWidth(lcd, noSats, 2);
      lcd.print(F(" Sats"));
    }
  }
}

// Menu item //////////////////////////////////////////////////////////////////////////////////////////

void UTCLocator(  // UTC, locator, # satellites
     byte mode    // 0 for original version
                  // 1 for same layout as LocalUTC(1) - new 28.02.2024
){
  // #ifdef FEATURE_FAKE_SERIAL_GPS_IN
  //       hourGPS = hour(now());
  //       minuteGPS = minute(now());
  //       secondGPS = second(now());
  // #endif

  loadNativeCharacters(languageNumber);

  lcd.setCursor(0, 0);  // top line *********
  //  if (gps.time.isValid())
  if (mode==0)
  { 
    sprintf(textBuffer, "%02d%c%02d%c%02d         UTC", hour(now()), dateTimeFormat[dateFormat].hourSep, minute(now()), dateTimeFormat[dateFormat].minSep, second(now()));
    lcd.print(textBuffer);
  }
  else  // mode == 1
  {
    sprintf(textBuffer, "%02d%c%02d%c%02d", hour(now()), dateTimeFormat[dateFormat].hourSep, minute(now()), dateTimeFormat[dateFormat].minSep, second(now()));
    lcd.print(textBuffer);
    #ifndef UTCLOCALLANGUAGE        // New 27.2.2024
      lcd.print(dayStr(weekdayGPS));
    #else
       nativeDayLong(now());   // output in "today"
      sprintf(todayFormatted, "%12s", today);   // print right-justified
      lcd.print(todayFormatted);
    #endif
  }
  // UTC date
  //if (dayGPS != 0)
  lcd.setCursor(0, 1);  // line 1
  if (mode==0)
  {
    #ifndef UTCLOCALLANGUAGE        // New 27.2.2024
      lcd.print(dayStr(weekdayGPS));
    #else
      nativeDayLong(now());   // output in "today"
      sprintf(todayFormatted, "%-12s", today);   // print right-justified
      lcd.print(todayFormatted);
    #endif
    
    lcd.print(F("   "));  // two more spaces 14.04.2018
  }
  else  // mode == 1
    lcd.print(F("UTC"));
  
  lcd.setCursor(10, 1);
  LcdDate(dayGPS, monthGPS, yearGPS);

  if (gps.location.isValid()) {

  #ifndef DEBUG_MANUAL_POSITION
      latitude = gps.location.lat();
      lon = gps.location.lng();
  #else
      latitude = latitude_manual;
      lon = longitude_manual;
  #endif

  Maidenhead(lon, latitude, textBuffer);
  lcd.setCursor(0, 3);  // last line *********
  lcd.print(textBuffer);
  lcd.print(F("       "));
  }
  //  if (gps.satellites.()) { // 16.11.2022
  if (gps.satellites.isUpdated()) {
    noSats = gps.satellites.value();
    lcd.setCursor(13, 3);
  } else noSats = 0;

  PrintFixedWidth(lcd, noSats, 2);
  lcd.print(F(" Sats"));
}

// Menu item //////////////////////////////////////////////////////////////////////////////////////////
void LocalSun(  // local time, sun x 3
  byte mode     // 0 for ScreenLocal
                // 1 for ScreenLocalSunSimpler
                // 2 toggle between Civil+Nautical (= 1) and Noon and now display
) 
{
  loadNativeCharacters(languageNumber);
  loadArrowCharacters();

  //
  // shows Actual (0 deg), Civil (-6 deg), and Nautical (-12 deg) sun rise/set
  //
  if (mode == 0) LcdShortDayDateTimeLocal(0, 2);  // line 0
  else LcdShortDayDateTimeLocal(0, 0);


//  if (gps.location.isValid()) {
//    if (minuteGPS != oldMinute) {
#ifndef DEBUG_MANUAL_POSITION
      latitude = gps.location.lat();
      lon = gps.location.lng();
#else
      latitude = latitude_manual;
      lon = longitude_manual;
#endif

  if (mode == 0) {
    LcdSolarRiseSet(1, ' ', ScreenLocalSun);
    LcdSolarRiseSet(2, 'C', ScreenLocalSun);
    LcdSolarRiseSet(3, 'N', ScreenLocalSun);
  } 
  else if (mode==1) {
    LcdSolarRiseSet(1, ' ', ScreenLocalSunSimpler);
    LcdSolarRiseSet(2, 'C', ScreenLocalSunSimpler);
    LcdSolarRiseSet(3, 'N', ScreenLocalSunSimpler);
  }
  else if (mode==2) {    // toggle 
    LcdSolarRiseSet(1, ' ', ScreenLocalSunSimpler);

    if (now() % 20 < 10) {
      LcdSolarRiseSet(2, 'C', ScreenLocalSunSimpler);

      if (now()%20 == 0)                                       // blank out line first, in case not written during midsummer
      {
        lcd.setCursor(0,3); lcd.print(F("                  "));  
      }
      LcdSolarRiseSet(3, 'N', ScreenLocalSunSimpler);
      //lcd.setCursor(17, 3); lcd.print(" "); // remove deg symbol
    }
    else {
      LcdSolarRiseSet(2, 'O', ScreenLocalSunAzEl);  // noon info
        lcd.setCursor(19, 2); lcd.print(" ");   // remove 'C'
      LcdSolarRiseSet(3, 'Z', ScreenLocalSunAzEl);  // az, el now
    }
//   }
// }
  }
  oldMinute = minuteGPS;
}


/*****
Purpose: Menu item
Finds local time, and three different sun rise/set times

Argument List: Global variables 

Return value: Displays on LCD
*****/

void LocalSunAzEl() {  // local time, sun x 3
  //
  // shows Actual (0 deg), Civil (-6 deg), and Nautical (-12 deg) sun rise/set
  //
  loadNativeCharacters(languageNumber);
  loadArrowCharacters();

  LcdShortDayDateTimeLocal(0, 0);  // line 0
  if (gps.location.isValid()) {
    if (minuteGPS != oldMinute) {
#ifndef DEBUG_MANUAL_POSITION
      latitude = gps.location.lat();
      lon = gps.location.lng();
#else
      latitude = latitude_manual;
      lon = longitude_manual;
#endif

      LcdSolarRiseSet(1, ' ', ScreenLocalSunAzEl);  // Actual Rise, Set times<
      LcdSolarRiseSet(2, 'O', ScreenLocalSunAzEl);  //Noon info
      LcdSolarRiseSet(3, 'Z', ScreenLocalSunAzEl);  //Current Az El info
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

void LocalSunMoon() {  // local time, sun, moon
  //
  // shows solar rise/set in a chosen definition (Actual, Civil, ...)
  //
  loadNativeCharacters(languageNumber);
  loadArrowCharacters();

  LcdShortDayDateTimeLocal(0, 0);  // line 0, (was time offset 2) to the left

  if (gps.location.isValid()) {
    if (minuteGPS != oldMinute) {

#ifndef DEBUG_MANUAL_POSITION
      latitude = gps.location.lat();
      lon = gps.location.lng();
#else
      latitude = latitude_manual;
      lon = longitude_manual;
#endif

      LcdSolarRiseSet(1, ' ', ScreenLocalSunMoon);  // line 1, Actual rise time
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
      Serial.print(F("LocalSunMoon: order: "));
      Serial.println(order);
#endif

      localTime = now() + utcOffset * 60;
      Hour = hour(localTime);
      Minute = minute(localTime);

      int packedTime = Hour * 100 + Minute;

      lcd.setCursor(2, 3);  // last line

      // find next event
      if (order == 1)  // Rise
      {
        pTime = pRise;
        lcd.write(UP_ARROW);
      } else  // Set (or order not initialized correctly)
      {
        pTime = pSet;
        lcd.write((byte)DOWN_ARROW);
      }

      if (pTime > -1) {
        int pHr = pTime / 100;
        int pMin = pTime - 100 * pHr;
        PrintFixedWidth(lcd, pHr, 2);
        lcd.print(dateTimeFormat[dateFormat].hourSep);
        PrintFixedWidth(lcd, pMin, 2, '0');
        lcd.print(" ");
      } else lcd.print(F(" - "));

      float PhaseM, PercentPhaseM;
      MoonPhaseAccurate(PhaseM, PercentPhaseM);
      //MoonPhase(PhaseM, PercentPhaseM);

#ifdef FEATURE_SERIAL_MOON
      Serial.println(F("LocalSunMoon: "));
      Serial.print(F(" PhaseM, PercentPhaseM "));
      Serial.print(PhaseM);
      Serial.print(F(", "));
      Serial.println(PercentPhaseM);
#endif

      lcd.setCursor(9, 3);

      MoonWaxWane(PhaseM);  //arrow up/down or ' ' (space)
      MoonSymbol(PhaseM);   // (, O, ), symbol

      PrintFixedWidth(lcd, (int)round(PercentPhaseM), 3);
      lcd.print("%");

      UpdateMoonPosition();  // calls K3NG moon2()
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

void LocalMoon() {  // local time, moon phase, elevation, next rise/set

  //  String textbuf;
  float percentage;
  loadNativeCharacters(languageNumber);
  loadArrowCharacters();

  LcdShortDayDateTimeLocal(0, 0);  // line 0, (was 1 position left) to line up with next lines

  if (gps.location.isValid()) {
    if (minuteGPS != oldMinute) {  // update display every minute

      // days since last new moon
      float Phase, PercentPhase;

      UpdateMoonPosition(); // calls K3NG moon2()

      lcd.setCursor(0, 3);  // line 3
      //
      //        lcd.print(F("  "));
      //        textbuf = String(moon_dist, 0);
      //        lcd.print(textbuf); lcd.print(" km");

      lcd.print(" ");
      PrintFixedWidth(lcd, int(round(moon_dist / 4067.0)), 3);
      lcd.print(F("% "));

      PrintFixedWidth(lcd, int(round(moon_dist / 1000.0)), 3);
      lcd.print(F("'km "));


      MoonPhase(Phase, PercentPhase);

      lcd.setCursor(14, 3);
      MoonWaxWane(Phase);  // arrow
      MoonSymbol(Phase);   // (,0,)

      lcd.setCursor(16, 3);
      PrintFixedWidth(lcd, (int)(abs(round(PercentPhase))), 3);
      lcd.print("%");

#ifndef DEBUG_MANUAL_POSITION
      latitude = gps.location.lat();
      lon = gps.location.lng();
#else
      latitude = latitude_manual;
      lon = longitude_manual;
#endif

      lcd.setCursor(0, 1);  // line 1

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
      localTime = now() + utcOffset * 60;
      Hour = hour(localTime);
      Minute = minute(localTime);

      int packedTime = Hour * 100 + Minute;

      // find next event
      if (order == 1)
      // Rise
      {
        pTime = pRise;
        Symb = UP_ARROW;
        Az = rAz;
      } else {
        pTime = pSet;
        Symb = DOWN_ARROW;
        Az = sAz;
      }

      lcd.setCursor(2, 2);  // line 2

      if (pTime > -1) {
        int pHr = pTime / 100;
        int pMin = pTime - 100 * pHr;

        lcd.write(Symb);
        lcd.print(F("   "));
        PrintFixedWidth(lcd, pHr, 2);
        lcd.print(dateTimeFormat[dateFormat].hourSep);
        PrintFixedWidth(lcd, pMin, 2, '0');
        lcd.print(" ");
        lcd.setCursor(13, 2);
        lcd.print(F("Az "));
        PrintFixedWidth(lcd, (int)round(Az), 3);
        lcd.write(DEGREE);
      } else lcd.print(F("  No Rise/Set       "));

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
    lon = longitude_manual;
#endif

    if (minuteGPS != oldMinute) {

      short pRise, pSet, pRise2, pSet2, packedTime;  // time in compact format '100*hr + min'
      double rAz, sAz, rAz2, sAz2;

      localTime = now() + utcOffset * 60;
      Hour = hour(localTime);
      Minute = minute(localTime);

      packedTime = Hour * 100 + Minute;  // local time 19.11.2021

      // ***** rise/set for this UTC day:

      GetMoonRiseSetTimes(float(utcOffset) / 60.0, latitude, lon, &pRise, &rAz, &pSet, &sAz);

      lcd.setCursor(0, 0);  // top line
      lcd.print(F("M "));

      int lineNo = 0;
      int lineUsed = 0;

      int MoonRiseHr = pRise / 100;
      int MoonRiseMin = pRise - 100 * MoonRiseHr;
      int MoonSetHr = pSet / 100;
      int MoonSetMin = pSet - 100 * MoonSetHr;

      lcd.setCursor(2, lineNo);  // row no 0

      // Determine if there is two, one or no rise/set events on the present date and which are in the future

      int NoOfEvents = 0;
      if (packedTime < pRise | packedTime < pSet) NoOfEvents = 1;
      if (packedTime < pRise && packedTime < pSet) NoOfEvents = 2;

#ifdef FEATURE_SERIAL_MOON
      Serial.println(NoOfEvents);
#endif

      if (NoOfEvents == 2) {
        if (pRise < pSet) lcd.setCursor(2, lineNo);  // row no 0
        else lcd.setCursor(2, lineNo + 1);           // row no 1
      }

      if (pRise > -1 && pRise > packedTime)  // only show a future event
      {
        lcd.write(UP_ARROW);
        lcd.print(" ");
        PrintFixedWidth(lcd, MoonRiseHr, 2, '0');
        lcd.print(dateTimeFormat[dateFormat].hourSep);
        PrintFixedWidth(lcd, MoonRiseMin, 2, '0');
        lcd.print(F("  "));
        PrintFixedWidth(lcd, (int)round(rAz), 4);
        lcd.write(DEGREE);
        lcd.print(F("  "));
        lineUsed = lineUsed + 1;
      }
//      else if (pRise < 0)
//      {
//        lcd.print(pRise); lcd.print(F("              "));
//      }
#ifdef FEATURE_SERIAL_MOON
      Serial.println(lineUsed);
#endif

      if (NoOfEvents == 2) {
        if (pRise < pSet) lcd.setCursor(2, lineNo + 1);  // row no 1
        else lcd.setCursor(2, lineNo);                   // row no 0
      }

      if (pSet > -1 && pSet > packedTime)  // only show a future event
      {
        lcd.write((byte)DOWN_ARROW);
        lcd.print(" ");
        PrintFixedWidth(lcd, MoonSetHr, 2, '0');
        lcd.print(dateTimeFormat[dateFormat].hourSep);  // doesn't handle 00:48 well with ' ' as separator
        PrintFixedWidth(lcd, MoonSetMin, 2, '0');
        lcd.print(F("  "));
        PrintFixedWidth(lcd, (int)round(sAz), 4);
        lcd.write(DEGREE);
        lcd.print(F("  "));
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

      MoonRiseHr = pRise2 / 100;
      MoonRiseMin = pRise2 - 100 * MoonRiseHr;
      MoonSetHr = pSet2 / 100;
      MoonSetMin = pSet2 - 100 * MoonSetHr;

      if (pRise2 < pSet2) lcd.setCursor(2, lineNo);
      else lcd.setCursor(2, lineNo + 1);

      lcd.write(UP_ARROW);
      lcd.print(" ");

      if (pRise2 > -1) {
        PrintFixedWidth(lcd, MoonRiseHr, 2, '0');
        lcd.print(dateTimeFormat[dateFormat].hourSep);
        PrintFixedWidth(lcd, MoonRiseMin, 2, '0');
        lcd.print(F("  "));
        PrintFixedWidth(lcd, (int)round(rAz2), 4);
        lcd.write(DEGREE);
        lcd.print(F("  "));
        lineUsed = lineUsed + 1;
      }
      //      else
      //      {
      //        lcd.print(pRise2); lcd.print("              ");
      //      }

      if (pRise2 < pSet2) lcd.setCursor(2, lineNo + 1);
      else lcd.setCursor(2, lineNo);

      lcd.write((byte)DOWN_ARROW);
      lcd.print(" ");
      if (pSet2 > -1) {
        PrintFixedWidth(lcd, MoonSetHr, 2, '0');
        lcd.print(dateTimeFormat[dateFormat].hourSep);  // doesn't handle 00:48 well with ' ' as separator
        PrintFixedWidth(lcd, MoonSetMin, 2, '0');
        lcd.print(F("  "));
        PrintFixedWidth(lcd, (int)round(sAz2), 4);
        lcd.write(DEGREE);
        lcd.print(F("  "));
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

        MoonRiseHr = pRise2 / 100;
        MoonRiseMin = pRise2 - 100 * MoonRiseHr;
        MoonSetHr = pSet2 / 100;
        MoonSetMin = pSet2 - 100 * MoonSetHr;

        if (pRise2 < pSet2) {
          lcd.setCursor(2, lineNo);
          lineUsed = lineNo;
        } else {
          lcd.setCursor(2, lineNo + 1);
          lineUsed = lineNo + 1;
        }

        if (lineUsed <= 3) {
          lcd.write(UP_ARROW);
          lcd.print(" ");

          if (pRise2 > -1) {
            PrintFixedWidth(lcd, MoonRiseHr, 2, '0');
            lcd.print(dateTimeFormat[dateFormat].hourSep);
            PrintFixedWidth(lcd, MoonRiseMin, 2, '0');
            lcd.print(F("  "));
            PrintFixedWidth(lcd, (int)round(rAz2), 4);
            lcd.write(DEGREE);
            lcd.print(F("  "));
          } else {
            lcd.print(pRise2);
            lcd.print(F("              "));
          }
        }

        if (pRise2 < pSet2) {
          lcd.setCursor(2, lineNo + 1);
          lineUsed = lineNo + 1;
        } else {
          lcd.setCursor(2, lineNo);
          lineUsed = lineNo;
        }

        if (lineUsed <= 3) {
          lcd.write((byte)DOWN_ARROW);
          lcd.print(" ");
          if (pSet2 > -1) {
            PrintFixedWidth(lcd, MoonSetHr, 2, '0');
            lcd.print(dateTimeFormat[dateFormat].hourSep);  // doesn't handle 00:48 well with ' ' as separator
            PrintFixedWidth(lcd, MoonSetMin, 2, '0');
            lcd.print(F("  "));
            PrintFixedWidth(lcd, (int)round(sAz2), 4);
            lcd.write(DEGREE);
            lcd.print(F("  "));
          } else {
            lcd.print(pSet2);
            lcd.print(F("              "));
          }
        }
      }

      lcd.setCursor(18, 3);
      lcd.print(F("  "));
    }
  }
  oldMinute = minuteGPS;
}

// Menu item ///////////////////////////////////////////////////////////////////////////////////////////

void lcdTimeZone(byte zoneNumber)  // display time on LCD in desired time zone. Called by TimeZones()
{
  // this function writes to Serial: "[TZ] Read from EEPROM, size =  4096 , offset =  0" - from inside library???

  // https://github.com/JChristensen/Timezone/blob/master/README.md:
  // This reads both the daylight and standard time rules previously stored at EEPROM address 100:
  // Timezone usPacific(100);

  Timezone tzLocal;
  time_t local;
  TimeChangeRule *tcrLocal;

  tzLocal = *timeZones_arr[zoneNumber];
  local = tzLocal.toLocal(utc, &tcrLocal);
  Hour = hour(local);
  Minute = minute(local);
  sprintf(textBuffer, "%02d%c%02d ", Hour, dateTimeFormat[dateFormat].hourSep, Minute);
  lcd.print(textBuffer);
  lcd.print(tcrLocal->abbrev);
}

/*****
Purpose: Display time in 6 time zones on LCD, 4 user selectable

Argument List: none

Return value: Display on LCD
*****/

void TimeZones() {  // local time, UTC, + 4 more time zones (user selectable)
                    //  https://github.com/khoih-prog/Timezone_Generic

  // show local time in many locations

  lcd.setCursor(0, 0);  // 1. line ********* always time zone set for clock
  lcdTimeZone(timeZoneNumber);

  lcd.setCursor(17, 0);  // end of line 1 shows seconds
  Seconds = second(localTime);
  sprintf(textBuffer, "%c%02d", dateTimeFormat[dateFormat].minSep, Seconds);
  lcd.print(textBuffer);

  lcd.setCursor(0, 1);  // 2. line  always UTC *********
  sprintf(textBuffer, "%02d%c%02d UTC  ", hour(now()), dateTimeFormat[dateFormat].hourSep, minute(now()));
  lcd.print(textBuffer);

  // First user selectable (e.g. China Standard Time)
  lcd.setCursor(0, 2);  // ******** 3. line
  lcdTimeZone(16);

  // Second user selectable (e.g. Indian Standard Time)
  lcd.setCursor(10, 2);
  lcdTimeZone(17);

  // Third user selectable (e.g. US Eastern Time Zone)
  lcd.setCursor(0, 3);  //////// line 4
  lcdTimeZone(3);

  // Fourth user selectable (e.g. Pacific US)
  lcd.setCursor(10, 3);
  lcdTimeZone(7);

  lcd.setCursor(19, 3);
  lcd.print(" ");  // blank out rest of menu number in lower right-hand corner
  oldMinute = minuteGPS;
}

// Menu item ////////////////////////////////////////

/*****
Purpose: Menu item
Finds local time in binary formats hour, minute, second

Argument List: byte mode = 0 - vertical BCD
                    mode = 1 - horizontal BCD
                    mode = 2 - horisontal binary

Return value: Displays on LCD
*****/

void Binary(byte mode) {  // binary local time

  int tens, ones;

  int BinaryTensHour[6], BinaryHour[6], BinaryTensMinute[6], BinaryMinute[6], BinaryTensSeconds[6], BinarySeconds[6];

  // get local time
  localTime = now() + utcOffset * 60;
  Hour = hour(localTime);
  Minute = minute(localTime);
  Seconds = second(localTime);

  // convert to BCD

  // must send a variable, not an equation, to DecToBinary as it does in-place arithmetic on input variable
  ones = Hour % 10;
  tens = (Hour - ones) / 10;
  DecToBinary(ones, BinaryHour);
  DecToBinary(tens, BinaryTensHour);

  ones = Minute % 10;
  tens = (Minute - ones) / 10;
  DecToBinary(tens, BinaryTensMinute);
  DecToBinary(ones, BinaryMinute);

  ones = Seconds % 10;
  tens = (Seconds - ones) / 10;
  DecToBinary(tens, BinaryTensSeconds);
  DecToBinary(ones, BinarySeconds);


  if (mode == 0)  // vertical digits:
  {
    lcd.setCursor(0, 0);
    lcd.print(F("BCD"));

    lcd.setCursor(9, 0);
    sprintf(textBuffer, " %1d  %1d  %1d", BinaryHour[2], BinaryMinute[2], BinarySeconds[2]);
    lcd.print(textBuffer);
    lcd.setCursor(19, 0);
    lcd.print("8");

    //   lcd.setCursor(0,1); lcd.print("hh mm ss");
    lcd.setCursor(9, 1);
    sprintf(textBuffer, " %1d %1d%1d %1d%1d", BinaryHour[3], BinaryTensMinute[3], BinaryMinute[3], BinaryTensSeconds[3], BinarySeconds[3]);
    lcd.print(textBuffer);
    lcd.setCursor(19, 1);
    lcd.print("4");

    lcd.setCursor(9, 2);
    sprintf(textBuffer, "%1d%1d %1d%1d %1d%1d", BinaryTensHour[4], BinaryHour[4], BinaryTensMinute[4], BinaryMinute[4], BinaryTensSeconds[4], BinarySeconds[4]);
    lcd.print(textBuffer);
    lcd.setCursor(19, 2);
    lcd.print("2");


    lcd.setCursor(9, 3);  //LSB
    sprintf(textBuffer, "%1d%1d %1d%1d %1d%1d  ", BinaryTensHour[5], BinaryHour[5], BinaryTensMinute[5], BinaryMinute[5], BinaryTensSeconds[5], BinarySeconds[5]);
    lcd.print(textBuffer);
    lcd.setCursor(19, 3);
    lcd.print("1");
  } else if (mode == 1) {
    //// horizontal BCD digits:

    lcd.setCursor(0, 0);
    lcd.print(F("BCD"));

    lcd.setCursor(9, 1);
    sprintf(textBuffer, "  %1d%1d ", BinaryTensHour[4], BinaryTensHour[5]);
    lcd.print(textBuffer);
    sprintf(textBuffer, "%1d%1d%1d%1d H", BinaryHour[2], BinaryHour[3], BinaryHour[4], BinaryHour[5]);
    lcd.print(textBuffer);

    lcd.setCursor(9, 2);
    sprintf(textBuffer, " %1d%1d%1d ", BinaryTensMinute[3], BinaryTensMinute[4], BinaryTensMinute[5]);
    lcd.print(textBuffer);
    sprintf(textBuffer, "%1d%1d%1d%1d M", BinaryMinute[2], BinaryMinute[3], BinaryMinute[4], BinaryMinute[5]);
    lcd.print(textBuffer);

    lcd.setCursor(9, 3);
    sprintf(textBuffer, " %1d%1d%1d ", BinaryTensSeconds[3], BinaryTensSeconds[4], BinaryTensSeconds[5]);
    lcd.print(textBuffer);
    sprintf(textBuffer, "%1d%1d%1d%1d S", BinarySeconds[2], BinarySeconds[3], BinarySeconds[4], BinarySeconds[5]);
    lcd.print(textBuffer);




    if (Seconds < secondsClockHelp)  // show help: weighting
    {
      lcd.setCursor(9, 0);
      lcd.print(F(" 421 8421"));
    } else {
      lcd.setCursor(9, 0);
      lcd.print(F("         "));
    }
  } else
  // horisontal 5/6-bit binary
  {
    // convert to binary:
    DecToBinary(Hour, BinaryHour);
    DecToBinary(Minute, BinaryMinute);
    DecToBinary(Seconds, BinarySeconds);

    lcd.setCursor(13, 1);
    sprintf(textBuffer, "%1d%1d%1d%1d%1d H", BinaryHour[1], BinaryHour[2], BinaryHour[3], BinaryHour[4], BinaryHour[5]);
    lcd.print(textBuffer);

    lcd.setCursor(12, 2);
    sprintf(textBuffer, "%1d%1d%1d%1d%1d%1d M", BinaryMinute[0], BinaryMinute[1], BinaryMinute[2], BinaryMinute[3], BinaryMinute[4], BinaryMinute[5]);
    lcd.print(textBuffer);

    lcd.setCursor(12, 3);
    sprintf(textBuffer, "%1d%1d%1d%1d%1d%1d S", BinarySeconds[0], BinarySeconds[1], BinarySeconds[2], BinarySeconds[3], BinarySeconds[4], BinarySeconds[5]);
    lcd.print(textBuffer);

    lcd.setCursor(0, 0);
    lcd.print(F("Binary"));

    if (Seconds < secondsClockHelp)  // show help: weighting
    {
      lcd.setCursor(13, 0);
      lcd.print(F(" 8421"));
    } else {
      lcd.setCursor(13, 0);
      lcd.print(F("     "));
    }
  }

  // Common for all modes:

  if (Seconds < secondsClockHelp)  // show time in normal numbers
  {
    sprintf(textBuffer, "%02d%c%02d%c%02d", Hour, dateTimeFormat[dateFormat].hourSep, Minute, dateTimeFormat[dateFormat].minSep, Seconds);
  } else {
    sprintf(textBuffer, "        ");
  }
  lcd.setCursor(0, 3);  // last line *********
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
  // updated 12.10.2023 with proper bars rather than approximate symbols

  loadSimpleBarCharacters();  // load user-defined characters for LCD, if not already loaded                        

  localTime = now() + utcOffset * 60;
  Hour = hour(localTime);
  Minute = minute(localTime);
  Seconds = second(localTime);

  // use a 12 character bar  just like a 12 hour clock with ticks every hour
  // for second ticks use ' " % #
  lcd.setCursor(0, 0);
  int imax = Hour;

  if (Hour == 13 && Minute == 0 && Seconds == 0) {
    lcd.print(F("                   "));
    lcd.setCursor(0, 0);
  }

  if (Hour > 12) imax = Hour - 12;

  if (Hour == 0) lcd.print(F("                "));
  for (int i = 0; i < imax; i++) {
    lcd.write(ALL_ON);                                 // fills square
    if (i == 2 | i == 5 | i == 8) lcd.write(ALL_OFF);  // empty
  }

  lcd.setCursor(18, 0);
  lcd.print(F("1h"));

  lcd.setCursor(0, 1);
  imax = Minute / 5;
  if (Minute == 0) lcd.print(F("                "));
  for (int i = 0; i < imax; i++) {
    lcd.write(ALL_ON);                                 // fills square
    if (i == 2 | i == 5 | i == 8) lcd.write(ALL_OFF);  // empty
  }
  if      (Minute % 5 == 1) lcd.write(ONE_BAR);
  else if (Minute % 5 == 2) lcd.write(TWO_BARS);
  else if (Minute % 5 == 3) lcd.write(THREE_BARS);
  else if (Minute % 5 == 4) lcd.write((byte)FOUR_BARS);
  lcd.setCursor(18, 1);
  lcd.print(F("5m"));

  // seconds in 12 characters, with a break every 3 characters
  lcd.setCursor(0, 2);
  imax = Seconds / 5;
  if (Seconds == 0) lcd.print(F("                "));
  for (int i = 0; i < imax; i++) {
    lcd.write(ALL_ON);                                 // fills square
    if (i == 2 | i == 5 | i == 8) lcd.write(ALL_OFF);  // empty
  }
  if (Seconds % 5 == 1) lcd.write(ONE_BAR);
  else if (Seconds % 5 == 2) lcd.write(TWO_BARS);
  else if (Seconds % 5 == 3) lcd.write(THREE_BARS);
  else if (Seconds % 5 == 4) lcd.write((byte)FOUR_BARS);

  lcd.setCursor(18, 2);
  lcd.print(F("5s"));

  lcd.setCursor(18, 3);
  if (Hour > 12) lcd.print(F("PM"));
  else lcd.print(F("AM"));

  lcd.setCursor(9, 3);
  if (Seconds < secondsClockHelp)  // show time in normal numbers
  {
    sprintf(textBuffer, "%02d%c%02d%c%02d", Hour % 12, dateTimeFormat[dateFormat].hourSep, Minute, dateTimeFormat[dateFormat].minSep, Seconds);
    lcd.print(textBuffer);
  } else {
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
  localTime = now() + utcOffset * 60;
  Hour = hour(localTime);
  Minute = minute(localTime);
  Seconds = second(localTime);

  lcd.setCursor(0, 0);
  // top line has 5 hour resolution
  if (Hour > 4) {
    lcd.print("-");
    lcd.write(ALL_ON);
    lcd.print("-");
    lcd.print(" ");  // fills square
  } else lcd.print(F("    "));
  if (Hour > 9) {
    lcd.print("-");
    lcd.write(ALL_ON);
    lcd.print("-");
    lcd.print(" ");  // fills square
  } else lcd.print(F("    "));

  if (Hour > 14) {
    lcd.print("-");
    lcd.write(ALL_ON);
    lcd.print("-");
    lcd.print(" ");  // fills square
  } else lcd.print(F("    "));
  if (Hour > 19) {
    lcd.print("-");
    lcd.write(ALL_ON);
    lcd.print("-");
    lcd.print(" ");  // fills square
  } else lcd.print(F("    "));

  lcd.setCursor(18, 0);
  lcd.print(F("5h"));

  // second line shows remainder and has 1 hour resolution

  lcd.setCursor(0, 1);
  imax = Hour % 5;
  if (imax > 0) {
    lcd.print("-");
    lcd.write(ALL_ON);
    lcd.print("-");
    lcd.print(" ");  // fills square
  } else lcd.print(F("    "));
  if (imax > 1) {
    lcd.print("-");
    lcd.write(ALL_ON);
    lcd.print("-");
    lcd.print(" ");  // fills square
  } else lcd.print(F("    "));
  if (imax > 2) {
    lcd.print("-");
    lcd.write(ALL_ON);
    lcd.print("-");
    lcd.print(" ");  // fills square
  } else lcd.print(F("    "));
  if (imax > 3) {
    lcd.print("-");
    lcd.write(ALL_ON);
    lcd.print("-");
    lcd.print(" ");  // fills square
  } else lcd.print(F("    "));

  lcd.setCursor(18, 1);
  lcd.print(F("1h"));

  // third line shows a bar for every 5 minutes
  int ii;
  lcd.setCursor(0, 2);

  // only overwrite old characters when needed, to avoid flicker

  imax = Minute / 5;
  if (imax == 0) {
    lcd.print(F("                  "));
    lcd.setCursor(0, 2);
  }

  for (ii = 0; ii < imax; ii++) {
    lcd.write(ALL_ON);
    if (ii == 2 || ii == 5 || ii == 8) lcd.print(" ");
  }

  lcd.setCursor(18, 2);
  lcd.print(F("5m"));

  // fourth line shows remainder and has 1 minute resolution
  lcd.setCursor(0, 3);

  imax = Minute % 5;
  if (imax > 0) {
    lcd.print("-");
    lcd.write(ALL_ON);
    lcd.print("-");
    lcd.print(" ");  // fills square
  } else lcd.print(F("    "));

  if (imax > 1) {
    lcd.print("-");
    lcd.write(ALL_ON);
    lcd.print("-");
    lcd.print(" ");  // fills square
  } else lcd.print(F("    "));

  if (imax > 2) {
    lcd.print("-");
    lcd.write(ALL_ON);
    lcd.print("-");
    lcd.print(" ");  // fills square
  } else lcd.print(F("    "));

  if (imax > 3) {
    lcd.print("-");
    lcd.write(ALL_ON);
    lcd.print("-");
    lcd.print(" ");  // fills square
  } else lcd.print(F("    "));

  lcd.setCursor(18, 3);
  lcd.print(F("1m"));
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
   
  loadSimpleBarCharacters();  // load user-defined characters for LCD, if not loaded                       

  // get local time
  localTime = now() + utcOffset * 60;
  Hour = hour(localTime);
  Minute = minute(localTime);
  Seconds = second(localTime);

  lcd.setCursor(0, 0);
  // top line has 10 hour resolution
  imax = Hour / 10;
  lcd.print(F("             "));
  lcd.setCursor(0, 0);
  for (ii = 0; ii < imax; ii++) {
    lcd.write(ALL_ON);
  }
  lcd.setCursor(17, 0);
  lcd.print(F("10h"));

  // second line shows remainder and has 1 hour resolution
  lcd.setCursor(0, 1);
  imax = Hour % 10;

  // only overwrite old characters when needed, to avoid flicker
  if (imax == 0) {
    lcd.print(F("             "));
    lcd.setCursor(0, 1);
  }
  for (ii = 0; ii < imax; ii++) {
    lcd.write(ALL_ON);
    if (ii == 4) lcd.print(" ");
  }
  lcd.setCursor(18, 1);
  lcd.print(F("1h"));

  // third line shows a bar for every 10 minutes
  lcd.setCursor(0, 2);
  imax = Minute / 10;

  // only overwrite old characters when needed, to avoid flicker
  if (imax == 0) {
    lcd.print(F("                  "));
    lcd.setCursor(0, 2);
  }

  for (ii = 0; ii < imax; ii++) {
    lcd.write(ALL_ON);
    if (ii == 4) lcd.print(" ");
  }
  lcd.setCursor(17, 2);
  lcd.print(F("10m"));

  // last line shows remainder and has 1 minute resolution
  lcd.setCursor(0, 3);

  // only overwrite old characters when needed, to avoid flicker

  imax = Minute % 10;
  if (imax == 0) {
    lcd.print(F("                  "));
    lcd.setCursor(0, 3);
  }

  for (ii = 0; ii < imax; ii++) {
    lcd.write(ALL_ON);
    if (ii == 4) lcd.print(" ");
  }

  // 14.10.2023: add 5 ticks per minute, i.e. every 12 seconds
  if      (Seconds / 12 == 1) lcd.write(ONE_BAR);
  else if (Seconds / 12 == 2) lcd.write(TWO_BARS);
  else if (Seconds / 12 == 3) lcd.write(THREE_BARS);
  else if (Seconds / 12 == 4) lcd.write((byte)FOUR_BARS);

  lcd.setCursor(18, 3);
  lcd.print(F("1m"));
}

/*****
Purpose: Menu item
Finds internal time: jd1970, j2000, etc

Argument List: None

Return value: Displays on LCD
*****/

void InternalTime() {  // UTC, Unix time, J2000, etc

  lcd.setCursor(0, 0);  // top line *********
  if (gps.time.isValid()) {

    float jd1970 = now() / 86400.0;  // cdn(now()); // now/86400, i.e. no of days since 1970 [No leap seconds]
    float j2000 = jd1970 - 10957.5;  // 1- line
    lcd.print(F("j2k "));
    lcd.print(j2000);

    lcd.setCursor(12, 0);
    sprintf(textBuffer, "%02d%c%02d%c%02d UTC ", hour(now()), dateTimeFormat[dateFormat].hourSep, minute(now()), dateTimeFormat[dateFormat].minSep, second(now()));
    lcd.print(textBuffer);

    lcd.setCursor(0, 2);
    lcd.print(F("jd1970 "));
    lcd.print(jd1970,3);
   
    lcd.setCursor(0, 1);
    Seconds = second(now());
    Minute = minute(now());
    Hour = hour(now());
    Day = day(now());
    Month = month(now());
    Year = year(now());

    // new 9.2.2024
    jd = get_julian_date(Day, Month, Year, Hour, Minute, Seconds);  // local - since year 4713 BC
    
    lcd.print(F("jd   "));
    lcd.print(jd,1);lcd.print("+"); lcd.print(jd_frac,3);  // more accurate
    // float jdd = jd + jd_frac; lcd.print(jdd);  // loses accuracy relative to previous line
   
    lcd.setCursor(0, 3);
    lcd.print(F("unix   "));
    lcd.print(now());   
    lcd.setCursor(18, 3); lcd.print(F("  ")); // blank out menu number    

    // lcd.print(F("local  "));
    // localTime = now() + utcOffset * 60;
    // lcd.print(localTime);
  }
}
/*****
Purpose: Menu item
Gives code status

Argument List: None

Return value: Displays on LCD
*****/

void CodeStatus(void) {
  lcd.setCursor(0, 0);
  lcd.print(F("* LA3ZA GPS clock *"));
  lcd.setCursor(0, 1);
  lcd.print(codeVersion);
  //lcd.setCursor(0, 2); lcd.print(F("GPS  ")); lcd.print(gpsBaud); lcd.print(" bps");
  lcd.setCursor(0, 2);
  lcd.print(F("GPS "));
  lcd.print(gpsBaud1[baudRateNumber]);
  lcd.print(F(" bps, PPS "));
  lcd.print(using_PPS);
  lcd.setCursor(0, 3);
  lcd.print(tcr->abbrev);
  lcd.setCursor(4, 3);
  PrintFixedWidth(lcd, utcOffset, 4);
  lcd.print(F(" min, "));  //timezone name and offset (min)
  lcd.print(languages[languageNumber]);
  lcd.print(F("    "));
}

/*****
Purpose: Menu item
Gives UTC time, locator, latitude/longitude, altitude and no of satellites

Argument List: None

Return value: Displays on LCD
*****/

void UTCPosition() {  // position, altitude, locator, # satellites
  String textBuffer;

  LcdUTCTimeLocator(0, 1);  // top line ********* start 1 position right - in order to line up with latitude/longitude
  // UTC date
  if (gps.location.isValid()) {

#ifndef DEBUG_MANUAL_POSITION
    latitude = gps.location.lat();
    lon = gps.location.lng();
    alt = gps.altitude.meters();
#else
    latitude = latitude_manual;
    lon = longitude_manual;
    alt = 0.0;
#endif

    int cycleTime = 10;  // 4.10.2022: was 4 seconds

    lcd.setCursor(0, 2);
    if ((now() / cycleTime) % 3 == 0) {  // change every cycleTime seconds

      // fixed formatting to handle 3-digit E-W, single-digit degree, and single-digit minute; 17.7.2023

      //  decimal degrees
      lcd.setCursor(1, 2);
      if (abs(latitude) < 10) lcd.print(" ");
      textBuffer = String(abs(latitude), 4);
      lcd.print(textBuffer);
      lcd.write(DEGREE);
      if (latitude < 0) lcd.print(F(" S   "));
      else lcd.print(F(" N   "));

      lcd.setCursor(0, 3);
      // textBuffer = String(abs(lon), 4);
      //int strLength = textBuffer.length();
      //lcd.print(textBuffer);
      if (abs(lon) < 100) lcd.print(" ");
      if (abs(lon) < 10) lcd.print(" ");
      lcd.print(abs(lon), 4);
      lcd.write(DEGREE);
      if (lon < 0) lcd.print(F(" W    "));
      else lcd.print(F(" E    "));
    } else if ((now() / cycleTime) % 3 == 1) {

      // degrees, minutes, seconds
      lcd.setCursor(0, 2);
      float mins;
      //textBuffer = String((int)abs(latitude));
      //lcd.print(textBuffer);
      PrintFixedWidth(lcd, (int)abs(latitude), 3);
      lcd.write(DEGREE);
      mins = abs(60 * (latitude - (int)latitude));  // minutes
      //textBuffer = String((int)mins);
      //lcd.print(textBuffer);
      PrintFixedWidth(lcd, (int)mins, 2, '0');
      lcd.write("'");
      // textBuffer = String((int)(abs(60 * (mins - (int)mins)))); // seconds
      // lcd.print(textBuffer);
      PrintFixedWidth(lcd, (int)(abs(60 * (mins - (int)mins))), 2, '0');
      lcd.write(34);
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
      PrintFixedWidth(lcd, (int)mins, 2, '0');
      lcd.write("'");
      // textBuffer = String((int)(abs(60 * (mins - (int)(mins)))));
      // lcd.print(textBuffer);
      PrintFixedWidth(lcd, (int)(abs(60 * (mins - (int)mins))), 2, '0');
      lcd.write(34);  // symbol for "
      if (lon < 0) lcd.print(F(" W "));
      else lcd.print(F(" E "));
    }

    else {

      // degrees, decimal minutes
      lcd.setCursor(0, 2);
      float mins;
      // textBuffer = String(int(abs(latitude)));
      // lcd.print(textBuffer);
      PrintFixedWidth(lcd, (int)abs(latitude), 3);
      lcd.write(DEGREE);
      mins = abs(60 * (latitude - (int)latitude));
      if (mins < 10) lcd.print('0');
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
      if (mins < 10) lcd.print('0');
      textBuffer = String(abs(mins), 2);  // double abs() to avoid negative number for x.00 degrees
      lcd.print(textBuffer);
      if (lon < 0) lcd.print(F("' W  "));
      else lcd.print(F("' E  "));
    }
  }
  // enough space on display for 2469 m
  lcd.setCursor(14, 2);
  PrintFixedWidth(lcd, (int)round(alt), 4, ' ');
  lcd.print(F(" m"));

  //  if (gps.satellites.isValid()) { // 16.11.2022
  if (gps.satellites.isUpdated()) {
    noSats = gps.satellites.value();
    if (noSats < 10) lcd.setCursor(14, 3);
    else lcd.setCursor(13, 3);
  } else {
    noSats = 0;
    lcd.setCursor(14, 3);
  }
  lcd.print(noSats);
  lcd.print(F(" Sats"));
}


/*****
Purpose: Menu item
Finds data for beacons of NCDXF  Northern California DX Foundation

Argument List: byte option 
  option=1: 14-21 MHz beacons on lines 1-3,  
  option=2: 21-28 MHz beacons on lines 1-3

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
  "N York ", "Inuvik ", "Califor", "Hawaii ", "N Zeala", "Perth  ", "Japan  ", "Nsibirs", "H Kong ", "Colombo", "Pretori",
  "Kenya  ", "Israel ", "Finland", "Madeira", "B Aires", "Peru   ", "Venezue"
};


void NCDXFBeacons(byte option) {
  // option=1: 14-21 MHz beacons on lines 1-3,  option=2: 21-28 MHz beacons on lines 1-3

  // Inspired by OE3GOD: https://www.hamspirit.de/7757/eine-stationsuhr-mit-ncdxf-bakenanzeige/

  int ii, iii, iiii, km, bandOffset;
  double lati, longi;
  char location[7];                                    // to hold 6 char + terminating '\0'
  int qrg[6] = { 14100, 18110, 21150, 24930, 28200 };  // 10.4.2023: from char to int to save RAM

  LcdUTCTimeLocator(0);  // top line *********

  ii = (60 * (minuteGPS % 3) + secondGPS+1) / 10;  // ii from 0 to 17; 11.01.2024: +1 added to shift on :00 rather than :01

  if (option <= 1) bandOffset = 0;  // 14-18 MHz
  else bandOffset = 2;              // 18-28 MHz

  for (iiii = 1; iiii < 4; iiii += 1) {  // step over lines 1,2,3
    lcd.setCursor(0, iiii);

    // modulo for negative numbers: https://twitter.com/parkerboundy/status/326924215833985024
    iii = ((ii - iiii + 1 - bandOffset % 18) + 18) % 18;
    lcd.print(qrg[iiii - 1 + bandOffset]);
    lcd.print(" ");
    lcd.print(reinterpret_cast<const __FlashStringHelper *>(callsign[iii]));

    if ((secondGPS+1)% 10 < 5) {     // 11.01.2024: +1 added to shift on :00 rather than :01
      lcd.print(" ");
      lcd.print(reinterpret_cast<const __FlashStringHelper *>((qth[iii])));
    } else {
      // copy from PROGMEM to local memory:
      strlcpy_P(location, locationP[iii], sizeof(location));  // does not overflow, just truncates

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

void WSPRsequence() {  // UTC, + WSPR band/frequency for coordinated WSPR
  // https://physics.princeton.edu//pulsar/K1JT/doc/wspr/wspr-main.html#BANDHOPPING
  // 20 min cycle over 10 bands from 160m to 10m

  int ii;
  int isec;
  // 10.4.2023: changed from char to int and float storage to save RAM:
  int band[11] = { 160, 80, 60, 40, 30, 20, 17, 15, 12, 10 };
  float qrg[11] = { 1838.100, 3570.100, 5366.200, 7040.100, 10140.200, 14097.100, 18106.100, 21096.100, 24926.100, 28126.100 };

  loadGapLessCharacters7();   // load LCD characters if not loaded

  LcdUTCTimeLocator(0);  // top line *********
  /*
     Each WSPR frequency is transmitted every 20 minutes in 2 min intervals.
  */
  lcd.setCursor(0, 1);
  lcd.print(F("WSPR band hopping:  "));

  //ii = (minuteGPS % 20) / 2;  // ii from 0 to 9, adresses band
  ii = (minuteGPS % 20) / 2;  // ii from 0 to 9, adresses band
  //Serial.print("ii, secondGPS  "); Serial.print(ii);Serial.print(" ");Serial.println(secondGPS);

  // WSPR transmission starts 1 second into even minute and lasts for 110.6 = 60 + 50.6 seconds
  if ((minuteGPS % 2 == 0 && secondGPS < 1) || (minuteGPS % 2 == 1 && secondGPS > 52)) {
    lcd.setCursor(0, 2);
    lcd.print(F("                    "));
    //lcd.setCursor(18, 3); lcd.print(F("  ")); // blank out menu number
    lcd.setCursor(0, 3);
    lcd.print(F("                    "));
  } else {
    isec = secondGPS - 1;
    if (minuteGPS % 2 == 1) isec = isec + 60;
    //Serial.print("minuteGPS "); Serial.println(minuteGPS);
    //Serial.print("isec      "); Serial.println(isec);
    lcd.setCursor(0, 2);
    lcd.print(band[ii]);
    lcd.print(F(" m "));
    lcd.print(qrg[ii], 1);
    lcd.print(F(" kHz  "));
    lcd.setCursor(17, 2);
    PrintFixedWidth(lcd, isec, 3);  // seconds into transmission

    // Progress bar on last line 7.11.2023
    gapLessBar(isec, 111, 0, 19, 3);
  }
  //
}

/*****
Purpose: Menu item
Shows clock in hex, octal, or binary format

Argument List: byte val = 0 - hex, 1 - octal, 3 - hex, octal, and binary

Return value: Displays on LCD
*****/

void HexOctalClock(byte val) {
  int BinaryHour[6], BinaryMinute[6], BinarySeconds[6];

  //  get local time
  localTime = now() + utcOffset * 60;
  Hour = hour(localTime);
  Minute = minute(localTime);
  Seconds = second(localTime);

  lcd.setCursor(0, 0);
  if (val == 0) lcd.print(F("Hex   "));
  else if (val == 1) lcd.print(F("Oct   "));

  if (val == 0)  // Hex
  {
    lcd.setCursor(7, 0);  // one line up 18.6.2023
    sprintf(textBuffer, "%02X%c%02X%c%02X", Hour, dateTimeFormat[dateFormat].hourSep, Minute, dateTimeFormat[dateFormat].minSep, Seconds);
    lcd.print(textBuffer);
    lcd.setCursor(17, 3);
    lcd.print(F("   "));
  } else if (val == 1)  // Oct
  {
    lcd.setCursor(7, 0);  // one line up 18.6.2023
    sprintf(textBuffer, "%02o%c%02o%c%02o", Hour, dateTimeFormat[dateFormat].hourSep, Minute, dateTimeFormat[dateFormat].minSep, Seconds);
    lcd.print(textBuffer);
    lcd.setCursor(17, 3);
    lcd.print(F("   "));
  } else  // Binary, oct, dec, hex
  {
    // convert to binary:
    DecToBinary(Hour, BinaryHour);
    DecToBinary(Minute, BinaryMinute);
    DecToBinary(Seconds, BinarySeconds);

    lcd.setCursor(0, 0);
    sprintf(textBuffer, "%1d%1d%1d%1d%1d", BinaryHour[1], BinaryHour[2], BinaryHour[3], BinaryHour[4], BinaryHour[5]);
    lcd.print(textBuffer);
    lcd.print(dateTimeFormat[dateFormat].hourSep);

    sprintf(textBuffer, "%1d%1d%1d%1d%1d%1d", BinaryMinute[0], BinaryMinute[1], BinaryMinute[2], BinaryMinute[3], BinaryMinute[4], BinaryMinute[5]);
    lcd.print(textBuffer);
    lcd.print(dateTimeFormat[dateFormat].minSep);

    sprintf(textBuffer, "%1d%1d%1d%1d%1d%1dB", BinarySeconds[0], BinarySeconds[1], BinarySeconds[2], BinarySeconds[3], BinarySeconds[4], BinarySeconds[5]);
    lcd.print(textBuffer);

    lcd.setCursor(19, 1);
    lcd.print("O");
    lcd.setCursor(7, 1);
    sprintf(textBuffer, "%02o%c%02o%c%02o", Hour, dateTimeFormat[dateFormat].hourSep, Minute, dateTimeFormat[dateFormat].minSep, Seconds);  // octal
    lcd.print(textBuffer);

    lcd.setCursor(18, 3);
    lcd.print(F(" H"));
    lcd.setCursor(7, 3);
    sprintf(textBuffer, "%02X%c%02X%c%02X", Hour, dateTimeFormat[dateFormat].hourSep, Minute, dateTimeFormat[dateFormat].minSep, Seconds);  // hex
    lcd.print(textBuffer);
  }


  //  help screen with normal clock for 0...secondsClockHelp seconds per minute
  if (Seconds < secondsClockHelp | val == 3)  // show time in normal numbers - always if simultaneous Bin, Oct, Hex
  {
    if (val == 3) {
      lcd.setCursor(19, 2);
      lcd.print("D");
    } else {
      lcd.setCursor(18, 3);
      lcd.print(F("  "));  // clear number in lower left corner
    }
    lcd.setCursor(7, 2);
    sprintf(textBuffer, "%02d%c%02d%c%02d", Hour, dateTimeFormat[dateFormat].hourSep, Minute, dateTimeFormat[dateFormat].minSep, Seconds);
    lcd.print(textBuffer);
  } else {
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
void EasterDates(int yr) {
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
  lcd.setCursor(0, 0);
  lcd.print(F("Easter  Greg. Julian"));

  if (minuteGPS != oldMinute) {

    for (int yer = yr; yer < yr + 3; yer++) {
      lcd.setCursor(2, ii);
      lcd.print(yer);
      lcd.print(F(": "));
      // Gregorian (West):
      K = -2;
      E = -10;
      ComputeEasterDate(yer, K, E, &PaschalFullMoon, &EasterDate, &EasterMonth);
      lcd.setCursor(8, ii);
      LcdDate(EasterDate, EasterMonth);

      // Julian (East)
      K = -3;
      E = -1;
      ComputeEasterDate(yer, K, E, &PaschalFullMoon, &EasterDate, &EasterMonth);
      JulianToGregorian(&EasterDate, &EasterMonth);  // add 13 days
      lcd.setCursor(14, ii);
      LcdDate(EasterDate, EasterMonth);
      lcd.print(" ");  // empty space at the end
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

Argument List: byte LevelMath  0 for +
                              1 for - or +
                              2 for *, -, or +
                              3 for /,*,-, or +

Return value: Displays on LCD
*****/

void MathClock(byte LevelMath) {
  int Term1Hr, Term2Hr, Term1Min, Term2Min;
  int rnd, HrMult, MinMult;

  //  get local time
  localTime = now() + utcOffset * 60;
  Hour = hour(localTime);
  Minute = minute(localTime);
  Seconds = second(localTime);


  if (now() % mathSecondPeriod == 0 | oldMinute == -1)  // ever so often + immediate start
  {

#ifdef FEATURE_SERIAL_MATH
    Serial.print(F("oldMinute, Seconds, mathSecondPeriod: "));
    Serial.print(oldMinute);
    Serial.print(":");
    Serial.print(Seconds);
    Serial.print(", ");
    Serial.println(mathSecondPeriod);
#endif


    HrMult = 0;
    MinMult = 0;

    if (LevelMath == 0)  // +
    {
      MathPlus(Hour, &Term1Hr, &Term2Hr);
      MathPlus(Minute, &Term1Min, &Term2Min);
    }

    else if (LevelMath == 1)  //  - and +
                              // was - : 4/5 probability, + : 1/5 probability. Now 50/50:
    {
      // was 0,1: +, 2,3,4,5: - now 0,1,2 or 3,4,5:
      if (random(0, 6) <= 2) MathPlus(Hour, &Term1Hr, &Term2Hr);
      else MathMinus(Hour, &Term1Hr, &Term2Hr);

      if (random(0, 6) <= 2) MathPlus(Minute, &Term1Min, &Term2Min);
      else MathMinus(Minute, &Term1Min, &Term2Min);
    }

    else if (LevelMath == 2)  // x - +
    // was x: 50% prob, -: 3/8 prob, +: 1/8 prob. Now 1/3, 1/3, 1/3:

    {
      rnd = random(0, 3);  // 0, 1, 2 was [0,...,7] - Hour
      if (rnd == 0) {
        MathMultiply(Hour, &Term1Hr, &Term2Hr);
        HrMult = 1;
      } else if (rnd == 1) MathPlus(Hour, &Term1Hr, &Term2Hr);
      else MathMinus(Hour, &Term1Hr, &Term2Hr);

      rnd = random(0, 3);  // Minute
      if (rnd == 0) {
        MathMultiply(Minute, &Term1Min, &Term2Min);
        MinMult = 1;
      } else if (rnd == 1) MathPlus(Minute, &Term1Min, &Term2Min);
      else MathMinus(Minute, &Term1Min, &Term2Min);
    }

    else if (LevelMath == 3)
    //was  /: 50% prob; x: 30% prob; -: 20% prob, no +, now 3 x 1/3 prob
    {
      rnd = random(0, 3);  //  Hour
      if (rnd == 0) {
        MathDivide(Hour, &Term1Hr, &Term2Hr);
        HrMult = 2;  // means divide
      } else if (rnd == 1) {
        MathMultiply(Hour, &Term1Hr, &Term2Hr);
        HrMult = 1;
      } else MathMinus(Hour, &Term1Hr, &Term2Hr);


      rnd = random(0, 3);  // Minute
      if (rnd == 0) {
        MathDivide(Minute, &Term1Min, &Term2Min);
        MinMult = 2;
      } else if (rnd == 1) {
        MathMultiply(Minute, &Term1Min, &Term2Min);
        MinMult = 1;
      } else MathMinus(Minute, &Term1Min, &Term2Min);
    }


    // display result on lcd

    // Hour:
    lcd.setCursor(5, 0);
    PrintFixedWidth(lcd, Term1Hr, 2);
    if (HrMult == 2)  // divide
    {
      lcd.print(" ");
      lcd.print(MATH_CLOCK_DIVIDE);
      lcd.print(" ");
    } else if (HrMult == 1) {
      lcd.print(" ");
      lcd.print(MATH_CLOCK_MULTIPLY);
      lcd.print(" ");
    } else {
      if (Term2Hr < 0) lcd.print(F(" - "));
      else lcd.print(F(" + "));
    }
    lcd.print(abs(Term2Hr));
    lcd.print(F("   "));
    lcd.setCursor(19, 0);
    lcd.print("h");

    // Minute:
    lcd.setCursor(8, 1);
    PrintFixedWidth(lcd, Term1Min, 2);
    if (MinMult == 2)  // divide
    {
      lcd.print(" ");
      lcd.print(MATH_CLOCK_DIVIDE);
      lcd.print(" ");
    } else if (MinMult == 1) {
      lcd.print(" ");
      lcd.print(MATH_CLOCK_MULTIPLY);
      lcd.print(" ");
    } else {
      if (Term2Min < 0) lcd.print(F(" - "));
      else lcd.print(F(" + "));
    }
    lcd.print(abs(Term2Min));
    lcd.print(F("   "));
    lcd.setCursor(19, 1);
    lcd.print("m");
  }

  // lower line
  // left corner:
  lcd.setCursor(0, 3);
  // show remaining time (13.7.2023):
  PrintFixedWidth(lcd, mathSecondPeriod - now() % mathSecondPeriod, 2);  //lcd.print(F("  "));

  // then show symbols to the right:
  lcd.setCursor(17, 3);

  if (LevelMath == 3) {
    lcd.print(MATH_CLOCK_DIVIDE);
    lcd.print(MATH_CLOCK_MULTIPLY);
    lcd.print("-");
  } else if (LevelMath == 2) {
    lcd.print(MATH_CLOCK_MULTIPLY);
    lcd.print(F("+-"));
  } else if (LevelMath == 1) lcd.print(F(" +-"));
  else lcd.print(F("  +"));


  //lcd.setCursor(18, 3); lcd.print(F("  "));

  oldMinute = minuteGPS;
}

/*****
Purpose: Menu item
Finds the Lunar eclipses for the next years

Argument List: none

Return value: Displays on LCD

*****/

void LunarEclipse() {
  int pDate[10];  // max 5 per day: packed date = 100*month + day, i.e. 1209 = 9 December
  int eYear[10];
  int pday, pmonth, yy;
  int i;

  if (minuteGPS != oldMinute) {


    lcd.setCursor(0, 0);
    lcd.print(F("Lunar Eclipses "));

    // Good up to and including 2021, but misses the almost eclipse 18-19 July 2027
    // Test: try 2028 with 3 eclipses, see https://www.timeanddate.com/eclipse/list-lunar.html
    yy = yearGPS;  //

#ifdef FEATURE_SERIAL_LUNARECLIPSE
    Serial.print((int)yy);
    Serial.println(F(" ****************"));
#endif

    for (i = 0; i < 10; i++) pDate[i] = 0;

    MoonEclipse(yy, pDate, eYear);
    int lineNo = 1;
    lcd.setCursor(2, lineNo);
    lcd.print(yy);
    lcd.print(":");
    int col = 8;

    for (i = 0; (i < 10 && pDate[i] != 0); i++) {
      if (eYear[i] == yy) {
        if (col > 14) {
          col = 2;
          lineNo = lineNo + 1;  // start another line if more than 3 eclipses this year (first time in 2028!)
        }
        lcd.setCursor(col, lineNo);
        pmonth = pDate[i] / 100;
        pday = pDate[i] - 100 * pmonth;
        LcdDate(pday, pmonth);
        lcd.print(" ");
        col = col + 6;
      }
    }

    for (i = 0; i < 10; i++) pDate[i] = 0;
    yy = yy + 1;
    MoonEclipse(yy, pDate, eYear);

    //pDate[2] = 729; //pDate[3] = 1209; pDate[4] = 101; // artificial data  for testing

    lineNo = lineNo + 1;
    lcd.setCursor(2, lineNo);
    lcd.print(yy);
    lcd.print(":");
    col = 8;
    for (i = 0; (i < 10 && pDate[i] != 0 && lineNo < 4); i++) {
      if (eYear[i] == yy) {
        if (col > 14) {
          col = 2;
          lineNo = lineNo + 1;
        }
        lcd.setCursor(col, lineNo);
        pmonth = pDate[i] / 100;
        pday = pDate[i] - 100 * pmonth;
        LcdDate(pday, pmonth);
        lcd.print(" ");
        col = col + 6;
      }
    }

    if (lineNo < 3) {
      for (i = 0; i < 10; i++) pDate[i] = 0;

      yy = yy + 1;
      MoonEclipse(yy, pDate, eYear);

      lineNo = lineNo + 1;
      lcd.setCursor(2, lineNo);
      lcd.print(yy);
      lcd.print(":");
      col = 8;

      for (i = 0; (i < 2 && pDate[i] != 0 && lineNo < 4); i++)  // never room for more than two
      {
        if (eYear[i] == yy) {
          lcd.setCursor(col, lineNo);
          pmonth = pDate[i] / 100;
          pday = pDate[i] - 100 * pmonth;
          LcdDate(pday, pmonth);
          lcd.print(" ");
          col = col + 6;
        }
      }
    }
    if (col < 14) {
      lcd.setCursor(18, 3);
      lcd.print(F("  "));  // erase lower right-hand corner if not already done
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

const char RomanOnes[10][6] PROGMEM = { { "     " }, { "I    " }, { "II   " }, { "III  " }, { "IV   " }, { "V    " }, { "VI   " }, { "VII  " }, { "VIII " }, { "IX   " } };  // left justified
const char RomanTens[6][4] PROGMEM = { { "" }, { "X" }, { "XX" }, { "XXX" }, { "XL" }, { "L" } };

void Roman() {
  int ones, tens;

  /* The longest symbol
   *  Hours:            18: XVIII, 23: XXIII
   *  Minutes, seconds: 38: XXXVIII
   *  Longest symbol is 5+1+7+1+7 = 21 letters long, so it doesn't fit a single line on a 20 line LCD
   */

  //  get local time
  localTime = now() + utcOffset * 60;
  Hour = hour(localTime);
  Minute = minute(localTime);
  Seconds = second(localTime);

  ones = Hour % 10;
  tens = (Hour - ones) / 10;

  lcd.setCursor(0, 0);
  lcd.print(reinterpret_cast<const __FlashStringHelper *>(RomanTens[tens]));
  lcd.print(reinterpret_cast<const __FlashStringHelper *>(RomanOnes[ones]));
  //lcd.setCursor(5, 0);lcd.print(dateTimeFormat[dateFormat].hourSep);

  ones = Minute % 10;
  tens = (Minute - ones) / 10;
  lcd.setCursor(6, 1);
  lcd.print(reinterpret_cast<const __FlashStringHelper *>(RomanTens[tens]));
  lcd.print(reinterpret_cast<const __FlashStringHelper *>(RomanOnes[ones]));
  //lcd.setCursor(13, 0); lcd.print(dateTimeFormat[dateFormat].minSep);

  ones = Seconds % 10;
  tens = (Seconds - ones) / 10;
  lcd.setCursor(12, 2);
  lcd.print(reinterpret_cast<const __FlashStringHelper *>(RomanTens[tens]));
  lcd.print(reinterpret_cast<const __FlashStringHelper *>(RomanOnes[ones]));

  //  help screen with normal clock for 0...secondsClockHelp seconds per minute
  if (Seconds < secondsClockHelp)  // show time in normal numbers
  {
    lcd.setCursor(0, 3);
    sprintf(textBuffer, "%02d%c%02d%c%02d", Hour, dateTimeFormat[dateFormat].hourSep, Minute, dateTimeFormat[dateFormat].minSep, Seconds);
    lcd.print(textBuffer);
  } else {
    lcd.setCursor(0, 3);
    lcd.print(F("        "));
  }
  lcd.setCursor(18, 3);
  lcd.print(F("  "));  // blank out number in lower right-hand corner
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
  double LST_hours, LST_degrees;

  float jd1970 = now() / 86400.0;  // cdn(now()); // now/86400, i.e. no of days since 1970
  float j2000 = jd1970 - 10957.5;  // 1- line

  double decimal_time = hour(now()) + (minute(now()) / 60.0) + (second(now()) / 3600.0);
  double LST = 100.46 + 0.985647 * j2000 + gps.location.lng() + 15 * decimal_time;
  LST_degrees = (LST - (floor(LST / 360) * 360));
  LST_hours = LST_degrees / 15;

  int rHours = (int)LST_hours;
  int rMinutes = ((int)floor((LST_hours - rHours) * 60));

  // compute local solar time based on Equation Of Time
  // EQUATIO: Sidereal & Solar Clock, by Wooduino
  // Routines from http://woodsgood.ca/projects/2015/06/14/equatio-sidereal-solar-clock/

  double tv;  // time variable offset in minutes
  doEoTCalc(&tv);

  // time correction factor: https://www.pveducation.org/pvcdrom/properties-of-sunlight/solar-time
  // note 4 minutes = 1 degree of rotation of earth

  // 2021-11-25 test with average of the three
  // rel https://fate.windada.com/cgi-bin/SolarTime_en: 35 sec too fast
  // rel to http://www.jgiesen.de/astro/astroJS/siderealClock/: 51 sec too slow

  // new test with Wikipedia method:
  // rel https://fate.windada.com/cgi-bin/SolarTime_en: 35 sec too fast: 2 sec 3-4 sec faster

  double tc = 4.0 * lon + tv;  // correction in minutes: Deviation from center of time zone + Equation of Time

  // display results:
 loadNativeCharacters(languageNumber);

  LcdShortDayDateTimeLocal(0, 0);  // line 0 local time

  lcd.setCursor(0, 1);
  lcd.print(F("Solar time"));
  lcd.setCursor(11, 1);
  time_t solar;
  solar = now() + (int)(tc * 60);
  Hour = hour(solar);
  Minute = minute(solar);
  Seconds = second(solar);
  //sprintf(textBuffer, " %02d%c%02d%c%02d", Hour, dateTimeFormat[dateFormat].hourSep, Minute, dateTimeFormat[dateFormat].minSep, Seconds);
  // drop seconds:
  sprintf(textBuffer, " %02d%c%02d%   ", Hour, dateTimeFormat[dateFormat].hourSep, Minute);
  lcd.print(textBuffer);

  lcd.setCursor(0, 2);
  lcd.print(F("Sidereal"));   // remove this for Metro, and "In view" starts on (18,2) and continues on (0,3)
  lcd.setCursor(12, 2);
  PrintFixedWidth(lcd, rHours, 2, '0');
  lcd.print(dateTimeFormat[dateFormat].hourSep);
  PrintFixedWidth(lcd, rMinutes, 2, '0');

#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
// For some reason needed for Metro to overwrite a duplicate "Si" "dereal"
  lcd.setCursor(18,1); lcd.print(F("  "));
  lcd.setCursor( 0,3); lcd.print(F("       ")); 
#endif

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
  lcd.setCursor(18, 3);
  lcd.print(F("  "));

#ifdef EXP_TIDE_SIDEREAL
  // tidal calculation
  int luniTidal = 359;  // minutes, lunitidal interval: how long after meridian high tide occurs, Asker (1 July 2023)
                        // lunitidal interval further varies within about +/-30 minutes according to the lunar phase. https://en.wikipedia.org/wiki/Lunitidal_interval

  float comp = -934;                   // minutes, offset for meridian on July 1, 2023 in Asker: local fudge factor!!!
  localTime = now() + utcOffset * 60;  // local time in sec

  long secondsMeridian = 86400 + 86400. / (CYCLELENGTH / 86400. - 1);  // CYCLELENGTH in sec = 29.53 days for moon phase
                                                                       // no double for Arduino
  long nextMeridian = secondsMeridian - localTime % secondsMeridian;   // no of seconds to next meridian

  //double fract = (localTime/float(secondsMeridian)) - int((localTime/float(secondsMeridian)));   // where are we in meridian cycle 0...1
  time_t next = localTime + nextMeridian + comp * 60.;  // time of meridian

  Hour = hour(next);
  Minute = minute(next);
  lcd.setCursor(0, 3);
  lcd.print(F("Mer "));  // meridian - time of closest approach of moon
  PrintFixedWidth(lcd, Hour, 2, '0');
  lcd.print(dateTimeFormat[dateFormat].hourSep);
  PrintFixedWidth(lcd, Minute, 2, '0');

  Hour = hour(next + 60. * luniTidal);  // High tide
  Minute = minute(next + 60. * luniTidal);
  lcd.print(F(" T "));  // tide
  PrintFixedWidth(lcd, Hour, 2, '0');
  lcd.print(dateTimeFormat[dateFormat].hourSep);
  PrintFixedWidth(lcd, Minute, 2, '0');
  //lcd.print(F(" Hi"));

  #ifdef FEATURE_SERIAL_MOON
    Serial.print(F("secondsMeridian "));
    Serial.println(secondsMeridian);
    Serial.print(F("nextMeridian    "));
    Serial.println(nextMeridian);
  #endif
#endif  // EXP_TIDE_SIDEREAL
}

/*****
Purpose: Menu item
Finds local time in Morse code

Argument List: none

Return value: Displays on LCD
*****/

void Morse()  // time in Morse code on LCD
{
  int ones, tens;

  //  get local time
  localTime = now() + utcOffset * 60;
  Hour = hour(localTime);
  Minute = minute(localTime);
  Seconds = second(localTime);

  ones = Hour % 10;
  tens = (Hour - ones) / 10;
  lcd.setCursor(0, 0);
  LcdMorse(tens);
  //  if (tens != 0) LcdMorse(tens);
  //  else lcd.print("     ");
  lcd.print(" ");
  LcdMorse(ones);

  ones = Minute % 10;
  tens = (Minute - ones) / 10;
  lcd.setCursor(4, 1);
  LcdMorse(tens);
  lcd.print(" ");
  LcdMorse(ones);

  ones = Seconds % 10;
  tens = (Seconds - ones) / 10;
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
  } else {
    lcd.setCursor(0, 3);
    lcd.print(F("        "));
  }
  lcd.setCursor(18, 3);
  lcd.print(F("  "));  // blank out number in lower right-hand corner
}

/*****
Purpose: Menu item
Finds local time written out with words

Argument List: none

Return value: Displays on LCD
*****/

const char WordOnes[10][6] PROGMEM = { { " oh  " }, { "one  " }, { "two  " }, { "three" }, { "four " }, { "five " }, { "six  " }, { "seven" }, { "eight" }, { "nine " } };  // left justified
const char CapiOnes[10][6] PROGMEM = { { "Oh   " }, { "One  " }, { "Two  " }, { "Three" }, { "Four " }, { "Five " }, { "Six  " }, { "Seven" }, { "Eight" }, { "Nine " } };  // left justified
const char WordTens[6][7] PROGMEM = { { "    Oh" }, { "   Ten" }, { "Twenty" }, { "Thirty" }, { " Forty" }, { " Fifty" } };
const char Teens[10][10] PROGMEM = { { "         " }, { "Eleven   " }, { "Twelve   " }, { "Thirteen " }, { "Fourteen " }, { "Fifteen  " }, { "Sixteen  " }, { "Seventeen" }, { "Eighteen " }, { "Nineteen " } };

void WordClockEnglish() {
  int ones, tens;

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
  if (Hour < 10) {
    lcd.print(reinterpret_cast<const __FlashStringHelper *>(CapiOnes[int(Hour)]));
    lcd.print(F("      "));
  } else if (Hour > 10 && Hour < 20) lcd.print(reinterpret_cast<const __FlashStringHelper *>(Teens[int(Hour) - 10]));
  else {
    ones = Hour % 10;
    tens = (Hour - ones) / 10;
    lcd.print(reinterpret_cast<const __FlashStringHelper *>(WordTens[tens]));
    if (ones != 0) {
      if (tens != 0) lcd.print("-");
      else lcd.print(" ");
      lcd.print(reinterpret_cast<const __FlashStringHelper *>(WordOnes[ones]));
    } else {
      if (tens == 0) lcd.print(reinterpret_cast<const __FlashStringHelper *>(WordOnes[ones]));
      lcd.print(F("       "));  // added extra space 14.5.2023
    }
  }

  lcd.setCursor(2, 1);
  if (Minute > 10 && Minute < 20) lcd.print(reinterpret_cast<const __FlashStringHelper *>(Teens[int(Minute) - 10]));
  else {
    ones = Minute % 10;
    tens = (Minute - ones) / 10;
    lcd.print(reinterpret_cast<const __FlashStringHelper *>(WordTens[tens]));
    if (ones != 0) {
      if (tens != 0) lcd.print("-");
      else lcd.print(" ");
      lcd.print(reinterpret_cast<const __FlashStringHelper *>(WordOnes[ones]));
    } else {
      if (tens == 0) lcd.print(reinterpret_cast<const __FlashStringHelper *>(WordOnes[ones]));
      lcd.print(F("      "));
    }
  }


  lcd.setCursor(4, 2);
  if (Seconds > 10 && Seconds < 20) lcd.print(reinterpret_cast<const __FlashStringHelper *>(Teens[int(Seconds) - 10]));
  else {

    // not rewritten due to 2048 character limit:
    ones = Seconds % 10;
    tens = (Seconds - ones) / 10;
    lcd.print(reinterpret_cast<const __FlashStringHelper *>(WordTens[tens]));
    if (ones != 0) {
      if (tens != 0) lcd.print("-");
      else lcd.print(" ");
      lcd.print(reinterpret_cast<const __FlashStringHelper *>(WordOnes[ones]));
    } else {
      if (tens == 0) lcd.print(reinterpret_cast<const __FlashStringHelper *>(WordOnes[ones]));
      lcd.print(F("      "));
    }
  }

  lcd.setCursor(0, 3);
  lcd.print(F("        "));
  lcd.setCursor(18, 3);
  lcd.print(F("  "));  // blank out number in lower right-hand corner
}



/*****
Purpose: Menu item
Run a demo by cycling through all specified memu items

Argument List: byte inDemo: 0 or 1
                           1 - first time in demo mode, initialization
                           0 - already in demo mode

Return value: Displays on LCD

Issues: may take up to dwellTimeDemo seconds before first screen is shown (typ 10-15 sec max)
*****/

void DemoClock(byte inDemo  // 0 or 1
) {
  //int dwell = dwellTimeDemo;
  if (inDemo == 0)  // already in demo mode
    ScreenSelect(demoDispState, 1);
  else  // first time in demo mode
  {
    lcd.setCursor(0, 0);
    lcd.print(F("  *** D E M O ***   "));
    lcd.setCursor(0, 1);
    lcd.print(F("Multi Face GPS Clock"));

    lcd.setCursor(14,2);
    sprintf(textBuffer, "%6s",demoStepTypeText[demoStepType]);   // print right-justified -> garbish
    lcd.print(textBuffer);
    
    lcd.setCursor(0, 3);
    lcd.print(F("github/la3za "));
    PrintFixedWidth(lcd, dwellTimeDemo, 3);
    lcd.print(F(" sec"));
  }
}


/*****
Purpose: 
Selects WordClock language 

Argument List: none

Return value: none
*****/

void WordClock() {
  if (strcmp(languages[languageNumber], "no")==0 || strcmp(languages[languageNumber], "ny")==0) 
  {
    loadAring();  // Å for Norwegian 
    WordClockNorwegian();
  }
  else WordClockEnglish();
}

/*****
Purpose: Menu item
Shows info about 3 outer and 2 inner planets + alternates between solar/lunar info

Argument List: byte inner = 1 for inner planets, else outer planets

Return value: Displays on LCD
*****/

void PlanetVisibility(byte inner  // inner = 1 for inner planets, all other values -> outer
) {
  //
  // Agrees with https://www.heavens-above.com/PlanetSummary.aspx?lat=59.8348&lng=10.4299&loc=Unnamed&alt=0&tz=CET
  // except for brightness or magnitude of mercury: this code says 0.2 when web says 0.6

  // Julian day ref noon Universal Time (UT) Monday, 1 January 4713 BC in the Julian calendar:
  //jd = get_julian_date (20, 1, 2017, 17, 0, 0);//UTC

  if (gps.location.isValid())  // new 24.09.2024 - avoid giving planet positions for lat, lon = (0.0, 0.0)
  {

    #ifndef DEBUG_MANUAL_POSITION    // new 24.09.2024 - avoid giving planet positions for lat, lon = (0.0, 0.0)
      latitude = gps.location.lat();
      lon = gps.location.lng();
    #else
      latitude = latitude_manual;
      lon = longitude_manual;
    #endif
  
    Seconds = second(now());
    Minute = minute(now());
    Hour = hour(now());
    Day = day(now());
    Month = month(now());
    Year = year(now());

    jd = get_julian_date(Day, Month, Year, Hour, Minute, Seconds);  // local - since year 4713 BC

  #ifdef FEATURE_SERIAL_PLANETARY
    Serial.println("JD:" + String(jd, DEC) + "+" + String(jd_frac, DEC));  // jd = 2457761.375000;
  #endif

    get_object_position(2, jd, jd_frac);  //earth -- must be included always

    lcd.setCursor(0, 0);  // top line *********
    lcd.print(F("    El"));
    lcd.write(DEGREE);
    lcd.print(F(" Az"));
    lcd.write(DEGREE);
    lcd.print(F("   % Magn"));

    if (inner == 1) {
      get_object_position(0, jd, jd_frac);    // Mercury
      lcd.setCursor(0, 2);
      lcd.print(F("Mer "));
      LCDPlanetData(altitudePlanet, azimuthPlanet, phase, magnitude);

      lcd.setCursor(0, 3);
      get_object_position(1, jd, jd_frac);    // Venus
      lcd.print(F("Ven "));
      LCDPlanetData(altitudePlanet, azimuthPlanet, phase, magnitude);

      lcd.setCursor(0, 1);
      if ((now() / 10) % 2 == 0)  // change every 10 seconds
      {
        // Moon
        float Phase, PercentPhase;
        lcd.print(F("Lun "));
        UpdateMoonPosition();           // calls K3NG moon2()
        MoonPhase(Phase, PercentPhase);
        LCDPlanetData(moon_elevation, moon_azimuth, PercentPhase / 100., -12.7);
      } else {
        // Sun
        lcd.print(F("Sun "));

        /////// Solar elevation //////////////////
        double sun_azimuth = 0;
        double sun_elevation = 0;
        // solar az, el now:
        calcHorizontalCoordinates(now(), latitude, lon, sun_azimuth, sun_elevation);
        LCDPlanetData(round(sun_elevation), round(sun_azimuth), 1., -26.7); // phase=100%, magnitude=26.7 hard-coded
      }

    } else  // outer planets
    {
      get_object_position(3, jd, jd_frac);  // Mars
      lcd.setCursor(0, 1);
      lcd.print(F("Mar "));
      LCDPlanetData(round(altitudePlanet), round(azimuthPlanet), phase, magnitude);

      get_object_position(4, jd, jd_frac);  // Jupiter
      lcd.setCursor(0, 2);
      lcd.print(F("Jup "));
      LCDPlanetData(round(altitudePlanet), round(azimuthPlanet), phase, magnitude);

      get_object_position(5, jd, jd_frac);  // Saturn
      lcd.setCursor(0, 3);
      lcd.print(F("Sat "));
      LCDPlanetData(round(altitudePlanet), round(azimuthPlanet), phase, magnitude);

      if (full) get_object_position(6, jd, jd_frac);  // Uranus
      if (full) get_object_position(7, jd, jd_frac);  // Neptune
    }
  }
}

/*****
Purpose: Menu item
Shows local time in 4 different calendars: Gregorian (Western), Julian (Eastern), Islamic, Hebrew

Argument List: none

Return value: Displays on LCD

Issues: Hebrew calendar is quite slow (3+ seconds) on an Arduino Mega
*****/

const char IslamicMonth[12][10] PROGMEM{ "Muharram ", "Safar    ", "Rabi I   ", "Rabi II  ", "Jumada I ", "Jumada II", "Rajab    ", "Sha'ban  ", "Ramadan  ", "Shawwal  ", "DhuAlQada", "DhuAlHija" };              // left justified
const char HebrewMonth[13][10]  PROGMEM{ "Nisan    ", "Iyyar    ", "Sivan    ", "Tammuz   ", "Av       ", "Elul     ", "Tishri   ", "Heshvan  ", "Kislev   ", "Teveth   ", "Shevat   ", "Adar     ", "Adar II  " };  // left justified
int mIsl = 0;  // pointer to month name
int mHeb = 0;   // pointer to month name

void ISOHebIslam() {  // ISOdate, Hebrew, Islamic

loadArrowCharacters();

#ifdef FEATURE_DATE_PER_SECOND                                 // for stepping date quickly and check calender function
  localTime = now() + utcOffset * 60 + dateIteration * 3600; //86400;  // fake local time by stepping up to 1 sec/day
  dateIteration = dateIteration + 1;
//  Serial.print(dateIteration); Serial.print(": ");
//  Serial.println(localTime);
#endif

  // algorithms in Nachum Dershowitz and Edward M. Reingold, Calendrical Calculations,
  // Software-Practice and Experience 20 (1990), 899-928
  // code from https://reingold.co/calendar.C

  lcd.setCursor(0, 0);  // top line *********
  // all dates are in local time
  GregorianDate a(month(localTime), day(localTime), year(localTime));
  LcdDate(a.GetDay(), a.GetMonth(), a.GetYear());
  ////    Serial.print("Absolute date ");Serial.println(a);
    
  
  lcd.setCursor(0, 1);
  JulianDate Jul(a);
  LcdDate(Jul.GetDay(), Jul.GetMonth(), Jul.GetYear());
  if (now() % 10 < 5) {
    lcd.print(F(" Julian   "));
  } 
  else {
    lcd.setCursor(0, 1);
    int ByzYear = Jul.GetYear() + 5508;              // Byzantine year = Annus Mundi rel to Sept 1, 5509 BC
    if (Jul.GetMonth() >= 9) ByzYear = ByzYear + 1;  // used by the Eastern Orthodox Church from
    LcdDate(Jul.GetDay(), Jul.GetMonth(), ByzYear);  // c. 691 to 1728 https://en.wikipedia.org/wiki/Byzantine_calendar
    lcd.print(F(" Byzantine"));
  }
    
  float Phase, PercentPhase; //  days since last new moon
  MoonPhase(Phase, PercentPhase); //UpdateMoonPosition();

  // alternate between clock and week #
  // alternate between month name and moon info for Islamic & Hebrew calendar  
   if (now() % 20 < 10) {

    localTime = now() + utcOffset * 60; // added 3.2.2024 - less latency in time calculation as routine is entered and at full minute
    lcd.setCursor(11,0);
    sprintf(textBuffer, "%02d%c%02d%c%02d ", hour(localTime), dateTimeFormat[dateFormat].hourSep, minute(localTime), dateTimeFormat[dateFormat].minSep, second(localTime));
    lcd.print(textBuffer);

    lcd.setCursor(11, 2);
    lcd.print(F("Moon:    ")); 
    lcd.setCursor(11,3);  lcd.print(F(" "));    
    MoonWaxWane(Phase);  // arrow
    MoonSymbol(Phase);   // (,0,)
    lcd.print(F(" ")); PrintFixedWidth(lcd, (int)(abs(round(PercentPhase))), 3);
    lcd.print(F("% "));
  }
  else
  {
    lcd.setCursor(10, 0); 
    IsoDate ISO(a);
    // if      (strcmp(languages[languageNumber], "no")==0) lcd.print(F(" Uke    "));
    // else if (strcmp(languages[languageNumber], "dk")==0) lcd.print(F(" Uge    "));
    // else if (strcmp(languages[languageNumber], "ny")==0) lcd.print(F(" Veke   "));
    // else if (strcmp(languages[languageNumber], "se")==0) lcd.print(F(" Vecka  "));
    // else if (strcmp(languages[languageNumber], "is")==0) lcd.print(F(" Vika   "));
    // else                                                 
    lcd.print(F(" Week   "));
    lcd.print(ISO.GetWeek());
    
    lcd.setCursor(11,2);
    if (mIsl > 0) lcd.print(reinterpret_cast<const __FlashStringHelper *>(IslamicMonth[mIsl - 1]));
    else          lcd.print(F("         "));
    lcd.setCursor(11,3);
    if (mHeb > 0) lcd.print(reinterpret_cast<const __FlashStringHelper *>(HebrewMonth[mHeb - 1]));
    else          lcd.print(F("         "));
  }
  
  lcd.setCursor(0, 2);
  IslamicDate Isl(a);
  mIsl = Isl.GetMonth();
  LcdDate(Isl.GetDay(), mIsl, Isl.GetYear());  

  if (minuteGPS != oldMinute) {  // update rest of display initially and then every minute
    // Hebrew calendar is complicated and *** very *** slow - takes ~3 sec. Therefore it is on the last line and only done occasionally
      //           Serial.print(now());Serial.println(" Hebrew ...");
      //lcd.setCursor(0, 3); lcd.print(F("..."));
      HebrewDate Heb(a);
      mHeb = Heb.GetMonth();
      lcd.setCursor(0, 3); LcdDate(Heb.GetDay(), mHeb, Heb.GetYear());
  }
  oldMinute = minuteGPS;
}

/*****
Purpose: 
Display all kinds of GPS-related info

Needs GPSParse()

Argument List: none

Return value: none
*****/

void GPSInfo() {
  // builds on the example program SatelliteTracker from the TinyGPS++ library
  // https://www.arduino.cc/reference/en/libraries/tinygps/


  float hdop;

#ifdef FEATURE_SERIAL_GPS
  Serial.println(F("*** Enter GPSInfo"));
#endif

  // GPSParse();

  if (totalGPGSVMessages.isValid()) {

    //  https://github.com/mikalhart/TinyGPSPlus/issues/52

    int totalMessages = atoi(totalGPGSVMessages.value());
    int currentMessage = atoi(messageNumber.value());

    if (totalMessages == currentMessage) {
#ifdef FEATURE_SERIAL_GPS
      Serial.print(F("Sats in use = "));
      Serial.print(gps.satellites.value());
      Serial.print(F(" Nums = "));

      for (int i = 0; i < MAX_SATELLITES; ++i) {
        if (sats[i].active) {
          Serial.print(i + 1);
          Serial.print(F(" "));
        }
      }
#endif

      // #ifdef FEATURE_SERIAL_GPS
      //   Serial.print(F(" SNR = "));
      // #endif

      // for (int i=0; i<MAX_SATELLITES; ++i)
      // {
      //   if (sats[i].active)
      //   {
      //     #ifdef FEATURE_SERIAL_GPS
      //         Serial.print(sats[i].snr);
      //         Serial.print(F(" "));
      //     #endif
      //     if (sats[i].snr >0)          // 0 when not tracking
      //     {
      //       total = total + 1;
      //       SNRAvg = SNRAvg + float(sats[i].snr);
      //     }
      //   }
      // }
      // // moved this } from end of routine to here on 19.11.2022:
    }  // if (totalMessages == currentMessage)

    lcd.setCursor(0, 0);
    lcd.print(F("In view "));
    //int noSats =  satsInView.value(); PrintFixedWidth(lcd, min(noSats,99), 2);
    if ((int)satsInView.value() < 10) lcd.print(" ");
    lcd.print(satsInView.value());
    lcd.print(F(" Sats "));

    noSats = gps.satellites.value();  // in list of http://arduiniana.org/libraries/tinygpsplus/
    lcd.setCursor(0, 1);
    lcd.print(F("In fix  "));  //printFixedWidth(lcd, noSats, 2);
    PrintFixedWidth(lcd, noSats, 2);
    //if (noSats<10) lcd.print(" ");
    //lcd.print(noSats);

   
    lcd.print(F(" SNR "));
    PrintFixedWidth(lcd, round(SNRAvg), 2);
    lcd.print(F(" dB"));
    //#endif
    //
    lcd.setCursor(0, 2);
    lcd.print(F("Mode    "));
    lcd.print(GPSMode.value());
    lcd.print(F("D Status  "));
    lcd.print(posStatus.value());

    hdop = gps.hdop.hdop();  // in list of http://arduiniana.org/libraries/tinygpsplus/
    lcd.setCursor(0, 3);
    lcd.print(F("Hdop  "));
    lcd.print(hdop);

    if (hdop < 1) lcd.print(F(" Ideal    "));  // 1-2 Excellent, 2-5 Good https://en.wikipedia.org/wiki/Dilution_of_precision_(navigation)
    else if (hdop < 2) lcd.print(F(" Excellent"));
    else if (hdop < 5) lcd.print(F(" Good     "));
    else lcd.print(F(" No good  "));

#ifdef FEATURE_SERIAL_GPS
    Serial.println();
    Serial.print(F("TotalSats="));
    Serial.print(totalSats);
    Serial.print(F(" InView="));
    Serial.print(satsInView.value());
    Serial.print(F(" In Fix="));
    Serial.print(noSats);

    Serial.print(F(" SNRAvg="));
    Serial.print((int)SNRAvg);
    Serial.print(F(" Mode="));
    Serial.print(GPSMode.value());  // 1-none, 2=2D, 3=3D
    Serial.print(F(" Status="));
    Serial.print(posStatus.value());  // A-valid, V-invalid
    Serial.println();
#endif

    for (int i = 0; i < MAX_SATELLITES; ++i)
      sats[i].active = false;
    //
    // removed 22.5.2023: made the display blink between valid values and 0 values as update doesn't happen every second
  }  // if (totalGPGSVMessages.isUpdated())

  //  else                              // new 19.11.2022, purpose? show loss of signal?
  //  {
  //          lcd.setCursor(0,0); lcd.print(F("In view  ")); lcd.print(0); lcd.print(F(" Sats"));
  //          lcd.setCursor(0,1); lcd.print(F("In fix   ")); lcd.print(0);
  //          lcd.print(F(" SNR  "));lcd.print(0); lcd.print(F(" dB"));
  //
  //          lcd.setCursor(0,2); lcd.print(F("Mode       Status   "));
  //          lcd.setCursor(0,3); lcd.print(F("Hdop                "));
  //  }
#ifdef FEATURE_SERIAL_GPS
  Serial.println(F("*** Exit  GPSInfo"));
#endif
}


///////////////////////////////////////////////////////////////////////////////////////
/*****
Purpose: Menu item
Finds local time as Big Number clock with digits over 3x2 squares 
https://github.com/upiir/character_display_big_digits

Argument List: None

Return value: Displays on LCD
*****/

void BigNumbers3(byte showUTC) {
  // big numbers 3 characters wide, 2 characters high
  byte imax;
  byte lineno = 0;  // 0,1,2 first (=upper) line for big digits

  loadThreeWideDigits();  // load 8 user-defined characterS if not loaded

  if (showUTC == 1) {
    Hour = hour(now());
    Minute = minute(now());
    Seconds = second(now());

    Day = day(now());
    Month = month(now());
    Year = year(now());
  } else {
    // get local time
    localTime = now() + utcOffset * 60;
    Hour = hour(localTime);
    Minute = minute(localTime);
    Seconds = second(localTime);

    Day = day(localTime);
    Month = month(localTime);
    Year = year(localTime);
  }

  imax = Hour / 10;
  draw_digit(imax, 0, lineno);  // draw 10's hour digit

  imax = Hour % 10;
  draw_digit(imax, 4, lineno);  // draw hour digit

  // : (colon)
  lcd.setCursor(8, lineno);
  lcd.write(DOT);  // centered dot
  lcd.setCursor(8, lineno + 1);
  lcd.write(DOT);


  imax = Minute / 10;
  draw_digit(imax, 10, lineno);  // draw 10's minute digit

  imax = Minute % 10;
  draw_digit(imax, 14, lineno);  // draw minute digit

  lcd.setCursor(18, 1);
  PrintFixedWidth(lcd, Seconds, 2, '0');

  if (lineno != 2) {
    lcd.setCursor(18, 3);
    lcd.print(F("  "));
  } else {
    lcd.setCursor(19, 3);
    lcd.print(F(" "));
  }

  lcd.setCursor(16, 3);
  if (showUTC == 1) lcd.print(F("UTC"));
  else lcd.print(tcr->abbrev);  // print time zone name

  lcd.setCursor(0, 3);
  LcdDate(Day, Month, Year);
}

/*
Purpose: Menu item
Finds local time as Big Number clock with digits over 2x3 squares 
https://github.com/upiir/character_display_big_digits

Argument List: None

Return value: Displays on LCD
*****/

void BigNumbers2(byte showUTC) {
  // Big numbers 2 characters wide, 3 high
  byte imax;
  byte lineno = 0;  // 0,1 first (=upper) line for big digits
  byte start = 0;   // start row for characters: left-justified: 0, ~centered: 1,2

  loadThreeHighDigits2();  // load 8 user-defined characterS if not already loaded
  if (showUTC == 1) {
    Hour = hour(now());
    Minute = minute(now());
    Seconds = second(now());

    Day = day(now());
    Month = month(now());
    Year = year(now());
  } else {
    // get local time
    localTime = now() + utcOffset * 60;
    Hour = hour(localTime);
    Minute = minute(localTime);
    Seconds = second(localTime);

    Day = day(localTime);
    Month = month(localTime);
    Year = year(localTime);
  }

  imax = Hour / 10;
  // draw 10's hour digit
  doNumber2(imax, lineno, start);

  imax = Hour % 10;
  // draw hour digit
  doNumber2(imax, lineno, start + 3);

  doNumber2(11, lineno, start + 5);  // colon

  imax = Minute / 10;
  // draw 10's minute digit
  doNumber2(imax, lineno, start + 6);

  imax = Minute % 10;
  // draw minute digit
  doNumber2(imax, lineno, start + 9);

  doNumber2(11, lineno, start + 11);  // colon

  imax = Seconds / 10;
  // draw 10's second digit
  doNumber2(imax, lineno, start + 12);

  imax = Seconds % 10;
  // draw second digit
  doNumber2(imax, lineno, start + 15);

  if (lineno != 2) {
    lcd.setCursor(18, 3);
    lcd.print(F("  "));
  } else {
    lcd.setCursor(19, 3);
    lcd.print(F(" "));
  }

  lcd.setCursor(16, 3);
  if (showUTC == 1) lcd.print(F("UTC"));
  else lcd.print(tcr->abbrev);  // print time zone name

  lcd.setCursor(0, 3);
  LcdDate(Day, Month, Year);
}


// Menu item //////////////////////////////////////////////////////////////////////////////////////////
/*****
Purpose: 
Display memorable dates in sorted order, and elapsed time since those dates. First displaye event is the next one in time

Argument List: none. Input is from clock_custom_routines

Return value: none

Issues: Follows UNIX time which is referenced to UTC, so changes birthday order according to UTC and not local time
*****/

void Reminder()  //
{
  int32_t diffSec;
  time_t Tsec;
  tmElements_t T;

  byte noOfReminderScreens = 5;      // e.g. 2 means that 4 names + 4 names will be shown, 5 is OK, i.e. all will be shown
  byte holdTimeReminderScreen = 10;  // or dwellTimeDemo?
  int ind;
  char yearSymbol;

  // check data read from EEPROM into variables lengthPersonData, person in setup()
  //readPersonEEPROM();   // read data into variables lengthPersonData, person (already done in setup())

  if (lengthPersonData <= 0 || lengthPersonData > MAX_NO_OF_PERSONS) {  // no data in EEPROM
    lcd.setCursor(0, 0);
    lcd.print(F("Reminder()"));
    lcd.setCursor(0, 1);
    lcd.print(F("EEPROM empty: "));
    lcd.print(EEPROM_OFFSET2);
    return;
  }

  float timeToBirthday[lengthPersonData + 2];
  float diffYearsF[lengthPersonData + 2];
  int indexArray[lengthPersonData + 2];
  float Age1970[lengthPersonData + 2];

  if (strcmp(languages[languageNumber], "no") == 0 || strcmp(languages[languageNumber], "ny") == 0)
    { 
      loadAring();  // Å for Norwegian 
      yearSymbol = char(SCAND_aa_SMALL);  // Scandinavian å
    }
  else yearSymbol = 'y';                  // 'English for 'year'

  // read from EEPROM already: lengthPersonData, person.*
  for (ind = 0; ind < lengthPersonData; ind++) {
          Serial.print(ind); Serial.print(": ");Serial.print(person[ind].Year);Serial.print(" ");Serial.print(person[ind].Month);
          Serial.print(" ");Serial.print(person[ind].Day);Serial.print(" ");Serial.println(person[ind].Name);
    Age1970[ind] = max(0.0, 1970.0 - person[ind].Year);  // non-zero if older than 1970
    T.Year = max(person[ind].Year - 1970, 0);            // zero if older than 1970, else difference
    T.Month = person[ind].Month;
    T.Day = person[ind].Day;
    T.Hour = 23;
    T.Minute = 59;
    T.Second = 59;  // Not 0,0,0 in order to ensure that a birhday today is shown first

    Tsec = makeTime(T);                    // seconds since 1/1/1970
    diffSec = now() - Tsec;                // difference in seconds to now
    diffSec = diffSec + utcOffset * 60.0;  // compensate for utcOffset in minutes: local = now() + utcOffset * 60;

    diffYearsF[ind] = diffSec / 31557600.0 + Age1970[ind];               // [no of sec / year = 31,557,600]
    timeToBirthday[ind] = 1 - (diffYearsF[ind] - int(diffYearsF[ind]));  // fraction of year

     //     Serial.print(" ");Serial.print(ind);Serial.print(" ");Serial.print(person[ind].Name);
     //     Serial.print(", "); Serial.print(diffYearsF[ind]); Serial.print(" "); Serial.println(timeToBirthday[ind],4);
    //      float oneDay = 1/365.; // = 0.00274
    //      if (timeToBirthday[ind] > 1.0 - oneDay) // within a day or so of birthday today - trick to show a birthday first even when on the same day
    //      {
    //        timeToBirthday[ind] = 0.0;  // for same day birthday
    //        Serial.print(" ---> ");Serial.println(timeToBirthday[ind]);
    //      }
    indexArray[ind] = ind;
  }

  bubbleSort(timeToBirthday, indexArray, lengthPersonData);  // sort: find indices according to first, second, etc birthday relative to today's date

  if (minuteGPS == oldMinute) {
  // display on LCD:
  if (secondGPS % holdTimeReminderScreen == 0)  // new display every holdTimeReminderScreen seconds
  {
    //      Serial.print("secondGPS "); Serial.println(secondGPS);
    //      Serial.print("lengthPersonData "); Serial.println(lengthPersonData);
    indStart = indStart + 4;
    //       Serial.print("(indStart + 1)/4 "); Serial.println((indStart + 1)/4);
    if (indStart >= lengthPersonData || (indStart + 1) / 4 >= noOfReminderScreens) indStart = 0;  // over the top >= or >?
                                                                                                  //       indStart = min(indStart, lengthPersonData-4); // alternative:fill up so the last screen always is filled
  }
    //   Serial.print("     indStart: ");Serial.println(indStart);
  }
  else
  {
    indStart = 0; // first use of Reminder() always outputs first rows of dates
  }

  for (ind = indStart; ind < indStart + 4; ind++) {
    lcd.setCursor(0, ind - indStart);
    if (ind < lengthPersonData) {
      lcd.print(person[indexArray[ind]].Name);
      lcd.print(" ");  // Name
      lcd.setCursor(9, ind - indStart);
      if (secondGPS % holdTimeReminderScreen < holdTimeReminderScreen/2)
        { 
          LcdDate(person[indexArray[ind]].Day, person[indexArray[ind]].Month, 0);  // Birth date: Day, Month
          lcd.print(F("  "));
          lcd.setCursor(15, ind - indStart);
          dtostrf(diffYearsF[indexArray[ind]], 4, 1, textBuffer);  
          // sprintf(today,"%%%d.%df", 4, 1);  // today = temporary storage for format string
          // sprintf(textBuffer, today, diffYearsF[indexArray[ind]]);  // replacement for dtostrf - no good!
          lcd.print(textBuffer); lcd.print(yearSymbol);             // Age in decimal years
        }
        else // 06.03.2024 
        {       
          LcdDate(person[indexArray[ind]].Day, person[indexArray[ind]].Month, person[indexArray[ind]].Year);  // Birth date: Day, Month, year
          lcd.print(" ");         
        }
    } else {
      lcd.print(F("                    "));  // blank line when there is no more data to display
    }
  }
  oldMinute = minuteGPS;
}

////////////////////////////////////////////////////////////////////////////////////////////////
/*****
Purpose: Menu item
Finds exact time for equinoxes, solstices (4 per year)
Verified against https://greenwichmeantime.com/equinox/ OK within 3-6 minutes in 2024
Can only give time in UTC as local UTC offset varies over the year, and software only has today's value

Argument List: none

Return value: Displays on LCD
*****/

int displayYear;

void Equinoxes() {

int year1 = year(now());
int year2 = year1 + 2;
if (oldMinute == -1 | displayYear < year1) displayYear = year1;

if (now() % 10 == 0) {      // increment every x seconds
  displayYear = displayYear+1;
  if (displayYear > year2) displayYear = year1;
}

EquinoxSolstice(displayYear);

  // float yy = (displayYear-2000.)/1000.0;  // e.g. 0.024 for 2024
  time_t tt;

  tt = springEquinox*86400;
  lcd.setCursor(0,0); lcd.print(displayYear); lcd.print(F(" UTC "));
  lcd.setCursor(9,0); 
  LcdDate(day(tt), month(tt));
  sprintf(textBuffer, " %02d%c%02d", hour(tt), dateTimeFormat[dateFormat].hourSep, minute(tt));
  lcd.print(textBuffer);  //lcd.print(F(" Equinox")); 

  tt = summerSolstice*86400;
  lcd.setCursor(9,1); //lcd.cursor();
  LcdDate(day(tt), month(tt));

  sprintf(textBuffer, " %02d%c%02d", hour(tt), dateTimeFormat[dateFormat].hourSep, minute(tt));
  lcd.print(textBuffer); //lcd.print(F(" Solstice"));
    
  tt = autumnEquinox*86400;
  lcd.setCursor(0,2); lcd.print(F("Equinox "));
  lcd.setCursor(9,2);
  LcdDate(day(tt), month(tt));
  sprintf(textBuffer, " %02d%c%02d", hour(tt), dateTimeFormat[dateFormat].hourSep, minute(tt));
  lcd.print(textBuffer); 

  tt = winterSolstice*86400;
  lcd.setCursor(0,3); lcd.print(F("Solstice "));
  lcd.setCursor(9,3);
  LcdDate(day(tt), month(tt));
  sprintf(textBuffer, " %02d%c%02d", hour(tt), dateTimeFormat[dateFormat].hourSep, minute(tt));
  lcd.print(textBuffer); 

  oldMinute = minuteGPS;
}


/*****
Purpose: Menu item for showing upcoming solar eclipses events in sorted order

Limitations:  No calculation, only based on a table which initially goes to 2030

Argument List: none

Return value: Displays on LCD
*****/

// Solar eclipses from https://www.timeanddate.com/eclipse/list.html
/*
 8. apr 2024 Total Solar Eclipse
 2. okt 2024 Annular Solar Eclipse
29. mar 2025 Partial Solar Eclipse
17. feb 2026 Annular Solar Eclipse
12. aug 2026 Total Solar Eclipse
 6. feb 2027 Annular Solar Eclipse
 2. aug 2027 Total Solar Eclipse
26. jan 2028 Annular Solar Eclipse
22. jul 2028 Total Solar Eclipse 
 1. jun 2030 Annular Solar Eclipse
25. nov 2030 Total Solar Eclipse
14. nov 2031 Total Solar Eclipse
30. mar 2033 Total Solar Eclipse
20. mar 2034 Total Solar Eclipse
*/

struct dateEvent
{
  int year;
  int monthDate;
};

dateEvent solarEclipse[] = {  // don't enter number with 0 i front -> octal interpretation
  {2024,  408}, {2024, 1002}, {2025, 329}, {2026, 217}, {2026, 812}, 
  {2027,  206}, {2027,  802}, {2028, 126}, {2028, 722}, {2030, 601}, 
  {2030, 1125}, {2031, 1114}, {2033, 330}, {2034, 320}};
// maximum number of solar eclipses (partial, annular, or total) is 5 per year.

void SolarEclipse() {

  int pday, pmonth, yy;
  byte noSolarEclipses;
  int i;

  lcd.setCursor(0, 0);
  lcd.print(F("Solar Eclipses "));

  yy = year(now());
  noSolarEclipses = sizeof(solarEclipse) / sizeof(solarEclipse[0]);
  int lineNo = 1;

  lcd.setCursor(2, lineNo); 
  for (i = 0; i < noSolarEclipses; i++)  // find first year with solar eclipse
  {
    if (solarEclipse[i].year >= yy){
      lcd.print(solarEclipse[i].year),lcd.print(":");
      yy = solarEclipse[i].year;
      break;
   }
  }
  int col = 8;
  for (i = 0; i < noSolarEclipses; i++)
  {
    if (solarEclipse[i].year == yy)  // find all eclipses in this year
    {
      if (col > 14) {
          col = 2;
          lineNo = lineNo + 1;  // start another line if more than 3 eclipses this year (first time in 2028!)
        }
      lcd.setCursor(col, lineNo);
      
      pmonth = solarEclipse[i].monthDate / 100;
      pday = solarEclipse[i].monthDate - 100 * pmonth;
      LcdDate(pday, pmonth);
      lcd.print(" ");
      col = col + 6;
    }
  }
  
  // 2. line (or 3.)
  yy = yy + 1;  // increment year 
  lineNo = lineNo + 1;
  lcd.setCursor(2, lineNo); 
  for (i = 0; i < noSolarEclipses; i++)
  {
    if (solarEclipse[i].year >= yy){
      lcd.print(solarEclipse[i].year),lcd.print(":");
      yy = solarEclipse[i].year;
      break;
   }
  }
  col = 8;
  for (i = 0; i < noSolarEclipses; i++)
  {
    if (solarEclipse[i].year == yy) 
    {
      if (col > 14) {
          col = 2;
          lineNo = lineNo + 1;  // start another line if more than 3 eclipses this year 
        }
      lcd.setCursor(col, lineNo);
      pmonth = solarEclipse[i].monthDate / 100;
      pday = solarEclipse[i].monthDate - 100 * pmonth;
      LcdDate(pday, pmonth);
      lcd.print(" ");
      col = col + 6;
    }
  }

// last line (if room)
if (lineNo < 3) {
  yy = yy + 1;  // increment year 
  lineNo = lineNo + 1;
  lcd.setCursor(2, lineNo); 
  for (i = 0; i < noSolarEclipses; i++)
  {
    if (solarEclipse[i].year >= yy){
      lcd.print(solarEclipse[i].year),lcd.print(":");
      yy = solarEclipse[i].year;
      break;
   }
  }
  col = 8;
  for (i = 0; i < noSolarEclipses; i++)
  {
    if (solarEclipse[i].year == yy) 
    {
      if (col > 14) {
          col = 2;
          lineNo = lineNo + 1;  // start another line if more than 3 eclipses this year 
        }
      lcd.setCursor(col, lineNo);
      pmonth = solarEclipse[i].monthDate / 100;
      pday = solarEclipse[i].monthDate - 100 * pmonth;
      LcdDate(pday, pmonth);
      lcd.print(" ");
      col = col + 6;
      }
    }
  }
  if (col < 14)
  {
    lcd.setCursor(18,3); lcd.print(F("  ")); // blank out lower right number if needed
  }
}



/*****
Purpose: Menu item for showing upcoming events in sorted order

Limitations:  Eclipse and Equinox/Solstice in UTC. Makes little difference in Europe,
              may perhaps miss with up to a day in North America and elsewhere

Argument List: none

Return value: Displays on LCD
*****/

void NextEvents() {

time_t timeNow; 
timeNow = now();   // [for testing ... + 86400*270;] 15.11.2031
//timeNow = 1952517685; // 15.11.2031
int displayYear = year(timeNow);

int lengthData = 4;   // no of events to sort
float eventDate[lengthData + 2];  // in compact format: day + 100*month + 100000 for next year. Must be ... + 2 in length
int indices[lengthData + 2];      // for sorting, near end of function

// *** Solstice & Equinoxes ***

time_t equiSolTime;
eventDate[0] = 0;
EquinoxSolstice(displayYear); // first check present year

if (timeNow <= springEquinox*86400)        equiSolTime  = springEquinox*86400;
else if (timeNow <= summerSolstice*86400)  equiSolTime  = summerSolstice*86400;
else if (timeNow <= autumnEquinox*86400)   equiSolTime  = autumnEquinox*86400;
else if (timeNow <= winterSolstice*86400)  equiSolTime  = winterSolstice*86400;
else  // check next year
{
  EquinoxSolstice(displayYear + 1);
  equiSolTime  = springEquinox*86400;
  eventDate[0] = 10000; // marks next year
}

eventDate[0] = eventDate[0] + day(equiSolTime) + 100*month(equiSolTime); // compact date

#ifdef FEATURE_SERIAL_NEXTEVENTS
  Serial.print(day(timeNow));Serial.print(".");Serial.println(month(timeNow));
  Serial.print(day(equiSolTime));Serial.print(".");Serial.println(month(equiSolTime));
  Serial.println(" ");
#endif

// *** Lunar Eclipse ***
int pDate[10];  // max 5 per day: packed date = 100*month + day, i.e. 1209 = 9 December
int eYear[10];
int pToday; 
int pday = 0, pmonth = 0;
eventDate[1] = 0;

pToday = day(timeNow) +  100*month(timeNow);  // packed format
#ifdef FEATURE_SERIAL_NEXTEVENTS
  Serial.println(pToday);
#endif

MoonEclipse(displayYear, pDate, eYear);
for (byte i = 0; (i < 10 && pDate[i] != 0); i++) {  
    #ifdef FEATURE_SERIAL_NEXTEVENTS
      Serial.print(i); Serial.print(": ");Serial.println(pDate[i]);
    #endif
    if (eYear[i] == displayYear && pToday <= pDate[i]) { // only handles this year
      pmonth = pDate[i] / 100;
      pday = pDate[i] - 100 * pmonth;
    }
}

if (pmonth == 0){             // nothing found this year, so check next year
  MoonEclipse(displayYear+1, pDate, eYear);
  pmonth = pDate[0] / 100;
  pday = pDate[0] - 100 * pmonth;
  eventDate[1] = 10000;  // marks next year
}

eventDate[1] = eventDate[1] + pday + 100*pmonth;

// *** Easter ***
eventDate[2] = 0;
int K, E;
int PaschalFullMoon, EasterDate, EasterMonth;

// Gregorian (West):
K = -2;
E = -10;
ComputeEasterDate(displayYear, K, E, &PaschalFullMoon, &EasterDate, &EasterMonth);
if (EasterDate+100*EasterMonth < pToday) { // next year's Easter
  ComputeEasterDate(displayYear+1, K, E, &PaschalFullMoon, &EasterDate, &EasterMonth);
  eventDate[2] = 10000;
}

eventDate[2] = eventDate[2] + EasterDate + 100*EasterMonth;

// *** Solar Eclipse ***
eventDate[3] = 0;
byte noSolarEclipses = sizeof(solarEclipse) / sizeof(solarEclipse[0]);
int i;

for (i = 0; i < noSolarEclipses; i++)  // find first year with solar eclipse
{
  if      (solarEclipse[i].year == displayYear && solarEclipse[i].monthDate >= pToday) break;
  else if (solarEclipse[i].year >  displayYear) break;
}
int smonth, sday;

if (i <=noSolarEclipses)
{
  #ifdef FEATURE_SERIAL_NEXTEVENTS
    Serial.print("Solar "); Serial.print(i); Serial.print(" ");Serial.println(displayYear);
    Serial.print("      "); Serial.print(solarEclipse[i].year);Serial.print(" "); Serial.println(solarEclipse[i].monthDate);
  #endif

  eventDate[3] = solarEclipse[i].monthDate + (solarEclipse[i].year - displayYear)*10000;
  smonth = solarEclipse[i].monthDate / 100;
  sday = solarEclipse[i].monthDate - 100 * smonth;
}

// lcd.setCursor(0,0); lcd.print(F("Next event:"));
// lcd.setCursor(15,0); LcdDate(day(timeNow+ utcOffset * 60), month(timeNow+ utcOffset * 60));   // local

// *** Sort ***

for (i=0; i<lengthData; i++) { indices[i] = i; }  // initial order

#ifdef FEATURE_SERIAL_NEXTEVENTS
  Serial.print("Indices before "); Serial.print(indices[0]); Serial.print(" "); Serial.print(indices[1]); Serial.print(" ");Serial.print(indices[2]);Serial.print(" ");Serial.println(indices[3]);
  Serial.print("Data "); Serial.print(eventDate[0]); Serial.print(" "); Serial.print(eventDate[1]); Serial.print(" ");Serial.print(eventDate[2]);Serial.print(" ");Serial.println(eventDate[3]);
#endif

bubbleSort(eventDate, indices, lengthData);

#ifdef FEATURE_SERIAL_NEXTEVENTS
  Serial.print("Indices after  "); Serial.print(indices[0]); Serial.print(" "); Serial.print(indices[1]); Serial.print(" ");Serial.print(indices[2]);Serial.print(" ");Serial.println(indices[3]);
#endif

// display

for (i = 0; i < lengthData; i++)
  {
    switch (indices[i]) {
    case 0: // equinox/solstice
      lcd.setCursor(0,i); 
      switch (month(equiSolTime)) {
        case 3:  {lcd.print(F("Equinox      ")); break; }
        case 6:  {lcd.print(F("Solstice     ")); break; }
        case 9:  {lcd.print(F("Equinox      ")); break; }
        case 12: {lcd.print(F("Solstice     "));}
      }
      lcd.setCursor(14,i);
      if (eventDate[i] > 5000) lcd.print("+");  // mark for next year
      else                     lcd.print(" ");
      lcd.setCursor(15,i);     LcdDate(day(equiSolTime), month(equiSolTime));
      break;

    case 1: // Lunar eclipse
      lcd.setCursor(0,i); lcd.print(F("Lunar Eclipse"));
      lcd.setCursor(14,i);
      if (eventDate[i] > 15000)      lcd.print("*");  // mark for 2+ years into future
      else if (eventDate[i] > 5000)  lcd.print("+");  // mark for next year 
      else                     lcd.print(" ");
      lcd.setCursor(15,i);     LcdDate(pday, pmonth);
      break;

    case 2: // Easter
      lcd.setCursor(0,i); lcd.print(F("Easter       "));
      lcd.setCursor(14,i);

      if (eventDate[i] > 5000) lcd.print("+");  // mark for next year 
      else                     lcd.print(" ");

      lcd.setCursor(15,i);     LcdDate(EasterDate, EasterMonth);
      break;

    case 3: // Solar eclipse 
      lcd.setCursor(0,i); lcd.print(F("Solar Eclipse"));
      lcd.setCursor(14,i);
      if (eventDate[i] > 15000)      lcd.print("*");  // mark for 2+ years into future
      else if (eventDate[i] > 5000)  lcd.print("+");  // mark for next year 
      else                           lcd.print(" ");
      lcd.setCursor(15,i);           LcdDate(sday, smonth);
      break;
    }
  }

} // NextEvents()




//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
////////////////////////////////////////////////////////////////////////////////////////////////////
// End of functions for Menu system ////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/// THE END ///
