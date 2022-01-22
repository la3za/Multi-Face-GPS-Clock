// Choose only one of these display interface options:
#define FEATURE_LCD_I2C               // serial interface to 20x4 LCD on 0x27 I2C address
//#define FEATURE_LCD_4BIT              // parallel interface to 20x4 LCD

// Choose:
/* Version 1.0.x:  pot + buttons
   Version 1.1.0:
       Rotary alone:      rotate to choose screen, push according to FEATURE_PUSH_FAVORITE
       Rotary + buttons:  As rotary alone + buttons may also be used to change screen
       Rotary + pot:      Rotary chooses screen, push chooses favorite, pot adjusts background light
*/

//#define FEATURE_POTENTIOMETER // for background light adjustment with external pot
int backlightVal = 150; // (0...256) initial backlight value (fixed value if neither pot nor rotary encoder adjust backlight); 

#define FEATURE_BUTTONS       // two push buttons increase/decrease screen number 
#define FEATURE_ROTARY_ENCODER  // enable rotary encoder with push button

#define FEATURE_PUSH_FAVORITE   // push: go to favorite screen, otherwise set background light
const int menuFavorite = ScreenLocalSunMoon; // screen number for favorite


#ifdef FEATURE_POTENTIOMETER
    #define FEATURE_PUSH_FAVORITE
#endif

// *** Customize menu system. ie. the order in which screen items are presented ***
// Perturbe the order of screens, making sure that each number *only appears once*
//


// Logical order, includes everything, (5 per line):

int menuIn[] = {ScreenLocalUTC, ScreenUTCLocator, ScreenLocalSun, ScreenLocalSunSimpler, ScreenLocalSunAzEl, 
                ScreenLocalSunMoon, ScreenLocalMoon, ScreenMoonRiseSet, ScreenLunarEclipse, ScreenEasterDates, 
                ScreenTimeZones, ScreenBinary, ScreenBinaryHorBCD, ScreenBinaryVertBCD, ScreenBar, 
                ScreenMengenLehrUhr, ScreenLinearUhr, ScreenInternalTime, Screencode_Status, ScreenHex, 
                ScreenOctal, ScreenHexOctalClock, ScreenMathClockAdd, ScreenMathClockSubtract, ScreenMathClockMultiply, 
                ScreenMathClockDivide,   ScreenNCDXFBeacons2, 
                ScreenNCDXFBeacons1, ScreenWSPRsequence, ScreenUTCPosition};


// My preferred subset, as above, but without binary clocks and without diagnostic displays.
/*int menuIn[] = {ScreenLocalUTC, ScreenUTCLocator,                 ScreenLocalSunSimpler, ScreenLocalSunAzEl, 
                ScreenLocalSunMoon, ScreenLocalMoon,ScreenMoonRiseSet, ScreenLunarEclipse, ScreenEasterDates, 
                ScreenTimeZones,                                                                   ScreenBar, 
                ScreenMengenLehrUhr, ScreenLinearUhr, ScreenInternalTime, Screencode_Status,
                             ScreenHexOctalClock, ScreenMathClockAdd, ScreenMathClockSubtract, ScreenMathClockMultiply, 
                ScreenMathClockDivide,  ScreenNCDXFBeacons2, 
                ScreenNCDXFBeacons1, ScreenWSPRsequence,  ScreenUTCPosition};
*/

// Solar, moon:
/*
int menuIn[] = {ScreenLocalUTC, ScreenUTCLocator, ScreenLocalSunSimpler, ScreenLocalSunAzEl, ScreenLocalSunMoon, 
                ScreenLocalMoon, ScreenMoonRiseSet, ScreenLunarEclipse};
*/

// Radio amateur functions WSPR, NCDXF beacons, sun, moon:
/*
int menuIn[] = {ScreenLocalUTC, ScreenUTCLocator, ScreenLocalSunSimpler, ScreenLocalSunAzEl, ScreenLocalSunMoon, 
                ScreenLocalMoon, ScreenMoonRiseSet, ScreenLunarEclipse, ScreenTimeZones, ScreenUTCPosition, 
                ScreenNCDXFBeacons2, ScreenNCDXFBeacons1, ScreenWSPRsequence};
*/

// Fancy clocks:
/*
int menuIn[] = {ScreenLocalUTC, ScreenUTCLocator, ScreenBinary, ScreenBinaryHorBCD, ScreenBinaryVertBCD, 
                ScreenBar, ScreenMengenLehrUhr, ScreenLinearUhr, ScreenHex, ScreenOctal, 
                ScreenHexOctalClock, ScreenRoman, ScreenMathClockAdd, ScreenMathClockSubtract, ScreenMathClockMultiply,
                ScreenMathClockDivide};
*/

const int noOfStates = sizeof(menuIn)/sizeof(menuIn[0]); // no of entries in Menu system
// *************************************************************************

// Day and clock separators & order of day/month in date: 
//    DATEORDER = 'L': Little-endian:  22.04.2016; 22.04 - EU
//    DATEORDER = 'M': Middle-endian.  04/22/2016; 04/22 - US
//    DATEORDER = 'B': Big-endian:     2016-04-22; 04-22 - ISO

// For EU, 22.04 = 22 April or 22.04.2016;:
char DATEORDER = 'L'; 
char DATE_SEP = '.'; // Alternatives: '.', '/', '–', ' ', ...
char HOUR_SEP = ':'; // Alternatives: ':', '.', 'h', ...
char MIN_SEP  = ':'; // Alternatives: ':', '.', 'm', ... 

// For US, 04/22 = April 22 or 04/22/2016: 
//char DATEORDER = 'M'; 
//char DATE_SEP = '/'; 
//char HOUR_SEP = ':'; 
//char MIN_SEP  = ':';

// For ISO, 04-22 = April 22 or 2016-04-22:
//char DATEORDER = 'B'; 
//char DATE_SEP = '-';
//char HOUR_SEP = 'h'; 
//char MIN_SEP  = '.';

//#define FEATURE_NATIVE_LANGUAGE    // DAY names from nativeDay(), nativeDayLong() for local time; Number clock in native language, otherwise English

const int SECONDS_CLOCK_HELP = 0;                 // no of seconds per minute where normal clock is also shown in binary, BCD etc clocks. 
                                                  // 0 - always off, 60 - always on

int  MATH_PER_MINUTE = 8; //15; // Number of math quizzes per minute, 1...6 in AlbertClock app
char MATH_CLOCK_MULTIPLY = 'x'; // '*', 'x', (char)165 = centered dot
char MATH_CLOCK_DIVIDE   = ':'; // '/', ':'

#define DWELL_TIME_DEMO 15 // seconds per mode as DemoClock cycles through all modes

const float OPTION_DAYS_WITHOUT_MOON_ARROW = 2.0;  // at full and at new moon
const float OPTION_DAYS_WITHOUT_MOON_SYMBOL = 2.0; // at full and at new moon
