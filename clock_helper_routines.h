////////////////////////////////////////////////////////////////////////////////////////////
/* Collection of helper functions //////////////////////////////////////////////////////////

GetNextRiseSet
MoonPhase
MoonPhaseAccurate
MoonWaxWane
MoonSymbol
update_moon_position

analogbuttonread

Maidenhead
locator_to_latlong
distance
decToBinary

printFixedWidth
LcdDate
LcdUTCTimeLocator
LcdShortDayDateTimeLocal
LcdSolarRiseSet

ComputeEasterDate
JulianToGregorian

MathPlus
MathMinus
MathMultiply
MathDivide

LcdMorse

///////////////////////////////////////////////////////////////////////////////////////////
*/





void GetNextRiseSet(
  short       *pRise,            // returned Moon Rise time
  double      *rAz,              // return Moon Rise Azimuth
  short       *pSet,             // returned Moon Set time
  double      *sAz,              // return Moon Set Azimuth
  int         *order              // 1 if rise is first, 2 if set is first
)
{
  short pLocal, pRise1, pSet1, pRise2, pSet2;
  double rAz1, sAz1, rAz2, sAz2;

  GetMoonRiseSetTimes(float(UTCoffset) / 60.0, latitude, lon, &pRise1, &rAz1, &pSet1, &sAz1);

  *pRise = pRise1;
  *rAz = rAz1;
  *pSet = pSet1;
  *sAz = sAz1;

  local = now() + UTCoffset * 60;

 // local = 1638052000; // 27.11.2021, ~23.30
  
  pLocal = 100 * hour(local) + minute(local);

#ifdef FEATURE_SERIAL_MOON
  //  Serial.print(F("zone "));Serial.println(zone);
  Serial.print(F("pRise, rAz  : ")); Serial.print(pRise1); Serial.print(F(", ")); Serial.println(rAz1);
  Serial.print(F("pSet, sAz   : ")); Serial.print(pSet1); Serial.print(F(", ")); Serial.println(sAz1);
  Serial.print(F("pLocal      : ")); Serial.println(pLocal);
#endif

  //  find rise/set times for next day also
  GetMoonRiseSetTimes(float(UTCoffset) / 60.0 - 24.0, latitude, lon, (short*) &pRise2, (double*) &rAz2, (short*) &pSet2, (double*) &sAz2);

#ifdef FEATURE_SERIAL_MOON
  Serial.print(F("pRise2, rAz2: ")); Serial.print(pRise2); Serial.print(F(", ")); Serial.println(rAz2);
  Serial.print(F("pSet2, sAz2 : ")); Serial.print(pSet2); Serial.print(F(", ")); Serial.println(sAz2);
#endif

  if ((pLocal > pRise1) | (pLocal > pSet1)) {
    if (pRise1 < pSet1)
    {
      *pRise = pRise2;
      *rAz = rAz2;
      *order = 2;  // set before rise
      if (pLocal > pSet1)
      {
        *pSet = pSet2;
        *sAz  = sAz2;
        *order = 1;  // rise before set
      }
    }
    else // pRise1 >= pSet1
    {
      *pSet = pSet2;
      *sAz  = sAz2;
      *order = 1; // rise is first
      if (pLocal > pRise1)
      {
        *pRise = pRise2;
        *rAz  = rAz2;
        *order = 2;  // set is first
      }
    }

  }
  else
  {
    if (*pRise < *pSet) *order = 1; // 1 if rise is first, 2 if set is first
    else                *order = 2;
  }

  /*
      pRise = pSet = -2;  // the moon never sets
      pRise = pSet = -1;  // the moon never rises
      pRise = -1;               // no MoonRise and the moon sets
      pSet = -1;                // the moon rises and never sets
  */

#ifdef FEATURE_SERIAL_MOON
  Serial.print(F("order: ")); Serial.println(*order);
  Serial.print(F("pRise, rAz: ")); Serial.print(*pRise); Serial.print(F(", ")); Serial.println(*rAz);
  Serial.print(F("pSet, sAz : ")); Serial.print(*pSet);  Serial.print(F(", ")); Serial.println(*sAz);
#endif
}



////////////////////////////////////////////////////////////////////////////////////

// from https://community.facer.io/t/moon-phase-formula-updated/35691

// (((#DNOW#-583084344)/2551442802)%1) // DNOW in ms, UNIX time
#define REF_TIME 583084
#define CYCLELENGTH 2551443 // 29.530588 days  

