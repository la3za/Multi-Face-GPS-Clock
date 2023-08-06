/*
EQUATIO: Sidereal & Solar Clock, by Wooduino

Routines from http://woodsgood.ca/projects/2015/06/14/equatio-sidereal-solar-clock/

doEoTCalc
doNumDays

*/

// number of days of month (m) and date (d) since beginning of year (y)
int doNumDays(int y, int m, int d) {
  int v=0;
  byte leapyear = ((y-2000)%4 == 0)? 1 : 0;
  switch(m) {
      case 12:  v += 30;        // Dec
      case 11:  v += 31;        // Nov
      case 10:  v += 30;        // Oct
      case 9:   v += 31;        // Sep
      case 8:   v += 31;        // Aug
      case 7:   v += 30;        // Jul
      case 6:   v += 31;        // Jun
      case 5:   v += 30;        // May
      case 4:   v += 31;        // Apr
      case 3:   v += 28 + leapyear;   // May (if year is leapyear, add extra day after February)
      case 2:   v += 31; break; // Feb
  }
  return v+d;                   // days from Jan 1 of given year
}

// ********************************************************************************** //
//                             EQUATION OF TIME CALCULATIONS
// ********************************************************************************** //

void doEoTCalc(double *tv) {

  // equation of time
  double tv1,tv2,tv3;                // total time variation (mins)
  int tvm,tvs,nd;                       // EoT time mins and seconds, days from Jan 1.

  nd = doNumDays(yearGPS, monthGPS, dayGPS);                          // days from Jan 1 (inc. leap year)

  #ifdef FEATURE_SERIAL_EQUATIO
  Serial.print("Days from Jan 1: "); Serial.println(nd);
  #endif
  
//// 1. based on "The Equation of Time" by Murray Bourne, 26 Aug 201
////    see http://www.intmath.com/blog/mathematics/the-equation-of-time-5039
    float dd = (2*PI*(float)nd/365.0);
    tv1 = -7.655 * sin(dd) + 9.873 * sin(2*dd + 3.588); // Effect of Orbit Eccentricity & Effect of Obliquit
//
//// 2. based on http://www.susdesign.com/popups/sunangle/eot.php
//  float B = 2*PI*((float)nd - 79.0)/365.0;               // B in radians (note should be -81)
//  tv2 = 9.87*sin(2*B) - 7.53*cos(B) - 1.5*sin(B);
//
//// 3. based on http://naturalfrequency.com/Tregenza_Sharples/Daylight_Algorithms/algorithm_1_12.htm
  tv3 = 60.0 * (0.170*sin(4*PI*(nd-80)/373.0) - 0.129*sin(2*PI*(nd-8)/355.0) );
//
//// tv3 deviates too much? On 25.11.2021: 11.48, 11.57, 13.15
//  
//  *tv = (tv1+tv2+tv3)/3.0;
////  *tv = (tv1+tv2)/2.0;

// Use Wikipedia's first method, https://en.wikipedia.org/wiki/Equation_of_time (Milne 1921)
  float M = 6.24004077 + 0.01720197*nd; // the mean anomaly, the angle from the periapsis of the elliptical orbit to the mean Sun
  *tv = -7.659 *sin(M) + 9.863* sin(2*M + 3.5932);
  


#ifdef FEATURE_SERIAL_EQUATIO
  Serial.println(F("tv1, tv2, tv3, tv (average) in minutes:"));
  printDouble(tv1,100); // with 2 digits
  printDouble(tv2,100);
  printDouble(tv3,100);
  printDouble(*tv,100);
#endif
  
//  tvm = int( abs(tv));                                  // whole mins
//  tvs = int((abs(tv) - float(tvm))*60.0 );              // whole secs
//
//  DateTime now = RTC.now();
//  DateTime solTime (now.unixtime() + int(tv*60));       // solat time (if EoT is -ve, clock is ahead of solar time)
//  sh = solTime.hour();
//  sm = solTime.minute();
//  ss = solTime.second();
}
