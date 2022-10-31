// The following C++ code is translated from the Lisp code
// in ``Calendrical Calculations'' by Nachum Dershowitz and
// Edward M. Reingold, Software---Practice & Experience,
// vol. 20, no. 9 (September, 1990), pp. 899--928.

// This code is in the public domain, but any use of it
// should publically acknowledge its source.

// Classes GregorianDate, JulianDate, IsoDate, IslamicDate,
// and HebrewDate

// main source              https://reingold.co/calendar.C
// adaptation for Arduino:  https://www.instructables.com/Hebrew-calendar-date-and-time-with-thermometer-on-/
// 

class IsoDate;


char* DayName[7] = {"Sunday", "Monday", "Tuesday", "Wednesday",
                    "Thursday", "Friday", "Saturday"};

// Absolute dates

// "Absolute date" means the number of days elapsed since the Gregorian date
// Sunday, December 31, 1 BC. (Since there was no year 0, the year following
// 1 BC is 1 AD.) Thus the Gregorian date January 1, 1 AD is absolute date
// number 1.

long XdayOnOrBefore(long d, long x) {
// Absolute date of the x-day on or before absolute date d.
// x=0 means Sunday, x=1 means Monday, and so on.

  return (d - ((d - x) % 7));
}


//  Gregorian dates

long LastDayOfGregorianMonth(long month, long year) {
// Compute the last date of the month for the Gregorian calendar.

  switch (month) {
  case 2:
    if ((((year % 4) == 0) && ((year % 100) != 0))
        || ((year % 400) == 0))
      return 29;
    else
      return 28;
  case 4:
  case 6:
  case 9:
  case 11: return 30;
  default: return 31;
  }
}

class GregorianDate {
private:
  long year;   // 1...
  long month;  // 1 == January, ..., 12 == December
  long day;    // 1..LastDayOfGregorianMonth(month, year)

public:
  GregorianDate(long m, long d, long y) { month = m; day = d; year = y; }

  GregorianDate(long d) { // Computes the Gregorian date from the absolute date.

    // Search forward year by year from approximate year
    year = d/366;
    while (d >= GregorianDate(1,1,year+1))
      year++;
    // Search forward month by month from January
    month = 1;
    while (d > GregorianDate(month, LastDayOfGregorianMonth(month,year), year))
      month++;
    day = d - GregorianDate(month,1,year) + 1;
  }

  operator long() { // Computes the absolute date from the Gregorian date.
    long N = day;           // days this month
    for (long m = month - 1;  m > 0; m--) // days in prior months this year
      N = N + LastDayOfGregorianMonth(m, year);
    return
      (N                    // days this year
       + 365 * (year - 1)   // days in previous years ignoring leap days
       + (year - 1)/4       // Julian leap days before this year...
       - (year - 1)/100     // ...minus prior century years...
       + (year - 1)/400);   // ...plus prior years divisible by 400
  }

  long GetMonth() { return month; }
  long GetDay() { return day; }
  long GetYear() { return year; }
  void SetDate(long m, long d, long y) { month = m; day = d; year = y; }

};

GregorianDate NthXday(long n, long x, long month, long year, long day = 0)
// The Gregorian date of nth x-day in month, year before/after optional day.
// x = 0 means Sunday, x = 1 means Monday, and so on.  If n<0, return the nth
// x-day before month day, year (inclusive).  If n>0, return the nth x-day
// after month day, year (inclusive).  If day is omitted or 0, it defaults
// to 1 if n>0, and month's last day otherwise.
{
  if (n > 0) {
    if (day == 0)
      day = 1;  // default for positive n
    return GregorianDate
      ((7 * (n - 1)) + XdayOnOrBefore(6 + GregorianDate(month, day, year), x));
  }
  else {
    if (day == 0)
      day = LastDayOfGregorianMonth(month, year);;  // default for negative n
    return GregorianDate
      ((7 * (n + 1)) + XdayOnOrBefore(GregorianDate(month, day, year), x));
  }
}

// Julian dates

const long JulianEpoch = -2; // Absolute date of start of Julian calendar

long LastDayOfJulianMonth(long month, long year) {
// Compute the last date of the month for the Julian calendar.
  switch (month) {
  case 2:
    if ((year % 4) == 0)
      return 29;
    else
      return 28;
  case 4:
  case 6:
  case 9:
  case 11: return 30;
  default: return 31;
  }
}

class JulianDate {
private:
  long year;   // 1...
  long month;  // 1 == January, ..., 12 == December
  long day;    // 1..LastDayOfJulianMonth(month, year)

public:
  JulianDate(long m, long d, long y) { month = m; day = d; year = y; }

  JulianDate(long d) { // Computes the Julian date from the absolute date.
    // Search forward year by year from approximate year
    year = (d + JulianEpoch)/366;
    while (d >= JulianDate(1,1,year+1))
      year++;
    // Search forward month by month from January
    month = 1;
    while (d > JulianDate(month, LastDayOfJulianMonth(month,year), year))
      month++;
    day = d - JulianDate(month,1,year) + 1;
  }