void MoonPhase(float& Phase, float& PercentPhase) {
  const float moonMonth = 29.530588; // .53059 varies from 29.18 to about 29.93 days
  long dif, moon;

  dif = (now() - REF_TIME); // Seconds since reference new moon
  moon = dif % CYCLELENGTH; // Seconds since last new moon

#ifdef FEATURE_SERIAL_MOON
  Serial.print(F("MoonPhase: now, dif, moon: "));
  Serial.println(now());
  Serial.println(dif);
  Serial.println(moon);
#endif

  Phase = abs(float(moon) / float(86400));               // in days

#ifdef FEATURE_SERIAL_MOON
  Serial.print(Phase); Serial.println(F(" days"));
#endif

  // The illumination approximates a sine wave through the cycle.
  // PercentPhase = 100 - 100*abs((Phase - 29.53/2)/(29.53/2)); // too big 39%, should be 31%
  // PercentPhase = 100*(sin((PI/2)*(1 - abs((Phase - moonMonth/2)/(moonMonth/2))))); // even bigger 57%
  // PercentPhase = 100*((2/pi)*asin(1 - abs((Phase - moonMonth/2)/(moonMonth/2)))) ;  //  too small 25%
  //http://www.dendroboard.com/forum/parts-construction/280865-arduino-moon-program.html

  PercentPhase = 50 * (1 - cos(2 * PI * Phase / moonMonth)); // in percent

#ifdef FEATURE_SERIAL_MOON
  Serial.print(PercentPhase); Serial.print(F("% "));
  Serial.print(Phase);        Serial.println(F(" days"));
#endif

}


// http://www.moongiant.com/moonphases/
// http://www.onlineconversion.com/unix_time.htm

//////////////////////////////////////////////////////////////////////////////////////////////////////
// More accurate formula?
/* Phase =

(((#DNOW#/2551442844-0.228535)
+0.00591997sin(#DNOW#/5023359217+3.1705094)
+0.017672776sin(#DNOW#/378923968-1.5388144)
-0.0038844429sin(#DNOW#/437435791+2.0017235)
-0.00041488sin(#DNOW#/138539900-1.236334))%1)

DNOW in ms, UNIX time

The result is a real number in the range 0 to 1
*/

