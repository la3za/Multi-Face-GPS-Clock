// Set up a set of time zones, starting in Central Europe and going westwards.
// may be selected by pushing button on rotary encoder and scrolling to correct menu

// included (i.e. called) inside loop(), every time the gps is updated, in order to be up to date with daylight saving

//
// https://github.com/JChristensen/Timezone
// https://github.com/JChristensen/Timezone/blob/master/examples/WorldClock/WorldClock.ino
// TimeChangeRule myRule = {abbrev, week, dayOfWeek, month, hour, offset};

// Central European Time (Frankfurt, Paris)
   TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     //Central European Summer Time, UTC + 120 min = 2 hrs
   TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};       //Central European Time. UTC + 60 min = 1 hr
   Timezone CE(CEST, CET);

// United Kingdom (London, Belfast) + Ireland
  TimeChangeRule BST = {"BST", Last, Sun, Mar, 1, 60};        // British Summer Time
  TimeChangeRule GMT = {"GMT", Last, Sun, Oct, 2, 0};         // Standard Time
  Timezone UK(BST, GMT);

// Canadian Atlantic
  TimeChangeRule ADT = {"ADT", Second, Sun, Mar, 2, -180};  //UTC - 3 hours
  TimeChangeRule AST = {"AST", First, Sun, Nov, 2, -240};   //UTC - 4 hours
  Timezone Atlantic(ADT, AST);
      
// Eastern US 
  TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, -240};  //UTC - 4 hours
  TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, -300};   //UTC - 5 hours
  Timezone usEastern(usEDT, usEST);

// US Central Time Zone (Chicago, Houston)
  TimeChangeRule usCDT = {"CDT", Second, Sun, Mar, 2, -300};
  TimeChangeRule usCST = {"CST", First, Sun, Nov, 2, -360};
  Timezone usCT(usCDT, usCST);

// US Mountain Time Zone (Denver, Salt Lake City)
  TimeChangeRule usMDT = {"MDT", Second, Sun, Mar, 2, -360};
  TimeChangeRule usMST = {"MST", First, Sun, Nov, 2, -420};
  Timezone usMT(usMDT, usMST);
 
// Arizona is US Mountain Time Zone but does not use DST
  TimeChangeRule AZ = {"AZ", First, Sun, Nov, 2, -420};
  Timezone usAZ(AZ);

// Pacific US 
  TimeChangeRule usPDT = {"PDT", Second, Sun, Mar, 2, -420};
  TimeChangeRule usPST = {"PST", First, Sun, Nov, 2, -480};
  Timezone usPacific(usPDT, usPST);

// Alaskan US
  TimeChangeRule usAKDT = {"AKDT", Second, Sun, Mar, 2, -480}; // UTC  - 8
  TimeChangeRule usAKST = {"AKST", First, Sun, Nov, 2, -540}; 
  Timezone usAlaska(usAKDT, usAKST);

// Hawaii US
  TimeChangeRule usHK = {"HST", Second, Sun, Mar, 2, -600}; // UTC - 10 hrs
  Timezone usHawaii(usHK);

/// New day! ////////

// Australia time zones: https://forum.arduino.cc/t/an-array-of-time-zones/406030

// Australia Eastern Time Zone (Sydney, Melbourne): OK
  TimeChangeRule aEDT = {"AEDT", First, Sun, Oct, 2, 660};    // UTC + 11 hours
  TimeChangeRule aEST = {"AEST", First, Sun, Apr, 3, 600};    // UTC + 10 hours
  Timezone ausNSW(aEDT, aEST);
  
  Timezone ausQLD(aEST, aEST); // no daylight saving

  //Australia Central Time Zone (Darwin)
  TimeChangeRule aCDT = {"ACDT", First, Sun, Oct, 2, 630};    //UTC + 10.5 hours
  TimeChangeRule aCST = {"ACST", First, Sun, Apr, 3, 570};    //UTC +  9.5 hours
  Timezone ausNT(aCST, aCST); // no daylight saving
  
  Timezone ausSA(aCDT, aCST);
 
// Japan standard time, no daylight saving
  TimeChangeRule JAP = {"JST", Second, Sun, Mar, 2, 540};  // Japan  Time = UTC + 9 hours
  Timezone Japan(JAP, JAP);

//Australia Western Time Zone (Perth)
  TimeChangeRule aWST = {"AWST", First, Sun, Apr, 3, 480};    //UTC + 8 hours
  Timezone ausWA(aWST, aWST); 
 
// China
  TimeChangeRule CN = {"CHN", Second, Sun, Mar, 2, 480};  // China  Time = UTC + 8 hours
  Timezone China(CN, CN);

// Indian Standard Time
  TimeChangeRule inIST = {"IND", Second, Sun, Mar, 2, 330};  // Indian Standard Time = UTC - 5 hours 30 min
  Timezone India(inIST, inIST);
 
// Turkey  Time
   TimeChangeRule TT = {"TUR", Second, Sun, Mar, 2, 180};  // Turkey  Time = UTC + 3 hours, no DST
   Timezone Turkey(TT, TT);
  
//Eastern European Time (Helsinki +)
   TimeChangeRule EEST = {"EEST", Last, Sun, Mar, 2, 180};     //Central European Summer Time, UTC + 120 min = 2 hrs
   TimeChangeRule EET = {"EET ", Last, Sun, Oct, 3, 120};       //Central European Time. UTC + 60 min = 1 hr
   Timezone EasternEurope(EEST, EET);
 
Timezone* timeZones_arr[NUMBER_OF_TIME_ZONES] =
{
    &CE, &UK, &Atlantic, &usEastern, &usCT, &usMT, &usAZ, &usPacific, &usAlaska, &usHawaii,
    &ausNSW, &ausQLD, &ausNT, &ausSA, &Japan, &ausWA, &China, &India, &Turkey, &EasternEurope
};



/// END ////
