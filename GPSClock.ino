#define CODE_VERSION "0.93 2019-08-30"
/*
    GPS clock on 20x4 I2C LCD
    Sverre Holm, LA3ZA Nov 2015 - July 2017 (Fix 2. July 2018)
    Sep 2018 - (Fix 9.9.2018) + changed order of menus
L
Features:
    Controlled by a GPS module outputting data over an RS232 serial interface, 
    and handled with the TinyGPS++ library
    Shows raw GPS data such as UTC time and date, position, altitude, and number of satellitess
    Shows derived GPS data such as 6-digit locator
    Finds local time and handles daylight saving automatically using the Timezone library
    Finds local sunset and sunrise, either actual value, or civil, nautical, or astronomical. 
    The library is Sunrise.
    The clock also gives local solar height based on the Sunpos library from the K3NG rotator controller. 
    Finally, the clock also provides the lunar phase based on ideas found here 
    and using a reference new moon on 11 November 2015, 11:47 (UNIX time 1447264020)

    Input   from GPS
    Output:
            UTC time with day
            Local time with day
            Automatic daylight saving for local time
            Longitude, latitude, altitude
            Locator, e.g. JO59fu
            Number of satellites used for position
            Moon phase
            Solar data:
               Actual, Civil rise and set times
               Time of solar noon
               Solar elevation
               Solar noon elevation shown if actual elevation is < -12 deg
               PWM control of LCD backlight via potentiometer on analog input
*/



// libraries

#include <TimeLib.h>      // https://github.com/PaulStoffregen/Time
                          //#include <Time.h>   
#include <TinyGPS++.h>    // http://arduiniana.org/libraries/tinygpsplus/
#include <Wire.h>         // For I2C. Comes with Arduino IDE
#include <LiquidCrystal_I2C.h> // Install NewliquidCrystal_1.3.4.zip
                          //#include <LiquidCrystal.h>
#include <Sunrise.h>     // https://github.com/chaeplin/Sunrise, http://www.andregoncalves.info/ag_blog/?p=47
// Now in AVR-Libc version 1.8.1, Aug. 2014 (not in Arduino official release)

// K3NG https://blog.radioartisan.com/yaesu-rotator-computer-serial-interface/
//      https://github.com/k3ng/k3ng_rotator_controller
#include <sunpos.h>      // http://www.psa.es/sdg/archive/SunPos.cpp (via https://github.com/k3ng/k3ng_rotator_controller/tree/master/libraries)
#include <moon2.h>        // via https://github.com/k3ng/k3ng_rotator_controller/tree/master/libraries

#include <Timezone.h>    // https://github.com/JChristensen/Timezone


////// load user-defined setups //////
#include "clock_hardware.h"
#include "clock_misc.h"
//////////////////////////////////////

// set the LCD address to 0x27 
// Set the pins on the I2C chip used for LCD connections:
//                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address


          
// also uses Serial1 for GPS input

/*
  4800; // OK for EM-406A and ADS-GM1
  9600; // OK for NEO-6M
*/



int dispState ;  // depends on button, decides what to display
int UTCoffset=0;   //  value found automatically by means of Timezone library
byte wkday;
char today[10];
double latitude, lon, alt;
int Year;
byte Month, Day, Hour, Minute, Seconds;
u32 noSats;

int packedRise;

double moon_azimuth = 0;
double moon_elevation = 0;
double moon_dist = 0;

float ag; // age of moon in days

int oldminute = -1;

time_t prevDisplay = 0; // when the digital clock was displayed

int yearGPS;
uint8_t monthGPS, dayGPS, hourGPS, minuteGPS, secondGPS, weekdayGPS;

int val; // pot values


// Serial1 <=> pin 19 on Mega
TinyGPSPlus gps; // The TinyGPS++ object


#ifdef MOON_DARK_BACKGROUND
//Light moon on black background
    byte moon7[8] = {0x1f, 0x1f, 0x1b, 0x17, 0x17, 0x17, 0x1b, 0x1f}; //
    byte moon6[8] = {0x1f, 0x1f, 0x1b, 0x13, 0x13, 0x13, 0x1b, 0x1f};  
    byte moon5[8] = {0x1f, 0x1f, 0x19, 0x11, 0x11, 0x11, 0x19, 0x1f};
    byte moon4[8] = {0x1f, 0x1f, 0x1b, 0x11, 0x11, 0x11, 0x1b, 0x1f}; // full moon 
    byte moon3[8] = {0x1f, 0x1f, 0x13, 0x11, 0x11, 0x11, 0x13, 0x1f};
    byte moon20[8] = {0x1f, 0x1f, 0x1b, 0x19, 0x19, 0x19, 0x1b, 0x1f}; // moon2 conflicts with library name
    byte moon1[8] = {0x1f, 0x1f, 0x1b, 0x1d, 0x1d, 0x1d, 0x1b, 0x1f};
    byte moon0[8] = {0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f}; // all black

#else
// Black moon on light background (first implemented)
    byte moon7[8] = {0x0, 0x0, 0x8, 0x10, 0x8, 0x0, 0x0, 0x0};  
    byte moon6[8] = {0x0, 0x0, 0xc, 0x18, 0xc, 0x0, 0x0, 0x0};    
    byte moon5[8] = {0x0, 0x4, 0xc, 0x18, 0xc, 0x4, 0x0, 0x0};   
    byte moon4[8] = {0x0, 0x4, 0x6, 0x3, 0x6, 0x4, 0x0, 0x0};
    byte moon3[8] = {0x0, 0x0, 0x6, 0x7, 0xF, 0x7, 0x6, 0x0};
    byte moon20[8] = {0x0, 0x0, 0x4, 0x2, 0x4, 0x0, 0x0, 0x0};
    byte moon1[8] = {0x0, 0x0, 0x2, 0x1, 0x2, 0x0, 0x0, 0x0};
    byte moon0[8] = {0x0, 0x0, 0x11, 0x0, 0x0, 0x0, 0x11, 0x0};
#endif


TimeChangeRule *tcr;        //pointer to the time change rule, use to get TZ abbrev
time_t utc, local;

void setup(){
  dispState = 0;
  lcd.begin(20, 4);

  // Store bit maps, designed using editor at http://omerk.github.io/lcdchargen/
  byte upDashedArrow[8] = {0x4, 0xa, 0x15, 0x0, 0x4, 0x0, 0x4, 0x0};
  byte downDashedArrow[8] = {0x4, 0x0, 0x4, 0x0, 0x15, 0xa, 0x4, 0x0};
  byte upArrow[8] = {0x4, 0xe, 0x15, 0x4, 0x4, 0x4, 0x4, 0x0};
  byte downArrow[8] = {0x4, 0x4, 0x4, 0x4, 0x15, 0xe, 0x4, 0x0};
  byte AA_small[8] = {B00100, B00000, B01110, B00001, B01111, B10001, B01111, B00000}; // å
  byte OE_small[8] = {B00000, B00001, B01110, B10101, B10101, B01110, B10000, B00000}; // ø

  // upload characters to the lcd
  // character 1 reserved for moon symbol
  lcd.createChar(2, upDashedArrow);
  lcd.createChar(3, downDashedArrow);
  lcd.createChar(4, upArrow);
  lcd.createChar(5, downArrow);
  lcd.createChar(6, AA_small); //   Danish, Norwegian, Swedish
  lcd.createChar(7, OE_small); //   Danish, Norwegian
  lcd.clear(); // you have to ;o)

  pinMode(LCD_pwm, OUTPUT);
  digitalWrite(LCD_pwm, HIGH);   // sets the backlight LED to full

  code_Status();  // start screen 
  
  lcd.print(" ... ");  // ... waiting for GPS
  delay(1000);

  Serial1.begin(GPSBaud);

#ifdef FEATURE_SERIAL_OUTPUT
  // set the data rate for the Serial port
  Serial.begin(115200);
  Serial.println(F("DeviceExample.ino"));
  Serial.println(F("A simple demonstration of TinyGPS++ with an attached GPS module"));
  Serial.print(F("Testing TinyGPS++ library v. ")); Serial.println(TinyGPSPlus::libraryVersion());
  Serial.println(F("by Mikal Hart"));
  Serial.println();
#endif
}

////////////////////////////////////// L O O P //////////////////////////////////////////////////////////////////