  operator long() { // Computes the absolute date from the Julian date.

    long N = day;                         // days this month
    for (long m = month - 1;  m > 0; m--) // days in prior months this year
      N = N + LastDayOfJulianMonth(m, year);
    return
      (N                     // days this year
       + 365 * (year - 1)    // days in previous years ignoring leap days
       + (year - 1)/4        // leap days before this year...
       + JulianEpoch);       // days elapsed before absolute date 1
  }

  long GetMonth() { return month; }
  long GetDay() { return day; }
  long GetYear() { return year; }
  long absolute();

};

// ISO dates

class IsoDate {
private:
  long year;  // 1...
  long week;  // 1..52 or 53
  long day;   // 1..7

public:
  IsoDate(long w, long d, long y) { week = w; day = d; year = y; }

  IsoDate(long d) { // Computes the ISO date from the absolute date.
    year = GregorianDate(d - 3).GetYear();
    if (d >= IsoDate(1,1,year+1))
      year++;
    if ((d % 7) == 0)
      day = 7;      // Sunday
    else
      day = d % 7;  // Monday..Saturday
    week = 1 + (d - IsoDate(1,1,year)) / 7;
  }

  operator long() { // Computes the absolute date from the ISO date.
    return
      XdayOnOrBefore(GregorianDate(1,4,year),1) // days in prior years
      + 7 * (week - 1)                          // days in prior weeks this year
      + (day - 1);                              // prior days this week
  }

  long GetWeek() { return week; }
  long GetDay() { return day; }
  long GetYear() { return year; }

};

// Islamic dates

const long IslamicEpoch = 227014; // Absolute date of start of Islamic calendar

long IslamicLeapYear(long year) {
// True if year is an Islamic leap year

  if ((((11 * year) + 14) % 30) < 11)
    return 1;
  else
    return 0;
}

long LastDayOfIslamicMonth(long month, long year) {
// Last day in month during year on the Islamic calendar.

  if (((month % 2) == 1) || ((month == 12) && IslamicLeapYear(year)))
    return 30;
  else
    return 29;
}

class IslamicDate {
private:
  long year;   // 1...
  long month;  // 1..13 (12 in a common year)
  long day;    // 1..LastDayOfIslamicMonth(month,year)

public:
  IslamicDate(long m, long d, long y) { month = m; day = d; year = y; }

  IslamicDate(long d) { // Computes the Islamic date from the absolute date.
    if (d <= IslamicEpoch) { // Date is pre-Islamic
      month = 0;
      day = 0;
      year = 0;
    }
    else {
      // Search forward year by year from approximate year
      year = (d - IslamicEpoch) / 355;
      while (d >= IslamicDate(1,1,year+1))
        year++;
      // Search forward month by month from Muharram
      month = 1;
      while (d > IslamicDate(month, LastDayOfIslamicMonth(month,year), year))
        month++;
      day = d - IslamicDate(month,1,year) + 1;
    }
  }

  operator long() { // Computes the absolute date from the Islamic date.
    return (day                      // days so far this month
            + 29 * (month - 1)       // days so far...
            + month/2                //            ...this year
            + 354 * (year - 1)       // non-leap days in prior years
            + (3 + (11 * year)) / 30 // leap days in prior years
            + IslamicEpoch);                // days before start of calendar
  }

  long GetMonth() { return month; }
  long GetDay() { return day; }
  long GetYear() { return year; }

};

// Hebrew dates

const long HebrewEpoch = -1373429; // Absolute date of start of Hebrew calendar

long HebrewLeapYear(long year) 
{
// True if year is an Hebrew leap year
  if ((((7 * year) + 1) % 19) < 7)
    return 1;
  else
    return 0;
}

long LastMonthOfHebrewYear(long year) 
{
// Last month of Hebrew year.
  if (HebrewLeapYear(year))
    return 13;
  else
    return 12;
}

long HebrewCalendarElapsedDays(long year) 
{
// Number of days elapsed from the Sunday prior to the start of the
// Hebrew calendar to the mean conjunction of Tishri of Hebrew year.

  long MonthsElapsed =
    (235 * ((year - 1) / 19))           // Months in complete cycles so far.
    + (12 * ((year - 1) % 19))          // Regular months in this cycle.
    + (7 * ((year - 1) % 19) + 1) / 19; // Leap months this cycle
  long PartsElapsed = 204 + 793 * (MonthsElapsed % 1080);
  long HoursElapsed =
    5 + 12 * MonthsElapsed + 793 * (MonthsElapsed  / 1080)
    + PartsElapsed / 1080;
  long ConjunctionDay = 1 + 29 * MonthsElapsed + HoursElapsed / 24;
  long ConjunctionParts = 1080 * (HoursElapsed % 24) + PartsElapsed % 1080;
  long AlternativeDay;
  if ((ConjunctionParts >= 19440)        // If new moon is at or after midday,
      || (((ConjunctionDay % 7) == 2)    // ...or is on a Tuesday...
          && (ConjunctionParts >= 9924)  // at 9 hours, 204 parts or later...
          && !(HebrewLeapYear(year)))   // ...of a common year,
      || (((ConjunctionDay % 7) == 1)    // ...or is on a Monday at...
          && (ConjunctionParts >= 16789) // 15 hours, 589 parts or later...
          && (HebrewLeapYear(year - 1))))// at the end of a leap year
    // Then postpone Rosh HaShanah one day
    AlternativeDay = ConjunctionDay + 1;
  else
    AlternativeDay = ConjunctionDay;
  if (((AlternativeDay % 7) == 0)// If Rosh HaShanah would occur on Sunday,
      || ((AlternativeDay % 7) == 3)     // or Wednesday,
      || ((AlternativeDay % 7) == 5))    // or Friday
    // Then postpone it one (more) day
    return (1+ AlternativeDay);
  else
    return AlternativeDay;
}

