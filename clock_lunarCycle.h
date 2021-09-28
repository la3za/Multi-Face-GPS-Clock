/*---------------------------------------------------------------------------
  https://github.com/jgoerzen/wview/blob/master/common/lunarCycle.c
  FILENAME:
        lunarCycle.c
 
  PURPOSE:
        Provide lunar cycle computation utility.
 
  REVISION HISTORY:
        Date            Engineer        Revision        Remarks
        08/17/2005      M.S. Teel       0               Original
        12/01/2009      M. Hornsby      1               Add Moon Rise and Set

        11.08.2021      S. Holm                         Adapted for Arduino
        
 
  NOTES:
        Arduino GPS clock uses these functions:
          GetMoonRiseSetTimes
          getSign
          localSiderealTime
          GetMoonLocation
          MoonTest
          moonInterpolate

        Not used:
          GetMoonPhase
          GetSunPosition
        
 
  LICENSE:
        Copyright (c) 2005, Mark S. Teel (mark@teel.ws)
  
        This source code is released for free distribution under the terms 
        of the GNU General Public License.
  
----------------------------------------------------------------------------*/

//  ... System header files
/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <config.h>

#include "sysdefs.h"
*/

#define WV_SECONDS_IN_DAY   (86400)

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

static double               VHz[3], RAn[3], Decl[3]; // Dec[] renamed to Decl[]
static MOONRISESET          MoonRise, MoonSet;

#define PI                  3.1415926535897932384626433832795
#define RAD                 (PI/180.0)
#define SMALL_FLOAT         (1e-12)


// Local methods:


// doesn't work without double precision, i.e. no good for Arduino:
static double GetJulianDate (int year, int month, double day)
{
    int     a, b, c, e;
    if (month < 3)
    {
        year--;          // decrease by 1
        month += 12;
    }
    if ((year > 1582) ||
        (year == 1582 && month > 10) ||
        (year == 1582 && month == 10 && day > 15))
    {
        a = year/100;
        b = 2 - a+a/4;
        c = (int)(365.25 * year);
        e = (int)(30.6001 * (month + 1));
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

//static double GetMoonPhase (int year, int month, int day, int hour)
static double GetMoonPhase (float zone) // zone is utcoffset in hours
{
  //  double      j = GetJulianDate(year,month,(double)day+(double)hour/24.0)-2444238.5;
    double      j = zone/24.0 + now()/86400.0 - 3652.0; // i.e. no of days since 1970 converted to 1.1.1980 + in local rather than UTC
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
        retVal *= -1; //  <0 if decreasing moon

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

    TU = jd / 36525.0; // jd = jd2000
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
static MOONLOCATION GetMoonLocation(double jd)
{
    double          d, f, g, h, m, n, s, u, v, w;
    MOONLOCATION    itshere;

    h = 0.606434 + 0.03660110129 * jd;
    m = 0.374897 + 0.03629164709 * jd;
    f = 0.259091 + 0.03674819520 * jd;
    d = 0.827362 + 0.03386319198 * jd;
    n = 0.347343 - 0.00014709391 * jd;
    g = 0.993126 + 0.00273777850 * jd;

    h = h - floor(h);
    m = m - floor(m);
    f = f - floor(f);
    d = d - floor(d);
    n = n - floor(n);
    g = g - floor(g);

    h = h*2*PI;
    m = m*2*PI;
    f = f*2*PI;
    d = d*2*PI;
    n = n*2*PI;
    g = g*2*PI;

    v = 0.39558 * sin(f + n);
    v = v + 0.08200 * sin(f);
    v = v + 0.03257 * sin(m - f - n);
    v = v + 0.01092 * sin(m + f + n);
    v = v + 0.00666 * sin(m - f);
    v = v - 0.00644 * sin(m + f - 2*d + n);
    v = v - 0.00331 * sin(f - 2*d + n);
    v = v - 0.00304 * sin(f - 2*d);
    v = v - 0.00240 * sin(m - f - 2*d - n);
    v = v + 0.00226 * sin(m + f);
    v = v - 0.00108 * sin(m + f - 2*d);
    v = v - 0.00079 * sin(f - n);
    v = v + 0.00078 * sin(f + 2*d + n);

    u = 1 - 0.10828 * cos(m);
    u = u - 0.01880 * cos(m - 2*d);
    u = u - 0.01479 * cos(2*d);
    u = u + 0.00181 * cos(2*m - 2*d);
    u = u - 0.00147 * cos(2*m);
    u = u - 0.00105 * cos(2*d - g);
    u = u - 0.00075 * cos(m - 2*d + g);

    w = 0.10478 * sin(m);
    w = w - 0.04105 * sin(2*f + 2*n);
    w = w - 0.02130 * sin(m - 2*d);
    w = w - 0.01779 * sin(2*f + n);
    w = w + 0.01774 * sin(n);
    w = w + 0.00987 * sin(2*d);
    w = w - 0.00338 * sin(m - 2*f - 2*n);
    w = w - 0.00309 * sin(g);
    w = w - 0.00190 * sin(2*f);
    w = w - 0.00144 * sin(m + n);
    w = w - 0.00144 * sin(m - 2*f - n);
    w = w - 0.00113 * sin(m + 2*f + 2*n);
    w = w - 0.00094 * sin(m - 2*d + g);
    w = w - 0.00092 * sin(2*m - 2*d);

    s = w/sqrt(u - v*v);                  // compute moon's  ...  right ascension
    itshere.rightascension = h + atan(s/sqrt(1 - s*s));

    s = v/sqrt(u);                        // declination ...
    itshere.declination = atan(s/sqrt(1 - s*s));

    itshere.parallax = 60.40974 * sqrt( u );          // and parallax

    return(itshere);
}



// Public methods:
#define PHASE_STR_MAX       128

/*

char *lunarPhaseGet (char *increase, char *decrease, char *full)
{
    static char     phaseStr[PHASE_STR_MAX];
    time_t          timeNow = time (NULL);
    double          phase;
    struct tm       bknTime;

    localtime_r (&timeNow, &bknTime);

    // compute the period value
    phase = GetMoonPhase (bknTime.tm_year+1900, bknTime.tm_mon+1,
                          bknTime.tm_mday, bknTime.tm_hour);

    if (phase < 0)
        snprintf(phaseStr, PHASE_STR_MAX-1, "%s %.0f%c %s", decrease, fabs(phase), '%', full);
    else
        snprintf(phaseStr, PHASE_STR_MAX-1, "%s %.0f%c %s", increase, phase, '%', full);

    return phaseStr;
}
*/

/*
* This is a C language implementation of the Sky and Telscope BASIC
* program 1989 page 78 http://media.skyandtelescope.com/binary/moonup.bas 
*
* Its based on the Java implementation by Stephen R. Schmitt
* http://home.att.net/~srschmitt/script_moon_rise_set.html
*/


// calculate MoonRise and MoonSet times
//
// Returns Rise and Set times times returned as packed time (hour*100 + minutes)
//
// packedRise > 0 && packedSet = -1 =>  the moon rises and never sets
// packedRise = -1 && packSet > 0   =>  no moon rise and the moon sets
// packedRise = packedSet = -1      =>  the moon never sets
// packedRise = packedSet = -2      =>  the moon never rises

int GetMoonRiseSetTimes
(
    double       zone,                   // Timezone offset from UTC/GMT in hours
    double       lat,                    // Latitude degress  N=> +, S=> -
    double       lon,                    // longitude degress E=> +, W=> -
    short        *packedRise,            // returned Moon Rise time
    double       *riseAz,                // return Moon Rise Azimuth
    short        *packedSet,             // returned Moon Set time
    double       *setAz                  // return Moon Set Azimuth
)
{
    int             k;
    MOONLOCATION    mp[3];
    double          localsidereal;
    double          ph;
    double          jd;

    // Julian day converted to J2000, i.e. relative to Jan 1.5, 2000
    // GetJulianDate() suffers from precision problem on Arduino as double = single = float
    //jd = GetJulianDate(year, month, (double)day) - 2451545.0;
    
    // should indicate beginning of the day, hence the truncation --- but why beginning of day?
    jd = trunc(now()/86400.0) - 10957.5; // i.e. no of days since 1970 converted to j2000

    localsidereal = localSiderealTime(lon, jd, zone); // local sidereal time
    
    //jd = float(now())/86400.0 - 10957.5; // i.e. no of days since 1970 converted to j2000
  
    #ifdef FEATURE_SERIAL_MOON
      Serial.println(F("GetMoonRiseSetTimes: "));
      Serial.print(F(" jd, zone, localsidereal ")); //, year, month, day: "));
      Serial.print(jd);Serial.print(F(", "));Serial.print(zone);Serial.print(F(", "));Serial.println(localsidereal);
    #endif

    jd = jd - zone / 24.0;                      // get moon position at day start

    for (k = 0; k < 3; k ++)
    {
        mp[k] = GetMoonLocation(jd);
        jd = jd + 0.5;
    }

    if (mp[1].rightascension <= mp[0].rightascension)
        mp[1].rightascension = mp[1].rightascension + 2*PI;

    if (mp[2].rightascension <= mp[1].rightascension)
        mp[2].rightascension = mp[2].rightascension + 2*PI;

    RAn[0] = mp[0].rightascension;
    Decl[0] = mp[0].declination;

    MoonRise.event = 0;                         // initialize
    MoonSet.event  = 0;

    for (k = 0; k < 24; k++)                    // check each hour of this day
    {
        ph = (k + 1.0)/24.0;

        RAn[2] = moonInterpolate(mp[0].rightascension, 
                                 mp[1].rightascension, 
                                 mp[2].rightascension, 
                                 ph);
        Decl[2] = moonInterpolate(mp[0].declination, 
                                 mp[1].declination, 
                                 mp[2].declination, 
                                 ph);

        VHz[2] = moonTest(k, localsidereal, lat, mp[1].parallax);

        RAn[0] = RAn[2];                       // advance to next hour
        Decl[0] = Decl[2];
        VHz[0] = VHz[2];
    }

    *packedRise = (short)(MoonRise.hr * 100 +  MoonRise.min);
    if (riseAz != NULL)
        *riseAz = MoonRise.az;

    *packedSet = (short)(MoonSet.hr * 100 +  MoonSet.min);
    if (setAz != NULL)
        *setAz = MoonSet.az;

    /*check for no MoonRise and/or no MoonSet  */

    if (! MoonRise.event && ! MoonSet.event)  // neither MoonRise nor MoonSet
    {
        if (VHz[2] < 0)
            *packedRise = *packedSet = -2;  // the moon never sets
        else
            *packedRise = *packedSet = -1;  // the moon never rises
    }
    else                                    //  check for MoonRise or MoonSet
    {
        if (! MoonRise.event)
            *packedRise = -1;               // no MoonRise and the moon sets
        else if (! MoonSet.event)
            *packedSet = -1;                // the moon rises and never sets
    }

    #ifdef FEATURE_SERIAL_MOON
 //     Serial.println(F("GetMoonRiseSetTimes: "));
      Serial.println(F("MoonRise hr, min; MoonSet hr, min: "));
      Serial.print(MoonRise.hr);Serial.print(F(", "));Serial.print(MoonRise.min);Serial.print(F(", "));
      Serial.print(MoonSet.hr);Serial.print(F(", "));Serial.println(MoonSet.min);
    #endif

    return; // ; ????????????
}
