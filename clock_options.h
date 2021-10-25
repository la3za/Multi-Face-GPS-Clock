// Choose only one of these display interface options:
#define FEATURE_LCD_I2C               // serial interface to 20x4 LCD on 0x27 I2C address
//#define FEATURE_LCD_4BIT              // parallel interface to 20x4 LCD

// Choose:
/* Original version 1.0:  pot + buttons
   Version 1.1:
       Rotary alone:      rotate to choose screen, push according to FEATURE_PUSH_FAVORITE
       Rotary + buttons:  As rotary alone + buttons may also be used to change screen
       Rotary + pot:      Rotary chooses screen, push chooses favorite, pot adjusts background light
*/

//#define FEATURE_POTENTIOMETER // for background light adjustment with external pot
int backlightVal = 100; // (0...256) initial backlight value (fixed value if neither pot nor rotary encoder adjust backlight); 

//#define FEATURE_BUTTONS         // 
#define FEATURE_ROTARY_ENCODER  //

#define FEATURE_PUSH_FAVORITE   // push: go to favorite screen, otherwise set background light
const int menuFavorite = 3; // screen number for favorite


#ifdef FEATURE_POTENTIOMETER
    #define FEATURE_PUSH_FAVORITE
#endif

// Customize menu system: order in which menu items are presented
// Perturbe the order of menus, making sure that each number only appears once
//
// Menu in normal order - i.e. natural order in program
//int menuIn[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22}; 

// Logical order
int menuIn[] = {0,1,2,3,4,5,22,6,7,8,9,10,11,12,13,14,19,20,21,15,16,17,18}; 


//int menuIn[] = {0,3,4};

// Solar, moon
// int menuIn[] = {0, 1, 2, 3, 4, 5};

// Radio amateur functions WSPR, NCDXF beacon, sun, moon 
// int menuIn[] = {0, 1, 2, 3, 4, 5, 6, 15, 16, 17, 18};

// Fancy clocks
// int menuIn[] = {0,1,7,8,9,10,11,12,19};

const int noOfStates = sizeof(menuIn)/sizeof(menuIn[0]); // no of entries in menu system


// Day and clock separators & order of day/month in date: 
//    DATEORDER = 'L': Little-endian:  22.04.2016; 22.04 - EU
//    DATEORDER = 'M': Middle-endian.  04/22/2016; 04/22 - US
//    DATEORDER = 'B': Big-endian:     2016-04-22; 04-22 - ISO

// For EU, 22.04 = 22 April:
char DATEORDER = 'L'; 
char DATE_SEP = '.'; // Alternatives: '.', '/', '–', ' ', ...
char HOUR_SEP = ':'; // Alternatives: ':', '.', 'h', ...
char MIN_SEP  = ':'; // Alternatives: ':', '.', 'm', ... 

// For US, 04/22 = April, 22: 
//char DATEORDER = 'M'; 
//char DATE_SEP = '/'; 
//char HOUR_SEP = ':'; 
//char MIN_SEP  = ':';

// For ISO, 04-22 = April 22:
//char DATEORDER = 'B'; 
//char DATE_SEP = '-';
//char HOUR_SEP = 'h'; 
//char MIN_SEP  = '.';

//#define FEATURE_DAY_NAME_NATIVE    // DAY names from nativeDay() and nativeDayLong() for local time, otherwise English

const int SECONDS_CLOCK_HELP = 15;                 // no of seconds per minute where normal clock is also shown in binary, BCD etc clocks. 
                                                   // 0 - always off, 60 - always on
const float OPTION_DAYS_WITHOUT_MOON_ARROW = 2.0;  // at full and at new moon
const float OPTION_DAYS_WITHOUT_MOON_SYMBOL = 2.0; // at full and at new moon
