////////////////////////////////////////////////////////////////////////////////////////////
/* Collection of helper functions //////////////////////////////////////////////////////////

EEPROMMyupdate

GetNextRiseSet
MoonPhase
MoonPhaseAccurate
MoonWaxWane
MoonSymbol
UpdateMoonPosition

AnalogButtonRead

Maidenhead
LocatorToLatLong
Distance
DecToBinary

PrintFixedWidth
LcdDate
LcdDateString
LcdUTCTimeLocator
LcdShortDayDateTimeLocal
LcdTimeLocalShortDayDate
LcdSolarRiseSet

ComputeEasterDate
JulianToGregorian

MathPlus
MathMinus
MathMultiply
MathDivide

printDouble
LcdMorse

LCDPlanetData

LCDChemicalElement
LCDChemicalGroupPeriod
LCDChemicalElementName

readIntFromEEPROM
updateIntIntoEEPROM
resetFunc
InitScreenSelect
RotarySecondarySetup
RotarySetup

GPSParse

ArrowCharacters
BarCharacters

ThreeWideDigits
draw_digit

ThreeHighDigits2
doNumber2

gapLessCharacters
gapLessBar

readPersonEEPROM
bubbleSort

EquinoxSolstice
calculateDayOfYear

isSquare
isCube
isFactorial
isPower2
isFibonacci
update_line
print_factors

///////////////////////////////////////////////////////////////////////////////////////////
*/
void CodeStatus(void);        // forward declaration
void Progress(void);          // forward declaration
void DemoClock(byte inDemo);  // forward declaration

void EEPROMMyupdate(int address, byte val, byte commit) // replaces EEPROM.update as it won't work for Metro
{ 

    if( EEPROM.read(address) != val ){
      EEPROM.write(address, val);
    #ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
      if (commit == 1)  EEPROM.commit();
    #endif
    }
}

////


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

  GetMoonRiseSetTimes(float(utcOffset) / 60.0, latitude, lon, &pRise1, &rAz1, &pSet1, &sAz1);

  *pRise = pRise1;
  *rAz = rAz1;
  *pSet = pSet1;
  *sAz = sAz1;

  localTime = now() + utcOffset * 60;

 // local = 1638052000; // 27.11.2021, ~23.30
  
  pLocal = 100 * hour(localTime) + minute(localTime);

#ifdef FEATURE_SERIAL_MOON
  //  Serial.print(F("zone "));Serial.println(zone);
  Serial.print(F("pRise, rAz  : ")); Serial.print(pRise1); Serial.print(F(", ")); Serial.println(rAz1);
  Serial.print(F("pSet, sAz   : ")); Serial.print(pSet1); Serial.print(F(", ")); Serial.println(sAz1);
  Serial.print(F("pLocal      : ")); Serial.println(pLocal);
#endif

  //  find rise/set times for next day also
  GetMoonRiseSetTimes(float(utcOffset) / 60.0 - 24.0, latitude, lon, (short*) &pRise2, (double*) &rAz2, (short*) &pSet2, (double*) &sAz2);

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
#define CYCLELENGTH 2551443 // sec <=> 29.53059 days: only defined here

void MoonPhase(float& Phase, float& PercentPhase) {
//  const float moonMonth = 29.530588; // varies from 29.18 to about 29.93 days
  const float moonMonth = CYCLELENGTH / 86400.; // 29.53 days = average, varies from 29.18 to about 29.93 days
  
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
//  const float moonMonth = 29.53059; // varies from 29.18 to about 29.93 days
  const float moonMonth = CYCLELENGTH / 86400.; // 29.53 days
  
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
  else if ((Phase > delta) && (Phase < CycleDays / 2. - delta))                 lcd.write(DASHED_UP_ARROW); // Waxing moon
  else if ((Phase >= CycleDays / 2. - delta) && (Phase <= CycleDays / 2. + delta))  lcd.print(' '); // Full moon
  else if ((Phase > CycleDays / 2. + delta) && (Phase < CycleDays - delta))       lcd.write(DASHED_DOWN_ARROW); //  Waning moon
  else                                                                      lcd.print(' '); // hardly visible

}

