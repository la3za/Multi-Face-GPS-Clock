// User options
// 1. Hardware options

// 2. Software options
// 2A. subsets of menus
// 2B. date format
// 2C. language
// 2D. time zones

// 3. Minor parameters



// ************ 1. Hardware options:
// Choose only one of these display interface options:
#define FEATURE_LCD_I2C               // serial interface to 20x4 LCD on 0x27 I2C address
//#define FEATURE_LCD_4BIT              // parallel interface to 20x4 LCD

// Choose:
/* Version 1.0.x:  pot + buttons
   Version 1.x.y:
       Rotary alone:      rotate to choose screen, push according to FEATURE_PUSH_FAVORITE
       Rotary + buttons:  As rotary alone + buttons may also be used to change screen
       Rotary + pot:      Rotary chooses screen, push chooses favorite, pot adjusts background light
   Version 2.0.0:
       Rotary alone:      New menu system, also supports buttons for up/down, and pot for backlight
*/

//#define FEATURE_POTENTIOMETER // for background light adjustment with external pot

#define FEATURE_BUTTONS       // two push buttons increase/decrease screen number 
                              // in addition to rotary encoder with push button

// ************ 2. Software options

// *** 2A. Subsets of menus
// *** Customize menu system. ie. the order in which screen items are presented ***
// *** and set up set of such menu systems 

struct Menu_type
{
  char  descr[15];
  int   order[noOfEntries];
};

// Perturbe the order of screens, making sure that each number *only appears once* per group:

// All = all except the obsolete ScreenLocalSun=2, which is superseded by ScreenLocalSunSimpler=23

Menu_type menuStruct[] =
{
  {"Favorites    ", 
      ScreenLocalUTCWeek, ScreenUTCLocator, ScreenLocalSunSimpler, ScreenLocalSunAzEl, ScreenLocalSunMoon, 
      ScreenLocalMoon,  ScreenPlanetsInner, ScreenPlanetsOuter, ScreenLunarEclipse, ScreenEasterDates, 
      ScreenISOHebIslam, ScreenTimeZones, ScreenUTCPosition, ScreenLocalUTC, ScreenRoman, 
      ScreenWordClock, ScreenChemical, ScreenCodeStatus, ScreenNCDXFBeacons1, ScreenNCDXFBeacons2, 
      ScreenWSPRsequence, ScreenSidereal, ScreenGPSInfo, ScreenDemoClock, 
      -1}, // Must end with negative number in order to enable counting of number of entries},
  {"All          ",  // except ScreenLocalSun (2)
      ScreenLocalUTCWeek, ScreenUTCLocator, ScreenLocalSunSimpler, ScreenLocalSunAzEl, ScreenLocalSunMoon, 
      ScreenLocalMoon, ScreenMoonRiseSet, ScreenPlanetsInner, ScreenPlanetsOuter,  ScreenLunarEclipse, 
      ScreenEasterDates, ScreenISOHebIslam, ScreenTimeZones, ScreenUTCPosition, ScreenLocalUTC, 
      ScreenBinary, ScreenBinaryHorBCD, ScreenBinaryVertBCD, ScreenBar, ScreenMengenLehrUhr, 
      ScreenLinearUhr, ScreenHex, ScreenOctal, ScreenHexOctalClock, ScreenMathClockAdd, 
      ScreenMathClockSubtract, ScreenMathClockMultiply, ScreenMathClockDivide, ScreenRoman, ScreenMorse, 
      ScreenWordClock, ScreenChemical, ScreenCodeStatus, ScreenInternalTime, ScreenNCDXFBeacons1, 
      ScreenNCDXFBeacons2, ScreenWSPRsequence, ScreenSidereal, ScreenGPSInfo, ScreenDemoClock,
      -1},  
  {"Calendar     ", 
      ScreenLocalUTCWeek, ScreenUTCLocator, ScreenLunarEclipse, ScreenEasterDates, ScreenISOHebIslam,   
      ScreenTimeZones,  ScreenUTCPosition, ScreenLocalUTC, ScreenCodeStatus, ScreenInternalTime, 
      ScreenSidereal, ScreenGPSInfo, ScreenDemoClock, 
      -1},
  {"Fancy clocks ",
      ScreenLocalUTCWeek, ScreenUTCLocator, ScreenBinary, ScreenBinaryHorBCD, ScreenBinaryVertBCD, 
      ScreenBar, ScreenMengenLehrUhr, ScreenLinearUhr, ScreenHex, ScreenOctal, 
      ScreenHexOctalClock,  ScreenMathClockAdd, ScreenMathClockSubtract, ScreenMathClockMultiply, ScreenMathClockDivide, 
      ScreenRoman, ScreenMorse, ScreenWordClock, ScreenChemical, ScreenInternalTime,
      ScreenCodeStatus, ScreenDemoClock, 
      -1},
  {"Astronomy    ",
      ScreenLocalUTCWeek, ScreenUTCLocator, ScreenLocalSunSimpler, ScreenLocalSunAzEl, ScreenLocalSunMoon, 
      ScreenLocalMoon, ScreenMoonRiseSet,  ScreenLunarEclipse, ScreenEasterDates, ScreenPlanetsInner, 
      ScreenPlanetsOuter, ScreenCodeStatus, ScreenInternalTime, ScreenSidereal, ScreenGPSInfo, 
      ScreenDemoClock, 
      -1},
  {"Radio amateur", 
      ScreenLocalUTCWeek, ScreenUTCLocator, ScreenLocalSunSimpler, ScreenLocalMoon, ScreenUTCPosition, 
      ScreenMorse, ScreenCodeStatus, ScreenInternalTime, ScreenNCDXFBeacons1, ScreenNCDXFBeacons2, 
      ScreenWSPRsequence, ScreenGPSInfo, ScreenDemoClock, 
      -1}
/*
  {"Test         ", 
     ScreenLocalUTCWeek, ScreenLocalSunSimpler, ScreenWordClock, ScreenChemical, ScreenCodeStatus, 
     ScreenInternalTime, ScreenDemoClock, -1}   // for testing of newly developed screens 
*/
};

