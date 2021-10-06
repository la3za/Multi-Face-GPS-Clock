////////////////////////////////////////////////////////////////////////////////////////////
/* Collection of helper functions //////////////////////////////////////////////////////////

GetNextRiseSet
MoonPhase
MoonPhaseAccurate
MoonWaxWane
MoonSymbol
update_moon_position
Maidenhead
analogbuttonread
locator_to_latlong
distance
decToBinary

printFixedWidth
LcdUTCTimeLocator
LcdShortDayDateTimeLocal
LcdSolarRiseSet

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
  dif = (dnow - REF_TIME); // Seconds since reference new moon
  dif = (dif % CYCLELENGTH) - 0.228535;
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

  binaryNum[0] = 0;   binaryNum[1] = 0;   binaryNum[2] = 0;   binaryNum[3] = 0;  binaryNum[4] = 0;
  // counter for binary array
  int i = 4;

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
  // call like this to print to lcd: printFixedWidth(lcd, val, 3);
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

void LcdShortDayDateTimeLocal(int lineno = 0, int moveLeft = 0) {
  // function that displays the following kind of info on lcd row "lineno"
  //  "Wed 8.9     22:30:46"

  char textbuffer[12]; // was [9] - caused all kinds of memory overwrite problems
  // get local time
  local = now() + UTCoffset * 60;
  Hour = hour(local);
  Minute = minute(local);
  Seconds = second(local);

  lcd.setCursor(0, lineno);

  // local date
  Day = day(local);
  Month = month(local);
  Year = year(local);
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
          lcd.print(static_cast<int>(Month)); lcd.print(DATE_SEP);
          lcd.print(static_cast<int>(Day));
        }
        else
        {
          lcd.print(static_cast<int>(Day)); lcd.print(DATE_SEP);
          lcd.print(static_cast<int>(Month));
        }
      }
      lcd.print("    "); // in order to erase remnants of long string as the month changes
      lcd.setCursor(10 - moveLeft, lineno);
      sprintf(textbuffer, "  %02d%c%02d%c%02d", Hour, HOUR_SEP, Minute, MIN_SEP, Seconds);
      lcd.print(textbuffer);
    }

/////////////////////////////////////////////////////////////////////////////////////////

void LcdSolarRiseSet(
  int lineno,                   // lcd line no 0, 1, 2, 3
  char RiseSetDefinition = ' ' // C - Civil, N - Nautical, A- Astronomical, default Actual
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

  if (RiseSetDefinition == 'A')     // astronomical: -18 deg
    mySunrise.Astronomical();
  else if (RiseSetDefinition == 'N') // Nautical:     -12 deg
    mySunrise.Nautical();
  else if (RiseSetDefinition == 'C') // Civil:        - 6 deg
    mySunrise.Civil();
  else mySunrise.Actual();           // Actual          0 deg


  // First: print sun rise time
  t = mySunrise.Rise(monthGPS, dayGPS); // Sun rise hour minute

  if (t >= 0) {
    h = mySunrise.Hour();
    m = mySunrise.Minute();

    if (RiseSetDefinition == ' ')
    {
      lcd.print("S "); lcd.write(UpArrow);
    }
    else if (RiseSetDefinition == 'C')
    {
      lcd.print("  "); lcd.write(DashedUpArrow);
    }
    else lcd.print("   ");

    printFixedWidth(lcd, h, 2);
    lcd.print(HOUR_SEP);
    printFixedWidth(lcd, m, 2, '0');
  }
  //lcd.print("       ");  // 4.2.2016 increased by one space. Unknown 't' before down arrow

  // Second: print sun set time

  t = mySunrise.Set(monthGPS, dayGPS); // Sun set time

  lcd.setCursor(9, lineno);
  if (RiseSetDefinition == ' ')         lcd.write(DownArrow);
  else if (RiseSetDefinition == 'C')  lcd.write(DashedDownArrow);
  else                                lcd.print(" ");

  if (t >= 0) {
    h = mySunrise.Hour();
    m = mySunrise.Minute();

    lcd.print(h, DEC);
    lcd.print(HOUR_SEP);
    if (m < 10) lcd.print("0");
    lcd.print(m, DEC);
  }

  // Middle text:
  //    ' ': Solar elevation right now
  //    'C': Time for local noon
  //    'N': Solar elevation at local noon
  //    'A': -

  lcd.setCursor(16, lineno);

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

  ///// Solar noon

  Sunrise my2Sunrise(latitude, lon, float(UTCoffset) / 60.);
  t = my2Sunrise.Noon(monthGPS, dayGPS);
  if (t >= 0) {
    hNoon = my2Sunrise.Hour();
    mNoon = my2Sunrise.Minute();
  }

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
      //          lcd.print(HOUR_SEP);  // save space by omitting ':' for solar noon
      if (mNoon < 10) lcd.print("0");
      lcd.print(mNoon, DEC);
    }

  }

  else if (RiseSetDefinition == 'N')
  {
    // find max solar elevation, i.e. at local noon

    c_time.dHours = hNoon - UTCoffset / 60.;
    c_time.dMinutes = mNoon;
    c_time.dSeconds = 0.0;

    sunpos(c_time, c_loc, &c_sposn);

    // Convert Zenith angle to elevation
    sun_elevation = 90. - c_sposn.dZenithAngle;
    sun_azimuth = c_sposn.dAzimuth;

    printFixedWidth(lcd, (int)float(sun_elevation), 3);
    lcd.write(DegreeSymbol);

  }
  else
    lcd.print("  ");
}

/// THE END ///