void MoonPhaseAccurate(float& Phase, float& PercentPhase) {
  const float moonMonth = 29.53059; // varies from 29.18 to about 29.93 days
  long dif, moon;
  time_t dnow;
  
  dnow = now(); // Unix time in sec
  dif = (dnow - REF_TIME);                      // Seconds since reference new moon
  dif = (dif % CYCLELENGTH) - 0.228535;         // seconds since last new moon

  // These three terms can max contribute 0.006+0.018+0.004 = 0.028 seconds, i.e. they are insignificant:
  dif = dif + 0.00591997*sin(dnow/5023359+3.1705094);   
  dif = dif + 0.017672776*sin(dnow/378924-1.5388144);   
  dif = dif - 0.0038844429* sin(dnow/437436+2.0017235); 
  
  moon = dif - 0.00041488*sin(dnow/138540-1.236334); // Seconds since last new moon
 
#ifdef FEATURE_SERIAL_MOON
  Serial.print(F("MoonPhaseAccurate: now, dif, moon: "));
  Serial.println(now());
  Serial.println(dif);
  Serial.println(moon);
#endif

  Phase = abs(float(moon) / float(86400));               // in days

#ifdef FEATURE_SERIAL_MOON
  Serial.print(Phase); Serial.println(F(" days"));
#endif

  PercentPhase = 50 * (1 - cos(2 * PI * Phase / moonMonth)); // in percent

#ifdef FEATURE_SERIAL_MOON
  Serial.print(PercentPhase); Serial.print(F("% "));
  Serial.print(Phase);        Serial.println(F(" days"));
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
void MoonWaxWane(float Phase) {
  // lcd.print an arrow

  float CycleDays = CYCLELENGTH / 86400.;
  float delta; // = 1.0;

  delta = OPTION_DAYS_WITHOUT_MOON_ARROW/2;
  
  if (Phase <= delta)                                                       lcd.print(' '); // hardly visible
  else if ((Phase > delta) && (Phase < CycleDays / 2. - delta))                 lcd.write(DashedUpArrow); // Waxing moon
  else if ((Phase >= CycleDays / 2. - delta) && (Phase <= CycleDays / 2. + delta))  lcd.print(' '); // Full moon
  else if ((Phase > CycleDays / 2. + delta) && (Phase < CycleDays - delta))       lcd.write(DashedDownArrow); //  Waning moon
  else                                                                      lcd.print(' '); // hardly visible

}

///////////////////////////////////////////////////////////////////////////////
void MoonSymbol(float Phase) {
  // lcd.print an ( or ) symbol [

  float CycleDays = CYCLELENGTH / 86400.; // 29.5 days
  float delta; // 1.0
  
  delta = OPTION_DAYS_WITHOUT_MOON_SYMBOL/2.0;
  
  if (Phase <= delta)                                                       lcd.print(' '); // hardly visible
  else if ((Phase > delta) && (Phase < CycleDays / 2. - delta))                 lcd.print(')'); // Waxing moon: ny
  else if ((Phase >= CycleDays / 2. - delta) && (Phase <= CycleDays / 2. + delta))  lcd.print('o'); // Full moon
  else if ((Phase > CycleDays / 2. + delta) && (Phase < CycleDays - delta))       lcd.print('('); // Waning Moon: ne
  else                                                                      lcd.print(' '); // hardly visible
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void update_moon_position() {
  // from K3NG
  String textbuf;

  double RA, Dec, topRA, topDec, LST, HA;


  // UTC time:
  moon2(yearGPS, monthGPS, dayGPS, (hourGPS + (minuteGPS / 60.0) + (secondGPS / 3600.0)), lon, latitude, &RA, &Dec, &topRA, &topDec, &LST, &HA, &moon_azimuth, &moon_elevation, &moon_dist);

#ifdef FEATURE_SERIAL_MOON
  Serial.print(F("moon2: "));
  Serial.print(F("RA, DEC: "));
  Serial.print(RA); Serial.print(F(", ")); Serial.println(Dec);
#endif
}



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

///////////////////////////////////////////////////////////////////////////////////////////////////////

void Maidenhead(double lon, double latitude, char loc[7]) {
  int lonTrunc, latTrunc, lonFirst, latFirst, lonSec, latSec;

  // Locator calculation  
  // http://en.wikipedia.org/wiki/Maidenhead_Locator_System
  // https://ham.stackexchange.com/questions/221/how-can-one-convert-from-lat-long-to-grid-square

  // Examples for debugging:
  //   BH52jn: lon = -14.9176000; latitude = -17.438000;
  //   FN31pr: lon = -72.727260;  latitude = 41.714775;
  //   JO59jw; lon = 10.750000;   latitude = 59.945556;
  //   JJ00aa: lon = 0;          latitude = 0;

  lon += 180.0;     // only positive: 0...360
  latitude += 90.0; // only positive: 0...180
  lonTrunc = ((int)(lon * 20 )) / 20.0;
  latTrunc = ((int)(latitude * 10 )) / 10.0;

  lonFirst = lonTrunc / 20;
  latFirst = latTrunc / 10;

  lonSec = (lonTrunc % 20) / 2;
  latSec = (latTrunc % 10) / 1;
  
  loc[0] = 'A' + lonFirst;                                  // 1 field = 20 deg
  loc[1] = 'A' + latFirst;                                  // 1 field = 10 deg
  
  loc[2] = '0' + lonSec;                                    // 1 square = 2 deg
  loc[3] = '0' + latSec;                                    // 1 square = 1 deg
  
  loc[4] = 'a' + (lon      - lonFirst*20 - lonSec*2) * 12;  // 1 subsquare = 5' = 1/12 deg
  loc[5] = 'a' + (latitude - latFirst*10 - latSec  ) * 24;  // 1 subsquare = 2.5' = 1/24 deg
  
  loc[6] = '\0';
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
      Calculate distance between two positions on earth,
      angles in degrees, result truncated to nearest km
      based on pyhamtools.locator http://pyhamtools.readthedocs.io/en/stable/reference.html#module-pyhamtools.locator
  */
  float d_lat, d_long, r_lat1, r_lat2, r_long1, r_long2, a, c;
  int km;
  float deg_per_rad, R;

  deg_per_rad = 57.29578; // degrees per radian

  R = 6371; // earth radius in km. Earth is assumed a perfect sphere

  r_lat1  = lat1 / deg_per_rad;
  r_long1 = long1 / deg_per_rad;
  r_lat2  = lat2 / deg_per_rad;
  r_long2 = long2 / deg_per_rad;

  d_lat  = r_lat2 - r_lat1;
  d_long = r_long2 - r_long1;

  a = sin(d_lat / 2) * sin(d_lat / 2) + cos(r_lat1) * cos(r_lat2) * sin(d_long / 2) * sin(d_long / 2);
  c = 2 * atan2(sqrt(a), sqrt(1 - a));
  km = R * c + 0.5; //distance in km (+0.5 to make truncation to integer into a round operation)

  return km;
}



////////////////////////////////////////////////////////////////////////////////////////////////////

void decToBinary(int n, int binaryNum[])
{
  //   https://www.geeksforgeeks.org/program-decimal-binary-conversion/
  // array to store binary number
  // LSB in position 3 (was 0)

  binaryNum[0] = 0;   binaryNum[1] = 0;   binaryNum[2] = 0;   binaryNum[3] = 0;  binaryNum[4] = 0; binaryNum[5] = 0;
  // counter for binary array
  int i = 5;

  while (n > 0) {

    // storing remainder in binary array
    binaryNum[i] = n % 2;
    n = n / 2;
    i--;
  }
}

//////////////////////////////////////////////////

void printFixedWidth(Print &out, int number, byte width, char filler = ' ') {
  int temp = number;
  //
  // call like this to print number to lcd: printFixedWidth(lcd, val, 3);
  // or for e.g. minutes printFixedWidth(lcd, minute, 2, '0')
  //
  // Default filler = ' ', can also be set to '0' e.g. for clock
  // If filler = ' ', it handles negative integers: width = 5 => '   -2'
  // but not if filler = '0': width = 5 => '000-2'
  //
  // https://forum.arduino.cc/t/print-lcd-text-justify-from-right/398351/5
  // https://forum.arduino.cc/t/u8glib-how-to-display-leading-zeroes/396694/3

  //do we need room for a minus?
  if (number < 0) {
    width--;
  }

  //see how wide the number is
  if (temp == 0) {
    width--;
  }
  else
  {
    while (temp && width) {
      temp /= 10;
      width--;
    }
  }

  //start by printing the rest of the width with filler symbol
  while (width) {
    out.print(filler);
    width--;
  }

  out.print(number); // finally print signed number
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void LcdUTCTimeLocator(int lineno)
// function that displays the following kind of info on lcd row "lineno"
//  "22:30:46 UTC  JO59fu"
{
  char textbuffer[15];

  if (gps.time.isValid()) {
    lcd.setCursor(0, lineno);
    //       sprintf(textbuffer, "%02d:%02d:%02d UTC  ", hourGPS, minuteGPS, secondGPS);
    sprintf(textbuffer, "%02d%c%02d%c%02d UTC ", hourGPS, HOUR_SEP, minuteGPS, MIN_SEP, secondGPS);
    lcd.print(textbuffer);
  }

  if (gps.satellites.isValid()) {

#ifndef DEBUG_MANUAL_POSITION
    latitude = gps.location.lat();
    lon = gps.location.lng();
#else
    latitude = latitude_manual;
    lon      = longitude_manual;
#endif

    char locator[7];
    Maidenhead(lon, latitude, locator);
    lcd.setCursor(14, lineno);
    lcd.print(locator);
  }
}

//------------------------------------------------------------------

void LcdDate(int Day, int Month, int Year=0) // print date, either day-month or day-month-year according to specified format
{
  if (DATEORDER == 'B')
    {
      if (Year !=0) 
      { 
        printFixedWidth(lcd, Year, 4); lcd.print(DATE_SEP);
      }
      printFixedWidth(lcd, Month, 2, '0'); lcd.print(DATE_SEP);
      printFixedWidth(lcd, Day, 2, '0');
    }
    else if (DATEORDER == 'M')
    {
      printFixedWidth(lcd, Month, 2, '0'); lcd.print(DATE_SEP);
      printFixedWidth(lcd, Day, 2, '0'); 
      if (Year !=0) 
      {
        lcd.print(DATE_SEP);printFixedWidth(lcd, Year, 4);
      }
    }
    else
    {
      printFixedWidth(lcd, Day, 2, '0'); lcd.print(DATE_SEP);
      printFixedWidth(lcd, Month, 2, '0'); 
      if (Year !=0) 
      {
        lcd.print(DATE_SEP);printFixedWidth(lcd, Year, 4);
      }
    }
  
}


//------------------------------------------------------------------

void LcdShortDayDateTimeLocal(int lineno = 0, int moveLeft = 0) {
  // function that displays the following kind of info on lcd row "lineno"
  //  "Wed 20.10     22:30:46" - date separator in fixed location, even if date is ' 9.8'

  char textbuffer[12]; // was [9] - caused all kinds of memory overwrite problems
  // get local time
  local = now() + UTCoffset * 60;
  Hour = hour(local);
  Minute = minute(local);
  Seconds = second(local);
  
  // local date
  Day = day(local);
  Month = month(local);
  Year = year(local);
    
  lcd.setCursor(0, lineno);
  if (dayGPS != 0)
  {
     #ifdef FEATURE_DAY_NAME_NATIVE
          nativeDay(local);
     #else
          sprintf(textbuffer, "%3s", dayShortStr(weekday(local)));
          #ifdef FEATURE_SERIAL_MENU
              Serial.println("LcdShortDayDateTimeLocal: ");
              Serial.println("  weekday(local), today");
              Serial.println(weekday(local));
              Serial.println(textbuffer);
          #endif
          lcd.print(textbuffer); lcd.print(" ");
      #endif
     
        lcd.setCursor(4, lineno);
    
        if ((DATEORDER == 'M') | (DATEORDER == 'B'))
        {
// modified so month takes up a fixed space without a leading zero:
//          lcd.print(static_cast<int>(Month)); lcd.print(DATE_SEP);
            printFixedWidth(lcd, Month, 2,' '); lcd.print(DATE_SEP);
            lcd.print(static_cast<int>(Day));
        }
        else
        {
// modified so day takes up a fixed space without a leading zero:
//        lcd.print(static_cast<int>(Day)); lcd.print(DATE_SEP);
          printFixedWidth(lcd, Day, 2,' '); lcd.print(DATE_SEP);
          lcd.print(static_cast<int>(Month));
        }
      }
      lcd.print("    "); // in order to erase remnants of long string as the month changes
      lcd.setCursor(11 - moveLeft, lineno);
      sprintf(textbuffer, " %02d%c%02d%c%02d", Hour, HOUR_SEP, Minute, MIN_SEP, Seconds); // corrected 18.10.2021
      lcd.print(textbuffer);
    }

/////////////////////////////////////////////////////////////////////////////////////////

void LcdSolarRiseSet(
  int lineno,                       // lcd line no 0, 1, 2, 3
  char RiseSetDefinition = ' ',     // default - Actual, C - Civil, N - Nautical, A - Astronomical,  O - nOon info, Z - aZ, el info
  int  ScreenMode = ScreenLocalSun  // One of ScreenLocalSun, ScreenLocalSunSimpler, ScreenLocalSunMoon, ScreenLocalSunAzEl
)
{
  // Horizon for solar rise/set: Actual (0 deg), Civil (-6 deg), Nautical (-12 deg), Astronomical (-18 deg)

  lcd.setCursor(0, lineno);

  // create a Sunrise object
  Sunrise mySunrise(latitude, lon, float(UTCoffset) / 60.);

  byte h, m;
  int hNoon, mNoon;
  int t; // t= minutes past midnight of sunrise (6 am would be 360)
  cTime c_time;
  cLocation c_loc;
  cSunCoordinates c_sposn;

  // https://www.timeanddate.com/astronomy/different-types-twilight.html
  if (RiseSetDefinition == 'A')     // astronomical: -18 deg
  // "During astronomical twilight, most celestial objects can be observed in the sky. However, the atmosphere still scatters and 
  // refracts a small amount of sunlight, and that may make it difficult for astronomers to view the faintest objects."
        mySunrise.Astronomical();
        
  else if (RiseSetDefinition == 'N') // Nautical:     -12 deg
  // "nautical twilight, dates back to the time when sailors used the stars to navigate the seas. 
  // During this time, most stars can be easily seen with naked eyes, and the horizon is usually also visible in clear weather conditions."
        mySunrise.Nautical();
        
  else if (RiseSetDefinition == 'C') // Civil:        - 6 deg 
  // "enough natural sunlight during this period that artificial light may not be required to carry out outdoor activities."
        mySunrise.Civil();
        
  else  mySunrise.Actual();           // Actual          0 deg


  // First: print sun rise time
  t = mySunrise.Rise(monthGPS, dayGPS); // Sun rise hour minute

  if (t >= 0) {
    h = mySunrise.Hour();
    m = mySunrise.Minute();

    if (ScreenMode == ScreenLocalSunSimpler | ScreenMode == ScreenLocalSunAzEl) lcd.print(" "); // to line up rise time with date on line above
    
    if (RiseSetDefinition == ' ')
    {
      lcd.print("  "); lcd.write(UpArrow);
    }
    else if (RiseSetDefinition == 'C')
    {
      lcd.print("  "); lcd.write(DashedUpArrow);
    }
    else lcd.print("   ");

    if (lineno==1) 
    {
        lcd.setCursor(0, lineno);
        lcd.print("S ");
    }

if (RiseSetDefinition == ' ' |RiseSetDefinition == 'C'|RiseSetDefinition == 'N'|RiseSetDefinition == 'A')
   {
    
    if (ScreenMode == ScreenLocalSunSimpler | ScreenMode == ScreenLocalSunAzEl) lcd.setCursor(4, lineno);
    else lcd.setCursor(3, lineno);
    
    printFixedWidth(lcd, h, 2);
    lcd.print(HOUR_SEP);
    printFixedWidth(lcd, m, 2, '0');
  }
  }
  
  // Second: print sun set time

  t = mySunrise.Set(monthGPS, dayGPS); // Sun set time

  lcd.setCursor(9, lineno);
  if (ScreenMode == ScreenLocalSunSimpler| ScreenMode == ScreenLocalSunAzEl) lcd.print("  ");
  if (RiseSetDefinition == ' ')       lcd.write(DownArrow);
  else if (RiseSetDefinition == 'C')  lcd.write(DashedDownArrow);
  else                                lcd.print(" ");

  if (t >= 0) {
    h = mySunrise.Hour();
    m = mySunrise.Minute();

    if (RiseSetDefinition == ' ' |RiseSetDefinition == 'C'|RiseSetDefinition == 'N'|RiseSetDefinition == 'A')
    {
      lcd.print(h, DEC);
      lcd.print(HOUR_SEP);
      if (m < 10) lcd.print("0");
      lcd.print(m, DEC);
      if (ScreenMode == ScreenLocalSunSimpler) 
      {
        lcd.print("  ");
        lcd.print(RiseSetDefinition); // show C, N, A to the very right
      }
    }
  }

SolarElevation:

  /////// Solar elevation //////////////////

  double dElevation;
  double dhNoon, dmNoon;

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

  float sun_azimuth = 0;
  float sun_elevation = 0;

  sunpos(c_time, c_loc, &c_sposn);

  // Convert Zenith angle to elevation
  sun_elevation = 90. - c_sposn.dZenithAngle;
  sun_azimuth = c_sposn.dAzimuth;

   if (RiseSetDefinition == 'Z') // print current aZimuth, elevation
      {
        lcd.setCursor(3, lineno);
        lcd.print("El ");
        printFixedWidth(lcd, (int)float(sun_elevation), 3);
        lcd.write(DegreeSymbol);
        lcd.setCursor(11, lineno);
        lcd.print("Az ");
        printFixedWidth(lcd, (int)float(sun_azimuth), 3);
        lcd.write(DegreeSymbol);
        lcd.print("  ");         
      }

  ///// Solar noon

  Sunrise my2Sunrise(latitude, lon, float(UTCoffset) / 60.);
  t = my2Sunrise.Noon(monthGPS, dayGPS);
  if (t >= 0) {
    hNoon = my2Sunrise.Hour();
    mNoon = my2Sunrise.Minute();
  }

  // find max solar elevation, i.e. at local noon
  
  c_time.dHours = hNoon - UTCoffset / 60.;
  c_time.dMinutes = mNoon;
  c_time.dSeconds = 0.0;

  sunpos(c_time, c_loc, &c_sposn);

  // Convert Zenith angle to elevation
  float sun_elevationNoon = 90. - c_sposn.dZenithAngle;
  float sun_azimuthNoon = c_sposn.dAzimuth;

 // Right margin text 
  //    ' ': Solar elevation right now
  //    'C': Time for local noon
  //    'N': Solar elevation at local noon
  //    'A': -

  if (ScreenMode == ScreenLocalSun | ScreenMode == ScreenLocalSunMoon)
  {
  // position rightmargin info here:
    lcd.setCursor(16, lineno);
 
    if (RiseSetDefinition == ' ')
    { 
      printFixedWidth(lcd, (int)float(sun_elevation), 3);
      lcd.write(DegreeSymbol);
    }
    else if (RiseSetDefinition == 'C')
    {
      if (t >= 0) {
        if (hNoon < 10) lcd.setCursor(16, 2); // added 4.7.2016 to deal with summer far North
        lcd.print(hNoon, DEC);
        //          lcd.print(HOUR_SEP);  
        if (mNoon < 10) lcd.print("0");
        lcd.print(mNoon, DEC);
      }
    }
    else if (RiseSetDefinition == 'N')
    {
      // Noon data:
  
      printFixedWidth(lcd, (int)float(sun_elevationNoon), 3);
      lcd.write(DegreeSymbol);
    }  
    
}      // if (ScreenMode == ...)


        if (RiseSetDefinition == 'O') // print sun's data at nOon
        {
          lcd.setCursor(3, lineno);
          lcd.print("El ");
          printFixedWidth(lcd, (int)float(sun_elevationNoon), 3);
          lcd.write(DegreeSymbol); 
          lcd.print(" ");   //        lcd.print(" @");
          lcd.setCursor(12, lineno);
          printFixedWidth(lcd, hNoon, 2);
          lcd.print(HOUR_SEP); 
          printFixedWidth(lcd, mNoon, 2,'0');         
          lcd.print("  ");       
        }
}

////////////////////////////////////////////////////////////////////////////////
void ComputeEasterDate( // find date of Easter Sunday
  int yr,         // input value for year
  int K, int E,   // see below
  int *PaschalFullMoon, // date in March for Paschal Full Moon (32 <=> 1 April and so on)
  int *EasterDate,      // date in March, April, May. Range: 22. March-25. April  
                        // Range: 4. April-8. May for Julian calendar in Gregorian dates
  int *EasterMonth      // 3, 4, or 5
)
/* Parameters K, E:
 *  Julian calendar: 
 *  K=-3 E=-1 
 *  
 *  Gregorian calendar: 
 *  1583-1693 K= l E=-8 
 *  1700-1799 K= 0 E=-9 
 *  1800-1899 K=-l E=-9 
 *  1900-2099 K=-2 E=-10 
 *  2100-2199 K=-3 E=-10
 */ 
{
  /*
  Werner Bergmann, Easter and the Calendar: The Mathematics of Determining 
  a Formula for the Easter Festival to Medieval Computing, 
  Journal for General Philosophy of Science / 
  Zeitschrift für allgemeine Wissenschaftstheorie , 1991
  Algorithm from pages 28-29, Bergmann

  Gregorian: Dates are given in Gregorian dates, i.e. 13 days before Julian dates at present
  
  Agrees with Julian and Gregorian dates here: https://webspace.science.uu.nl/~gent0113/easter/easter_text4c.htm
  but not with Julian dates here: https://en.wikipedia.org/wiki/List_of_dates_for_Easter 
  as 13 needs to be added to date in order to give Julian Easter in Gregorian dates
  */
  
  int x,y,z, n, concurrent, epact, ES; 
 
  // I. Concurrent
  x = yr-8;
  y = floor(x/4);         // leap year cycle
  z = x%4;
  concurrent = (x+y+K)%7;
  
  // II. Epact
  y = yr%19;            // 19 year periodicity of moon
  epact = (y*11 + E)%30;
  
  // III. Paschal Full Moon 
  if (epact <= 14)  *PaschalFullMoon = 21+14-epact;
  else              *PaschalFullMoon = 21+44-epact;
  
  // IV. Easter Sunday
  // 21 + (8 - Cone.) + n * 7 > paschal full moon
  
  n = ceil(float(*PaschalFullMoon-21-(8-concurrent))/7 + 0.001); // added 0.001 because > is the condition, not >=. Check year 2001
  *EasterDate = 21+(8-concurrent)+n*7;
  if (*EasterDate <= 31)
  {
    *EasterMonth = 3;
    *EasterDate = *EasterDate;
  }
  else if (*EasterDate <=61)
  {
    *EasterMonth = 4;
    *EasterDate = *EasterDate-31;
  }
  else
  {
    *EasterMonth = 5;
    *EasterDate = *EasterDate-61;
  }

//  Serial.print(F("concurrent, epact, PaschalFullMoon, n "));Serial.print(concurrent); Serial.print(" "); 
//  Serial.print(epact);Serial.print(" "); Serial.print(*PaschalFullMoon);Serial.print(" "); Serial.println(n);
  
 }

 ///////////////////////////////////////////////////////////////////////////////////////////////

 void JulianToGregorian(int *Date, int *Month)
  {
  // Add 13 days to get the dates in Gregorian notation (valid this century++):
  // only valid for dates in March and April

    *Date = *Date + 13;
  
  if (*Date > 31 & *Month == 3)
  {
    *Date = *Date - 31;
    *Month = *Month + 1;
  }
  else if (*Date > 30 & *Month == 4)
  {
    *Date = *Date - 30;
    *Month = *Month + 1; 
  }
 } 

 ///////////////////////////////////////////////////////////////////////////////////////////////

// void MathPlusMinus(int Term0,       // input number 
//                      int *Term1,      // output factor one
//                      int *Term2,      // output factor two 
//                      int OptionMath // 0, 1, ...
// )
// {
//  // random(min,max)
//  // min: lower bound of the random value, inclusive (optional).
//  // max: upper bound of the random value, exclusive.
//
//  if (OptionMath == 1) // +/- with equal probabilities
//    {
//        *Term1 = 0;  // avoid Term1 = 0
//        while (*Term1 == 0 | *Term1 == Term0)  *Term1 = random(max(0,Term0-9), Term0+10); // limit  term to +/-1...9: 
//    }
//    else if (OptionMath == 0) // +
//    {
//        // hour = 0 => 0+0
//        *Term1 = random(max(0,Term0-9), Term0+1);   // limit term1 to 0...term0 for OptionMath=0
//    }
//  
//  *Term2 = Term0 - *Term1;
// }

///////////////////////////////////////////////////////////////////////////////////////////////

 void MathPlus(int Term0,       // input number 
                      int *Term1,      // output factor one
                      int *Term2      // output factor two 
 )
 {
  #ifdef FEATURE_SERIAL_MATH
      Serial.print("*** MathPlus:                 + ");Serial.println(Term0);
  #endif
  // random(min,max)
  // min: lower bound of the random value, inclusive (optional).
  // max: upper bound of the random value, exclusive.

  // hour = 0 => 0+0: must allow zero
  *Term1 = random(max(0,Term0-9), Term0+1);   // limit Term1 to 1...Term0
  *Term2 = Term0 - *Term1;
  if (*Term2 > *Term1) // sort to get smallest last
    {
      int tmp = *Term1; *Term1 = *Term2; *Term2 = tmp;
    }  
 }

///////////////////////////////////////////////////////////////////////////////////////////////

 void MathMinus(int Term0,       // input number 
                      int *Term1,      // output factor one
                      int *Term2      // output factor two 
 )
 {
  #ifdef FEATURE_SERIAL_MATH
      Serial.print("*** MathMinus:                - ");Serial.println(Term0);
  #endif
  // random(min,max)
  // min: lower bound of the random value, inclusive (optional).
  // max: upper bound of the random value, exclusive.

  // avoid Term1=Term0, i.e. 0-0
  *Term1 = random(max(1,Term0+1), Term0+10); // limit  Term1 to 1 ... Term0+9 
  *Term2 = Term0 - *Term1;
        #ifdef FEATURE_SERIAL_MATH
          Serial.print("Term2, Term1 ");Serial.print(*Term1);Serial.print("  ");Serial.println(*Term2);
        #endif 
 }

 /////////////////////////////////////////////////////////////////////////////////////
 void MathMultiply( int Term0,       // input number 
                      int *Term1,      // output factor one
                      int *Term2      // output factor two 
 )
 {
  int i, j, k;
   // must have one more term than highest possible value of minute:
  int possible[] = {1, 2, 3, 4, 5, 7, 8, 11, 12, 13, 15, 16, 17, 18, 19, 20, 23,24,25,26,27,28, 29,30, 31, 37, 41, 43, 47, 53, 59, 61};
  int factors[15];

      #ifdef FEATURE_SERIAL_MATH
            Serial.print("*** MathMultiply:             * ");Serial.println(Term0);
      #endif
  
      if (Term0 !=0)
      {
        
        j = 0;
        for (i = 0; possible[i] <= Term0; i++)
         {
          
          if (Term0%possible[i] == 0)
          {
            factors[j] = possible[i];
            j = j+1; 
            #ifdef FEATURE_SERIAL_MATH
              Serial.print("i, possible[i] ");
              Serial.print(i);Serial.print("  ");
              Serial.println(possible[i]);
            #endif
          }
         }
    // random(min,max)
    // min: lower bound of the random value, inclusive (optional).
    // max: upper bound of the random value, exclusive.
  
    #ifdef FEATURE_SERIAL_MATH
      Serial.print("factors[i] ");Serial.print(factors[0]);Serial.print("  ");Serial.print(factors[1]);Serial.print("  ");
      Serial.print(factors[2]);Serial.print("  ");Serial.println(factors[3]);
    #endif 
    // choose randomly among possible factors stored in factors:
        k = random(0,j);
        *Term1 = factors[k]; 
      #ifdef FEATURE_SERIAL_MATH
        Serial.print("k, Term1 ");Serial.print(k);Serial.print("  ");Serial.println(*Term1);
      #endif 
      *Term2 = Term0 / *Term1; 
      if (*Term2 > *Term1) // sort to get smallest last
        {
          int tmp = *Term1; *Term1 = *Term2; *Term2 = tmp;
        }   
      }
      else //Term0 == 0
      {
        *Term1=0;
        *Term2=0;
      }  
 }

void MathDivide(int Term0,      // input number 
                int *Term1,     // output factor one: Term0 = Term1/Term2
                int *Term2      // output factor two 
)
{
  int factors[15];
  int y, n, j, k;

  #ifdef FEATURE_SERIAL_MATH
      Serial.print("*** MathDivide:               : ");Serial.println(Term0);
  #endif


  if (Term0 !=0)
      {
        j = 0;
        // find divisor > 0, < sqrt() and with only two digits in first factor:
        for (n = 1; (n <= sqrt(Term0) & Term0*n < 100); n++)
        {
          if (Term0%n == 0)
                {
                  factors[j] = n;
                  #ifdef FEATURE_SERIAL_MATH
                    Serial.print("j, factors[j] ");
                    Serial.print(j);  Serial.print("  ");
                    Serial.println(factors[j]);
                  #endif
                  j = j+1; 
                }
        }
        // choose randomly among possible factors stored in factors:
              k = random(0,j);
              *Term2 = factors[k]; 
              #ifdef FEATURE_SERIAL_MATH
                Serial.print("k, Term2 ");Serial.print(k);Serial.print("  ");Serial.println(*Term2);
              #endif 
            *Term1 = Term0 * *Term2; 
      }   
     else // Term0 = 0
        {
           *Term1 = 0;
           *Term2 = random(1,10);
        }
}

////////////////////////////////////////////////////////////////////////////
// https://forum.arduino.cc/t/printing-a-double-variable/44327

void printDouble( double val, unsigned int precision){

// prints val with number of decimal places determine by precision
// NOTE: precision is 1 followed by the number of zeros for the desired number of decimial places
// example: printDouble( 3.1415, 100); // prints 3.14 (two decimal places)

    Serial.print (int(val));  //prints the int part
    Serial.print("."); // print the decimal point
    unsigned int frac;
    if(val >= 0)
        frac = (val - int(val)) * precision;
    else
        frac = (int(val)- val ) * precision;
    Serial.println(frac,DEC) ;
} 


void LcdMorse(int num)
{
  switch (num) {
    case 1:
      lcd.print((char)165);lcd.print("-");lcd.print("-");lcd.print("-");lcd.print("-");
      break;
    case 2:
      lcd.print((char)165);lcd.print((char)165);lcd.print("-");lcd.print("-");lcd.print("-");
      break;
    case 3:
      lcd.print((char)165);lcd.print((char)165);lcd.print((char)165);lcd.print("-");lcd.print("-");
      break; 
    case 4:
      lcd.print((char)165);lcd.print((char)165);lcd.print((char)165);lcd.print((char)165);lcd.print("-");
      break;  
    case 5:
      lcd.print((char)165);lcd.print((char)165);lcd.print((char)165);lcd.print((char)165);lcd.print((char)165);
      break;
    case 6:
      lcd.print("-");lcd.print((char)165);lcd.print((char)165);lcd.print((char)165);lcd.print((char)165);
      break; 
    case 7:
      lcd.print("-");lcd.print("-");lcd.print((char)165);lcd.print((char)165);lcd.print((char)165);
      break; 
    case 8:
      lcd.print("-");lcd.print("-");lcd.print("-");lcd.print((char)165);lcd.print((char)165);
      break; 
    case 9:
       lcd.print("-");lcd.print("-");lcd.print("-");lcd.print("-");lcd.print((char)165);
       break; 
    default:
      lcd.print("-");lcd.print("-");lcd.print("-");lcd.print("-");lcd.print("-");
      break;                
  }
}
 /// THE END ///
