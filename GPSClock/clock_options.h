// *** User options *** 
// The following #defines should normally all be off: 
//
//#define GNSS  // = Global navigation satellite system (GNSS) = GPS + GLONASS + Beidou + Galileo. Not just GPS - Affects GPSInfo()

// #define NEXTVERSION   // next version experimental feature (if there are any ...)
//#define TESTSCREENS    // extra screen set for testing recent functions. No need to use it for a normal user

//#define MORELANGUAGES // More than the default set of languages

//#define UTC_ENGLISH_DAY_NAME  // use English for day name for UTC display. Only affects ScreenUTCLocator. Default: local language 

//#define OLD_LCD_LIBRARY   // old LiquidCrystal_I2C - shouldn't be any need to use it

//#define EXP_TIDE_SIDEREAL  // Turn on/off experimental (unfinished) options - 

#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE // can be set in clock_hardware.h
   #define MORELANGUAGES  // More than the default set of languages
#endif

// 1. Software options
// 1A. subsets of menus
// 1B. date format
// 1C. language
// 1D. time zones

// 2. Minor parameters

/// ***************************************************************

// ************ 1. Software options

// *** 1A. Subsets of menus
// *** Customize menu system. ie. the order in which screen items are presented ***
// *** and set up set of such menu systems 

// Perturbe the order of screens, making sure that each number *only appears once* per group:

// All = all except the obsolete:
//   ScreenLocalSun,      superseded by ScreenLocalSunSimpler
//   ScreenLocalSunAzEl,  superseded by ScreenLocalSunSimpler
   
