// debugging options 

//#define FEATURE_SERIAL_SOLAR // serial output for debugging of solar
//#define FEATURE_SERIAL_GPS // serial output for debugging of GPS
//#define FEATURE_SERIAL_MOON // serial output for test of moon functions
//#define FEATURE_SERIAL_MENU // serial output for menu & general testing
//#define FEATURE_SERIAL_TIME // serial output for testing of time

//#define DEBUG_MANUAL_POSITION // Used for testing of location functions, when one is elsewhere than that of the GPS
                              // Note that altitude = 0 during such a test.

// London:
//float latitude_manual  = 51.5; // N-S, degrees, South is negative
//float longitude_manual = 0.0; // E-W, degrees, West is negative

// EM72hp
float latitude_manual  = 32.656360;
float longitude_manual = -85.395540; 

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
