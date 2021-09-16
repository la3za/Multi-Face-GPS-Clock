// Customize menu system: order in which menu items are presented
// Perturbe the order of menus, making sure that each number only appears once
//
// Menu in normal order - i.e. natural order in program
int menuIn[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18}; 

// Solar, moon
//int menuIn[] = {0, 1, 2, 3, 4, 5};

// Radio amateur functions WSPR, NCDXF beacon, sun, moon 
// int menuIn[] = {0, 1, 2, 3, 4, 5, 6, 15, 16, 17, 18};

// Fancy clocks
// int menuIn[] = {0,1,7,8,9,10,11,12};

int noOfStates = sizeof(menuIn)/sizeof(menuIn[0]); // no of entries in menu system

// if LCD_NATIVE is set, day names from antiveDay() and nativeDayLong() are used for local time,
// otherwise English is used both for local and UTC time
#define FEATURE_LCD_NATIVE

#define CLOCK_HELP // if defined, binary etc clocks will also show normal clock first 10 seconds of every minute

// Day and clock separators & order of day/month in date: 
//    DATEORDER = 'L': Little-endian:  22.04.2016 - EU
//    DATEORDER = 'M': Middle-endian.  04/22/2016 - US
//    DATEORDER = 'B': Big-endian:     2016-04-22 - ISO

// For EU:
char DATEORDER = 'L'; // 1 for 13.9 = 13 September, 
char DATE_SEP = '.'; // Alternatives: '.', '/', '–', ' '.
char HOUR_SEP = ':'; // Alternatives: ':', '.', 'h'.
char MIN_SEP  = ':'; // Alternatives: ':', '.', ' '. 

// For US:
//char DATEORDER = 'M'; // 0 for 9/13 = September, 13 
//char DATE_SEP = '/'; 
//char HOUR_SEP = ':'; 
//char MIN_SEP  = ':';

// For ISO:
//char DATEORDER = 'B';
//char DATE_SEP = '-';
//char HOUR_SEP = ':'; 
//char MIN_SEP  = ':';