// 9 letter description + numbers:   
Menu_type menuStruct[] =
{
  {"All      ",  
      ScreenLocalUTCWeek, ScreenUTCLocator, ScreenLocalSunSimpler, ScreenLocalSunMoon, ScreenLocalMoon, 
      ScreenMoonRiseSet, ScreenPlanetsInner, ScreenPlanetsOuter, ScreenISOHebIslam, 
      ScreenNextEvents, ScreenEquinoxes, ScreenSolarEclipse, ScreenLunarEclipse, ScreenEasterDates,
      ScreenTimeZones, ScreenUTCPosition, ScreenLocalUTC, 
      // clocks:
      ScreenBinary, ScreenBinaryHorBCD, ScreenBinaryVertBCD, ScreenBar, ScreenMengenLehrUhr, ScreenLinearUhr, 
      ScreenHex, ScreenOctal, ScreenHexOctalClock, ScreenMathClockAdd, ScreenMathClockSubtract, 
      ScreenMathClockMultiply, ScreenMathClockDivide, ScreenRoman, ScreenMorse, ScreenWordClock, 
      ScreenChemical, 
      // status
      ScreenCodeStatus, ScreenInternalTime, 
      // radio amateur
      ScreenNCDXFBeacons1, ScreenNCDXFBeacons2, ScreenWSPRsequence, 
      ScreenSidereal, ScreenGPSInfo, ScreenBigNumbers2, ScreenBigNumbers2UTC, 
      ScreenBigNumbers3, ScreenBigNumbers3UTC, 
      #ifndef ARDUINO_SAMD_VARIANT_COMPLIANCE
         ScreenReminder,
      #endif 
      ScreenProgress, ScreenLocalMonth, ScreenFactorization, ScreenDemoClock, 
      -1}, 
  {"Fav 1    ", 
      ScreenLocalUTCWeek, ScreenUTCLocator, ScreenLocalSunSimpler, ScreenLocalSunMoon, ScreenLocalMoon,  
      ScreenPlanetsInner, ScreenPlanetsOuter, ScreenISOHebIslam, ScreenNextEvents, ScreenEquinoxes, 
      ScreenSolarEclipse, ScreenLunarEclipse, ScreenEasterDates, ScreenTimeZones, ScreenUTCPosition, 
      ScreenLocalUTC, 
      // clocks:
      ScreenRoman, ScreenWordClock, ScreenChemical, ScreenCodeStatus, 
      // radio amateur
      ScreenNCDXFBeacons1, ScreenNCDXFBeacons2, ScreenWSPRsequence, 
      ScreenSidereal, ScreenGPSInfo, ScreenBigNumbers2, ScreenBigNumbers2UTC, ScreenBigNumbers3, ScreenBigNumbers3UTC, 
      #ifndef ARDUINO_SAMD_VARIANT_COMPLIANCE
         ScreenReminder,
      #endif   
      ScreenProgress, ScreenLocalMonth, ScreenFactorization, ScreenDemoClock, 
      -1}, // Must end with negative number in order to enable counting of number of entries},
  {"Fav 2    ", 
      ScreenLocalUTCWeek, ScreenLocalSunSimpler, ScreenLocalMoon, ScreenPlanetsInner, ScreenPlanetsOuter, 
      ScreenISOHebIslam, ScreenNextEvents, ScreenProgress, ScreenDemoClock,
      -1}, 
  {"Calendar ", 
      ScreenLocalUTCWeek, ScreenUTCLocator, ScreenLunarEclipse, ScreenEasterDates, ScreenISOHebIslam,   
      ScreenTimeZones,  ScreenUTCPosition,  ScreenCodeStatus, ScreenSidereal, ScreenGPSInfo, 
  //    ScreenReminder, ScreenEquinoxes, ScreenNextEvents, ScreenDemoClock, 
      #ifndef ARDUINO_SAMD_VARIANT_COMPLIANCE
         ScreenReminder,
      #endif   
      ScreenProgress, ScreenDemoClock, 
      -1},
  {"Clocks   ",
      ScreenLocalUTCWeek, ScreenUTCLocator, ScreenBinary, ScreenBinaryHorBCD, ScreenBinaryVertBCD, 
      ScreenBar, ScreenMengenLehrUhr, ScreenLinearUhr, ScreenHex, ScreenOctal, 
      ScreenHexOctalClock,  ScreenMathClockAdd, ScreenMathClockSubtract, ScreenMathClockMultiply, ScreenMathClockDivide, 
      ScreenInternalTime, ScreenCodeStatus, ScreenRoman, ScreenMorse, ScreenWordClock, 
      ScreenChemical, ScreenBigNumbers2, ScreenBigNumbers2UTC, ScreenBigNumbers3, ScreenBigNumbers3UTC, 
      ScreenProgress, ScreenDemoClock, 
      -1},
  {"Astro    ",
      ScreenLocalUTCWeek, ScreenUTCLocator, ScreenLocalSunSimpler, ScreenLocalSunMoon, ScreenLocalMoon, 
      ScreenMoonRiseSet,  ScreenLunarEclipse, ScreenEasterDates, ScreenPlanetsInner, ScreenPlanetsOuter, 
      ScreenISOHebIslam, ScreenCodeStatus, ScreenInternalTime, ScreenSidereal, ScreenGPSInfo, 
   //   ScreenEquinoxes, ScreenNextEvents, ScreenDemoClock, 
      ScreenEquinoxes, ScreenDemoClock, 
      -1},
  {"Radio    ", 
      ScreenLocalUTCWeek, ScreenUTCLocator, ScreenLocalSunSimpler, ScreenLocalMoon, ScreenUTCPosition, 
      ScreenMorse, ScreenCodeStatus, ScreenNCDXFBeacons1, ScreenNCDXFBeacons2, ScreenWSPRsequence, 
      ScreenGPSInfo, ScreenDemoClock, 
      -1}
#ifdef TESTSCREENS
  ,
  {"Test     ", 
      ScreenLocalUTCWeek, ScreenGPSInfo, ScreenFactorization, ScreenProgress, ScreenDemoClock,
      -1}
#endif 
};

// *** 1B. Date/time format
// **************************************************************************
// *** Date time format options selectable by rotary

Date_Time dateTimeFormat[]=

//  name,      L/M/B, dateSep, hourSep, minSep
{
   {"EU      ", 'L', '.',      ':',    ':'}, // 22.04 = 22.04.2016, 12:04:32
   {"US      ", 'M', '/',      ':',    ':'}, // 04/22 = 04/22/2016, 12:04:32
   {"ISO     ", 'B', '-',      ':',    ':'}, // 04-22 = 2016-04-22, 12:04:32
   {"French  ", 'L', '/',      'h',    ':'}, // 22/04/2016, 12h04:32
   {"British ", 'L', '/',      ':',    ':'}, // 22/04/2016, 12:04:32
   {"Period  ", 'L', '.',      '.',    '.'}, // 22.04.2016, 12.04.32
   {"Dot     ", 'L', (char)DOT,':',    ':'}  // 22*04*2016, 12:04:32
};

//Order of day/month/year in date: 
//    'L': Little-endian:  22.04.2016; 22.04 - EU
//    'M': Middle-endian.  04/22/2016; 04/22 - US
//    'B': Big-endian:     2016-04-22; 04-22 - ISO


// *** 1C. Language
// *************************************************************************
// Native language support for names of days of week 

