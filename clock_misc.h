//#include "clock_timezone.h"
#define AUTO_UTC_OFFSET
//Central European Time (Frankfurt, Paris):
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     //Central European Summer Time
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};       //Central European Time
Timezone CE(CEST, CET);

#define SUN_LOW -18 // -6, -12, -18, under this value, the solar height changes to peak value at noon rather than actual value
#define MOON_DARK_BACKGROUND

#define LCD_NORSK

//#define FEATURE_SERIAL_OUTPUT

// display order of menus
//int noOfStates = 12; // one more, i.e. 9, 2.7.2018; 10 on 12.09.2018

//int menuOrder[] = {0,1,2,3,4,5,6,7,8,9,10,11}; // as many as noOfStates

// NCDXF beacons first:
//int menuOrder[] = {5,6,2,3,4,0,1,7,8,9,10,11}; // as many as noOfStates


// testing moon rise time
int noOfStates = 10;
int menuOrder[] = {4,8,2,3,0,5,6,7,1,9,10,11}; // as many as noOfStates
