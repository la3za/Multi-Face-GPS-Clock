#define FEATURE_DAY_NAME_NATIVE    // DAY names from nativeDay() and nativeDayLong() for local time, otherwise English
#define FEATURE_CLOCK_SOME_SECONDS   // if defined, binary etc clocks will also show normal clock first 10 seconds of every minute

const float OPTION_DAYS_WITHOUT_MOON_ARROW = 2.0;  // at full and at new moon
const float OPTION_DAYS_WITHOUT_MOON_SYMBOL = 2.0; // at full and at new moon

// Choose only one of these display interface options:
#define FEATURE_LCD_I2C               // serial interface to 20x4 LCD on 0x27 I2C address
//#define FEATURE_LCD_4BIT              // parallel interface to 20x4 LCD (untested)


// Customize menu system: order in which menu items are presented
// Perturbe the order of menus, making sure that each number only appears once
//
// Menu in normal order - i.e. natural order in program
int menuIn[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18}; 


//int menuIn[] = {0,3,4};

// Solar, moon
//int menuIn[] = {0, 1, 2, 3, 4, 5};

// Radio amateur functions WSPR, NCDXF beacon, sun, moon 
// int menuIn[] = {0, 1, 2, 3, 4, 5, 6, 15, 16, 17, 18};

// Fancy clocks
// int menuIn[] = {0,1,7,8,9,10,11,12};

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