long DaysInHebrewYear(long year) {
// Number of days in Hebrew year.

  return ((HebrewCalendarElapsedDays(year + 1)) -
          (HebrewCalendarElapsedDays(year)));
}

long LongHeshvan(long year) {
// True if Heshvan is long in Hebrew year.

  if ((DaysInHebrewYear(year) % 10) == 5)
    return 1;
  else
    return 0;
}

long ShortKislev(long year) {
// True if Kislev is short in Hebrew year.

  if ((DaysInHebrewYear(year) % 10) == 3)
    return 1;
  else
    return 0;
}

long LastDayOfHebrewMonth(long month, long year) {
// Last day of month in Hebrew year.

  if ((month == 2)
      || (month == 4)
      || (month == 6)
      || ((month == 8) && !(LongHeshvan(year)))
      || ((month == 9) && ShortKislev(year))
      || (month == 10)
      || ((month == 12) && !(HebrewLeapYear(year)))
      || (month == 13))
    return 29;
  else
    return 30;
}

class HebrewDate {
private:
  long year;   // 1...
  long month;  // 1..LastMonthOfHebrewYear(year)
  long day;    // 1..LastDayOfHebrewMonth(month, year)

public:
  HebrewDate(long m, long d, long y) { month = m; day = d; year = y; }

  HebrewDate(long d) { // Computes the Hebrew date from the absolute date.
    year = (d + HebrewEpoch) / 366; // Approximation from below.
    // Search forward for year from the approximation.
    while (d >= HebrewDate(7,1,year + 1))
      year++;
    // Search forward for month from either Tishri or Nisan.
    if (d < HebrewDate(1, 1, year))
      month = 7;  //  Start at Tishri
    else
      month = 1;  //  Start at Nisan
    while (d > HebrewDate(month, (LastDayOfHebrewMonth(month,year)), year))
      month++;
    // Calculate the day by subtraction.
    day = d - HebrewDate(month, 1, year) + 1;
  }

  operator long() { // Computes the absolute date of Hebrew date.
    long DayInYear = day; // Days so far this month.
    if (month < 7) { // Before Tishri, so add days in prior months
                     // this year before and after Nisan.
      long m = 7;
      while (m <= (LastMonthOfHebrewYear(year))) {
        DayInYear = DayInYear + LastDayOfHebrewMonth(m, year);
        m++;
      };
      m = 1;
      while (m < month) {
        DayInYear = DayInYear + LastDayOfHebrewMonth(m, year);
        m++;
      }
    }
    else { // Add days in prior months this year
      long m = 7;
      while (m < month) {
        DayInYear = DayInYear + LastDayOfHebrewMonth(m, year);
        m++;
      }
    }
    return (DayInYear +
            (HebrewCalendarElapsedDays(year)// Days in prior years.
             + HebrewEpoch));         // Days elapsed before absolute date 1.
  }

  long GetMonth() { return month; }
  long GetDay() { return day; }
  long GetYear() { return year; }

//  void SetDate(long d)
//  { 
//    // Search forward for year from the approximation.
//    while (d >= HebrewDate(7,1,year + 1))
//      year++;
//    while (d < HebrewDate(7,1,year))
//      year--;
//    // Search forward for month from either Tishri or Nisan.
//    if (d < HebrewDate(1, 1, year))
//      month = 7;  //  Start at Tishri
//    else
//      month = 1;  //  Start at Nisan
//    while (d > HebrewDate(month, (LastDayOfHebrewMonth(month,year)), year))
//      month++;
//    // Calculate the day by subtraction.
//    day = d - HebrewDate(month, 1, year) + 1;
//  }
//  void GetDate(char * buffer)
//  {
//    long print_month;
//    if (HebrewLeapYear(year)==1)
//    {
//      if (month==12)
//      {
//        sprintf(buffer, "%02lu/%s/%04lu",day,"A6",year);
//      }
//      else if(month==13)
//      {
//        sprintf(buffer, "%02lu/%s/%04lu",day,"B6",year);
//      }
//      else
//      {
//        print_month = 1+(month+5)%12;
//        sprintf(buffer, "%02lu/%02lu/%04lu",day,print_month,year);
//      }
//    }
//    else
//    {
//      print_month = 1+(month+5)%12;
//      sprintf(buffer, "%02lu/%02lu/%04lu",day,print_month,year);
//    }
//  }
};
