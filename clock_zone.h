// automatic daylight saving time 

#define AUTO_UTC_OFFSET

long UTCoffset = 0; // value in minutes (< +/- 720), only used if AUTO_UTC_OFFSET is not set 
                     // usually found automatically by means of Timezone library
                       
/////////////////////////////////////////////////////////////////////////
				   
// https://github.com/JChristensen/Timezone
// https://github.com/JChristensen/Timezone/blob/master/examples/WorldClock/WorldClock.ino
// TimeChangeRule myRule = {abbrev, week, dayOfWeek, month, hour, offset};
//
//Central European Time (Frankfurt, Paris): OK
 TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     //Central European Summer Time, UTC + 120 min = 2 hrs
 TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};       //Central European Time. UTC + 60 min = 1 hr
 Timezone CE(CEST, CET);

// Australia Eastern Time Zone (Sydney, Melbourne): OK
//  TimeChangeRule aEDT = {"AEDT", First, Sun, Oct, 2, 660};    // UTC + 11 hours
//  TimeChangeRule aEST = {"AEST", First, Sun, Apr, 3, 600};    // UTC + 10 hours
//  Timezone ausET(aEDT, aEST);

// Indian Standard Time
//  TimeChangeRule inIST = {"IND", Second, Sun, Mar, 2, 330};  // Indian Standard Time = UTC - 5 hours 30 min
//  Timezone India(inIST, inIST);
    
// Eastern US 
//  TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, -240};  //UTC - 4 hours
//  TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, -300};   //UTC - 5 hours
//  Timezone usEastern(usEDT, usEST);


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