// order here defines languageNumber: 0, 1, 2, 3, ...        
char languages[][12] = {"en ", "es ", "fr ", "de ", "nb ", "sv ", "da ", "is ", 
#ifdef MORELANGUAGES
  "fo ","non", // Faroese, Old Norse, https://en.wikipedia.org/wiki/Old_Norse
#endif
  "nn ", "nl ",
#ifdef MORELANGUAGES
   "ar "};  //  Arabic (lebanese)
#else
  }; 
#endif

// Order in myDays must match languages[][] above
const char myDays[][7][12] PROGMEM = {
            {"Sunday",     "Monday",    "Tuesday",     "Wednesday",  "Thursday",    "Friday",      "Saturday"},     // English, en. Max 9 chars, 0 special
            {"Domingo",    "Lunes",     "Martes",      "MiXrcoles",  "Jueve",       "Viernes",     "SXbado"},       // Spanish, es. Max 9 chars, 2 special
            {"dimanche",   "lundi",     "mardi",       "mercredi",   "jeudi",       "vendredi",    "samedi"},       // French, fr. Max 8 chars, 0 specila
            {"Sonntag",    "Montag",    "Dienstag",    "Mittwoch",   "Donnerstag",  "Freitag",     "Samstag"},      // German, de. Max 10 chars  , 0 special
            {"SXndag",     "Mandag",    "Tirsdag",     "Onsdag",     "Torsdag",     "Fredag",      "LXrdag"},       // Norsk Bokmål, nb. Max 7 chars, 1 special                 
            {"SXndag",     "MXndag",    "Tisdag",      "Onsdag",     "Torsdag",     "Fredag",      "LXrdag"},       // Swedish, sv. Max 7, 2 special  
            {"SXndag",     "Mandag",    "Tirsdag",     "Onsdag",     "Torsdag",     "Fredag",      "LXrdag"},       // Danish, da  = Norwegian. Max 7, 1 special 
            {"Sunnudagur", "MXnudagur", "XriXjudagur", "MiXvikudag", "Fimmtudagur", "FXstudagur",  "Laugardagur"},  // Icelandic, is. Max 11, 3 special
#ifdef MORELANGUAGES
            {"Sunnudagur", "Mxnadagur", "Txsdagur",    "Mikudagur",  "Hxsdagur",    "FrXggjadag.", "Leygardagur"},  // Faeroe Islands. fo. Max 12 (11), 3 special
            {"Sunnudagr",  "MXnudagr",  "Tysdagr",     "Oxinsdagr",  "Xxrsdagr",    "Frjxdagr",    "Laugardagr"},   // Old Norse, non. Max 10, 4 special
#endif
            {"SXndag",     "MXndag",    "Tysdag",      "Onsdag",     "Torsdag",     "Fredag",      "Laurdag"},      // Nynorsk (Norwegian), nn. Max 7, 2 special
            {"zondag",     "maandag",   "dinsdag",     "woensdag",   "donderdag",   "vrijdag",     "zaterdag"},     // Dutch, nl. Max 9, 0 special
#ifdef MORELANGUAGES
            {"aHad",       "itnein",    "talaata",     "arba3aa",    "khamis",      "jum3a",       "sabt"}};        // Lebanese (Levantine Arabic), ar. Max 7, 0 special
#else        
            };
#endif



// *** 1D. Time zones ****************************************************************************

// select time zone for display in Screen# "ScreenTimeZones". Points to array of time zones defined in clock_zone.h
int userTimeZones[4] = {16, 17, 3, 7};  // user selectable - point to time zone in clock_custom_routines.h 
                                        // make sure to put time zones with 4-letter designations in positions 1 or 3 if needed

#define AUTO_UTC_OFFSET
#ifndef AUTO_UTC_OFFSET
  utcOffset = 0;    // value in minutes (< +/- 720), value is only read if AUTO_UTC_OFFSET is not set 
                    // usually found automatically by means of Timezone library
#endif
// time zone definitions + daylight saving rules in clock_zone.h  


// ************ 2. Minor parameters for some of the displays

char MATH_CLOCK_MULTIPLY = 'x'; // '*', 'x', (char)165 = centered dot.  Multiplication sign
char MATH_CLOCK_DIVIDE   = ':'; // '/', ':'.                            Division sign

const float OPTION_DAYS_WITHOUT_MOON_ARROW = 2.0;  // at full and at new moon
const float OPTION_DAYS_WITHOUT_MOON_SYMBOL = 2.0; // at full and at new moon

const uint32_t menuTimeOut = 30000; // in msec, i.e. 30 sec time-out of menu system -> return to main clock function
