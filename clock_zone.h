// automatic daylight saving time 
#define AUTO_UTC_OFFSET
long utcOffset = 0; // value in minutes (< +/- 720), only used if AUTO_UTC_OFFSET is not set 
                     // usually found automatically by means of Timezone library
                       
/////////////////////////////////////////////////////////////////////////
				   
// https://github.com/JChristensen/Timezone
// https://github.com/JChristensen/Timezone/blob/master/examples/WorldClock/WorldClock.ino
// TimeChangeRule myRule = {abbrev, week, dayOfWeek, month, hour, offset};
//
// Australia Eastern Time Zone (Sydney, Melbourne): OK
//  TimeChangeRule aEDT = {"AEDT", First, Sun, Oct, 2, 660};    // UTC + 11 hours
//  TimeChangeRule aEST = {"AEST", First, Sun, Apr, 3, 600};    // UTC + 10 hours
//  Timezone ausET(aEDT, aEST);

// Indian Standard Time
//  TimeChangeRule inIST = {"IND", Second, Sun, Mar, 2, 330};  // Indian Standard Time = UTC - 5 hours 30 min
//  Timezone India(inIST, inIST);

// Moscow Standard Time (MSK, does not observe DST)
// TimeChangeRule msk = {"MSK", Last, Sun, Mar, 1, 180};
// Timezone tzMSK(msk);

//Central European Time (Frankfurt, Paris): OK
 TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     //Central European Summer Time, UTC + 120 min = 2 hrs
 TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};       //Central European Time. UTC + 60 min = 1 hr
 Timezone CE(CEST, CET);

// United Kingdom (London, Belfast)
// TimeChangeRule BST = {"BST", Last, Sun, Mar, 1, 60};        // British Summer Time
// TimeChangeRule GMT = {"GMT", Last, Sun, Oct, 2, 0};         // Standard Time
// Timezone UK(BST, GMT);
    
// Eastern US 
//  TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, -240};  //UTC - 4 hours
//  TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, -300};   //UTC - 5 hours
//  Timezone usEastern(usEDT, usEST);

// US Central Time Zone (Chicago, Houston)
//  TimeChangeRule usCDT = {"CDT", Second, Sun, Mar, 2, -300};
//  TimeChangeRule usCST = {"CST", First, Sun, Nov, 2, -360};
//  Timezone usCT(usCDT, usCST);

// US Mountain Time Zone (Denver, Salt Lake City)
// TimeChangeRule usMDT = {"MDT", Second, Sun, Mar, 2, -360};
// TimeChangeRule usMST = {"MST", First, Sun, Nov, 2, -420};
// Timezone usMT(usMDT, usMST);

// Arizona is US Mountain Time Zone but does not use DST
// Timezone usAZ(usMST);

// Pacific US 
//  TimeChangeRule usPDT = {"PDT", Second, Sun, Mar, 2, -420};
//  TimeChangeRule usPST = {"PST", First, Sun, Nov, 2, -480};
//  Timezone usPacific(usPDT, usPST);
//

/* also requires as change in clock_zone2.h:
 
         local = CE.toLocal(utc, &tcr); // timezone library
 or
        local     = usEastern.toLocal(utc, &tcr);
        where tcr: Address of a pointer to a TimeChangeRule: 'CE' / 'usEastern' in above examples.
*/