void loop() {

  val = analogRead(potentiometer);   // read the input pin for control of backlight

  // analogRead values go from 0 to 1023, analogWrite values from 0 to 255
  // compress using sqrt to get smoother characteristics for the eyes
  analogWrite(LCD_pwm, (int)(255 * sqrt((float)val / 1023))); //

  byte button = analogbuttonread(0);

  if (button == 2) {
    dispState += 1;
    lcd.clear();
    oldminute = -1; // to get immediate display of some info
    lcd.print(dispState);
    delay(300);
  }
  else if (button == 1) {
    dispState -= 1;
    lcd.clear();
    oldminute = -1; // to get immediate display of some info
    if (dispState <0) dispState += noOfStates;
    lcd.print(dispState);
    delay(300);
    
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
        local = CE.toLocal(utc, &tcr); // timezone library
        UTCoffset = (long)(local - utc) / SECS_PER_HOUR;
        
#else
        UTCoffset = 1;
        local = utc + UTCoffset * SECS_PER_HOUR;
#endif

        if (age < 500) {
          // set the Time to the latest GPS reading
          setTime(hourGPS, minuteGPS, secondGPS, dayGPS, monthGPS, yearGPS);

          weekdayGPS = weekday();
          adjustTime(UTCoffset * SECS_PER_HOUR);
        }
      }
    }
    if (timeStatus() != timeNotSet) {
      if (now() != prevDisplay) { //update the display only if the time has changed
        prevDisplay = now();
 /*
 * USER INTERFACE:
 */    
        if      ((dispState % noOfStates) == menuOrder[0]) LocalUTC(); // local time, date; UTC, locator
        else if ((dispState % noOfStates) == menuOrder[1]) UTCLocator(); // UTC, locator, # sats
        else if ((dispState % noOfStates) == menuOrder[2]) LocalSunMoon(); // local time, sun, moon
        else if ((dispState % noOfStates) == menuOrder[3]) LocalSun(); // local time, sun x 3
        else if ((dispState % noOfStates) == menuOrder[4]) LocalMoon(); // local time, moon size and elevation
        else if ((dispState % noOfStates) == menuOrder[5]) NCDXFBeacons(1); // UTC + NCDXF beacons, 14-21 MHz
        else if ((dispState % noOfStates) == menuOrder[6]) NCDXFBeacons(2); // UTC + NCDXF beacons, 18-28 MHz     
        else if ((dispState % noOfStates) == menuOrder[7]) UTCPosition(); // position
        else if ((dispState % noOfStates) == menuOrder[8]) MoonRiseTime(); // Moon rises and sets at these times
        else if ((dispState % noOfStates) == menuOrder[9]) code_Status(); // 
        
        else if ((dispState % noOfStates) == menuOrder[10]) displayInfo(); // full info (a bit crowded)
        else if ((dispState % noOfStates) == menuOrder[11]) displayTest(); // Test moon display, show auto offset info
      }
    }
  }
}

////////////////////////////////////// L O O P //////////////////////////////////////////////////////////////////

