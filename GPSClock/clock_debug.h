// Debugging options 
// Serial port output on/off:

//#define FEATURE_SERIAL_GPS // serial output for debugging of GPS, GPSInfo
//#define FEATURE_SERIAL_PLANETARY // serial output for debugging of planet predictions
//#define FEATURE_SERIAL_SOLAR // serial output for debugging of solar
//#define FEATURE_SERIAL_MOON // serial output for test of moon functions
//#define FEATURE_SERIAL_MENU // serial output for menu & general testing incl demo mode
//#define FEATURE_SERIAL_TIME // serial output for testing of time/time zone (*** not OK with metro M0 ??)
//#define FEATURE_SERIAL_MATH // serial output for debugging of math clock
//#define FEATURE_SERIAL_LUNARECLIPSE // serial output for debugging of moon eclipse
//#define FEATURE_SERIAL_EQUATIO // serial output for debugging of Equation of Time (solar time)  
//#define FEATURE_SERIAL_CHAR_SETS  // debug loading of new LCD character sets
//#define FEATURE_SERIAL_EEPROM  // debug EEPROM read
//#define FEATURE_SERIAL_LOAD_CHARACTERS  // check loading of new custom characters to LCD
//#define FEATURE_SERIAL_NEXTEVENTS  // debug NextEvent()
//#define FEATURE_FACTORIZATION        // ScreenFactorization clock debug

// In LocalUTC(), WordClockNorwegian(), LcdSolarRiseSet(), ISOHebIslam():
//#define FEATURE_DATE_PER_SECOND   // for stepping date/hour/min (86400/3600/60 sec step) quickly and check calender function (local time only)
#define SPEED_UP_FACTOR 3600.0 //3600, 86400;  // multiplied by date and added to time (seconds)

//#define STEP_FASTER  // for incrementing one minute per second in BigNumbers3(), 1 sec per 10 sec in LocalUTC(3) - factorization clock

//#define FEATURE_DAY_PER_SECOND    // for stepping through day names quickly

//#define FEATURE_INTERRUPTTEST   // debugging interrupt on Metro Express     

//#define DEBUG_MANUAL_POSITION // Used for testing of location functions, when one is elsewhere than the position of the GPS
                                // using position defined below
                                // Note that altitude = 0 during such a test.

#ifdef DEBUG_MANUAL_POSITION
  // test leading zeroes
  float latitude_manual  = 1.056360;
  float longitude_manual = -156.095540; 

  // London:
  //float latitude_manual  = 51.5; // N-S, degrees, South is negative
  //float longitude_manual = 0.0;  // E-W, degrees, West is negative

  // EM72hp
  //float latitude_manual  = 32.656360;
  //float longitude_manual = -85.395540; 

  // FN74wx
  //float latitude_manual  = 44.9791;
  //float longitude_manual = -64.11358; 

  // Edmund T. Tyson, N5JTY, Conversion between Geodetic and Grid Locator Systems, 
  // QST, Jan 1989, in DM72dx:
  //float latitude_manual  = 32.98; //32 58.8;
  //float longitude_manual = -105.7333; // -105 44.0

  // New Dehli:
  //float latitude_manual  = 28.6; 
  //float longitude_manual = 77.2; 
#endif


//#define FEATURE_FAKE_SERIAL_GPS_IN  // for faking GPS from a GPS simulator (https://github.com/panaaj/nmeasimulator)
// Demo 12.03.2022: didn't work properly with Time Zones & sidereal time, where clock does not advance when this mode is enabled

// Must only be used alone. Don't use if you do not exactly understand this switch:
// Switches GPS input from Serial1 to Serial to fake missing GPS coverage for demo purposes:
// Ex: enter this in appropriate COMx window of Arduino GUI at 9600 baud:
// $GPRMC,192447.781,A,5950.292010,N,01025.680006,E,0.0,17.4,010222,,,*12