///////////////////////////////////////////////////////////////////////////////
void MoonSymbol(float Phase) {
  // lcd.print an ( or ) symbol [

  float CycleDays = CYCLELENGTH / 86400.; // 29.53 days
  float delta; // 1.0
  
  delta = OPTION_DAYS_WITHOUT_MOON_SYMBOL/2.0;
  
  if (Phase <= delta)                                                       lcd.print(' '); // hardly visible
  else if ((Phase > delta) && (Phase < CycleDays / 2. - delta))                 lcd.print(')'); // Waxing moon: ny
  else if ((Phase >= CycleDays / 2. - delta) && (Phase <= CycleDays / 2. + delta))  lcd.print('o'); // Full moon
  else if ((Phase > CycleDays / 2. + delta) && (Phase < CycleDays - delta))       lcd.print('('); // Waning Moon: ne
  else                                                                      lcd.print(' '); // hardly visible
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void UpdateMoonPosition() {
  // from K3NG
//  String textbuf;

  double RA, Dec, topRA, topDec, LST, HA;

  // UTC time:
  moon2(year(now()), month(now()), day(now()), (hour(now()) + (minute(now()) / 60.0) + (second(now()) / 3600.0)), lon, latitude, &RA, &Dec, &topRA, &topDec, &LST, &HA, &moon_azimuth, &moon_elevation, &moon_dist);

#ifdef FEATURE_SERIAL_MOON
  Serial.print(F("moon2: "));
  Serial.print(F("RA, DEC: "));
  Serial.print(RA); Serial.print(F(", ")); Serial.println(Dec);
#endif
}



//------------------------------------------------------------------

byte AnalogButtonRead() {
  // K3NG keyer code
  // button numbers start with 0

  int analog_line_read = analogRead(analog_buttons_pin);
  // 10 k from Vcc to A0, then n x 1k to gnd, n=0...9;
  //if (analog_line_read < 500) { // any of 10 buttons will trigger

  if (analog_line_read < 131 && analog_line_read > 50) { // button 1
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

void LocatorToLatLong(char loc[7], double &latitude, double &longitude) {
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

int Distance(double lat1, double long1, double lat2, double long2) {
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

void DecToBinary(int n, int binaryNum[])
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

void PrintFixedWidth(Print &out, int number, byte width, char filler = ' ') {
  int temp = number;
  //
  // Sverre Holm 2022
  // call like this to print number to lcd: PrintFixedWidth(lcd, val, 3);
  // or for e.g. minutes PrintFixedWidth(lcd, minute, 2, '0')
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

void LcdUTCTimeLocator(int lineno, int col=0)
// function that displays the following kind of info on lcd row "lineno" and start column "col" (only 0 or 1)
//  "22:30:46 UTC  JO59fu"
{

// #ifdef FEATURE_PC_SERIAL_GPS_IN
//         hourGPS = hour(now());
//         minuteGPS = minute(now());
//         secondGPS = second(now());
// #endif


//  if (gps.time.isValid()) {
    lcd.setCursor(min(max(col,0),1), lineno);
    sprintf(textBuffer, "%02d%c%02d%c%02d UTC ", hour(now()), dateTimeFormat[dateFormat].hourSep, minute(now()), dateTimeFormat[dateFormat].minSep, second(now()));
    lcd.print(textBuffer);
//  }

  if (gps.satellites.isValid()) {

#ifndef DEBUG_MANUAL_POSITION
    latitude = gps.location.lat();
    lon = gps.location.lng();
#else
    latitude = latitude_manual;
    lon      = longitude_manual;
#endif

//    char locator[7];
    Maidenhead(lon, latitude, textBuffer);
    lcd.setCursor(14, lineno);
    lcd.print(textBuffer);
  }
}

//------------------------------------------------------------------

void LcdDate(int Day, int Month, int Year=0) // print date, either day-month or day-month-year according to specified format
{
  if (dateTimeFormat[dateFormat].dateOrder == 'B')
    {
      if (Year !=0) 
      { 
        PrintFixedWidth(lcd, Year, 4); lcd.print(dateTimeFormat[dateFormat].dateSep);
      }
      PrintFixedWidth(lcd, Month, 2, '0'); lcd.print(dateTimeFormat[dateFormat].dateSep);
      PrintFixedWidth(lcd, Day, 2, '0');
    }
    else if (dateTimeFormat[dateFormat].dateOrder == 'M')
    {
      PrintFixedWidth(lcd, Month, 2, '0'); lcd.print(dateTimeFormat[dateFormat].dateSep);
      PrintFixedWidth(lcd, Day, 2, '0'); 
      if (Year !=0) 
      {
        lcd.print(dateTimeFormat[dateFormat].dateSep);PrintFixedWidth(lcd, Year, 4);
      }
    }
    else
    {
      PrintFixedWidth(lcd, Day, 2, '0'); lcd.print(dateTimeFormat[dateFormat].dateSep);
      PrintFixedWidth(lcd, Month, 2, '0'); 
      if (Year !=0) 
      {
        lcd.print(dateTimeFormat[dateFormat].dateSep);PrintFixedWidth(lcd, Year, 4);
      }
    }
  
}

//------------------------------------------------------------------

void LcdDateString(int Day, String Mnth, int Year=0) // print date, either day-month or day-month-year according to specified format
{
  if (dateTimeFormat[dateFormat].dateOrder == 'B')
    {
      if (Year !=0) 
      { 
        PrintFixedWidth(lcd, Year, 4); lcd.print(F(" "));
      }
      lcd.print(Mnth); lcd.print(F(" "));
      PrintFixedWidth(lcd, Day, 2, '0');
    }
    else if (dateTimeFormat[dateFormat].dateOrder == 'M')
    {
     lcd.print(Mnth); lcd.print(F(" "));
      PrintFixedWidth(lcd, Day, 2, '0'); 
      if (Year !=0) 
      {
        lcd.print(F(" "));PrintFixedWidth(lcd, Year, 4);
      }
    }
    else
    {
      PrintFixedWidth(lcd, Day, 2, '0'); lcd.print(F(" "));
      lcd.print(Mnth); 
      if (Year !=0) 
      {
        lcd.print(F(" "));PrintFixedWidth(lcd, Year, 4);
      }
    }
  
}

//------------------------------------------------------------------

void LcdShortDayDateTimeLocal(int lineno = 0, int moveLeft = 0) {
  // function that displays the following kind of info on lcd row "lineno"
  //  "Wed 20.10     22:30:46" - date separator in fixed location, even if date is ' 9.8'
  // get local time
  localTime = now() + utcOffset * 60;
  Hour = hour(localTime);
  Minute = minute(localTime);
  Seconds = second(localTime);
  
  // local date
  Day = day(localTime);
  Month = month(localTime);
  Year = year(localTime);
    
  lcd.setCursor(0, lineno);
  if (dayGPS != 0)
  { 
        nativeDayLong(localTime);
        // 17.05.2023:
        if     (strcmp(languages[languageNumber],"de ") == 0)  
                sprintf(textBuffer,"%2.2s",today);  // 2 letters for day name in German
        else if (strcmp(languages[languageNumber],"nl ") == 0) 
                sprintf(textBuffer,"%2.2s.",today);  // 2 letters + dot Dutch 
        else    sprintf(textBuffer,"%3.3s",today);  // else 3 letters
        lcd.print(textBuffer);

        lcd.setCursor(4, lineno);
        if ((dateTimeFormat[dateFormat].dateOrder == 'M') | (dateTimeFormat[dateFormat].dateOrder == 'B'))
        {
// modified so month takes up a fixed space without a leading zero:
//          lcd.print(static_cast<int>(Month)); lcd.print(dateTimeFormat[dateFormat].dateSep);
            PrintFixedWidth(lcd, Month, 2,' '); lcd.print(dateTimeFormat[dateFormat].dateSep);
            lcd.print(static_cast<int>(Day));
        }
        else
        {
// modified so day takes up a fixed space without a leading zero:
//        lcd.print(static_cast<int>(Day)); lcd.print(dateTimeFormat[dateFormat].dateSep);
          PrintFixedWidth(lcd, Day, 2,' '); lcd.print(dateTimeFormat[dateFormat].dateSep);
          lcd.print(static_cast<int>(Month));
        }
      }
      lcd.print(F("    ")); // in order to erase remnants of long string as the month changes
      lcd.setCursor(11 - moveLeft, lineno);
      sprintf(textBuffer, " %02d%c%02d%c%02d", Hour, dateTimeFormat[dateFormat].hourSep, Minute, dateTimeFormat[dateFormat].minSep, Seconds); // corrected 18.10.2021
      lcd.print(textBuffer);
    }

    //------------------------------------------------------------------

void LcdTimeLocalShortDayDate(int lineno = 0, int moveRight = 0) {  // unused as it didn't fit LocalSun 
                                                                    // as sun set times are all in 24 hr system
  // function that displays the following kind of info on lcd row "lineno"
  //  "22:30:46   Wed 20.10" - date separator in fixed location, even if date is ' 9.8'
  // get local time
  localTime = now() + utcOffset * 60;

  Minute = minute(localTime);
  Seconds = second(localTime);

    if (Twelve24Local > 12) Hour = hour(localTime);
    else  {
      Hour = hourFormat12(localTime);
      if (oldMinute == -1 || 
        ( (Hour==0 || Hour == 12) && Minute == 00 && Seconds == 0) ) {   // load am/pm symbol as seldom as possible into lcd
        if (isAM(localTime)) {
          memcpy_P(buffer, am, 8);                 // am symbol (clashes with Norse, Faroese char set)
          lcd.createChar(AM_PM, buffer);
          #ifdef FEATURE_SERIAL_LOAD_CHARACTERS
            Serial.println(F("loaded AM character"));
          #endif
        }
        else if (isPM(localTime)) {
          memcpy_P(buffer, pm, 8);                 // pm symbol
          lcd.createChar(AM_PM, buffer);
          #ifdef FEATURE_SERIAL_LOAD_CHARACTERS
            Serial.println(F("loaded PM character"));
          #endif
        }
      }
    }

  
  // local date
  Day = day(localTime);
  Month = month(localTime);
  Year = year(localTime);
    
  lcd.setCursor(moveRight, lineno);
  if (dayGPS != 0)
  { 
  //    lcd.setCursor(11 - moveLeft, lineno);
  #ifdef LEADING_ZERO
     sprintf(textBuffer, "%02d%c%02d%c%02d", Hour, dateTimeFormat[dateFormat].hourSep, Minute, dateTimeFormat[dateFormat].minSep, Seconds); // corrected 18.10.2021
  #else   
        if (Twelve24Local > 12) sprintf(textBuffer, "%02d%c%02d%c%02d", Hour, dateTimeFormat[dateFormat].hourSep, Minute, dateTimeFormat[dateFormat].minSep, Seconds); // corrected 18.10.2021
        else sprintf(textBuffer, "%2d%c%02d%c%02d", Hour, dateTimeFormat[dateFormat].hourSep, Minute, dateTimeFormat[dateFormat].minSep, Seconds); // no zero-padding of hour
  #endif 
        lcd.print(textBuffer);

        lcd.setCursor(8 + moveRight, 0);
        if (isAM(localTime)) {
          lcd.print((char)AM_PM);
        // lcd.print(F(" AM   "));
        }
        else if (isPM(localTime)) { 
          lcd.print((char)AM_PM);
        // lcd.print(F(" PM"));
        }

        lcd.setCursor(11,lineno);
        nativeDayLong(localTime);
        // 17.05.2023:
        if     (strcmp(languages[languageNumber],"de ") == 0)  
                sprintf(textBuffer,"%2.2s",today);  // 2 letters for day name in German
        else if (strcmp(languages[languageNumber],"nl ") == 0) 
                sprintf(textBuffer,"%2.2s.",today);  // 2 letters + dot Dutch 
        else    sprintf(textBuffer,"%3.3s",today);  // else 3 letters
        lcd.print(textBuffer);

        lcd.setCursor(15, lineno);
        if ((dateTimeFormat[dateFormat].dateOrder == 'M') | (dateTimeFormat[dateFormat].dateOrder == 'B'))
        {
// modified so month takes up a fixed space without a leading zero:
//          lcd.print(static_cast<int>(Month)); lcd.print(dateTimeFormat[dateFormat].dateSep);
            PrintFixedWidth(lcd, Month, 2,' '); lcd.print(dateTimeFormat[dateFormat].dateSep);
            lcd.print(static_cast<int>(Day));
        }
        else
        {
// modified so day takes up a fixed space without a leading zero:
//        lcd.print(static_cast<int>(Day)); lcd.print(dateTimeFormat[dateFormat].dateSep);
          PrintFixedWidth(lcd, Day, 2,' '); lcd.print(dateTimeFormat[dateFormat].dateSep);
          lcd.print(static_cast<int>(Month));
        }
      }
 //     lcd.print(F("    ")); // in order to erase remnants of long string as the month changes
   
    }


/////////////////////////////////////////////////////////////////////////////////////////

void LcdSolarRiseSet(
  int lineno,                       // lcd line no 0, 1, 2, 3
  char RiseSetDefinition = ' ',     // default - Actual, C - Civil, N - Nautical, A - Astronomical,  O - nOon info, Z - aZ, el info, 
                                    // W - which azimiuth at actual sunrise/set
  int  ScreenMode = ScreenLocalSun  // One of ScreenLocalSun, ScreenLocalSunSimpler, ScreenLocalSunMoon, ScreenLocalSunAzEl
)
{
  // Horizon for solar rise/set: Actual (0 deg), Civil (-6 deg), Nautical (-12 deg), Astronomical (-18 deg)
  // 19.02.2024 Rewritten from https://github.com/chaeplin/Sunrise to https://github.com/jpb10/SolarCalculator
  //            Sunrise library is obsolete, won't compile for Metro Express without a fix, and also inaccurate
 
  double transit, sunrise, sunset;  // time in utc hours of events  
  int m, hr, mn;                    // time in hr, mn local time
  tmElements_t tm_riseSet;
  time_t riseSetTime;
  
  #ifdef FEATURE_DATE_PER_SECOND  // for stepping date quickly and check calender function   
    // check June when sun hardly sets
    //monthGPS = 6;
    dayGPS =  dateIteration;
    dateIteration = dateIteration + 1;
  #endif
  // https://www.timeanddate.com/astronomy/different-types-twilight.html
  if (RiseSetDefinition == 'A')     // astronomical: -18 deg
  // "During astronomical twilight, most celestial objects can be observed in the sky. However, the atmosphere still scatters and 
  // refracts a small amount of sunlight, and that may make it difficult for astronomers to view the faintest objects."
  // Astronomisk tussmørke
        calcAstronomicalDawnDusk(yearGPS, monthGPS, dayGPS, latitude, lon, transit, sunrise, sunset);
        
  else if (RiseSetDefinition == 'N') // Nautical:     -12 deg
  // "nautical twilight, dates back to the time when sailors used the stars to navigate the seas. 
  // During this time, most stars can be easily seen with naked eyes, and the horizon is usually also visible in clear weather conditions."
  // Nautisk tussmørke
        calcNauticalDawnDusk(yearGPS, monthGPS, dayGPS, latitude, lon, transit, sunrise, sunset);
        
  else if (RiseSetDefinition == 'C') // Civil:        - 6 deg 
  // "enough natural sunlight during this period that artificial light may not be required to carry out outdoor activities."
  // Alminnelig tussmørke	
        calcCivilDawnDusk(yearGPS, monthGPS, dayGPS, latitude, lon, transit, sunrise, sunset);
        
  else  
    //mySunrise.Actual();           // Actual          0 deg
        calcSunriseSunset(yearGPS, monthGPS, dayGPS, latitude, lon, transit, sunrise, sunset);  // time in hours

  // (1) First: print sun rise time
  //t = mySunrise.Rise(monthGPS, dayGPS); // Sun rise hour minute

  if (sunrise >= 0) {             // if not satisfied, then e.g. for 'N' then sun never dips below 18 deg at night, as in mid summer in Oslo
  
    m = int(round(sunrise * 60 + utcOffset));
    hr = (m / 60) % 24;
    mn = m % 60;
    
    lcd.setCursor(0, lineno);
    if (ScreenMode == ScreenLocalSunSimpler || ScreenMode == ScreenLocalSunAzEl) lcd.print(" "); // to line up rise time with date on line above
    
    if (RiseSetDefinition == ' ') // Actual
    {
         lcd.print(F("  ")); lcd.write(UP_ARROW);
    }
    else if (RiseSetDefinition == 'C')  // Civil
    {
         lcd.print(F("  ")); lcd.write(DASHED_UP_ARROW);
    }
    else lcd.print(F("   "));              // Nautical

    if (lineno==1) 
    {
        lcd.setCursor(0, lineno);
        lcd.print(F("S "));         // S - for Sun or Solar
    }

    if (RiseSetDefinition == ' ' ||RiseSetDefinition == 'C'||RiseSetDefinition == 'N'||RiseSetDefinition == 'A')
    {
      if (ScreenMode == ScreenLocalSunSimpler || ScreenMode == ScreenLocalSunAzEl) lcd.setCursor(4, lineno);
      else lcd.setCursor(3, lineno);
      
      PrintFixedWidth(lcd, hr, 2);
      lcd.print(dateTimeFormat[dateFormat].hourSep);
      PrintFixedWidth(lcd, mn, 2, '0');
    }
  }
      
  // (2) Second: print sunset time

  lcd.setCursor(9, lineno);
  if (ScreenMode == ScreenLocalSunSimpler || ScreenMode == ScreenLocalSunAzEl) lcd.print(F("  "));
  if (RiseSetDefinition == ' ')       lcd.write((byte)DOWN_ARROW);
  else if (RiseSetDefinition == 'C')  lcd.write(DASHED_DOWN_ARROW);
  else                                lcd.print(" ");

  if (sunset >= 0) {   
    m = int(round(sunset * 60 + utcOffset));
    hr = (m / 60) % 24;
    mn = m % 60;

    if (RiseSetDefinition == ' ' ||RiseSetDefinition == 'C'||RiseSetDefinition == 'N'||RiseSetDefinition == 'A')
    {
      // improved format 18.06.2022
      PrintFixedWidth(lcd, hr, 2);
      lcd.print(dateTimeFormat[dateFormat].hourSep);
      PrintFixedWidth(lcd, mn, 2, '0'); 
      lcd.print(" ");        // new 5.3.2025 (remove degree symbol) 
    }
  }

// 18.06.2022: the following if {} moved out of if {} above, in order to show e.g. 'N' even around midsummer
  if (ScreenMode == ScreenLocalSunSimpler) 
  {
  // Right margin text 
  //    ' ': Solar elevation right now
  //    'C': Time for local noon
  //    'N': Solar elevation at local noon
  //    'A': -
        lcd.setCursor(18, lineno);
        lcd.print(" ");
        lcd.print(RiseSetDefinition); // show C, N, A to the very right 
  }

  /////// Solar elevation //////////////////

  double sun_azimuth = 0;
  double sun_elevation = 0;
  double sun_elevationTransit;

  // solar az, el now:
  calcHorizontalCoordinates(now(), latitude, lon, sun_azimuth, sun_elevation);

  if (RiseSetDefinition == 'Z') // print current aZimuth, elevation
    {
      lcd.setCursor(0, lineno);
      lcd.print(F("nowEl "));              // added "now" 18.6.2023
      PrintFixedWidth(lcd, (int)round(sun_elevation), 3);
      lcd.write(DEGREE);
      lcd.setCursor(11, lineno);
      lcd.print(F("Az "));
      PrintFixedWidth(lcd, (int)round(sun_azimuth), 3);
      lcd.write(DEGREE);
      lcd.print(F("  "));         
    }

  ///// Solar noon in utc
    m = int(round(transit * 60));
    hr = (m / 60) % 24;
    mn = m % 60;
 
  // find utc time of transit (unix time)
  tmElements_t tm_transit= {0, (uint8_t)mn, (uint8_t)hr, (uint8_t)weekday(), (uint8_t)day(), (uint8_t)month(), (uint8_t)(year()-1970) };
  time_t transitTime = makeTime(tm_transit);     
  calcHorizontalCoordinates(transitTime, latitude, lon, sun_azimuth, sun_elevationTransit);

///// Solar noon in local time
  if (transit >= 0) {  
    m = int(round(transit * 60 + utcOffset));
    hr = (m / 60) % 24;
    mn = m % 60;
  }
 
  if (ScreenMode == ScreenLocalSun || ScreenMode == ScreenLocalSunMoon)
  {
  // position rightmargin info here:
    lcd.setCursor(16, lineno);
 
    if (RiseSetDefinition == ' ')
    { 
      PrintFixedWidth(lcd, (int)round(sun_elevation), 3);  // actual rise time
      lcd.write(DEGREE);
    }
    else if (RiseSetDefinition == 'C')
    {
      if (sunset >= 0) 
      {
        if (hr < 10) lcd.setCursor(16, 2); // added 4.7.2016 to deal with summer far North
        lcd.print(hr, DEC);
        //          lcd.print(dateTimeFormat[dateFormat].hourSep);  
        if (mn < 10) lcd.print("0");
        lcd.print(mn, DEC);
      }
    }
    else if (RiseSetDefinition == 'N')          // 
    {
      // Noon data:
      PrintFixedWidth(lcd, (int)round(sun_elevation), 3);
      lcd.write(DEGREE);
    }  
  }      // if (ScreenMode == ...)

  if (RiseSetDefinition == 'O') // print sun's data at n_O_on
  {
    lcd.setCursor(0, lineno);
    lcd.print(F("maxEl "));         // added "max" 18.6.2023
    PrintFixedWidth(lcd, (int)round(sun_elevationTransit), 3);
    lcd.write(DEGREE);              // added 27.04.2022
    lcd.print(" ");   
    lcd.setCursor(12, lineno);
    PrintFixedWidth(lcd, hr, 2);
    lcd.print(dateTimeFormat[dateFormat].hourSep); 
    PrintFixedWidth(lcd, mn, 2,'0');         
    lcd.print(F("  ")); 
  }

  if (RiseSetDefinition == 'W') // Where is sun's azimuth at actual rise and set times 5.3.2025
   {
// Done:   calcSunriseSunset(yearGPS, monthGPS, dayGPS, latitude, lon, transit, sunrise, sunset);  // time in hours
// find utc time of sunrise/set (unix time)
    tm_riseSet= {0, (uint8_t)mn, (uint8_t)sunrise, (uint8_t)weekday(), (uint8_t)day(), (uint8_t)month(), (uint8_t)(year()-1970) };
    riseSetTime = makeTime(tm_riseSet);   
    calcHorizontalCoordinates(riseSetTime, latitude, lon, sun_azimuth, sun_elevation);
    lcd.setCursor(0, lineno);  
    lcd.print(F("Az ")); lcd.write(UP_ARROW);
    lcd.print(F("  "));PrintFixedWidth(lcd, (int)round(sun_azimuth), 3);lcd.write(DEGREE);lcd.print(" "); 
    
    tm_riseSet= {0, (uint8_t)mn, (uint8_t)sunset, (uint8_t)weekday(), (uint8_t)day(), (uint8_t)month(), (uint8_t)(year()-1970) };
    riseSetTime = makeTime(tm_riseSet); 
    calcHorizontalCoordinates(riseSetTime, latitude, lon, sun_azimuth, sun_elevation);
    lcd.setCursor(11, lineno);lcd.write(DOWN_ARROW);
    lcd.print(F("  "));PrintFixedWidth(lcd, (int)round(sun_azimuth), 3);lcd.write(DEGREE);lcd.print(" "); 
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
  
  if (*Date > 31 && *Month == 3)
  {
    *Date = *Date - 31;
    *Month = *Month + 1;
  }
  else if (*Date > 30 && *Month == 4)
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
 //int y, n, j, k;
  int n, j, k;

  #ifdef FEATURE_SERIAL_MATH
      Serial.print("*** MathDivide:               : ");Serial.println(Term0);
  #endif


  if (Term0 !=0)
      {
        j = 0;
        // find divisor > 0, < sqrt() and with only two digits in first factor:
        for (n = 1; (n <= sqrt(Term0) && Term0*n < 100); n++)
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

// prints val with number of decimal places determined by precision
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
      lcd.write(DOT);lcd.print(F("----"));
      break;
    case 2:
      lcd.write(DOT);lcd.write(DOT);lcd.print(F("---"));
      break;
    case 3:
      lcd.write(DOT);lcd.write(DOT);lcd.write(DOT);lcd.print(F("--"));
      break; 
    case 4:
      lcd.write(DOT);lcd.write(DOT);lcd.write(DOT);lcd.write(DOT);lcd.print(F("-"));
      break;  
    case 5:
      lcd.write(DOT);lcd.write(DOT);lcd.write(DOT);lcd.write(DOT);lcd.write(DOT);
      break;
    case 6:
      lcd.print(F("-"));lcd.write(DOT);lcd.write(DOT);lcd.write(DOT);lcd.write(DOT);
      break; 
    case 7:
      lcd.print(F("--"));lcd.write(DOT);lcd.write(DOT);lcd.write(DOT);
      break; 
    case 8:
      lcd.print(F("---"));lcd.write(DOT);lcd.write(DOT);
      break; 
    case 9:
      lcd.print(F("----"));lcd.write(DOT);
      break; 
    default:
      lcd.print(F("-----"));
      break;                
  }
}

////////////////////////////////////////////////////////////////

void LCDPlanetData(float altitudePlanet, float azimuthPlanet, float phase, float magnitude)
{
    PrintFixedWidth(lcd, (int)round(altitudePlanet), 3);lcd.print(" ");PrintFixedWidth(lcd, (int)round(azimuthPlanet), 3);
    lcd.print(" "); PrintFixedWidth(lcd, (int)round(100*phase), 3); lcd.print(" "); 
    if (magnitude >=0) {lcd.print("+");} // instead of minus sign
    
    if (abs(magnitude) < 10)  lcd.print(String(magnitude, 1));
    else                      lcd.print(String(magnitude, 0)); 
                  
}

////////////////////////////////////////////////////////////////


void LCDChemicalElement(int Hr, int Mn, int Sec)
//
// print 1 or 2-letter chemical abbreviation
{
const char Elements[60][3] = {{"  "}, {"H "}, {"He"}, {"Li"}, {"Be"}, {"B "}, {"C "}, {"N "}, {"O "}, {"F "}, {"Ne"},
                              {"Na"}, {"Mg"}, {"Al"}, {"Si"}, {"P "}, {"S "}, {"Cl"}, {"Ar"}, {"K "}, {"Mg"},
                              {"Sc"}, {"Ti"}, {"V "}, {"Cr"}, {"Mn"}, {"Fe"}, {"Co"}, {"Ni"}, {"Cu"}, {"Zn"}, 
                              {"Ga"}, {"Ge"}, {"As"}, {"Se"}, {"Br"}, {"Kr"}, {"Rb"}, {"Sr"}, {"Y "}, {"Zr"},
                              {"Nb"}, {"Mo"}, {"Tc"}, {"Ru"}, {"Rh"}, {"Pd"}, {"Ag"}, {"Cd"}, {"In"}, {"Sn"},
                              {"Sb"}, {"Te"}, {"I "}, {"Xe"}, {"Cs"}, {"Ba"}, {"La"}, {"Ce"}, {"Pr"}}; 

// Look up 2-letter chemical element from periodic table of elements for local time
      lcd.print(Elements[Hr]);  lcd.print(":"); // Hour
      lcd.print(Elements[Mn]);  lcd.print(":"); // Minute
      lcd.print(Elements[Sec]); lcd.print(" "); // Second
}

////////////////////////////////////////////////////////////////

const char ElementNames[][13] PROGMEM = {
  "Hydrogen    ", "Helium      ", "Lithium     ", "Beryllium   ", "Boron       ", 
  "Carbon      ", "Nitrogen    ", "Oxygen      ", "Fluorine    ", "Neon        ",
  "Sodium      ", "Magnesium   ", "Aluminum    ", "Silicon     ", "Phosphorus  ",
  "Sulfur      ", "Chlorine    ", "Argon       ", "Potassium   ", "Calcium     ",
  "Scandium    ", "Titanium    ", "Vanadium    ", "Chromium    ", "Manganese   ",
  "Iron        ", "Cobalt      ", "Nickel      ", "Copper      ", "Zinc        ",
  "Gallium     ", "Germanium   ", "Arsenic     ", "Selenium    ", "Bromine     ",
  "Krypton     ", "Rubidium    ", "Strontium   ", "Yttrium     ", "Zirconium   ",
  "Niobium     ", "Molybdenum  ", "Technetium  ", "Ruthenium   ", "Rhodium     ",
  "Palladium   ", "Silver      ", "Cadmium     ", "Indium      ", "Tin         ",
  "Antimony    ", "Tellurium   ", "Iodine      ", "Xenon       ", "Cesium      ",
  "Barium      ", "Lanthanum   ", "Cerium      ", "Praseodymium"};

const char  ElementNavn[][13] PROGMEM = {
  "Hydrogen    ", "Helium      ", "Litium      ", "Beryllium   ", "Bor         ", 
  "Karbon      ", "Nitrogen    ", "Oksygen     ", "Fluor       ", "Neon        ",
  "Natrium     ", "Magnesium   ", "Aluminium   ", "Silisium    ", "Fosfor      ",
  "Svovel      ", "Klor        ", "Argon       ", "Kalium      ", "Kalsium     ",
  "Scandium    ", "Titan       ", "Vanadium    ", "Krom        ", "Mangan      ",
  "Jern        ", "Kobolt      ", "Nikkel      ", "Kobber      ", "Sink        ",
  "Gallium     ", "Germanium   ", "Arsen       ", "Selen       ", "Brom        ",
  "Krypton     ", "Rubidium    ", "Strontium   ", "Yttrium     ", "Zirkonium   ",
  "Niob        ", "Molybden    ", "Technetium  ", "Ruthenium   ", "Rhodium     ",
  "Palladium   ", "Solv        ", "Kadmium     ", "Indium      ", "Tinn        ",
  "Antimon     ", "Tellur      ", "Jod         ", "Xenon       ", "Cesium      ",
  "Barium      ", "Lantan      ", "Cerium      ", "Praseodym   "};  

void LCDChemicalElementName(int ElementNo) {

  if (ElementNo >= 1 && ElementNo <= 59) {
    if (strcmp(languages[languageNumber],"nb ")==0 ||strcmp(languages[languageNumber],"nn ")==0) 
      if (ElementNo == 47) 
      {
        strcpy(textBuffer,"Solv      "); textBuffer[1] = char(NO_DA_oe_SMALL); // sølv
      }        
      else strncpy_P(textBuffer, ElementNavn[ElementNo - 1], 12);    // Norwegian
      
    else   strncpy_P(textBuffer, ElementNames[ElementNo - 1], 12);   // English
          
          textBuffer[12] = '\0';  // end here as there is some rubbish in array beyond desired string ...
          lcd.print(textBuffer);
          
  } else  lcd.print(F("            ")); // empty for element 0

}


////////////////////////////////////////////////////////////////

void LCDChemicalGroupPeriod(int ElementNo)
//
// find group and period in periodic system from chemical Element number
{
int group = 0, period = 0;
      if (ElementNo == 1)
      {
        group = ElementNo;   period = 1;
      }
      else if (ElementNo <= 2)
      {
        group = 18; period = 1;
      }
      else if (ElementNo <= 4)
      {
        group = ElementNo-2; period = 2;
      }
      else if (ElementNo <= 10)
      {
        group = ElementNo+8; period = 2;
      }
      else if (ElementNo <= 12)
      {
        group = ElementNo-10; period = 3;
      }
      else if (ElementNo <= 18)
      {
        group = ElementNo; period = 3;
      }
      else if (ElementNo <= 36)
      {
        group = ElementNo-18; period = 4;
      }
      else if (ElementNo <= 54)
      {
        group = ElementNo-36; period = 5;
      }
      else if (ElementNo <= 56)
      {
        group = ElementNo-54; period = 6;
      }

      if (ElementNo == 0)
      {
        lcd.print(F("           ")); // empty
      }
      else if (ElementNo <= 56)
      {
        lcd.print(F("Gr ")); PrintFixedWidth(lcd, group, 2); lcd.print(" ");
        lcd.print(F("Per ")); lcd.print(period); lcd.print(" ");
      }
      else
      {
        lcd.print(F("Gr  -")); lcd.print(" ");  // no group for Lanthanides
        lcd.print(F("Per 6")); lcd.print(" ");
      }
      
}

////////////////////////////////////

//https://www.instructables.com/two-ways-to-reset-arduino-in-software/
void(*resetFunc)(void) = 0; // declare reset function @ address 0


//////////////////////////////////////////////////////////////////////////////////

int readIntFromEEPROM(int address)
// from https://roboticsbackend.com/arduino-store-int-into-eeprom/
{
  return (EEPROM.read(address) << 8) + EEPROM.read(address + 1);
}

void updateIntIntoEEPROM(int address, int number)
// modified from writeIntIntoEEPROM from https://roboticsbackend.com/arduino-store-int-into-eeprom/
{ 
  EEPROMMyupdate(address, number >> 8, 0);
  EEPROMMyupdate(address + 1, number & 0xFF, 1);
}

String readStringFromEEPROM(int addrOffset)
{
  int newStrLen = EEPROM.read(addrOffset);
  char data[newStrLen + 1];
  for (int i = 0; i < newStrLen; i++)
  {
    data[i] = EEPROM.read(addrOffset + 1 + i);
  }
  data[newStrLen] = '\0'; // !!! NOTE !!! Remove the space between the slash "/" and "0" (I've added a space because otherwise there is a display bug)
  return String(data);
}

////////////////////////////////////////////////////////////
void InitScreenSelect()
{
noOfStates = 0;

while ((menuStruct[subsetMenu].order[noOfStates] >= 0) && (noOfStates <= lengthOfMenuIn))
    noOfStates = noOfStates + 1;  

  // initialize and unroll menu system order
  for (iiii = 0; iiii < int(sizeof(menuOrder)/sizeof(menuOrder[0])); iiii += 1) menuOrder[iiii] = -1; // fix 5.10.2022
  for (iiii = 0; iiii < noOfStates; iiii += 1) menuOrder[menuStruct[subsetMenu].order[iiii]] = iiii;
}

//////////////////////////////////////////

int secondaryMenuNumber;

void RotarySecondarySetup(){ // June 2023
// menu system for secondary menu
  uint32_t startTime; // for time-out out of menu
  int toggleInternRotary = 0;

  lcd.setCursor(0,2);
  //lcd.print(secondaryMenuNumber);lcd.print(" "); // removed 22.11.2024

  #ifdef FEATURE_SERIAL_MENU
      Serial.print(F("2ndary # ")); Serial.println(secondaryMenuNumber);
  #endif
  
  switch (secondaryMenuNumber) {

case 0: // GPS baud rate //////////////
 {
  baudRateNumber = EEPROM.read(EEPROM_OFFSET1+5); 
  #ifdef FEATURE_SERIAL_MENU
      Serial.print(F("baudRateNumber ")); Serial.println(baudRateNumber);
  #endif
  int8_t oldBaudRateNumber = baudRateNumber;
  
  int noOfMenuIn = sizeof(gpsBaud1)/sizeof(gpsBaud1[1]); 
  #ifdef FEATURE_SERIAL_MENU
      Serial.print(F("noOfMenuIn ")); Serial.println(noOfMenuIn);
      Serial.println(gpsBaud1[baudRateNumber]);
  #endif
  lcd.setCursor(0,2); PrintFixedWidth(lcd,baudRateNumber, 2); lcd.print(" "); PrintFixedWidth(lcd, gpsBaud1[baudRateNumber], 6);
  startTime = millis();
  while (toggleInternRotary == 0)
   {
  // During each loop, check the encoder to see if it has been changed.
    volatile unsigned char rotaryResult = r.process();  
    if (rotaryResult) {
    if (rotaryResult == r.counterClockwise()) {              
        baudRateNumber = baudRateNumber - 1;
        if (baudRateNumber < 0) baudRateNumber = baudRateNumber + noOfMenuIn;
      }
      else if (rotaryResult == r.clockwise()){               
        baudRateNumber = baudRateNumber + 1;
        if (baudRateNumber >= noOfMenuIn) baudRateNumber = baudRateNumber - noOfMenuIn;
      }
      lcd.setCursor(0,2); PrintFixedWidth(lcd,baudRateNumber, 2); lcd.print(" "); PrintFixedWidth(lcd, gpsBaud1[baudRateNumber], 6);
      startTime = millis();  // reset counter if rotary is moved
    } 

    if (millis() - startTime > menuTimeOut) // check for time-out and return
        {
          lcd.clear();
          return;  // time-out
        }
    if (r.buttonPressedReleased(25)) {            // 25ms = debounce_delay
        toggleInternRotary = toggleInternRotary + 1; // internal variable
        lcd.clear();
    }   
  } // while

     EEPROMMyupdate(EEPROM_OFFSET1+5, baudRateNumber, 1);
     if (baudRateNumber != oldBaudRateNumber)  
     {
        delay(500);
        Serial1.end();   // close serial    // replaced restFunc() 22.02.2024
        gpsBaud = gpsBaud1[baudRateNumber];
        Serial1.begin(gpsBaud);  // restart with new baud rate
       // resetFunc();  // call reset if value has changed 
     }   
 
  CodeStatus();  // show relevant screen to remind operator what parameter was changed
  delay(1500);
  lcd.clear();
  break;
} // case 0 // baudrate

 case 1: //  toggle using_PPS on / off //////////////
  {
    using_PPS = EEPROM.read(EEPROM_OFFSET1+9);
    lcd.setCursor(0,2); lcd.print(F("PPS Interrupt: ")); lcd.print(using_PPS);
    startTime = millis();
    while (toggleInternRotary == 0)
    { 
    // During each loop, check the encoder to see if it has been changed.
      volatile unsigned char rotaryResult = r.process();   
      if (rotaryResult) {
      if (rotaryResult == r.counterClockwise()) {              // decrease  value
          using_PPS = !using_PPS;
        }
        else if (rotaryResult == r.clockwise()){               // increase  value
          using_PPS = !using_PPS;
        }
      lcd.setCursor(0,2); lcd.print(F("PPS Interrupt: ")); lcd.print(using_PPS);
      startTime = millis();  // reset counter if rotary is moved
      }

      if (millis() - startTime > menuTimeOut) // check for time-out and return
          {
            lcd.clear();
            return;  // time-out
          }

      if (r.buttonPressedReleased(25)) {            // 25ms = debounce_delay
          toggleInternRotary = toggleInternRotary + 1; // internal variable
          lcd.clear();
      }   
    } // while

      EEPROMMyupdate(EEPROM_OFFSET1+9, using_PPS, 1);

    CodeStatus();  // show relevant screen to remind operator what parameter was changed
    delay(1500);
    lcd.clear();
    break;
  } // case 1: using_PPS 1 or 0


 case 2: // no of seconds per screen as DemoClock cycles through all screen //////////////
 {
  dwellTimeDemo = EEPROM.read(EEPROM_OFFSET1+7);  
  lcd.setCursor(0,2); PrintFixedWidth(lcd, dwellTimeDemo, 3); lcd.print(F(" sec per screen"));
  startTime = millis();
  while (toggleInternRotary == 0)
  { 
   // During each loop, check the encoder to see if it has been changed.
    volatile unsigned char rotaryResult = r.process();   
    if (rotaryResult) {
    if (rotaryResult == r.counterClockwise()) {              // decrease  value
        dwellTimeDemo = max(dwellTimeDemo - 1,  2);          // minimum time hardcoded here = 2 sec
      }
      else if (rotaryResult == r.clockwise()){               // increase  value
        dwellTimeDemo = dwellTimeDemo + 1; dwellTimeDemo = min(dwellTimeDemo, 60); // maximum time hardcoded = 60 sec
      }
    lcd.setCursor(0,2); PrintFixedWidth(lcd, dwellTimeDemo, 3); lcd.print(F(" sec per screen"));
    startTime = millis();  // reset counter if rotary is moved
    }

    if (millis() - startTime > menuTimeOut) // check for time-out and return
        {
          lcd.clear();
          return;  // time-out
        }

    if (r.buttonPressedReleased(25)) {            // 25ms = debounce_delay
        toggleInternRotary = toggleInternRotary + 1; // internal variable
        lcd.clear();
    }   
  } // while
  
  EEPROMMyupdate(EEPROM_OFFSET1+7, dwellTimeDemo, 1);
  DemoClock(1); // show relevant screen to remind operator what parameter was changed 
  delay(1500);

  lcd.clear();
  break;
 } // case 2: dwellTimeDemo 


case 3: // demo step type as DemoClock cycles through all screen //////////////
 {
  demoStepType = EEPROM.read(EEPROM_OFFSET1+10);  
  lcd.setCursor(0,2); lcd.print(F("Demo step:")); lcd.print(F("       "));
  lcd.setCursor(11,2); lcd.print(demoStepTypeText[demoStepType]);
  startTime = millis();
  while (toggleInternRotary == 0)
  { 
   // During each loop, check the encoder to see if it has been changed.
    volatile unsigned char rotaryResult = r.process();   
    if (rotaryResult) {
    if (rotaryResult == r.counterClockwise()) {              // decrease  value
        demoStepType = demoStepType - 1; if (demoStepType <0) demoStepType = demoStepType + 3;
      }
      else if (rotaryResult == r.clockwise()){               // increase  value
        demoStepType = demoStepType + 1; if (demoStepType > 2) demoStepType = demoStepType - 3;
      }
    lcd.setCursor(0,2); lcd.print(F("Demo step:")); lcd.print(F("       "));
    lcd.setCursor(11,2); lcd.print(demoStepTypeText[demoStepType]);
    startTime = millis();  // reset counter if rotary is moved
    }

    if (millis() - startTime > menuTimeOut) // check for time-out and return
        {
          lcd.clear();
          return;  // time-out
        }

    if (r.buttonPressedReleased(25)) {            // 25ms = debounce_delay
        toggleInternRotary = toggleInternRotary + 1; // internal variable
        lcd.clear();
    }   
  } // while
  
  EEPROMMyupdate(EEPROM_OFFSET1+10, demoStepType, 1);
  DemoClock(1);  // show relevant screen to remind operator what parameter was changed
  delay(1500);
  
  lcd.clear();
  break;
 } // case 3: demoStepType 

 
 case 4: // time for normal clock to be on per minute in most fancy clock displays ////////////////// 
 {
  secondsClockHelp = EEPROM.read(EEPROM_OFFSET1+6);  
  lcd.setCursor(0,2); PrintFixedWidth(lcd, secondsClockHelp, 3); lcd.print(F(" sec per min"));
  startTime = millis();
  while (toggleInternRotary == 0)
  { 
   // During each loop, check the encoder to see if it has been changed.
    volatile unsigned char rotaryResult = r.process();   
    if (rotaryResult) {
    if (rotaryResult == r.counterClockwise()) {              // decrease  value
        secondsClockHelp = max(secondsClockHelp - 6,   0);
      }
      else if (rotaryResult == r.clockwise()){               // increase  value
        secondsClockHelp = min(secondsClockHelp + 6, 60);
      }
    lcd.setCursor(0,2); PrintFixedWidth(lcd, secondsClockHelp, 3); lcd.print(F(" sec per min"));
    startTime = millis();  // reset counter if rotary is moved
    }

    if (millis() - startTime > menuTimeOut) // check for time-out and return
        {
          lcd.clear();
          return;  // time-out
        }

    if (r.buttonPressedReleased(25)) {            // 25ms = debounce_delay
        toggleInternRotary = toggleInternRotary + 1; // internal variable
        lcd.clear();
    }   
  } // while
  
  EEPROMMyupdate(EEPROM_OFFSET1+6, secondsClockHelp, 1);
  lcd.clear();
  break;
 } // case 4: secondsClockHelp 

 case 5: // no of seconds per math quiz //////////////
 {
  mathSecondPeriod = EEPROM.read(EEPROM_OFFSET1+8);  
  lcd.setCursor(0,2); PrintFixedWidth(lcd, mathSecondPeriod, 3); lcd.print(F(" sec per quiz  "));
  startTime = millis();
  while (toggleInternRotary == 0)
  { 
   // During each loop, check the encoder to see if it has been changed.
    volatile unsigned char rotaryResult = r.process();   
    if (rotaryResult) {
    if (rotaryResult == r.counterClockwise()) {              // decrease  value
        mathSecondPeriod = max(mathSecondPeriod - 1,  1);
      }
      else if (rotaryResult == r.clockwise()){               // increase  value
        mathSecondPeriod = min(mathSecondPeriod + 1, 60);
      }
    lcd.setCursor(0,2); PrintFixedWidth(lcd, mathSecondPeriod, 3); lcd.print(F(" sec per quiz  "));
    startTime = millis();  // reset counter if rotary is moved
    }

    if (millis() - startTime > menuTimeOut) // check for time-out and return
        {
          lcd.clear();
          return;  // time-out
        }

    if (r.buttonPressedReleased(25)) {            // 25ms = debounce_delay
        toggleInternRotary = toggleInternRotary + 1; // internal variable
        lcd.clear();
    }   
  } // while
  
  EEPROMMyupdate(EEPROM_OFFSET1+8, mathSecondPeriod, 1);
  lcd.clear();
  break;
 } // case 5: mathSecondPeriod

case 6: // case 6: 1st day of week //////////////
 {
  firstDayWeek = EEPROM.read(EEPROM_OFFSET1 + 11);  
  lcd.setCursor(3,2); //PrintFixedWidth(lcd, firstDayWeek, 3);
  dayName(firstDayWeek-1); lcd.print(today);lcd.print(F("    "));
  startTime = millis();
  while (toggleInternRotary == 0)
  { 
   // During each loop, check the encoder to see if it has been changed.
    volatile unsigned char rotaryResult = r.process();   
    if (rotaryResult) {
    if (rotaryResult == r.counterClockwise()) {              // decrease  value
        firstDayWeek = firstDayWeek-1;  
        if (firstDayWeek < 1) firstDayWeek += 7; 
      }
      else if (rotaryResult == r.clockwise()){               // increase  value
        firstDayWeek = 1 + (firstDayWeek-1 + 1) % 7; 
      }
    lcd.setCursor(3,2); //PrintFixedWidth(lcd, firstDayWeek, 3); 
    dayName(firstDayWeek-1);  lcd.print(today);lcd.print(F("    "));
    
    startTime = millis();  // reset counter if rotary is moved
    }

    if (millis() - startTime > menuTimeOut) // check for time-out and return
        {
          lcd.clear();
          return;  // time-out
        }

    if (r.buttonPressedReleased(25)) {            // 25ms = debounce_delay
        toggleInternRotary = toggleInternRotary + 1; // internal variable
        lcd.clear();
    }   
  } // while
  
  EEPROMMyupdate(EEPROM_OFFSET1 + 11, firstDayWeek, 1);
  Progress();  // show relevant screen to remind operator what parameter was changed
  delay(1500);
  
  lcd.clear();
  break;
 } // case 6: 1st day of week

 default:  
      lcd.clear();
      return;            // exit! 
 }
}
/////////////////// ROTARY -> SETUP PARAMETERS ///////////////////

void RotarySetup()  //  May-June 2023
{
  int menuNumber = 0; 
  int maxMenuNumber = 6;  // for the 0-6 cases below
  int toggleInternRotary = 0;

  uint32_t startTime; // for time-out out of menu

  #ifdef FEATURE_SERIAL_TIME   // OK her   
 //       Serial.print(F("A: timeZoneNumber "));Serial.print(timeZoneNumber);Serial.print(F(" "));Serial.println(tcr[timeZoneNumber] -> abbrev);
  #endif
  
  lcd.clear();
  lcd.setCursor(0,0); lcd.print(F("1. Clock subset >   ")); // menuNumber = 0

// Top-level: Select which menu to enter: 

    startTime = millis();
    while (toggleInternRotary == 0)
    {
    volatile unsigned char rotaryResultTop = r.process();
      if (rotaryResultTop) {
        if (rotaryResultTop == r.counterClockwise()) { 
          menuNumber = menuNumber - 1;
          if (menuNumber < 0) menuNumber = menuNumber + maxMenuNumber + 1;
        }
        else if (rotaryResultTop == r.clockwise()){   
          menuNumber = menuNumber + 1;
          if (menuNumber > maxMenuNumber) menuNumber = menuNumber - maxMenuNumber - 1;
        }
        lcd.setCursor(0,0);        
        switch(menuNumber) {
        case 0: lcd.print(F("1. Clock subset >    ")); break;
        case 1: lcd.print(F("2. Backlight >       ")); break;
        case 2: lcd.print(F("3. Date format >     ")); break;
        case 3: lcd.print(F("4. Time zone >       ")); break;
        case 4: lcd.print(F("5. 12/24 hrs clock > ")); break;
        case 5: lcd.print(F("6. Local language >  ")); break;
        case 6: lcd.print(F("7. Secondary menu >  ")); break;
         }
        startTime = millis();  // reset counter if rotary is moved
        }
        if (millis() - startTime > menuTimeOut) // check for time-out and return
        {
          menuNumber = 99; // do nothing
          lcd.clear();       // 07.08.2023
          return;  // time-out
        }
    
     if (r.buttonPressedReleased(25)) {            // 25ms = debounce_delay
            toggleInternRotary = 1;                // goto next level
     }
    } // while
///// end top-level /////////////

  #ifdef FEATURE_SERIAL_TIME   // problem allerede her   
 //       Serial.print(F("B: timeZoneNumber "));Serial.print(timeZoneNumber);Serial.print(F(" "));Serial.println(tcr[timeZoneNumber] -> abbrev);
  #endif

switch (menuNumber) {

case 0: // 0000000000 subset of clock menu ////////////////////////
{
  subsetMenu = EEPROM.read(EEPROM_OFFSET1+1); 
  startTime = millis();
  int noOfMenuIn = sizeof(menuStruct)/sizeof(menuStruct[0]);
  
  noOfStates = 0;
  while ((menuStruct[subsetMenu].order[noOfStates] >= 0) && (noOfStates <= lengthOfMenuIn))
        noOfStates = noOfStates + 1;                // find no of entries in this submenu
  lcd.setCursor(0,1); lcd.print((char)(97+subsetMenu));lcd.print(F(". "));lcd.print(menuStruct[subsetMenu].descr);
  lcd.print("(");PrintFixedWidth(lcd, noOfStates, 2);lcd.print(")");
  
  while (toggleInternRotary == 1)
  { 
   // During each loop, check the encoder to see if it has been changed.
    volatile unsigned char rotaryResultSubset = r.process();   
    if (rotaryResultSubset) {
    if (rotaryResultSubset == r.counterClockwise()) {              
        subsetMenu = subsetMenu - 1;
          if (subsetMenu < 0) subsetMenu = subsetMenu + noOfMenuIn ;
      }
      else if (rotaryResultSubset == r.clockwise()){               
        subsetMenu = (subsetMenu + 1);
          if (subsetMenu >= noOfMenuIn) subsetMenu = subsetMenu - noOfMenuIn ;
      }

      noOfStates = 0;
      while ((menuStruct[subsetMenu].order[noOfStates] >= 0) && (noOfStates <= lengthOfMenuIn))
        noOfStates = noOfStates + 1;                // find no of entries in this submenu
      lcd.setCursor(0,1); lcd.print((char)(97+subsetMenu));lcd.print(F(". "));lcd.print(menuStruct[subsetMenu].descr);
      lcd.print("(");PrintFixedWidth(lcd, noOfStates, 2);lcd.print(")");
      startTime = millis();  // reset counter if rotary is moved
    } 

    if (millis() - startTime > menuTimeOut) // check for time-out and return
        {
          lcd.clear();
          InitScreenSelect();   //  find no of entries in menuIn
          dispState = 0; // go back to first submenu
          return;  // time-out
        }
    if (r.buttonPressedReleased(25)) {               // 25ms = debounce_delay
        toggleInternRotary = toggleInternRotary + 1; // jump out of while()
        lcd.clear();
    }   
  } // while

EEPROMMyupdate(EEPROM_OFFSET1+1, subsetMenu, 1);
InitScreenSelect();   //  find no of entries in menuIn
dispState = 0; // go back to first submenu
lcd.clear();  
break;
} // case 0: subset of clock menu 


 case 1: // 1111111111 backlight ////////////////// 
 {
  backlightVal = EEPROM.read(EEPROM_OFFSET1+0);  
  lcd.setCursor(0,1); PrintFixedWidth(lcd, backlightVal, 6);
  startTime = millis();
  byte step = 10;
  while (toggleInternRotary == 1)
  { 
   // During each loop, check the encoder to see if it has been changed.
    volatile unsigned char rotaryResultBacklight = r.process();   
    if (rotaryResultBacklight) {
      step = 10;
    if (rotaryResultBacklight == r.counterClockwise()) {              // decrease backlight value
        if (backlightVal <= 30) step = 2;            // smaller steps for low light (for better photos)
        backlightVal = max(backlightVal - step,   2);
      }
      else if (rotaryResultBacklight == r.clockwise()){               // increase backlight value
        if (backlightVal < 30) step = 2;            // smaller steps for low light (for better photos)
        backlightVal = min(backlightVal + step, 255);
      }
    lcd.setCursor(0,1); PrintFixedWidth(lcd, backlightVal, 6);
    analogWrite(LCD_PWM, backlightVal); 
    startTime = millis();  // reset counter if rotary is moved
    }

    if (millis() - startTime > menuTimeOut) // check for time-out and return
        {
          lcd.clear();
          return;  // time-out
        }

    if (r.buttonPressedReleased(25)) {            // 25ms = debounce_delay
        toggleInternRotary = toggleInternRotary + 1; // jump out of while()
        lcd.clear();
    }   
  } // while
  
  EEPROMMyupdate(EEPROM_OFFSET1, backlightVal, 1);
  lcd.clear();
  break;
 } // case 1: backlight 

 
case 2: // 2222222222 date format ////////////////
// code from here instead ? https://stackoverflow.com/questions/53679924/how-to-constrain-a-value-within-a-range
 { 
  dateFormat = EEPROM.read(EEPROM_OFFSET1+2); 
  Day = day(localTime);
  Month = month(localTime);
  Year = year(localTime);
  
  lcd.setCursor(0,1); lcd.print((char)(97+dateFormat)); lcd.print(F(". "));lcd.print(dateTimeFormat[dateFormat].descr);
  lcd.setCursor(0,3); LcdDate(Day, Month, Year);
  sprintf(textBuffer, " %02d%c%02d%c%02d", Hour, dateTimeFormat[dateFormat].hourSep, Minute, dateTimeFormat[dateFormat].minSep, Seconds);
  lcd.print(textBuffer);
  
  int noOfMenuIn = sizeof(dateTimeFormat)/sizeof(dateTimeFormat[0]);

  startTime = millis();
  while (toggleInternRotary == 1)
  { 
   // During each loop, check the encoder to see if it has been changed.
    volatile unsigned char rotaryResult = r.process();   
    if (rotaryResult) {
    if (rotaryResult == r.counterClockwise()) {              
        dateFormat = dateFormat - 1;
        if (dateFormat < 0) dateFormat = dateFormat + noOfMenuIn;
      }
      else if (rotaryResult == r.clockwise()){               
        dateFormat = dateFormat + 1;
        if (dateFormat >= noOfMenuIn) dateFormat = dateFormat - noOfMenuIn;
      }

      lcd.setCursor(0,1); lcd.print((char)(97+dateFormat)); lcd.print(F(". "));lcd.print(dateTimeFormat[dateFormat].descr);
      lcd.setCursor(0,3); LcdDate(Day, Month, Year);
      sprintf(textBuffer, " %02d%c%02d%c%02d", Hour, dateTimeFormat[dateFormat].hourSep, Minute, dateTimeFormat[dateFormat].minSep, Seconds);
      lcd.print(textBuffer);

      //lcd.print(dateTimeFormat[dateFormat].dateOrder);  PrintFixedWidth(lcd, dateFormat, 3);
      startTime = millis();  // reset counter if rotary is moved
    } 

    if (millis() - startTime > menuTimeOut) // check for time-out and return
        {
          lcd.clear();
          return;  // time-out
        }
    if (r.buttonPressedReleased(25)) {            // 25ms = debounce_delay
        toggleInternRotary = toggleInternRotary + 1; // jump out of while()
        lcd.clear();
    }   
  } // while

  EEPROMMyupdate(EEPROM_OFFSET1+2, dateFormat, 1);
  lcd.clear();
  break;
 }  // case 2: date format
 

case 3: // 333333333 time zone /////////////////////////////////////////
{ 
  #ifdef FEATURE_SERIAL_TIME      
//        Serial.print(F("C: timeZoneNumber "));Serial.print(timeZoneNumber);Serial.print(F(" "));Serial.println(tcr[timeZoneNumber] -> abbrev);
  #endif
  
  lcd.setCursor(0,1);

  timeZoneNumber = EEPROM.read(EEPROM_OFFSET1+4); 
  if ((timeZoneNumber < 0) || (timeZoneNumber >= NUMBER_OF_TIME_ZONES-1)) // if EEPROM stores invalid value
       timeZoneNumber = 0;                                      // set to default value 

  lcd.setCursor(0,1); lcd.print((char)(97+timeZoneNumber)); //PrintFixedWidth(lcd, timeZoneNumber, 2); 
  lcd.print(F(". ")); lcd.print(tcr -> abbrev);lcd.print(F("  "));

  lcd.setCursor(9,3); lcd.print(F("UTC"));
  utcOffset = localTime / long(60) - utc / long(60); // order of calculation is important 
  if (utcOffset >=0)  lcd.print("+");
  lcd.print(float(utcOffset)/60); lcd.print(F("  "));

  int firstZone = 0;// 1;// 0; first index used in tcr and local: only !=0 for debugging
  
  startTime = millis();
  while (toggleInternRotary == 1)
  { 
   // During each loop, check the encoder to see if it has been changed.
    volatile unsigned char rotaryResult = r.process();   
    if (rotaryResult) {
    if (rotaryResult == r.counterClockwise()) {              
        timeZoneNumber = timeZoneNumber - 1;
        if (timeZoneNumber < firstZone) timeZoneNumber = timeZoneNumber + NUMBER_OF_TIME_ZONES - firstZone;
      }
      else if (rotaryResult == r.clockwise()){               
        timeZoneNumber = timeZoneNumber + 1;
        if (timeZoneNumber > NUMBER_OF_TIME_ZONES-1) timeZoneNumber = timeZoneNumber - NUMBER_OF_TIME_ZONES + firstZone;
      }
#ifdef FEATURE_SERIAL_TIME      
 //       Serial.print(F("Z: timeZoneNumber "));Serial.print(timeZoneNumber);Serial.print(F(" "));Serial.println(tcr[timeZoneNumber] -> abbrev);
#endif
      
      lcd.setCursor(0,1); lcd.print((char)(97+timeZoneNumber)); lcd.print(F(". "));
      
      tz = *timeZones_arr[timeZoneNumber]; 
      localTime = tz.toLocal(utc,&tcr);

      lcd.print(tcr -> abbrev);lcd.print(F("  "));
      utcOffset = localTime / long(60) - utc / long(60); // order of calculation is important
      lcd.setCursor(9,3); lcd.print(F("UTC")); 
      if (utcOffset >=0)  lcd.print("+");
      lcd.print(float(utcOffset)/60); lcd.print(F("  "));
  
      startTime = millis();  // reset counter if rotary is moved
    } 

    if (millis() - startTime > menuTimeOut) // check for time-out and return
        {
          lcd.clear();
          return;  // time-out
        }
    if (r.buttonPressedReleased(25)) {            // 25ms = debounce_delay
        toggleInternRotary = toggleInternRotary + 1; // jump out of while()
    }   
  } // while
 
  EEPROMMyupdate(EEPROM_OFFSET1+4, timeZoneNumber, 1);
  lcd.clear();
  break;
}  // case 3: time zone 


case 4: // case 4: 12/24 hrs local time //////////////
{
  Twelve24Local = EEPROM.read(EEPROM_OFFSET1 + 12);  
  lcd.setCursor(0,1); PrintFixedWidth(lcd, Twelve24Local, 1);
  lcd.print(F(" hrs local clock  "));
  startTime = millis();
  while (toggleInternRotary == 1)
  { 
   // During each loop, check the encoder to see if it has been changed.
    volatile unsigned char rotaryResult = r.process();   
    if (rotaryResult) {
      if      (Twelve24Local <= 12) Twelve24Local = 24;
      else if (Twelve24Local  > 12) Twelve24Local = 12;
      lcd.setCursor(0,1); lcd.print(Twelve24Local); 
      startTime = millis();  // reset counter if rotary is moved
    }

    if (millis() - startTime > menuTimeOut) // check for time-out and return
        {
          lcd.clear();
          return;  // time-out
        }

    if (r.buttonPressedReleased(25)) {            // 25ms = debounce_delay
        toggleInternRotary = toggleInternRotary + 1; // internal variable
        lcd.clear();
    }   
  } // while
  
  EEPROMMyupdate(EEPROM_OFFSET1 + 12, Twelve24Local, 1);
  //Progress();  // show relevant screen to remind operator what parameter was changed
  //delay(1500);
  
  lcd.clear();
  break;
}  // case 4: 12/24 hrs local time

 case 5: // 55555 local language for day names /////////////////////
 {
  languageNumber = EEPROM.read(EEPROM_OFFSET1+3); 
  startTime = millis();
  byte numLanguages = sizeof(languages) / sizeof(languages[0]);
  
  lcd.setCursor(0,1);
  lcd.print((char)(97+languageNumber));lcd.print(F(". "));lcd.print(languages[languageNumber]);     
   
  nativeDayLong(localTime);
  sprintf(todayFormatted,"%-12s", today);
  lcd.setCursor(5,3); lcd.print(todayFormatted);

  while (toggleInternRotary == 1)
  { 
   // During each loop, check the encoder to see if it has been changed.
     volatile unsigned char rotaryResult = r.process();
    if (rotaryResult) {    
    if (rotaryResult == r.counterClockwise()) {              
        languageNumber = languageNumber - 1;
        if (languageNumber < 0) languageNumber = languageNumber + numLanguages;
      }
      else if (rotaryResult == r.clockwise()){               
        languageNumber = languageNumber + 1;
          if (languageNumber >= numLanguages) languageNumber = languageNumber - numLanguages;
      }
     
      lcd.setCursor(0,1);lcd.print((char)(97+languageNumber));lcd.print(F(". "));  // OK
      lcd.print(languages[languageNumber]);
      loadNativeCharacters(languageNumber);  // reload user-defined characters for native languages
      
      nativeDayLong(localTime);
      sprintf(todayFormatted,"%-12s", today);
      lcd.setCursor(5,3);; lcd.print(todayFormatted);

      startTime = millis();  // reset counter if rotary is moved
    }  // this bracket was placed higher up, so lcd.print was performed continuously, OK for Mega, but Metro was deaf to rotary switch. 17.2.2024
 
    if (millis() - startTime > menuTimeOut) // check for time-out and return
        {
          lcd.clear();
          return;  // time-out
        }       

    if (r.buttonPressedReleased(25)) {               // 25ms = debounce_delay
        toggleInternRotary = toggleInternRotary + 1; // jump out of while()
        lcd.clear();
    }   
  } // while

  EEPROMMyupdate(EEPROM_OFFSET1+3, languageNumber, 1); 
  lcd.clear();
  break;
  
 }  // case 5: local language 
 
case 6: // 666666 Secondary menu //////////////
 {
  int noOfMenuIn = 7;       // no of secondary menu items 7: 18.11.2024
  lcd.setCursor(0,1); lcd.print(F("a. GPS baudrate >    "));
  
  startTime = millis();
  secondaryMenuNumber = 0;
  while (toggleInternRotary == 1)
   {
  // During each loop, check the encoder to see if it has been changed.
    volatile unsigned char rotaryResult = r.process();  
    if (rotaryResult) {
    if (rotaryResult == r.counterClockwise()) {              
        secondaryMenuNumber = secondaryMenuNumber - 1;
        if (secondaryMenuNumber < 0) secondaryMenuNumber = secondaryMenuNumber + noOfMenuIn;
      }
      else if (rotaryResult == r.clockwise()){               
        secondaryMenuNumber = secondaryMenuNumber + 1;
        if (secondaryMenuNumber >= noOfMenuIn) secondaryMenuNumber = secondaryMenuNumber - noOfMenuIn;
      }
      lcd.setCursor(0,1);
      switch (secondaryMenuNumber) { 
      case 0: lcd.print(F("a. GPS baudrate >   ")); break;
      case 1: lcd.print(F("b. GPS PPS >        ")); break;  // moved up from f.) 09.11.2024
      case 2: lcd.print(F("c. Demo dwell time >")); break;
      case 3: lcd.print(F("d. Demo step type > ")); break;
      case 4: lcd.print(F("e. FancyClock help >")); break;
      case 5: lcd.print(F("f. Time, math quiz >")); break;
      case 6: lcd.print(F("g. 1st day of week >")); break;
      // if ((menuOrder[ScreenProgress] > 0) && (menuOrder[ScreenProgress] <= lengthOfMenuIn))   // this screen is in the set of screens selected
      // {
      //   lcd.print(F("g. 1st day, scr ")); lcd.print(menuOrder[ScreenProgress]); lcd.print(F(" >"));
      // }
      //   else lcd.print(F("g. 1st day         >"));
      // break;
      }
      startTime = millis();  // reset counter if rotary is moved
    } 

    if (millis() - startTime > menuTimeOut) // check for time-out and return
        {
          lcd.clear();
          return;  // time-out
        }
    if (r.buttonPressedReleased(25)) {            // 25ms = debounce_delay
        RotarySecondarySetup();  // secondaryMenuNumber is interpreted by RotarySecondarySetup
        return;
    }   
  } // while

  //lcd.clear();
  break;
} // case 6 // secondary menu


default:  
      return;            // exit!     

} // switch
  
}

////////////////////////////////////////////////


void GPSParse()
{
// From TinyGPSParse.ino
// Purpose is to extract additional GPS info, in particular SNR per satellite. Used in GPSInfo()

#ifdef FEATURE_SERIAL_GPS 
      Serial.println(F("*** Enter  GPSParse"));
    #endif

  if (totalGPGSVMessages.isUpdated())    
  {
    for (int i=0; i<4; ++i)
    {
      int no = atoi(satNumber[i].value());
      #ifdef FEATURE_SERIAL_GPS
            Serial.print(F("SatNumber is ")); Serial.println(no);
      #endif
              
        if (no >= 1 && no <= MAX_SATELLITES)
        {
          sats[no-1].elevation = atoi(elevation[i].value());
          sats[no-1].azimuth = atoi(azimuth[i].value());
          sats[no-1].snr = atoi(snr[i].value());
//                sats[no-1].snr = min(sats[no-1].snr, 99);    // to limit wild values 30.05.2023
          sats[no-1].active = true;
        }
        #ifdef FEATURE_SERIAL_GPS
        //      Serial.print(F(", SNR ")); Serial.println(sats[no-1].snr);
        #endif  
    }

      int totalMessages = atoi(totalGPGSVMessages.value());
      int currentMessage = atoi(messageNumber.value());
      if (totalMessages == currentMessage)
      {   
        #ifdef FEATURE_SERIAL_GPS 
          Serial.print(F("Sats=")); Serial.print(gps.satellites.value());
          Serial.print(F(" Nums="));
        #endif
       
        #ifdef FEATURE_SERIAL_GPS 
        for (int i=0; i<MAX_SATELLITES; ++i)
          if (sats[i].active)
          {
            Serial.print(i+1);
            Serial.print(F(" "));
          }
        Serial.print(F(" Elevation="));
        for (int i=0; i<MAX_SATELLITES; ++i)
          if (sats[i].active)
          {
            Serial.print(sats[i].elevation);
            Serial.print(F(" "));
          }
        Serial.println();
        Serial.print(F("        Azimuth="));
        
        for (int i=0; i<MAX_SATELLITES; ++i)
          if (sats[i].active)
          {
            Serial.print(sats[i].azimuth);
            Serial.print(F(" "));
          }
        
          Serial.print(F(" SNR="));
        #endif
          SNRAvg = 0.0;
          totalSats = 0;
          for (int i=0; i<MAX_SATELLITES; ++i)
          if (sats[i].active)
           {
             #ifdef FEATURE_SERIAL_GPS
               Serial.print(sats[i].snr);
               Serial.print(F(" "));
             #endif
            if (sats[i].snr >0)          // 0 when not tracking
             {
              totalSats = totalSats + 1; 
              SNRAvg = SNRAvg + float(sats[i].snr);
             }
           }
          if (totalSats>0) SNRAvg = SNRAvg/totalSats; 
          else                      SNRAvg = 0;               // 16.11.2022
        
          #ifdef FEATURE_SERIAL_GPS
            Serial.println();Serial.print(" SNRAvg "); Serial.print(SNRAvg); 
            Serial.print(", "); Serial.println(round(SNRAvg));
          #endif

          for (int i=0; i<MAX_SATELLITES; ++i)
          sats[i].active = false;
      }

  } // (totalGPGSVMessages.isUpdated())

    #ifdef FEATURE_SERIAL_GPS 
      Serial.println(F("*** Exit  GPSParse"));
    #endif

}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Store bit maps, designed using editor at https://maxpromer.github.io/LCD-Character-Creator/
const byte upDashedArray[8]   PROGMEM = {0x4, 0xa, 0x15, 0x0, 0x4, 0x0, 0x4, 0x0};
const byte downDashedArray[8] PROGMEM = {0x4, 0x0, 0x4, 0x0, 0x15, 0xa, 0x4, 0x0};
const byte upArray[8]         PROGMEM = {0x4, 0xe, 0x15, 0x4, 0x4, 0x4, 0x4, 0x0};
const byte downArray[8]       PROGMEM = {0x4, 0x4, 0x4, 0x4, 0x15, 0xe, 0x4, 0x0};

void loadArrowCharacters()
{
  if (LCDchar0_3 != LCDARROWS)
  {
  // upload characters to the lcd
  memcpy_P(buffer,upDashedArray, 8);
  lcd.createChar(DASHED_UP_ARROW, buffer);
  memcpy_P(buffer,downDashedArray, 8);
  lcd.createChar(DASHED_DOWN_ARROW, buffer);
  memcpy_P(buffer,upArray, 8);
  lcd.createChar(UP_ARROW, buffer);
  memcpy_P(buffer,downArray, 8); 
  lcd.createChar((byte)DOWN_ARROW, buffer);
  lcd.clear();  // in order to set the LCD back to the proper memory mode after custom characters have been created

  LCDchar0_3 = LCDARROWS;
  
  #ifdef FEATURE_SERIAL_LOAD_CHARACTERS
     Serial.println(F("loadArrowCharacters"));
  #endif
  }

}

////////////////////////////////////////////////////////////////////////////////////////////
const byte oneFilled[8] PROGMEM = {
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B10000
};

const byte twoFilled[8] PROGMEM = {
  B11000,
  B11000,
  B11000,
  B11000,
  B11000,
  B11000,
  B11000,
  B11000
};

const byte threeFilled[8] PROGMEM = {
  B11100,
  B11100,
  B11100,
  B11100,
  B11100,
  B11100,
  B11100,
  B11100
};

const byte fourFilled[8] PROGMEM = {
  B11110,
  B11110,
  B11110,
  B11110,
  B11110,
  B11110,
  B11110,
  B11110
};

void loadSimpleBarCharacters()
{

  if (LCDchar0_3 != LCDBARS)
  {
    // upload characters to the lcd
      memcpy_P(buffer,oneFilled, 8);
      lcd.createChar(ONE_BAR, buffer);
      memcpy_P(buffer,twoFilled, 8);
      lcd.createChar(TWO_BARS, buffer);
      memcpy_P(buffer,threeFilled, 8);
      lcd.createChar(THREE_BARS, buffer);
      memcpy_P(buffer,fourFilled, 8);
      lcd.createChar(FOUR_BARS, buffer);
      lcd.clear();  // in order to set the LCD back to the proper memory mode after custom characters have been created

      LCDchar0_3 = LCDBARS;

      #ifdef FEATURE_SERIAL_LOAD_CHARACTERS
        Serial.println(F("loadSimpleBarCharacters"));
      #endif
  }
}


////////////////////////////////////////////////////////////////////////////////////
// the 8 arrays that form each segment of the custom numbers, 2 high, 3 wide
/*
LT= left top
UB= upper bar
RT= right top
LL= lower left
LB= lower bar
LR= lower right
UMB= upper middle bars(upper middle section of the '8')
LMB= lower middle bars(lower middle section of the '8')
*/

const byte LT[8] PROGMEM = 
{
  B00111,
  B01111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
};
const byte UB[8] PROGMEM =
{
  B11111,
  B11111,
  B11111,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};
const byte RT[8] PROGMEM =
{
  B11100,
  B11110,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
};
const byte LL[8] PROGMEM =
{
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B01111,
  B00111
};
const byte LB[8] PROGMEM =
{
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
  B11111,
  B11111
};
const byte LR[8] PROGMEM =
{
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11110,
  B11100
};
const byte UMB[8] PROGMEM =
{
  B11111,
  B11111,
  B11111,
  B00000,
  B00000,
  B00000,
  B11111,
  B11111
};
const byte LMB[8] PROGMEM =
{
  B11111,
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
  B11111,
  B11111
};

void loadThreeWideDigits()
// https://forum.arduino.cc/t/large-alphanumeric-on-lcd/8946/3
{

  if (LCDchar0_3 != LCD3WIDE || LCDchar4_5 != LCD3WIDE || LCDchar6_7 != LCD3WIDE)
  {

    // assignes each segment a write number
    memcpy_P(buffer,LT, 8);
    lcd.createChar((byte)0 , buffer);
    memcpy_P(buffer,UB, 8);
    lcd.createChar(1 , buffer);
    memcpy_P(buffer,RT, 8);
    lcd.createChar(2 , buffer);
    memcpy_P(buffer,LL, 8);
    lcd.createChar(3 , buffer);
    memcpy_P(buffer,LB, 8);
    lcd.createChar(4 , buffer);
    memcpy_P(buffer,LR, 8);
    lcd.createChar(5 , buffer);
    memcpy_P(buffer,UMB, 8);
    lcd.createChar(6 , buffer);
    memcpy_P(buffer,LMB, 8);
    lcd.createChar(7 , buffer);
    lcd.clear();  // in order to set the LCD back to the proper memory mode after custom characters have been created

    LCDchar0_3 = LCD3WIDE;
    LCDchar4_5 = LCD3WIDE;
    LCDchar6_7 = LCD3WIDE;

    #ifdef FEATURE_SERIAL_LOAD_CHARACTERS
            Serial.println(F("loadThreeWideDigits"));
    #endif
  }
}

// position in display
int x = 0;
int y = 0;

///////////////////////////////////////////////////////

void custom0()
{ // uses segments to build the number 0
  lcd.setCursor(x, y); // set cursor to column 0, line 0 (first row)
  lcd.write(byte(0));  // call each segment to create
  lcd.write(byte(1));  // top half of the number
  lcd.write(byte(2));
  lcd.setCursor(x, y+1); // set cursor to colum 0, line 1 (second row)
  lcd.write(byte(3));  // call each segment to create
  lcd.write(byte(4));  // bottom half of the number
  lcd.write(byte(5));
}

void custom1()
{
  lcd.setCursor(x,y+0);
  //lcd.write(byte(1)); // 25.10.2023
  lcd.write(ALL_OFF);
  //lcd.write(byte(2)); // 25.10.2023
  lcd.write(byte(0));
  lcd.write(ALL_OFF);
  lcd.setCursor(x,y+1);
  lcd.write(ALL_OFF);
  lcd.write(byte(5));
  lcd.write(ALL_OFF);
}

void custom2()
{
  lcd.setCursor(x,y);
  lcd.write(byte(6));
  lcd.write(byte(6));
  lcd.write(byte(2));
  lcd.setCursor(x, y+1);
  lcd.write(byte(3));
  lcd.write(byte(7));
  lcd.write(byte(7));
}

void custom3()
{
  lcd.setCursor(x,y);
  lcd.write(byte(6));
  lcd.write(byte(6));
  lcd.write(byte(2));
  lcd.setCursor(x, y+1);
  lcd.write(byte(7));
  lcd.write(byte(7));
  lcd.write(byte(5)); 
}

void custom4()
{
  lcd.setCursor(x,y);
  lcd.write(byte(3));
  lcd.write(byte(4));
  lcd.write(byte(2));
  lcd.setCursor(x, y+1);
  lcd.write(ALL_OFF);
  lcd.write(ALL_OFF);
  lcd.write(byte(5));
}

void custom5()
{
  lcd.setCursor(x,y);
  lcd.write(byte(0));
  lcd.write(byte(6));
  lcd.write(byte(6));
  lcd.setCursor(x, y+1);
  lcd.write(byte(7));
  lcd.write(byte(7));
  lcd.write(byte(5));
}

void custom6()
{
  lcd.setCursor(x,y);
  lcd.write(byte(0));
  lcd.write(byte(6));
  lcd.write(byte(6));
  lcd.setCursor(x, y+1);
  lcd.write(byte(3));
  lcd.write(byte(7));
  lcd.write(byte(5));
}

void custom7()
{
  lcd.setCursor(x,y);
  lcd.write(byte(1));
  lcd.write(byte(1));
  lcd.write(byte(2));
  lcd.setCursor(x,y+1);
  lcd.write(ALL_OFF);
  lcd.write(byte(0));
  lcd.write(ALL_OFF);
}

void custom8()
{
  lcd.setCursor(x,y);
  lcd.write(byte(0));
  lcd.write(byte(6));
  lcd.write(byte(2));
  lcd.setCursor(x,y+1);
  lcd.write(byte(3));
  lcd.write(byte(7));
  lcd.write(byte(5));
}

void custom9()
{
  lcd.setCursor(x,y);
  lcd.write(byte(0));
  lcd.write(byte(6));
  lcd.write(byte(2));
  lcd.setCursor(x, y+1);
  lcd.write(ALL_OFF);
  lcd.write(ALL_OFF);
  lcd.write(byte(5));
}

void draw_digit(byte digit, byte xpos, byte ypos)
// three width wide digits
{
  x = xpos;
  y = ypos;
  switch (digit) {
  case 0:
    custom0();
    break;
  case 1:
    custom1();
    break;
  case 2:
    custom2();
    break;
  case 3:
    custom3();
    break;
  case 4:
    custom4();
    break;
  case 5:
    custom5();
    break;
  case 6:
    custom6();
    break;
  case 7:
    custom7();
    break;
  case 8:
    custom8();
    break;
  case 9:
    custom9();
    break;
  }
}

////////////////////////////////////////////////////////////////
// 2x3 numbers variant 2 with space in second half of character below

const byte c0[8] PROGMEM = //upper small, in: 1
{
  B00111,
  B00111,
  B00111,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};
const byte c1[8] PROGMEM = //left line, in 1,4,6,0
{
  B11100,
  B11100,
  B11100,
  B11100,
  B11100,
  B11100,
  B11100,
  B11100
};
const byte c2[8] PROGMEM =  // upper filled, in 1,2,3,5,6,7,8,9,0
{
  B11111,
  B11111,
  B11111,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};
const byte c3[8] PROGMEM = // mirrored inverted L, in 2,3,4,6,7,8,9,0
{
  B11111,
  B11111,
  B11111,
  B00111,
  B00111,
  B00111,
  B00111,
  B00111
};
const byte c4[8] PROGMEM = // inverted L, in 2,5,6,7,9,0
{
  B11111,
  B11111,
  B11111,
  B11100,
  B11100,
  B11100,
  B11100,
  B11100
};
const byte c5[8] PROGMEM =  // right line, in 4,0
{
  B00111,
  B00111,
  B00111,
  B00111,
  B00111,
  B00111,
  B00111,
  B00111,
};
const byte c6[8] PROGMEM =  // colon
{
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B01110,
  B01110,
  B01110
};

/////////////////////////////////
void loadThreeHighDigits2()
{

  if (LCDchar0_3 != LCD3HIGH || LCDchar4_5 != LCD3HIGH || LCDchar6_7 != LCD3HIGH)
  {
    memcpy_P(buffer,c0, 8);
    lcd.createChar((byte)0, buffer);                      // digit piece
    memcpy_P(buffer,c1, 8);
    lcd.createChar(1, buffer);                      // digit piece
    memcpy_P(buffer,c2, 8);
    lcd.createChar(2, buffer);                      // digit piece
    memcpy_P(buffer,c3, 8);
    lcd.createChar(3, buffer);                      // digit piece
    memcpy_P(buffer,c4, 8);
    lcd.createChar(4, buffer);                      // digit piece
    memcpy_P(buffer,c5, 8);
    lcd.createChar(5, buffer);                      // digit piece
    memcpy_P(buffer,c6, 8);
    lcd.createChar(6, buffer);                      // digit piece
    lcd.clear();  // in order to set the LCD back to the proper memory mode after custom characters have been created

    LCDchar0_3 = LCD3HIGH;
    LCDchar4_5 = LCD3HIGH;
    LCDchar6_7 = LCD3HIGH;

    #ifdef FEATURE_SERIAL_LOAD_CHARACTERS
        Serial.println(F("loadThreeHighDigits2"));
    #endif
  }

}

////////////////////////////////////////////////////////
void doNumber2(byte num, byte r, byte c) {
    lcd.setCursor(c,r);
    switch(num) {
      case 0: lcd.write(byte(4)); lcd.write(byte(3)); 
              lcd.setCursor(c,r+1); lcd.write(byte(1)); lcd.write(byte(5)); 
              lcd.setCursor(c,r+2); lcd.write(byte(2)); lcd.write(byte(2)); break;
            
      case 1: lcd.write(byte(0)); lcd.write(byte(1)); 
              lcd.setCursor(c,r+1); lcd.print(" "); lcd.write(byte(1));
              lcd.setCursor(c,r+2); lcd.write(byte(0)); lcd.write(byte(2)); break;
            
      case 2: lcd.write(byte(2)); lcd.write(byte(3)); 
              lcd.setCursor(c,r+1); lcd.write(byte(4)); lcd.write(byte(2)); 
              lcd.setCursor(c,r+2); lcd.write(byte(2)); lcd.write(byte(2)); break;  
            
      case 3: lcd.write(byte(2)); lcd.write(byte(3)); 
              lcd.setCursor(c,r+1); lcd.write(byte(0)); lcd.write(byte(3)); 
              lcd.setCursor(c,r+2); lcd.write(byte(2)); lcd.write(byte(2)); break;  
              
            
      case 4: lcd.write(byte(1)); lcd.write(byte(5)); 
              lcd.setCursor(c,r+1); lcd.write(byte(2)); lcd.write(byte(3)); 
              lcd.setCursor(c,r+2); lcd.print(" "); lcd.write(byte(0)); break;  
            
      case 5: lcd.write(byte(4)); lcd.write(byte(2)); 
              lcd.setCursor(c,r+1); lcd.write(byte(2)); lcd.write(byte(3)); 
              lcd.setCursor(c,r+2); lcd.write(byte(2)); lcd.write(byte(2)); break; 

      case 6: lcd.write(byte(1)); lcd.print(" ");     
              lcd.setCursor(c,r+1); lcd.write(byte(4)); lcd.write(byte(3)); 
              lcd.setCursor(c,r+2); lcd.write(byte(2)); lcd.write(byte(2)); break;  

      case 7: lcd.write(byte(2)); lcd.write(byte(3));
              lcd.setCursor(c,r+1); lcd.print(" "); lcd.write(byte(5)); 
              lcd.setCursor(c,r+2); lcd.print(" ");  lcd.write(byte(0)); break;  

      case 8: lcd.write(byte(4)); lcd.write(byte(3)); 
              lcd.setCursor(c,r+1); lcd.write(byte(4)); lcd.write(byte(3)); 
              lcd.setCursor(c,r+2); lcd.write(byte(2)); lcd.write(byte(2)); break;  
   
      case 9: lcd.write(byte(4)); lcd.write(byte(3)); 
              lcd.setCursor(c,r+1); lcd.write(byte(2)); lcd.write(byte(3)); 
              lcd.setCursor(c,r+2);  lcd.print(" "); lcd.write(byte(0)); break;  

       case 11: lcd.setCursor(c,r); lcd.write(byte(6));       // colon
                lcd.setCursor(c,r+1); lcd.write(byte(6)); break; 
    } 
}


// Progress bar with every other vertical line filled, 
// advantage: space between characters in display are no longer visible
// only 7 dots high, not the full 8. For WSPR time bar
// https://robodoupe.cz/2015/progress-bar-pro-arduino-a-lcd-displej/
  const byte g70[8] PROGMEM = {
    B00000,
    B10101,
    B10000,
    B10000,
    B10000,
    B10000,
    B10000,
    B10101}; //0 |__ 
  const byte g71[8] PROGMEM = {
    B00000,
    B10101,
    B10100,
    B10100, 
    B10100, 
    B10100, 
    B10100, 
    B10101}; //1 ||_ 
  const byte g72[8] PROGMEM = {
    B00000, 
    B10101,
    B10101,
    B10101, 
    B10101, 
    B10101, 
    B10101, 
    B10101}; //2 ||| 
  const byte g73[8] PROGMEM = {
    B00000,
    B10101,
    B00000,
    B00000,
    B00000, 
    B00000, 
    B00000, 
    B10101}; //3 ___ 
  const byte g74[8] PROGMEM = {
    B00000,
    B10101,
    B00001,
    B00001,
    B00001, 
    B00001, 
    B00001, 
    B10101}; //4 __| 
  const byte g75[8] PROGMEM = {
    B00000,
    B10101,
    B10001,
    B10001,
    B10001, 
    B10001, 
    B10001, 
    B10101}; //5 |_| 

byte filled;
byte empty;

//////////////////////////////////////////////////////////
void loadGapLessCharacters7A()
{
  // same as loadGapLessCharacters7(), except initial test whether loaded previously

  if (LCDchar0_3 != LCDGAPLESS || LCDchar4_5 != LCDGAPLESS || LCDchar6_7 != LCDGAPLESS) // more conditions 31.10.2024
  {
    memcpy_P(buffer,g70, 8);
    lcd.createChar((byte)0, buffer);
    memcpy_P(buffer,g71, 8);
    lcd.createChar(1, buffer);
    memcpy_P(buffer,g72, 8);
    lcd.createChar(2, buffer);
    memcpy_P(buffer,g73, 8);
    lcd.createChar(3, buffer);
    memcpy_P(buffer,g74, 8);
    lcd.createChar(4, buffer);
    memcpy_P(buffer,g75, 8);
    lcd.createChar(5, buffer);
    lcd.clear();  // in order to set the LCD back to the proper memory mode after custom characters have been created
    filled = 2;
    empty = 3;

    LCDchar0_3 = LCDGAPLESS;
    LCDchar4_5 = LCDGAPLESS;
    LCDchar6_7 = LCDGAPLESS;

    #ifdef FEATURE_SERIAL_LOAD_CHARACTERS
      Serial.println(F("loadGapLessCharacters7"));
    #endif
  }

}

////////////////////////////////////////////////////////////////////////////////////////////////////

void gapLessBar(unsigned nr, unsigned total, unsigned firstPos, unsigned lastPos, unsigned line)
// https://robodoupe.cz/2015/progress-bar-pro-arduino-a-lcd-displej/
// framed progress bar, every other segment only, so that gap between characters melts in
// data is displayed in columns firstPos ... lastPos

{
    firstPos = max(firstPos, 0);        // 1...NCOLS-1, first position with data
    lastPos  = min(lastPos,  NCOLS-1);  // ... NCOLS-1, last position with data
    lastPos  = max(lastPos, firstPos+1); // cannot go right to left!!
    line     = max(line,0); line = min(line,NROWS);

    int Nseg = lastPos - firstPos +1 ;  // no of positions to use on LCD (first, ..., last)

    float noOfSubSegments = 3.0; // no of subsegments used per character, out of a total of 5, i.e. 2 gaps
    float segmentNoReal = (float(nr)/float(total) )* (Nseg-1.0/noOfSubSegments); // 1/segmentNoReal 19.12.2023
    int segmentNoInt    = int(segmentNoReal);
    byte subSegmentNo   = min((int)noOfSubSegments-1,(byte)round(noOfSubSegments *(segmentNoReal-segmentNoInt)));  // added min, round 2.11.2024 

    int startPos;

    /*
    Example: 
    total = 111, firstPos = 0, lastPos = 19 => Nseg = 20
    
    nr =   0 => segmentNoReal= 0.0 , segmentNoInt= 0, subSegmentNo=0.
    nr =   1 => segmentNoReal= 0.18, segmentNoInt= 0, subSegmentNo=0.53 -> 0
    nr =   2 => segmentNoReal= 0.35, segmentNoInt= 0, subSegmentNo=1.06 -> 1
    nr =   3 => segmentNoReal= 0.53, segmentNoInt= 0, subSegmentNo=1.59 -> 1
    nr =   4 => segmentNoReal= 0.71, segmentNoInt= 0, subSegmentNo=2.13 -> 2
    nr =   5 => segmentNoReal= 0.89, segmentNoInt= 0, subSegmentNo=2.65 -> 2

    nr =   6 => segmentNoReal= 1.06, segmentNoInt= 1, subSegmentNo=0.19 -> 0

    nr = 111 => segmentNoReal=19.67, segmentNoInt=19, subSegmentNo=2

    */

    // draw on LCD

    // 1: left-hand symbol
    lcd.setCursor(firstPos, line);
    if (segmentNoInt == 0) 
    {
        lcd.write(byte(0)); // initial left-hand symbol, |::
        startPos = firstPos + segmentNoInt;
    }
    else 
    {
        lcd.write(filled);  // filled,                   |||
        startPos = firstPos + segmentNoInt + 1;   // +1 added - removes flickering, 06.10.1024
    }
    
    // 2: upper and lower frame, after actual segment :::
     {  

     for (int j = startPos ; j < (int)lastPos; j++) // 8.10.2024:  
       {
         lcd.setCursor(j,line);lcd.write(empty);  // :::
       }
     }
 
    // 3: draw 0 ... Nseg completely filled segments

    int jmax = firstPos + segmentNoInt ;
    for (int j = firstPos+1; j < jmax; j++)
      {
       // if (j>0)
        {
        // #ifdef DEBUG
        //   lcd.setCursor(0,3); lcd.print(j);lcd.print(" "); 
        //   lcd.setCursor(6,3); lcd.print(segmentNoInt);lcd.print(" ");
        //   lcd.print(firstPos);lcd.print(" ");lcd.print(lastPos);lcd.print(" ");
        // #endif
        lcd.setCursor(j, line); lcd.write(filled);
        }
      }

     // 4: draw closing, right-hand symbol
    if (segmentNoInt < Nseg-2) 
    {
      lcd.setCursor(lastPos,line); lcd.write(byte(4)); // ::|, final   symbol  
    }

    // 5 draw sub segment
    if (segmentNoInt < Nseg-1)  // nothing if final segment has been filled
    {
      lcd.setCursor(firstPos + segmentNoInt, line);                        
      lcd.write(subSegmentNo); // 0, 1, 2 =|::, ||:, |||
      if (segmentNoInt == Nseg-2) lcd.write(4);           // added 19.12.2023 - only important when counting down. = end-character
    }
    else if (segmentNoInt == Nseg-1)
    {  // 4 = ::|, 5 = |:|, filled = |||
      lcd.setCursor(firstPos + segmentNoInt, line);
      if (subSegmentNo == 0) lcd.write(5);      // |__|
      if (subSegmentNo == 1) lcd.write(filled); // 
      if (subSegmentNo == 2) lcd.write(filled); // |||
     }

    // debug info on screen
    #ifdef DEBUG
      if (line < NROWS)
      {
        lcd.setCursor(firstPos,line+1); lcd.print(segmentNoReal); lcd.print(F("    "));
        lcd.setCursor(firstPos+6,line+1); lcd.print(segmentNoInt); lcd.print(" ");lcd.print(subSegmentNo);lcd.print(F("      "));
      }
    #endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define MAX_NO_OF_PERSONS 18  // for Reminder() - best if equal to or larger than size of data set in EEPROM
#define LENGTH_NAME       10  // min 9 + 1, was 11

typedef struct
  {
      char Name[LENGTH_NAME];
      byte Day; // was int
      byte Month;
      int  Year;
  }   person_type;

person_type person[MAX_NO_OF_PERSONS];
byte lengthPersonData;
byte indStart = 0;          // must be outside of Reminder()
uint8_t secondInternal = 0; // must be outside of Reminder()

///////////////////////////////////////////////////////////////////////////////////////////////////////
void readPersonEEPROM()
{
  int addrOffset;  // not byte as 256 is too small

  lengthPersonData = EEPROM.read(EEPROM_OFFSET2);      // position 0,  length of struct 
  #ifdef FEATURE_SERIAL_EEPROM
      Serial.print("Number of records: ");Serial.println(lengthPersonData);
  #endif 

  lengthPersonData = min(MAX_NO_OF_PERSONS,lengthPersonData); // 25.12.2024, limit no of records to what fits into struct

  for (int i=0; i < lengthPersonData; ++i)
  {
    addrOffset = EEPROM_OFFSET2 + 1 + i*20;
    String Nan = readStringFromEEPROM(addrOffset);      // positions 1, ... , 13
      Nan.toCharArray(person[i].Name,LENGTH_NAME);
    person[i].Day = EEPROM.read(addrOffset + 14);
    person[i].Month = EEPROM.read(addrOffset + 15);
    int Yr = readIntFromEEPROM(addrOffset + 16); // positions 16, 17
      person[i].Year = Yr;
    #ifdef FEATURE_SERIAL_EEPROM  
      Serial.print(i); Serial.print(", "); Serial.print(addrOffset); Serial.print(": "); Serial.print(person[i].Name); Serial.print(": ");Serial.print(person[i].Day);Serial.print("-");Serial.print(person[i].Month);Serial.print("-");Serial.println(person[i].Year);
    #endif  
  }  
}

///////////////////////////////////////////////////////////////////////////////////////////
// from https://forum.arduino.cc/t/the-order-of-bubble-sorting/657618/5
// modified for float input

void bubbleSort(float a[], int index_array[], int size) {

//  Serial.print(*index_array);Serial.println(" ");
//  Serial.print(*a); Serial.println(" ");    
  
  for (int i = 0; i < (size - 1); i++) {
    for (int o = 0; o < (size - (i + 1)); o++) {
        if (a[o] > a[o + 1]) {             
        float t = a[o];
        int indB = index_array[o];
        a[o] = a[o + 1];
        index_array[o] = index_array[o + 1];
        a[o + 1] = t;
        index_array[o + 1] = indB;
        }
    }
  }
}

float springEquinox, summerSolstice, autumnEquinox, winterSolstice;

/////////////////////////////////////////////////////////////////
void EquinoxSolstice(int Year)
{
  // Inspired by https://github.com/4nickel/season
  // which builds on Jean Meeus, Astronomical Algorithms, 2nd ed, 1998, Chapter 27
  // but using jd1970 rather than jd (with ref to 4713 BC) as output:
  //    i.e. subtract 2440587.5 from the formulas
  

  // // Precision: 2nd, 3rd, and 4th order terms don't influence result => spring equinox predicted to be 03:03, but is 03:06 or 3:07 in 2024
  float yy = (Year-2000.)/1000.0;  // e.g. 0.024 for 2024
  
  // constant was 2451623.80984 
  // Precision: 2nd, 3rd, and 4th order terms don't influence result => spring equinox predicted to be 03:03, but is 03:06 or 3:07 in 2024
  float spring1 =  11036.30984 + 365242.37404*yy; // utc
  float spring2 = 5.169*yy*yy - 0.411*yy*yy*yy - 0.057*yy*yy*yy*yy; //*100
  springEquinox = spring1 + 0.01*spring2; // second term only adds 2.6 seconds in 2024

  // constant was 2451716.56767
  summerSolstice = 11129.06767 + 365241.62603*yy + 0.00325*yy*yy + 0.00888*yy*yy*yy - 0.00030*yy*yy*yy*yy;

  // constant was 2451810.21715
  autumnEquinox = 11222.71715 + 365242.01767*yy - 0.11575*yy*yy + 0.00337*yy*yy*yy + 0.00078*yy*yy*yy*yy; 

  // constant was 2451900.05952
  winterSolstice = 11312.55952 + 365242.74049*yy - 0.06223*yy*yy - 0.00823*yy*yy*yy + 0.00032*yy*yy*yy*yy; 

  //lcd.setCursor(0,0); lcd.print(displayYear); lcd.print(" "); lcd.print(yy,3);
  //lcd.setCursor(0,1); lcd.print(springEquinox); lcd.print(" "); lcd.print(springEquinox2,7);  
}

/////////////////////////////////////////////////////////////////
// From jrleeman/toDayOfYear.ino, https://gist.github.com/jrleeman/3b7c10712112e49d8607

int calculateDayOfYear(int day, int month, int year) {
  
  // Given a day, month, and year (4 digit), returns 
  // the day of year. Errors return 999.
  
  int daysInMonth[] = {31,28,31,30,31,30,31,31,30,31,30,31};
  
  // Verify we got a 4-digit year
  if (year < 1000) {
    return 999;
  }
  
  // Check if it is a leap year, this is confusing business
  // See: https://support.microsoft.com/en-us/kb/214019
  if (year%4  == 0) {
    if (year%100 != 0) {
      daysInMonth[1] = 29;
    }
    else {
      if (year%400 == 0) {
        daysInMonth[1] = 29;
      }
    }
   }

  // Make sure we are on a valid day of the month
  if (day < 1) 
  {
    return 999;
  } else if (day > daysInMonth[month-1]) {
    return 999;
  }
  
  int doy = 0;
  for (int i = 0; i < month - 1; i++) {
    doy += daysInMonth[i];
  }
  
  doy += day;
  return doy;
}

/////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
const byte zeroBar[8] PROGMEM = {
  B11111,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B11111
};
const byte oneBar[8] PROGMEM = {
  B11111,
  B00000,
  B10000,
  B10000,
  B10000,
  B10000,
  B00000,
  B11111
};

const byte twoBar[8] PROGMEM = {
  B11111,
  B00000,
  B11000,
  B11000,
  B11000,
  B11000,
  B00000,
  B11111
};

const byte threeBar[8] PROGMEM = {
  B11111,
  B00000,
  B11100,
  B11100,
  B11100,
  B11100,
  B00000,
  B11111
};

const byte fourBar[8] PROGMEM = {
  B11111,
  B00000,
  B11110,
  B11110,
  B11110,
  B11110,
  B00000,
  B11111
};

const byte fiveBar[8] PROGMEM = {
  B11111,
  B00000,
  B11111,
  B11111,
  B11111,
  B11111,
  B00000,
  B11111
};



////////////////////////////////////////////////////////////////////////////////////////////////////////
void framedProgressBar(int nr, int total, int firstPos, int lastPos, int line)

// framed progress bar
// data is displayed in columns firstPos+1 ... lastPos-1,
// and frame ends on firstPos, lastPos

{
  firstPos = max(firstPos, 0);        // 1...NCOLS-1, first position with data
  lastPos = min(lastPos, NCOLS - 1);  // ... NCOLS-2, last position with data
  line = max(line, 0);
  line = min(line, NROWS);

  int Nseg = lastPos - firstPos - 1;  // no of positions to use on LCD (first, last = frame)

  float segmentNoReal = Nseg * float(nr) / float(total);  //
  int segmentNoInt = int(segmentNoReal);
  byte subSegmentNo = min(4,(int)round(5 * (segmentNoReal - segmentNoInt))); // added min, round 2.11.2024

  // draw on LCD

  // First: left-hand symbol
  lcd.setCursor(firstPos, line);
  lcd.write(byte(6));  // initial symbol

  // Second: upper and lower frame
  for (int j = firstPos + 1; j < lastPos; j++) {
    lcd.setCursor(j, line);
    lcd.write(byte(empty));  // over- and underbar - or- empty
  }

  // Third: draw closing, right-hand symbol
  lcd.setCursor(lastPos, line);
  lcd.write(byte(7));  // final   symbol

  // Fourth draw 0 ... Nseg completely filled segments
  for (int j = firstPos + 1; j <= firstPos + segmentNoInt; j++) {
    lcd.setCursor(j, line);
    lcd.write(filled);
  }

  // Third draw sub segment
  if (segmentNoInt != Nseg)  // nothing if final segment has been filled
  {
    lcd.setCursor(firstPos + 1 + segmentNoInt, line);
    if (subSegmentNo == 0) lcd.write(empty);
    else lcd.write(subSegmentNo);  // 1...4
  }


// debug info on screen
#ifdef DEBUG
  if (line < NROWS) {
    lcd.setCursor(6, line + 1);
    lcd.print(segmentNoReal);
    lcd.print("    ");
    lcd.setCursor(12, line + 1);
    lcd.print(segmentNoInt);
    lcd.print(" ");
    lcd.print(subSegmentNo);
    lcd.print("      ");
  }
#endif
}
////////////
/////////////


// beginning symbol, curved (xxxx)
// Bar 5:
const byte beg1[8] PROGMEM = {
  B00001,
  B00010,
  B00010,
  B00010,
  B00010,
  B00010,
  B00010,
  B00001
};
// end symbol
const byte end1[8] PROGMEM = {
  B10000,
  B01000,
  B01000,
  B01000,
  B01000,
  B01000,
  B01000,
  B10000
};

void loadCurvedFramedBarCharactersA() {  // Bar 5, ( xxxx )
  empty = 0;
  filled = 5;
  if (LCDchar0_3 != LCDFRAMEDBARS || LCDchar4_5 != LCDFRAMEDBARS || LCDchar6_7 != LCDFRAMEDBARS) 
 {
    memcpy_P(buffer,zeroBar, 8);
    lcd.createChar(empty, buffer);
    memcpy_P(buffer,oneBar, 8);
    lcd.createChar(1, buffer);
    memcpy_P(buffer,twoBar, 8);
    lcd.createChar(2, buffer);
    memcpy_P(buffer,threeBar, 8);
    lcd.createChar(3, buffer);
    memcpy_P(buffer,fourBar, 8);
    lcd.createChar(4, buffer);
    memcpy_P(buffer,fiveBar, 8);
    lcd.createChar(filled, buffer);

    memcpy_P(buffer,beg1, 8);
    lcd.createChar(6, buffer);
    memcpy_P(buffer,end1, 8);
    lcd.createChar(7, buffer);

    LCDchar0_3 = LCDFRAMEDBARS; 
    LCDchar4_5 = LCDFRAMEDBARS;
    LCDchar6_7 = LCDFRAMEDBARS;
 }
}

int isSquare (int n) {    //only tested for n < 2459
  if (n==0 || n==1) return 0;
  int nsqrt = (int)sqrt(n);
  if (nsqrt*nsqrt == n) return nsqrt;
  else return 0;
}

int isCube (int n) {      //only tested for n < 2459
  if (n==0 || n==1) return 0;
  // special case for problem values, otherwise OK up to 14^3=2744
  else if (n==64)   return 4;
  else if (n==512)  return 8;
  else if (n==2197) return 13;  // not a time

  int ncuberoot = (int)pow(n,1.0/3.0);
  if (ncuberoot*ncuberoot*ncuberoot == n) return ncuberoot;
  else return 0;
}

int isFactorial (int n) {   //only good for n < 5959
  if      (n==2)    return 2;
  else if (n==6)    return 3;
  else if (n==24)   return 4;
  else if (n==120)  return 5;
  else if (n==720)  return 6;
  else if (n==5040) return 7;
  else              return 0;
}

int isPower2 (int n) { // range for n: 8 ... 5959
//  if (n==4)          return 2;
  if (n==8)          return 3;
  else if (n==16)    return 4;
  else if (n==32)    return 5;
  else if (n==64)    return 6;  // not a time, yes 06:4_
  else if (n==128)   return 7;
  else if (n==256)   return 8;
  else if (n==512)   return 9;
  else if (n==1024)  return 10;
  else if (n==2048)  return 11;
  else if (n==4096)  return 12; // not a time
  else               return 0;
}

int isFibonacci (int n) // 17 first Fibonacci numbers
{
  if (n==2)          return 3;
  else if (n==3)     return 4;
  else if (n==5)     return 5;
  else if (n==8)     return 6;
  else if (n==13)    return 7;
  else if (n==21)    return 8;
  else if (n==34)    return 9;
  else if (n==55)    return 10;
  else if (n==89)    return 11;  // not a time
  else if (n==144)   return 12;
  else if (n==233)   return 13;
  else if (n==377)   return 14;  // not a time
  else if (n==610)   return 15;
  else if (n==987)   return 16;  // not a time
  else if (n==1597)  return 17;  // not a time
  else if (n==2584)  return 18;  // not a time
  else if (n==4181)  return 19;  // not a time
  else               return 0;
  }


int isPerfect (int n) {   // Perfect numbers
// Ex:   6 = 1*2*3   = 1+2+3
//      28 = 1*2*2*7 = 1+2+4+7+14
  if      (n==6)    return 1;
  else if (n==28)   return 2;
  else if (n==496)  return 3;
  else if (n==8128) return 4;
  else              return 0;
}


void update_line() { 
  #ifdef FEATURE_FACTORIZATION
    Serial.print("In: Row, col "); Serial.print(lineFactor); Serial.print(" ");Serial.println(cursorFactor);
  #endif
  if (cursorFactor > 15) {
    lineFactor += 1;
    cursorFactor = 0;
    #ifdef FEATURE_FACTORIZATION
      Serial.print("If: Row, col "); Serial.print(lineFactor); Serial.print(" ");Serial.println(cursorFactor);
    #endif
    lcd.setCursor(cursorFactor, lineFactor);
  }
}

// modified from https://wokwi.com/projects/400199259002725377
 bool print_factors(int number) {
  bool prime=false;
  if (number <= 2) 
  {
    if (number != 0) prime=true;
    return prime;
  }
   int num = number;
  
   while (num > 0 && num % 2 == 0)  {  // essential to have num > 0 - otherwise sometimes eternal loop ...
      lcd.print(2);
      cursorFactor += 1;
      update_line();
      num /= 2;
      if (num >= 2) 
      { 
       lcd.print((char)DOT);
       cursorFactor += 1;
       update_line();
      }
   }
   for (int i = 3; i <= sqrt(num); i += 2){
       while (num % i == 0){
         lcd.print(i);
         cursorFactor += 1;
         update_line();

         num /= i;
         if (num % i == 0 || num > 2) 
         {
           lcd.print((char)DOT);
           cursorFactor += 1;
           update_line();
         }
       }
    }
    if (num > 2) {
       if (num != number) { lcd.print(num); prime=0; }
       else prime=1;
    }
    return prime;
 }


 /// THE END ///