void displayTest() { // cycle moon phase display, show UTCoffset

  // moon

  int ii, jj;
  float Phase;
  lcd.setCursor(2, 0);
  
#ifdef AUTO_UTC_OFFSET
  lcd.print("Auto");
#endif

  lcd.setCursor(2, 1);
  lcd.print("UTC offset "); lcd.print(UTCoffset); lcd.print(" hrs");

  for (jj = 0; jj < 1; jj += 1) {
    for (ii = 0; ii < 8; ii += 1) {
      moonSymbol(ii); // Moon symbol in LCD memory 1
      lcd.setCursor(2, 3);
      lcd.print(ii); lcd.print(" ");
      Phase = (float(ii) / 8.0) * 29.53;
      lcd.print(Phase); lcd.print("d");
      lcd.setCursor(12, 3);

      lcd.write(1); // Moon phase symbol
      
      if (Phase < 29.53/2){
        lcd.write(2); // dashed up-arrow
      }
      else
      {
        lcd.write(3); // dashed down-arrow
      }
     
      delay(1000); // 500
    }
  }
  delay(1000);
  dispState += 1;
  lcd.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////

void LocalUTC() { // local time, UTC,  locator

  char textbuffer[9];
  // get local time
  Hour = hour();
  Minute = minute();
  Seconds = second();

  lcd.setCursor(0, 0); // 1. line *********
  sprintf(textbuffer, "%02d:%02d:%02d", Hour, Minute, Seconds);
  lcd.print(textbuffer);
  lcd.print("      ");
  // local date
  Day = day();
  Month = month();
  Year = year();
  if (dayGPS != 0)
  {
    char today[4];

#ifdef LCD_NORSK
    const char* myDays[] = {"Sondag ", "Mandag ", "Tirsdag", "Onsdag ", "Torsdag", "Fredag ", "Lordag "};
    //const char* myDays[] = {"Son", "Man", "Tir", "Ons", "Tor", "Fre", "Lor"};
    int addr = weekday() - 1;
    lcd.setCursor(13, 0);

    if (addr == 2 | addr == 4) {
      strncpy(today, myDays[addr], 8);
    }
    else {
      strncpy(today, myDays[addr], 7);
      lcd.print(" ");
    }
    lcd.print(today);

    if (addr == 0 | addr == 6) {
      lcd.setCursor(15, 0);
      lcd.write(7); // replace 'o' with 'ø'
    }

#else // English
    lcd.setCursor(12, 0);
    sprintf(today, "%06s", dayStr(weekday()));
    lcd.print(today); lcd.print(" ");
#endif
    lcd.setCursor(0,1); lcd.print("                    ");
    lcd.setCursor(10, 1);lcd.print("  "); // 2.7.2018
    
    if (Day < 10 & Month < 10) lcd.setCursor(12, 1);
    else if (Day < 10 | Month < 10) lcd.setCursor(11, 1);
    else lcd.setCursor(10, 1);
    
    lcd.print(static_cast<int>(Day)); lcd.print(".");
    lcd.print(static_cast<int>(Month)); lcd.print(".");
    lcd.print(static_cast<int>(Year));
  }
  lcd.setCursor(0, 3); // 4. line *********
  lcd.print("                    ");
  
  if (gps.time.isValid()) {
    lcd.setCursor(0, 3);
    sprintf(textbuffer, "%02d:%02d:%02d UTC", hourGPS, minuteGPS, secondGPS);
    lcd.print(textbuffer);
    lcd.setCursor(14, 3);
  }
  if (gps.satellites.isValid()) {
    latitude = gps.location.lat();
    lon = gps.location.lng();
    char locator[7];
    Maidenhead(lon, latitude, locator);
    lcd.print(locator);
  }
  
  
  oldminute = minuteGPS;
}

///////////////////////////////////////////////////////////////////////////////////////////////
void LocalMoon() { // local time, moon size and elevation

  char textbuffer[9];
  String textbuf;
  float percentage;
  
  
  // get local time
  Hour = hour();
  Minute = minute();
  Seconds = second();

  lcd.setCursor(0, 0); // 1. line *********
  sprintf(textbuffer, "%02d:%02d:%02d", Hour, Minute, Seconds);
  lcd.print(textbuffer);

  // local date
  Day = day();
  Month = month();
  Year = year();
  if (dayGPS != 0)
  {
    lcd.setCursor(10, 0);
    if (Day < 10 & Month < 10) lcd.setCursor(12, 0);
    else if (Day < 10 | Month < 10) lcd.setCursor(11, 0);
    lcd.print(static_cast<int>(Day)); lcd.print(".");
    lcd.print(static_cast<int>(Month)); lcd.print(".");
    lcd.print(static_cast<int>(Year));
  }
/////////////////////////////////
   if (gps.location.isValid()) {
    if (minuteGPS != oldminute) {
           
      // days since last new moon
      float Phase, PercentPhase;
      int QPhase;
      lcd.setCursor(0, 1);
      lcd.print("Moon:");
      MoonPhase(Phase, QPhase, PercentPhase);
      moonSymbol(QPhase); // Moon symbol in LCD memory 3
      lcd.setCursor(10, 1);

      lcd.write(1); // Moon phase symbol
      if (Phase < 29.53/2) lcd.write(2); // dashed up-arrow
      else                 lcd.write(3); // dashed down-arrow
    
      //textbuf = String(Phase, 1);    
      textbuf = String(PercentPhase,0);
      lcd.print("   ");
      
      if (PercentPhase <10)       lcd.setCursor(16, 1);
      //if (Phase < 10) lcd.setCursor(16, 1);
      else if (PercentPhase <100) lcd.setCursor(15, 1);
      else                        lcd.setCursor(14, 1);
      
      lcd.print(textbuf); 
      //lcd.print("d");
      lcd.print(" %");

      
  
      latitude = gps.location.lat();
      lon = gps.location.lng();

      update_moon_position();

      lcd.setCursor(0,3);
      lcd.print("Az ");
      textbuf = String(moon_azimuth, 1);
      lcd.print(textbuf); lcd.write(223); // degree symbol
      lcd.print("   "); // to blank out rest of longer symbol

//      lcd.setCursor(11,2);
//      textbuf = String(moon_dist, 0);
//      lcd.print(textbuf);lcd.print(" Km");
      
      lcd.setCursor(11,3);
      lcd.print("El ");
      textbuf = String(moon_elevation, 1);
      lcd.print(textbuf); lcd.write(223); // degree symbol
      lcd.print("   "); // to blank out rest of longer symbol

//      lcd.setCursor(15,3);
//      percentage = float(moon_dist)/4067;
//      lcd.print(percentage,0); lcd.print(" %"); 

// Moon rise or set time:

       lcd.setCursor(0,2);
       
      
       if (moon_elevation < 0) 
          lcd.print("   will soon rise! ");
          
       else                    
          lcd.print("   soon to set!   ");

////////////////////////////////
  oldminute = minuteGPS;
  }
 }

}


////////////////////////////////////////////////////////////////////////////////////////////

void UTCPosition() {     // position, altitude, locator, # satellites
  char textbuffer[20];
  String textbuf;

  lcd.setCursor(0, 0); // 1. line *********
  if (gps.time.isValid()) {
    sprintf(textbuffer, "%02d:%02d:%02d UTC", hourGPS, minuteGPS, secondGPS);
    lcd.print(textbuffer);
  }

  // UTC date

  if (gps.location.isValid()) {
    latitude = gps.location.lat();
    lon = gps.location.lng();
    alt = gps.altitude.meters();

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
      lcd.print(textbuf); lcd.write(223); // degree symbol
      mins = 60 * (latitude - floor(latitude));
      textbuf = String(floor(mins), 0); // round down = floor
      lcd.print(textbuf); lcd.write("'");
      textbuf = String(floor(0.5 + 60 * (mins - floor(mins))), 0); // round
      lcd.print(textbuf); lcd.write(34); lcd.print(", ");

      textbuf = String(floor(lon), 0);
      lcd.print(textbuf);
      lcd.write(223); // degree symbol
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
      lcd.print(textbuf); lcd.write(223); // degree symbol
      mins = 60 * (latitude - floor(latitude));
      textbuf = String(mins, 2); // round down = floor
      lcd.print(textbuf); lcd.write("' ");

      textbuf = String(floor(lon), 0);
      lcd.print(textbuf);
      lcd.write(223); // degree symbol
      mins = 60 * (lon - floor(lon));
      textbuf = String(mins, 2);
      lcd.print(textbuf); lcd.write("' ");
    }

    char locator[7];
    Maidenhead(lon, latitude, locator);
    lcd.setCursor(14, 0); // 1. line *********
    lcd.print(locator);
  }
  if (gps.satellites.isValid()) {
    noSats = gps.satellites.value();
    if (noSats < 10) lcd.setCursor(14, 3);
    else lcd.setCursor(13, 3); lcd.print(noSats); lcd.print(" Sats");
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

void UTCLocator() {     // UTC, locator, # satellites
  char textbuffer[20];

  lcd.setCursor(0, 0); // 1. line *********
  if (gps.time.isValid()) {
    sprintf(textbuffer, "%02d:%02d:%02d         UTC", hourGPS, minuteGPS, secondGPS);
    lcd.print(textbuffer);
  }

  // UTC date

  if (dayGPS != 0)
  {
    lcd.setCursor(0, 1);
    char today[4];
    lcd.print(dayStr(weekdayGPS)); lcd.print("   "); // two more spaces 14.04.2018

    lcd.setCursor(10, 1);lcd.print("  "); // 2.7.2018
    
    if (Day < 10 & Month < 10) lcd.setCursor(12, 1);
    else if (Day < 10 | Month < 10) lcd.setCursor(11, 1);
    else lcd.setCursor(10, 1);
    
    lcd.print(static_cast<int>(dayGPS)); lcd.print(".");
    lcd.print(static_cast<int>(monthGPS)); lcd.print(".");
    lcd.print(static_cast<int>(yearGPS));
  }
  if (gps.location.isValid()) {
    latitude = gps.location.lat();
    lon = gps.location.lng();
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

////////////////////////////////////////////////////////////////////////////////////

void NCDXFBeacons(int option) {     // UTC, + info about NCDXF beacons
// option=1: 14-21 MHz beacons on lines 1-3
// option=2: 21-28 MHz beacons on lines 1-3
// Inspired by OE3GOD: https://www.hamspirit.de/7757/eine-stationsuhr-mit-ncdxf-bakenanzeige/ 
         
  int ii, iii, iiii, offset, km;
  char textbuffer[20];
  double lati, longi;

  char* callsign[18]={
  " 4U1UN", " VE8AT", "  W6WX", " KH6RS", "  ZL6B", "VK6RBP", "JA2IGY", "  RR9O", "  VR2B", "  4S7B", " ZS6DN",
  "  5Z4B", " 4X6TU", "  OH2B", "  CS3B", " LU4AA", "  OA4B", "  YV5B"}; 

  char* location[18]={
  "FN30as", "EQ79ax", "CM97bd", "BL10ts", "RE78tw", "OF87av", "PM84jk", "NO14kx", "OL72bg", "MJ96wv", "KG44dc",
  "KI88ks", "KM72jb", "KP20dh", "IM12or", "GF05tj", "FH17mw", "FJ69cc"};
// OH2B @ KP20dh and not just KP20: https://automatic.sral.fi/?stype=beacon&language=en

  char* qth[18] = {
  "N York ", "Nunavut", "Califor", "Hawaii ", "N Zeala", "Austral", "Japan  ", "Siberia", "H Kong ", "Sri Lan", "S Afric",
  "Kenya  ", "Israel ", "Finland", "Madeira", "Argenti", "Peru   ", "Venezue"};

char* qrg[5] = {"14100","18110","21150","24930","28200"};

   
  lcd.setCursor(0, 0); // 1. line *********
  if (gps.time.isValid()) {
    sprintf(textbuffer, "%02d:%02d:%02d UTC", hourGPS, minuteGPS, secondGPS);
    lcd.print(textbuffer);
  }
  if (gps.satellites.isValid()) {
    char locator[7];
    latitude = gps.location.lat();
    lon = gps.location.lng();
    Maidenhead(lon, latitude, locator);
    lcd.print(" ");
    lcd.print(locator);
  }
/*
 * Each beacon transmits once on each band once every three minutes, 24 hours a day.
 * At the end of each 10 second transmission, the beacon steps to the next higher band 
 * and the next beacon in the sequence begins transmitting.
 */

  ii = (60*(minuteGPS % 3)+secondGPS)/10; // ii from 0 to 17
  
    if (option <=1) offset = 0; // 14-18 MHz
    else            offset = 2; // 18-28 MHz
    
    for (iiii = 1; iiii < 4; iiii += 1) {
      lcd.setCursor(0, iiii);
      //
      // modulo for negative numbers: https://twitter.com/parkerboundy/status/326924215833985024
      iii = ((ii-iiii+1-offset % 18)+18)%18;
      lcd.print(qrg[iiii-1+offset]); lcd.print(" "); lcd.print(callsign[iii]); 
      if (secondGPS % 10 < 5){lcd.print(" "); lcd.print(qth[iii]); }  // first half of cycle: location
      else                                                            // second half of cycle: distance
      {
        locator_to_latlong(location[iii], lati, longi);// position of beacon 
        km = distance(lati, longi, latitude, lon);    // distance beacon - GPS 

        if (km < 1000)        lcd.print("   ");
        else if (km < 10000)  lcd.print("  ");
        else                  lcd.print(" "); 
        lcd.print(km); lcd.print("km");
      }
    } 
}

////////////////////////////////////////////////////////////////////////////////////


void LocalSunMoon() { // local time, sun, moon

  char textbuffer[9];
  String textbuf;
  // get local time
  Hour = hour();
  Minute = minute();
  Seconds = second();

  lcd.setCursor(0, 0); // 1. line **************************************
 
  // local date
  Day = day();
  Month = month();
  Year = year();
  if (dayGPS != 0)
  {
     char today[4];
 
#ifdef LCD_NORSK
    nativeDate();
#else
    sprintf(today, "%03s", dayShortStr(weekday()));
    lcd.print(today); lcd.print(" ");
#endif
    //lcd.print(" ");
    lcd.print(static_cast<int>(Day)); lcd.print(".");
    lcd.print(static_cast<int>(Month));
  }
  lcd.print("    "); // in order to erase remnants of long string as the month changes
  
  lcd.setCursor(10, 0);
  sprintf(textbuffer, "  %02d:%02d:%02d", Hour, Minute, Seconds);
  lcd.print(textbuffer);
  
  if (gps.location.isValid()) {
    if (minuteGPS != oldminute) {
      latitude = gps.location.lat();
      lon = gps.location.lng();

      lcd.setCursor(0, 1); // 2. line ****************************************  
  
      // create a Sunrise object
      Sunrise mySunrise(latitude, lon, UTCoffset);

      mySunrise.Actual(); //Actual, Civil, Nautical, Astronomical
      char SunChar = 'A';

      byte h, m;
      int t; // t= minutes past midnight of sunrise (6 am would be 360)
      t = mySunrise.Rise(monthGPS, dayGPS); // (month,day) - january=1

//goto SolarElevation;
 
      if (t >= 0) {
        h = mySunrise.Hour();
        m = mySunrise.Minute();
        lcd.print("S ");
        lcd.write(4); // up-arrow
        lcd.print(h, DEC);
        lcd.print(":");
        if (m < 10) lcd.print("0");
        lcd.print(m, DEC);
      }
      lcd.print("       ");  // 4.2.2016 increased by one space. Unknown 't' before down arrow

      t = mySunrise.Set(monthGPS, dayGPS);
      //lcd.print(" ");
      lcd.setCursor(14, 1);
      lcd.write(5); // down arrow
      if (t >= 0) {
        h = mySunrise.Hour();
        m = mySunrise.Minute();

        lcd.print(h, DEC);
        lcd.print(":");
        if (m < 10) lcd.print("0");
        lcd.print(m, DEC);
      }

SolarElevation:

      /////// Solar elevation //////////////////

      int hNoon, mNoon;
      double dElevation;
      double dhNoon,dmNoon;

/////////////////// from K3NG /// sunpos ////////////////////////////////
      float sun_azimuth = 0;
      float sun_elevation = 0;
      cTime c_time;
      cLocation c_loc;
      cSunCoordinates c_sposn;
      
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
    
      sunpos(c_time, c_loc, &c_sposn);
    
      // Convert Zenith angle to elevation
      sun_elevation = 90. - c_sposn.dZenithAngle;
      sun_azimuth = c_sposn.dAzimuth;
  
      Sunrise my2Sunrise(latitude, lon, UTCoffset);
      t = my2Sunrise.Noon(monthGPS, dayGPS);

      if (t >= 0) {
        hNoon = my2Sunrise.Hour();
        mNoon = my2Sunrise.Minute();
      }


      if (sun_elevation < SUN_LOW) {
        /* At night, show instead max solar elevation at noon
          -6 deg - Civil Twilight: Approx the limit where terrestrial objects can be clearly distinguished.
          -12 deg - Nautical: The horizon is clearly visible,
          -18 deg - Astronomical: Sky is fully dark, and astronomers can easily make observations
        */
        
        
        c_time.dHours = hNoon-UTCoffset;
        c_time.dMinutes = mNoon;
        c_time.dSeconds = 0.0;
      
        //helios.calcSunPos(yearGPS, monthGPS, idayGPS,
        //                    dhNoon - UTCoffset, dmNoon, 0.0, lon, latitude); // peak elevation
        sunpos(c_time, c_loc, &c_sposn);
    
      // Convert Zenith angle to elevation
        sun_elevation = 90. - c_sposn.dZenithAngle;
        sun_azimuth = c_sposn.dAzimuth;
         
      }

      lcd.setCursor(8, 1);
      String textbuf = String(sun_elevation, 0); // was ..., 0) 
      
      //if (abs(dElevation) < 10) lcd.setCursor(9, 1);
      lcd.print(textbuf);
      lcd.write(223); // degree symbol
      
      ///// Solar elevation end
   
       lcd.setCursor(0, 2); // 3. line *********************************

      // create a Sunrise object

      my2Sunrise.Civil(); //Actual, Civil, Nautical, Astronomical
      SunChar = 'C';
     
      int tRise;
      tRise = my2Sunrise.Rise(monthGPS, dayGPS); // (month,day) - january=1

      if (tRise >= 0) {
        h = my2Sunrise.Hour();
        m = my2Sunrise.Minute();
        lcd.print("  ");
        lcd.write(2); // dashed up-arrow
        lcd.print(h, DEC);
        lcd.print(":");
        if (m < 10) lcd.print("0");
        lcd.print(m, DEC);
      }

Noon:
      
      //t = mySunrise.Noon(Month, Day);
      t = my2Sunrise.Noon(monthGPS, dayGPS);
      lcd.print(" ");

      if (t >= 0) {
        hNoon = my2Sunrise.Hour();
        mNoon = my2Sunrise.Minute();
        lcd.print(hNoon, DEC);
        lcd.print(":");
        if (mNoon < 10) lcd.print("0");
        lcd.print(mNoon, DEC);
      }
      lcd.print("  ");

//goto LCDlastline;
      
      int tSet;
      tSet = my2Sunrise.Set(monthGPS, dayGPS);
      lcd.setCursor(14, 2);
      lcd.write(3); // dashed down arrow
      if (tSet >= 0) {
        h = my2Sunrise.Hour();
        m = my2Sunrise.Minute();
        if (h<10) lcd.setCursor(16, 2); // added 4.7.2016 to deal with summer far North
        lcd.print(h, DEC);
        lcd.print(":");
        if (m < 10) lcd.print("0");
        lcd.print(m, DEC);
      }
    
      // days since last new moon

      float Phase, PercentPhase;
      int QPhase;
           
      lcd.setCursor(0, 3);
      lcd.print("M ");

      MoonPhase(Phase, QPhase, PercentPhase);
      moonSymbol(QPhase); // Moon symbol in LCD memory 3
      lcd.setCursor(2, 3);
      if (Phase < 29.53/2){
        lcd.write(2); // dashed up-arrow
      }
      else
      {
        lcd.write(3); // dashed down-arrow
      }   
      lcd.write(1); // Moon phase symbol
      
      lcd.print("        ");

      
      update_moon_position();     
      lcd.setCursor(8, 3);
      textbuf = String(moon_elevation, 0);
      if (moon_elevation < 10 & moon_elevation > 0) lcd.print(" ");
      lcd.print(textbuf); lcd.write(223); // degree symbol
      lcd.print("   "); // to blank out rest of longer symbol
        
      lcd.print("      "); // 6 spaces blanks rest of line
      if (PercentPhase <10) lcd.setCursor(16,3);
      else if (PercentPhase <100) lcd.setCursor(15,3);
      else lcd.setCursor(14,3);
      
      textbuf = String(PercentPhase,0);
      lcd.print(textbuf); lcd.print("%");
      
//      if (QPhase > 4) QPhase = 8-QPhase;
//      lcd.setCursor(17, 3);
//      lcd.print(QPhase); lcd.print("/4");
      
//      textbuf = String(Phase, 1);
//      if (Phase < 10) lcd.setCursor(16, 3);
//      else lcd.setCursor(15, 3);
//      lcd.print(textbuf); lcd.print("d");

    }
  }
  oldminute = minuteGPS;
}


////////////////////////////////////////////////////////////////////////////////////////////

void LocalSun() { // local time, sun x 3

  char textbuffer[9];
  // get local time
  Hour = hour();
  Minute = minute();
  Seconds = second();

  lcd.setCursor(0, 0); // 1. line **************************************
  
  // local date
  Day = day();
  Month = month();
  Year = year();
  if (dayGPS != 0)
  {
     char today[4];
 
#ifdef LCD_NORSK
    nativeDate();
#else
    sprintf(today, "%03s", dayShortStr(weekday()));
    lcd.print(today); lcd.print(" ");
#endif

    lcd.print(static_cast<int>(Day)); lcd.print(".");
    lcd.print(static_cast<int>(Month));
  }
  lcd.print("    "); // in order to erase remants of long string as the month changes
  
  lcd.setCursor(10, 0);
  sprintf(textbuffer, "  %02d:%02d:%02d", Hour, Minute, Seconds);
  lcd.print(textbuffer);

  
  if (gps.location.isValid()) {
    if (minuteGPS != oldminute) {
      latitude = gps.location.lat();
      lon = gps.location.lng();

      lcd.setCursor(0, 1); // 2. line ****************************************
      
  
      // create a Sunrise object
      Sunrise mySunrise(latitude, lon, UTCoffset);

      mySunrise.Actual(); //Actual, Civil, Nautical, Astronomical
      char SunChar = 'A';

      byte h, m;
      int t; // t= minutes past midnight of sunrise (6 am would be 360)
      t = mySunrise.Rise(monthGPS, dayGPS); // (month,day) - january=1


      if (t >= 0) {
        h = mySunrise.Hour();
        m = mySunrise.Minute();
        lcd.print("S ");
        lcd.write(4); // up-arrow
        lcd.print(h, DEC);
        lcd.print(":");
        if (m < 10) lcd.print("0");
        lcd.print(m, DEC);
      }
      lcd.print("       ");  // 4.2.2016 increased by one space. Unknown 't' before down arrow

      t = mySunrise.Set(monthGPS, dayGPS);
      //lcd.print(" ");
      lcd.setCursor(14, 1);
      lcd.write(5); // down arrow
      if (t >= 0) {
        h = mySunrise.Hour();
        m = mySunrise.Minute();

        lcd.print(h, DEC);
        lcd.print(":");
        if (m < 10) lcd.print("0");
        lcd.print(m, DEC);
      }

      /////// Solar elevation //////////////////
//      Helios helios;
      int hNoon, mNoon;
     double dElevation;
//      double dAzimuth;
//      int idayGPS;
//      double dhourGPS, dminuteGPS, dsecondGPS;
      double dhNoon,dmNoon;
//      idayGPS = dayGPS;
//      dhourGPS = hourGPS;
//      dminuteGPS = minuteGPS;
//      dsecondGPS = secondGPS;

/////////////////// from K3NG /// sunpos ////////////////////////////////
      float sun_azimuth = 0;
      float sun_elevation = 0;
      cTime c_time;
      cLocation c_loc;
      cSunCoordinates c_sposn;
      
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
    
      sunpos(c_time, c_loc, &c_sposn);
    
      // Convert Zenith angle to elevation
      sun_elevation = 90. - c_sposn.dZenithAngle;
      sun_azimuth = c_sposn.dAzimuth;
    
      Sunrise my2Sunrise(latitude, lon, UTCoffset);
      t = my2Sunrise.Noon(monthGPS, dayGPS);

      if (t >= 0) {
        hNoon = my2Sunrise.Hour();
        mNoon = my2Sunrise.Minute();
      }

      lcd.setCursor(9, 1);  // actual solar elevation
      String textbuf = String(sun_elevation, 0); // was ..., 0) 
      
      // if (abs(dElevation) < 10) lcd.setCursor(9, 1);
      lcd.print(textbuf);
      lcd.write(223); // degree symbol
      
      ///// Solar elevation end

     // if (sun_elevation < SUN_LOW) {
        /* Find max solar elevation at noon
          -6 deg - Civil Twilight: Approx the limit where terrestrial objects can be clearly distinguished.
          -12 deg - Nautical: The horizon is clearly visible,
          -18 deg - Astronomical: Sky is fully dark, and astronomers can easily make observations
        */
         
     // }Sa

      
     
      lcd.setCursor(0, 2); // 3. line *********************************
     
      // create a Sunrise object

      my2Sunrise.Civil(); //Actual, Civil, Nautical, Astronomical
      SunChar = 'C';
     
      int tRise;
      tRise = my2Sunrise.Rise(monthGPS, dayGPS); // (month,day) - january=1

      if (tRise >= 0) {
        h = my2Sunrise.Hour();
        m = my2Sunrise.Minute();
        lcd.print("  ");
        lcd.write(2); // dashed up-arrow
        lcd.print(h, DEC);
        lcd.print(":");
        if (m < 10) lcd.print("0");
        lcd.print(m, DEC);
      }

Noon:
      
      //t = mySunrise.Noon(Month, Day);
      t = my2Sunrise.Noon(monthGPS, dayGPS);
      lcd.print(" ");

      if (t >= 0) {
        hNoon = my2Sunrise.Hour();
        mNoon = my2Sunrise.Minute();
        lcd.print(hNoon, DEC);
        lcd.print(":");
        if (mNoon < 10) lcd.print("0");
        lcd.print(mNoon, DEC);
      }
      lcd.print("  ");
    
      int tSet;
      tSet = my2Sunrise.Set(monthGPS, dayGPS);
      lcd.setCursor(14, 2);
      lcd.write(3); // dashed down arrow
      if (tSet >= 0) {
        h = my2Sunrise.Hour();
        m = my2Sunrise.Minute();
        if (h<10) lcd.setCursor(16, 2); // added 4.7.2016 to deal with summer far North
        lcd.print(h, DEC);
        lcd.print(":");
        if (m < 10) lcd.print("0");
        lcd.print(m, DEC);
      }

      lcd.setCursor(0, 3); // 4. line *********************************
     
      // create a Sunrise object

      my2Sunrise.Nautical(); //Actual, Civil, Nautical, Astronomical
      tRise = my2Sunrise.Rise(monthGPS, dayGPS); // (month,day) - january=1

      if (tRise >= 0) {
        h = my2Sunrise.Hour();
        m = my2Sunrise.Minute();
        lcd.print("  ");
        lcd.print(" ");
        lcd.print(h, DEC);
        lcd.print(":");
        if (m < 10) lcd.print("0");
        lcd.print(m, DEC);
      }
     
      tSet = my2Sunrise.Set(monthGPS, dayGPS);
      lcd.setCursor(14, 3);
      lcd.print(" ");
      if (tSet >= 0) {
        h = my2Sunrise.Hour();
        m = my2Sunrise.Minute();
        if (h<10) lcd.setCursor(16, 2); // added 4.7.2016 to deal with summer far North
        lcd.print(h, DEC);
        lcd.print(":");
        if (m < 10) lcd.print("0");
        lcd.print(m, DEC);
      }

       // find max solar elevation at local noon
        c_time.dHours = hNoon-UTCoffset;
        c_time.dMinutes = mNoon;
        c_time.dSeconds = 0.0;
      
        //helios.calcSunPos(yearGPS, monthGPS, idayGPS,
        //                    dhNoon - UTCoffset, dmNoon, 0.0, lon, latitude); // peak elevation
        sunpos(c_time, c_loc, &c_sposn);
    
      // Convert Zenith angle to elevation
        sun_elevation = 90. - c_sposn.dZenithAngle;
        sun_azimuth = c_sposn.dAzimuth;   
        textbuf = String(sun_elevation, 0);
        lcd.setCursor(9, 3);
        lcd.print(textbuf);
        lcd.write(223); // degree symbol
    }
  }
  oldminute = minuteGPS;
}

////////////////////////////////////////////////////////////////////////////////////////////

void displayInfo() {   // full info
  double latitude, lon, alt;
  int Year;
  byte Month, Day, Hour, Minute, Seconds;
  u32 noSats;
  char textbuffer[9];

  // get local time
  Hour = hour();
  Minute = minute();
  Seconds = second();

  lcd.setCursor(0, 1); // 2. line *********
  sprintf(textbuffer, "%02d:%02d", Hour, Minute);
  lcd.print(textbuffer);

  // local date
  Day = day();
  Month = month();
  Year = year();
  if (dayGPS != 0)
  {

    lcd.setCursor(11, 1);
    char today[4];
    if (Day < 10 & Month < 10) lcd.setCursor(13, 1);
    else if (Day < 10 | Month < 10) lcd.setCursor(12, 1);

#ifdef LCD_NORSK
    nativeDate();
#else
    sprintf(today, "%03s", dayShortStr(weekday()));
    lcd.print(today); lcd.print(" ");
#endif
    lcd.print(static_cast<int>(Day)); lcd.print(".");
    lcd.print(static_cast<int>(Month));
  }

  if (gps.time.isValid()) {
    lcd.setCursor(0, 0); // 1. line *********
    sprintf(textbuffer, "%02d:%02d:%02d UTC", hourGPS, minuteGPS, secondGPS);
    lcd.print(textbuffer);
  }

  if (gps.location.isValid()) {
    latitude = gps.location.lat();
    lon = gps.location.lng();
    char locator[7];
    Maidenhead(lon, latitude, locator);
    lcd.setCursor(14, 0); // 1. line *********
    lcd.print(locator);
  }

  if (gps.location.isValid()) {
    if (minuteGPS != oldminute)
    {
      // create a Sunrise object
      Sunrise mySunrise(latitude, lon, UTCoffset);
      mySunrise.Civil(); //Actual, Civil, Nautical, Astronomical
      char SunChar = 'C';

      byte h, m;
      int t; // t= minutes past midnight of sunrise (6 am would be 360)
      //t = mySunrise.Rise(Month, Day); // (month,day) - january=1
      t = mySunrise.Rise(monthGPS, dayGPS); // (month,day) - january=1

      if (t >= 0) {
        h = mySunrise.Hour();
        m = mySunrise.Minute();

        lcd.setCursor(0, 2); // 3. line *********
        lcd.print("S ");

        lcd.print(h, DEC);
        lcd.print(":");
        if (m < 10) lcd.print("0");
        lcd.print(m, DEC);

      }
      //t = mySunrise.Noon(Month, Day);
      t = mySunrise.Noon(monthGPS, dayGPS);
      lcd.print(" ");
      if (t >= 0) {
        h = mySunrise.Hour();
        m = mySunrise.Minute();

        lcd.print(h, DEC);
        lcd.print(":");
        if (m < 10) lcd.print("0");
        lcd.print(m, DEC);
      }

      //t = mySunrise.Set(Month, Day);
      t = mySunrise.Set(monthGPS, dayGPS);
      lcd.print(" ");
      if (t >= 0) {
        h = mySunrise.Hour();
        m = mySunrise.Minute();

        lcd.print(h, DEC);
        lcd.print(":");
        if (m < 10) lcd.print("0");
        lcd.print(m, DEC);
      }

      lcd.print(" "); lcd.print(SunChar);

      // Moon info
      lcd.setCursor(0, 3); // 4. line *********

      lcd.print("M ");

#ifdef LCD_NORSK
      //       lcd.print("M"); lcd.write(6); lcd.print("ne ");
#else
      //       lcd.print("Moon ");
#endif
      float Phase, PercentPhase;
      int QPhase;
      MoonPhase(Phase, QPhase, PercentPhase);
      lcd.write(1); lcd.print(" ");// Moon phase symbol
      lcd.print(Phase); lcd.print("d "); // days since last new moon
      //lcd.print(QPhase); // Quantized to 0..7
      moonSymbol(QPhase); // Moon symbol in LCD memory 1
      lcd.setCursor(9, 3);
    }
  }

  if (gps.satellites.isValid()) {
    noSats = gps.satellites.value();

    if (noSats < 10) lcd.setCursor(14, 3); // 2. line *********
    else lcd.setCursor(13, 3); // 2. line *********
    lcd.print(noSats); lcd.print(" Sats");
  }

  oldminute = minuteGPS;


#ifdef FEATURE_SERIAL_OUTPUT

  // On Arduino, GPS characters may be lost during lengthy Serial.print()
  // On Teensy, Serial prints to USB, which has large output buffering and
  //   runs very fast, so it's not necessary to worry about missing 4800
  //   baud GPS characters.

  Serial.print(F("Location: "));
  if (gps.location.isValid()) {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
  }
  else Serial.print(F("INVALID"));

  Serial.print(F("  Date/Time: "));
  if (gps.date.isValid()) {
    Serial.print(gps.date.day());
    Serial.print(F("."));
    Serial.print(gps.date.month());
    Serial.print(F("."));
    Serial.print(gps.date.year());
  }
  else Serial.print(F("INVALID"));

  Serial.print(F(" "));
  if (gps.time.isValid()) {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    Serial.print(gps.time.centisecond());
  }
  else Serial.print(F("INVALID"));
  Serial.println();
#endif

}

////////////////////////////////////////////////////////////////////////////////////////////
// Collection of helper functions //////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

void Maidenhead(double lon, double latitude, char loc[7]) {
  int lonTrunc, latTrunc;

  // Locator calculation  based on Python code of
  // http://en.wikipedia.org/wiki/Maidenhead_Locator_System

  // Examples for debugging:
  //   BH52jn: lon = -14.9176000; latitude = -17.438000;
  //   FN31pr: lon = -72.727260;  latitude = 41.714775;
  //   JO59jw; lon = 10.750000;   latitude = 59.945556;
  //   JJ00aa: lon = 0;          latitude = 0;

  lon += 180.0;
  latitude += 90.0;
  lonTrunc = ((int)(lon * 20 )) / 20.0;
  latTrunc = ((int)(latitude * 10 )) / 10.0;

  loc[0] = 'A' + lonTrunc / 20; // 1 field = 20 deg
  loc[1] = 'A' + latTrunc / 10;  // 1 field = 10 deg
  loc[2] = '0' + (lonTrunc % 20) / 2; // 1 square = 2 deg
  loc[3] = '0' + (latTrunc % 10) / 1; // 1 square = 1 deg
  loc[4] = 'a' + (lon - lonTrunc) * 12; // 1 subsquare = 5' = 1/12 deg
  loc[5] = 'a' + (latitude - latTrunc) * 24; // 1 subsquare = 2.5' = 1/24 deg
  loc[6] = '\0';
}

///////////////////////////////////////////////////////////////////////
// Last new moon: November 11 17:47 UTC 2015
#define REF_TIME 1447264020 // in Unix time
//#define REF_TIME 1263539400 // 2010, gives slightly larger value - so some numerical drift
#define CYCLELENGTH 2551443 // 29.53 days

void MoonPhase(float &Phase, int &QPhase, float &PercentPhase) {
  const float moonMonth = 29.53;
  long dif, moon;
  dif = (now() - REF_TIME); // Seconds since reference new moon
  moon = dif % CYCLELENGTH; // Seconds since last new moon
  Phase = abs(moon / float(86400)); // in days
  QPhase = (int)(( 8 * Phase / moonMonth) + 0.5); // value from 0 .. 8
  QPhase = QPhase % 8; // 8 is the same as 0
  // 0 new moon, 2 half, 4 full moon, 6 half
  // http://forum.arduino.cc/index.php?topic=48337.0 :
  // The illumination approximates to a sin wave through the cycle.
 // PercentPhase = 100 - 100*abs((Phase - 29.53/2)/(29.53/2)); // too big 39%, should be 31%
 // PercentPhase = 100*(sin((PI/2)*(1 - abs((Phase - moonMonth/2)/(moonMonth/2))))); // even bigger 57%
 // PercentPhase = 100*((2/pi)*asin(1 - abs((Phase - moonMonth/2)/(moonMonth/2)))) ;  //  too small 25%
 //http://www.dendroboard.com/forum/parts-construction/280865-arduino-moon-program.html
 PercentPhase = 50*(1-cos(2*PI*Phase/moonMonth));
}

/* Moon
  Based on ideas from  http://forum.arduino.cc/index.php?topic=48337.0
  The last new moon on the 15/1/2010 at 7:10 which is unix time 1263539400
  A full cycle is 2551442.803 seconds
*/
//
// http://www.moongiant.com/moonphases/
// http://www.onlineconversion.com/unix_time.htm


//------------------------------------------------------------------

byte analogbuttonread(byte button_number) {
  // K3NG keyer code
  // button numbers start with 0

  int analog_line_read = analogRead(analog_buttons_pin);
  // 10 k from Vcc to A0, then n x 1k to gnd, n=0...9;
  //if (analog_line_read < 500) { // any of 10 buttons will trigger
  
  if (analog_line_read < 131 & analog_line_read > 50) { // button 1
      return 1;
  }
  else if (analog_line_read < 51) { // button 0 
      return 2; 
    }
    else  return 0;
}

void moonSymbol(int QPhase) {
  byte store = 1;
  if (QPhase == 0) lcd.createChar(store, moon0);
  if (QPhase == 1) lcd.createChar(store, moon1);
  if (QPhase == 2) lcd.createChar(store, moon20);
  if (QPhase == 3) lcd.createChar(store, moon3);
  if (QPhase == 4) lcd.createChar(store, moon4);
  if (QPhase == 5) lcd.createChar(store, moon5);
  if (QPhase == 6) lcd.createChar(store, moon6);
  if (QPhase == 7) lcd.createChar(store, moon7);
}

//------------------------------------------------------------------

void locator_to_latlong(char loc[7], double &latitude, double &longitude) {
 /* 
 convert locator to latitude, longitude  
 based on pyhamtools.locator http://pyhamtools.readthedocs.io/en/stable/reference.html#module-pyhamtools.locator
 locator  must have 6 digits
*/
    int i;
    for (i = 0; i < 6; i += 1) {
      loc[i] = toUpperCase(loc[i]);
    }
       
    longitude = ((int)loc[0] - (int)('A')) * 20.0 - 180.0;
    latitude = ((int)loc[1] - (int)('A')) * 10.0 - 90.0;
    
    longitude += ((int)loc[2] - (int)('0')) * 2.0;
    latitude += ((int)loc[3] - (int)('0'));

    longitude += ((int)loc[4] - (int)('A')) * (2.0 / 24.0);
    latitude  += ((int)loc[5] - (int)('A')) * (1.0 / 24.0);

    // move to center of subsquare
    longitude += 1.0 / 24.0;
    latitude += 0.5 / 24.0;
}

//////////////////////////////////////////////////////////////////////////////////////

int distance(double lat1, double long1, double lat2, double long2) {
/*
 *  Calculate distance between two positions on earth, 
 *  angles in degrees, result truncated to nearest km
 *  based on pyhamtools.locator http://pyhamtools.readthedocs.io/en/stable/reference.html#module-pyhamtools.locator
 */
  float d_lat, d_long, r_lat1, r_lat2, r_long1, r_long2, a, c;
  int km;
  float deg_per_rad, R;
  
  deg_per_rad = 57.29578; // degrees per radian
   
  R = 6371; // earth radius in km

  r_lat1  = lat1/deg_per_rad;
  r_long1 = long1/deg_per_rad;
  r_lat2  = lat2/deg_per_rad;
  r_long2 = long2/deg_per_rad;
  
  d_lat  = r_lat2 - r_lat1;
  d_long = r_long2-r_long1;  

  a = sin(d_lat/2) * sin(d_lat/2) + cos(r_lat1) * cos(r_lat2) * sin(d_long/2) * sin(d_long/2);
  c = 2 * atan2(sqrt(a), sqrt(1-a));
  km = R * c + 0.5; //distance in km (+0.5 to make truncation to integer into a round operation)
  
  return km;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

 void update_moon_position(){
// from K3NG
  //update_time();
   String textbuf;

  double RA, Dec, topRA, topDec, LST, HA;
//  update_time();
//  lcd.setCursor(0,2); lcd.print(Year);lcd.print(". ");lcd.print(Month);lcd.print(". ");lcd.print(Day);
//  lcd.setCursor(0,2); lcd.print(Hour);lcd.print(":");lcd.print(Minute);lcd.print(" = ");lcd.print(Hour + (Minute / 60.0) + (Seconds / 3600.0));
//  lcd.setCursor(0,3); textbuf = String(lon, 4);lcd.print(textbuf);lcd.print("N, ");
//  textbuf = String(latitude, 4);lcd.print(textbuf);lcd.print("E");
 
// UTC time:
  moon2(yearGPS, monthGPS, dayGPS, (hourGPS + (minuteGPS / 60.0) + (secondGPS / 3600.0)), lon, latitude, &RA, &Dec, &topRA, &topDec, &LST, &HA, &moon_azimuth, &moon_elevation, &moon_dist);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void nativeDate(void) {
/*
 * Print weekday in native (= non-English language)
 */
   //const char* myDays[] = {"Sondag", "Mandag", "Tirsdag", "Onsdag", "Torsdag", "Fredag", "Lordag"};
   
    const char* myDays[] = {"Son", "Man", "Tir", "Ons", "Tor", "Fre", "Lor"};
    int addr = weekday() - 1;
    strncpy(today, myDays[addr], 4);

    if (addr == 0 | addr == 6) { // søndag, lørdag
      lcd.print(today[0]);
      lcd.write(7); // ø
      lcd.print(today[2]);
    }
    else  lcd.print(today);
  
    lcd.print(" ");
    }

void code_Status(void) {
  lcd.setCursor(0, 0); lcd.print("LA3ZA GPS clock");
  lcd.setCursor(0, 1); lcd.print("Ver ");lcd.print(CODE_VERSION);
  lcd.setCursor(0, 3); lcd.print("GPS ");lcd.print(GPSBaud); lcd.print(" bps");
}

/* New 12.09.2018
 *  
 */

void MoonRiseTime(void) {
  
  lcd.setCursor(0, 0); lcd.print("*** Moon ***   ");
  //lcd.setCursor(0, 1); lcd.print("Moon rise time ");
  //lcd.setCursor(0, 2); lcd.print("Moon set time ");

  int Yyear, Mmonth;
  double Dday;
  
  Dday = (float)Day;
  Mmonth = (int)Month;
  Yyear = (int)Year;
  double JD = GetJulianDate(Yyear, Mmonth, Dday);
  
  lcd.setCursor(0, 1); lcd.print(JD);
  lcd.setCursor(0, 2); lcd.print(Yyear); lcd.print(" "); lcd.print(Mmonth);lcd.print(" ");lcd.print(Dday);
   
 // GetMoonRiseSetTimes(Yyear, Mmonth, Dday, UTCoffset ,latitude, lon, *packedRise,*riseAz,*packedset,*setAz);
  lcd.setCursor(0, 3); lcd.print(packedRise);
}
// ************************************************************************

  typedef struct
{
    double  rightascension;
    double  declination;
    double  parallax;
}
MOONLOCATION;
typedef struct
{
    int     hr;
    int     min;
    double  az;
    int     event;
}
MOONRISESET;

static double               VHz[3], RAn[3], Decl[3];
static MOONRISESET          MoonRise, MoonSet;

#define PI                  3.1415926535897932384626433832795
#define RAD                 (PI/180.0)
#define SMALL_FLOAT         (1e-12)



//int GetMoonRiseSetTimes
//(
//    int          year,
//    int          month,
//    int          day,
//    double       zone,                   // Timezone offset from UTC/GMT in hours
//    double       lat,                    // Latitude degress  N=> +, S=> -
//    double       lon,                    // longitude degress E=> +, W=> -
//    short        *packedRise,            // returned Moon Rise time
//    double       *riseAz,                // return Moon Rise Azimuth
//    short        *packedSet,             // returned Moon Set time
//    double       *setAz                  // return Moon Set Azimuth
//)
//{
//    int             k;
//    MOONLOCATION    mp[3];
//    double          localsidereal;
//    double          ph;
//    double          jd;
//
//    // Julian day relative to Jan 1.5, 2000
//    jd = GetJulianDate(year, month, (double)day) - 2451545.0;
//
//    localsidereal = localSiderealTime(lon, jd, zone); // local sidereal time
//
//    jd = jd - zone / 24.0;                      // get moon position at day start
//
//    for (k = 0; k < 3; k ++)
//    {
// //       mp[k] = GetMoonLocation(jd);
//        jd = jd + 0.5;
//    }
//
//    if (mp[1].rightascension <= mp[0].rightascension)
//        mp[1].rightascension = mp[1].rightascension + 2*PI;
//
//    if (mp[2].rightascension <= mp[1].rightascension)
//        mp[2].rightascension = mp[2].rightascension + 2*PI;
//
//    RAn[0] = mp[0].rightascension;
//    Dec[0] = mp[0].declination;
//
//    MoonRise.event = 0;                         // initialize
//    MoonSet.event  = 0;
//
//    for (k = 0; k < 24; k++)                    // check each hour of this day
//    {
//        ph = (k + 1.0)/24.0;
//
//        RAn[2] = moonInterpolate(mp[0].rightascension, 
//                                 mp[1].rightascension, 
//                                 mp[2].rightascension, 
//                                 ph);
//        Dec[2] = moonInterpolate(mp[0].declination, 
//                                 mp[1].declination, 
//                                 mp[2].declination, 
//                                 ph);
//
//        VHz[2] = moonTest(k, localsidereal, lat, mp[1].parallax);
//
//        RAn[0] = RAn[2];                       // advance to next hour
//        Dec[0] = Dec[2];
//        VHz[0] = VHz[2];
//    }
//
//    *packedRise = (short)(MoonRise.hr * 100 +  MoonRise.min);
//    if (riseAz != NULL)
//        *riseAz = MoonRise.az;
//
//    *packedSet = (short)(MoonSet.hr * 100 +  MoonSet.min);
//    if (setAz != NULL)
//        *setAz = MoonSet.az;
//
//    /*check for no MoonRise and/or no MoonSet  */
//
//    if (! MoonRise.event && ! MoonSet.event)  // neither MoonRise nor MoonSet
//    {
//        if (VHz[2] < 0)
//            *packedRise = *packedSet = -2;  // the moon never sets
//        else
//            *packedRise = *packedSet = -1;  // the moon never rises
//    }
//    else                                    //  check for MoonRise or MoonSet
//    {
//        if (! MoonRise.event)
//            *packedRise = -1;               // no MoonRise and the moon sets
//        else if (! MoonSet.event)
//            *packedSet = -1;                // the moon rises and never sets
//    }
//
//    return OK;
//}
  
 





  // Local methods:

static double GetJulianDate (int year, int month, double day)
{
    int     a, b;
    long    c, e;
    if (month < 3)
    {
        year--;
        month += 12;
    }
    if ((year > 1582) ||
        (year == 1582 && month > 10) ||
        (year == 1582 && month == 10 && day > 15))
    {
        a = (int)(year/100);
        b = (int)(2 - a+a/4);
        c = (long)(365.25 * year);
        e = (long)(30.6001 * (month + 1));
        return (b + c + e + day + 1720994.5);
    }
    else
    {
        return 0;
    }
}

static double GetSunPosition (double j)
{
    double      n, x, e, l, dl, v;
    int         i;

    n = 360/365.2422 * j;
    i = (int)(n/360);
    n = n - i*360.0;
    x = n - 3.762863;
    if (x < 0)
        x += 360;
    x *= RAD;
    e = x;
    do
    {
        dl = e - 0.016718 * sin(e) - x;
        e = e - dl/(1 - 0.016718 * cos(e));
    }
    while (fabs(dl) >= SMALL_FLOAT);

    v = 360/PI * atan(1.01686011182 * tan(e/2));
    l = v + 282.596403;
    i = (int)(l/360);
    l = l - i*360.0;
    return l;
}



////////////////////////////////////////////////////////////////////
static double GetMoonPosition (double j, double ls)
{

    double      ms, l, mm, n, ev, sms, ae, ec;
    int         i;

    ms = 0.985647332099*j - 3.762863;
    if (ms < 0)
        ms += 360.0;
    l = 13.176396*j + 64.975464;
    i = (int)(l/360);
    l = l - i*360.0;
    if (l < 0)
        l += 360.0;
    mm = l-0.1114041*j-349.383063;
    i = (int)(mm/360);
    mm -= i*360.0;
    n = 151.950429 - 0.0529539*j;
    i = (int)(n/360);
    n -= i*360.0;
    ev = 1.2739*sin((2*(l-ls)-mm)*RAD);
    sms = sin(ms*RAD);
    ae = 0.1858*sms;
    mm += ev-ae- 0.37*sms;
    ec = 6.2886*sin(mm*RAD);
    l += ev+ec-ae+ 0.214*sin(2*mm*RAD);
    l= 0.6583*sin(2*(l-ls)*RAD)+l;
    return l;
}

//////////////////////////////////////////////////////////////////////

static double GetMoonPhase (int year, int month, int day, int hour)
{
    double      j = GetJulianDate(year,month,(double)day+(double)hour/24.0)-2444238.5;
    double      ls = GetSunPosition(j);
    double      lm = GetMoonPosition(j, ls);
    double      t = lm - ls;
    double      retVal;

    retVal = (1.0 - cos((lm - ls)*RAD))/2;
    retVal *= 1000;
    retVal += 0.5;
    retVal /= 10;
    if (t < 0)
        t += 360;
    if (t > 180)
        retVal *= -1;

    return retVal;
}

// Return the sign of a number.
static int getSign( double num)
{
    if (num < 0)
        return(-1);
    if (num > 0)
        return(1);
    return(0);
}

// Local Sidereal Time for zone in Radians
static double localSiderealTime( double lon, double jd, double tz )
{

    double TU, lmst, gmst;

    double WV_SECONDS_IN_DAY=86400; // added 10.05.2019

    TU = jd / 36525.0;
    gmst = 24110.54841 + TU*(8640184.812866 + TU*(0.093104 + TU*(-6.2E-6)));
    lmst = gmst - 86636.6 * tz / 24.0 + WV_SECONDS_IN_DAY * lon / 360.0;
    lmst = lmst / WV_SECONDS_IN_DAY; // rotations
    lmst = lmst - floor(lmst); // fraction of a circle

    return lmst*2.0*PI;
}

/* 3-point interpolation */
static double moonInterpolate( double f0, double f1, double f2, double p )
{
    double  a, b, f;

    a = f1 - f0;
    b = f2 - f1 - a;
    f = f0 + p*(2*a + b*(2*p - 1));

    return f;
}

/*  test an hour for an event  */
static double moonTest(int k, double t0, double lat, double plx)
{
    double ha[3];
    double a, b, c, d, e, s, z;
    double hr, min, time;
    double az, hz, nz, dz;
    double K1 = 15 * PI / 180.0 * 1.0027379;
    double DR = PI / 180.0;

    if (RAn[2] < RAn[0])
        RAn[2] = RAn[2] + 2.0*PI;

    ha[0] = t0 - RAn[0] + k*K1;
    ha[2] = t0 - RAn[2] + k*K1 + K1;

    ha[1]  = (ha[2] + ha[0])/2.0;                /* hour angle at half hour */
    Decl[1] = (Decl[2] + Decl[0])/2.0;              /* declination at half hour */

    s = sin(DR*lat);
    c = cos(DR*lat);

    // refraction + sun semidiameter at horizon + parallax correction
    z = cos(DR*(90.567 - 41.685/plx));

    if (k <= 0)                                // first call of function
        VHz[0] = s * sin(Decl[0]) + c * cos(Decl[0]) * cos(ha[0]) - z;

    VHz[2] = s * sin(Decl[2]) + c * cos(Decl[2]) * cos(ha[2]) - z;

    if (getSign(VHz[0]) == getSign(VHz[2]))
        return VHz[2];                         // no event this hour

    VHz[1] = s * sin(Decl[1]) + c * cos(Decl[1]) * cos(ha[1]) - z;

    a = 2.0*VHz[2] - 4.0*VHz[1] + 2.0*VHz[0];
    b = 4.0*VHz[1] - 3.0*VHz[0] - VHz[2];
    d = b*b - 4.0*a*VHz[0];

    if (d < 0.0)
        return VHz[2];                         // no event this hour

    d = sqrt(d);
    e = (-b + d)/(2.0*a);

    if (( e > 1 )||( e < 0.0 ))
        e = (-b - d)/(2.0*a);

    time = k + e + 1.0/120.0;                      // time of an event + round up
    hr   = floor(time);
    min  = floor((time - hr)*60.0);

    hz = ha[0] + e * (ha[2] - ha[0]);            // azimuth of the moon at the event
    nz = -cos(Decl[1]) * sin(hz);
    dz = c * sin(Decl[1]) - s * cos(Decl[1]) * cos(hz);
    az = atan2(nz, dz)/DR;
    if (az < 0.0)
        az = az + 360.0;

    if ((VHz[0] < 0.0) && (VHz[2] > 0.0))
    {
        MoonRise.hr = (int)hr;
        MoonRise.min = (int)min;
        MoonRise.az = az;
        MoonRise.event = 1;
    }

    if ((VHz[0] > 0.0) && (VHz[2] < 0.0))
    {
        MoonSet.hr = (int)hr;
        MoonSet.min = (int)min;
        MoonSet.az = az;
        MoonSet.event = 1;
    }

    return VHz[2];
}


/*
* moon's position using fundamental arguments 
* (Van Flandern & Pulkkinen, 1979)
*/

//static MOONLOCATION GetMoonLocation(double jd)

//{
//    double          d, f, g, h, m, n, s, u, v, w;
//    MOONLOCATION    itshere;
//
//    h = 0.606434 + 0.03660110129 * jd;
//    m = 0.374897 + 0.03629164709 * jd;
//    f = 0.259091 + 0.03674819520 * jd;
//    d = 0.827362 + 0.03386319198 * jd;
//    n = 0.347343 - 0.00014709391 * jd;
//    g = 0.993126 + 0.00273777850 * jd;
//
//    h = h - floor(h);
//    m = m - floor(m);
//    f = f - floor(f);
//    d = d - floor(d);
//    n = n - floor(n);
//    g = g - floor(g);
//
//    h = h*2*PI;
//    m = m*2*PI;
//    f = f*2*PI;
//    d = d*2*PI;
//    n = n*2*PI;
//    g = g*2*PI;
//
//    v = 0.39558 * sin(f + n);
//    v = v + 0.08200 * sin(f);
//    v = v + 0.03257 * sin(m - f - n);
//    v = v + 0.01092 * sin(m + f + n);
//    v = v + 0.00666 * sin(m - f);
//    v = v - 0.00644 * sin(m + f - 2*d + n);
//    v = v - 0.00331 * sin(f - 2*d + n);
//    v = v - 0.00304 * sin(f - 2*d);
//    v = v - 0.00240 * sin(m - f - 2*d - n);
//    v = v + 0.00226 * sin(m + f);
//    v = v - 0.00108 * sin(m + f - 2*d);
//    v = v - 0.00079 * sin(f - n);
//    v = v + 0.00078 * sin(f + 2*d + n);
//
//    u = 1 - 0.10828 * cos(m);
//    u = u - 0.01880 * cos(m - 2*d);
//    u = u - 0.01479 * cos(2*d);
//    u = u + 0.00181 * cos(2*m - 2*d);
//    u = u - 0.00147 * cos(2*m);
//    u = u - 0.00105 * cos(2*d - g);
//    u = u - 0.00075 * cos(m - 2*d + g);
//
//    w = 0.10478 * sin(m);
//    w = w - 0.04105 * sin(2*f + 2*n);
//    w = w - 0.02130 * sin(m - 2*d);
//    w = w - 0.01779 * sin(2*f + n);
//    w = w + 0.01774 * sin(n);
//    w = w + 0.00987 * sin(2*d);
//    w = w - 0.00338 * sin(m - 2*f - 2*n);
//    w = w - 0.00309 * sin(g);
//    w = w - 0.00190 * sin(2*f);
//    w = w - 0.00144 * sin(m + n);
//    w = w - 0.00144 * sin(m - 2*f - n);
//    w = w - 0.00113 * sin(m + 2*f + 2*n);
//    w = w - 0.00094 * sin(m - 2*d + g);
//    w = w - 0.00092 * sin(2*m - 2*d);
//
//    s = w/sqrt(u - v*v);                  // compute moon's  ...  right ascension
//    itshere.rightascension = h + atan(s/sqrt(1 - s*s));
//
//    s = v/sqrt(u);                        // declination ...
//    itshere.declination = atan(s/sqrt(1 - s*s));
//
//    itshere.parallax = 60.40974 * sqrt( u );          // and parallax
//
//    return(itshere);
//}



//// Public methods:
//#define PHASE_STR_MAX       128
//char *lunarPhaseGet (char *increase, char *decrease, char *full)
//{
//    static char     phaseStr[PHASE_STR_MAX];
//    time_t          timeNow = time (NULL);
//    double          phase;
//    struct tm       bknTime;
//
//    localtime_r (&timeNow, &bknTime);
//
//    // compute the period value
//    phase = GetMoonPhase (bknTime.tm_year+1900, bknTime.tm_mon+1,
//                          bknTime.tm_mday, bknTime.tm_hour);
//
//    if (phase < 0)
//        snprintf(phaseStr, PHASE_STR_MAX-1, "%s %.0f%c %s", decrease, fabs(phase), '%', full);
//    else
//        snprintf(phaseStr, PHASE_STR_MAX-1, "%s %.0f%c %s", increase, phase, '%', full);
//
//    return phaseStr;
//}

// calculate MoonRise and MoonSet times
//
// Returns Rise and Set times times returned as packed time (hour*100 + minutes)
//
// packedRise > 0 && packedSet = -1 =>  the moon rises and never sets
// packedRise = -1 && packSet > 0   =>  no moon rise and the moon sets
// packedRise = packedSet = -1      =>  the moon never sets
// packedRise = packedSet = -2      =>  the moon never rises


//////////////////////////////////////////////