// *** 2B. Date/time format
// **************************************************************************
// *** Date time format options selectable by rotary


struct Date_Time
{
  char descr[10];
  char dateOrder;
  char dateSep;
  char hourSep;
  char minSep;
};

Date_Time dateTimeFormat[]=

//  name,      L/M/B, dateSep, hourSep, minSep
{
   {"EU      ", 'L', '.',      ':',    ':'}, // 22.04 = 22.04.2016, 12:04:32
   {"US      ", 'M', '/',      ':',    ':'}, // 04/22 = 04/22/2016, 12:04:32
   {"ISO     ", 'B', '-',      ':',    ':'}, // 04-22 = 2016-04-22, 12:04:32
   {"French  ", 'L', '/',      'h',    ':'}, // 22/04/2016, 12h04:32
   {"British ", 'L', '/',      ':',    ':'}, // 22/04/2016, 12:04:32
   {"Period  ", 'L', '.',      '.',    '.'}, // 22.04.2016, 12.04.32
   {"Dot     ", 'L', (char)165,':',    ':'}  // 22*04*2016, 12:04:32
};

//Order of day/month/year in date: 
//    'L': Little-endian:  22.04.2016; 22.04 - EU
//    'M': Middle-endian.  04/22/2016; 04/22 - US
//    'B': Big-endian:     2016-04-22; 04-22 - ISO


// *** 2C. Language
// *************************************************************************
// Set display language where DAY names come from nativeDay(), nativeDayLong() for local time (UTC is always in English) 
// NumberClock and ChemicalElements is in Norwegian for 'no', otherwise English

//;  // doesn't seem to be needed 

// first languages are 'en', 'fr', 'de' and then ...
#define FEATURE_LANGUAGE_GROUP 0        //'en', 'fr', 'de', 'no',       'es'           
//#define FEATURE_LANGUAGE_GROUP 1      //'en', 'fr', 'de', 'no', 'se'             
//#define FEATURE_LANGUAGE_GROUP 2      //'en', 'fr', 'de', 'dk', 'se', 'es'        
//#define FEATURE_LANGUAGE_GROUP 3      //'en', 'fr', 'de', 'dk',            'is'  

// 'no' and 'dk' have exactly the same day names, but 'no' also gives chemical names and word clock in Norwegian

// don't work: 
//#define FEATURE_LANGUAGE_GROUP2 dk_se_es // etc

// *** 2D. Time zones ****************************************************************************

// select time zone for display in Screen# "ScreenTimeZones". Points to array of time zones defined in clock_zone.h

int userTimeZones[4] = {16, 17, 3, 7};  // user selectable - point to time zone in clock_custom_routines.h 
                                        // make sure to put time zones with 4-letter designations in positions 1 or 3 if needed

#define AUTO_UTC_OFFSET
#ifndef AUTO_UTC_OFFSET
  utcOffset = 0;    // value in minutes (< +/- 720), value is only read if AUTO_UTC_OFFSET is not set 
                    // usually found automatically by means of Timezone library
#endif
// time zone definitions + daylight saving rules in clock_zone.h  


// ************ 3. Minor parameters for some of the displays

char MATH_CLOCK_MULTIPLY = 'x'; // '*', 'x', (char)165 = centered dot.  Multiplication sign
char MATH_CLOCK_DIVIDE   = ':'; // '/', ':'.                            Division sign

const float OPTION_DAYS_WITHOUT_MOON_ARROW = 2.0;  // at full and at new moon
const float OPTION_DAYS_WITHOUT_MOON_SYMBOL = 2.0; // at full and at new moon

const uint32_t menuTimeOut = 30000; // in msec, i.e. 30 sec time-out of menu system -> return to main clock function
