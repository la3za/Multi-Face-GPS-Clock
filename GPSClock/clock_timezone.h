// Set up a set of time zones, starting in Central Europe and going westwards.
// may be selected by pushing button on rotary encoder and scrolling to correct menu

// included (i.e. called) inside loop(), every time the gps is updated, in order to be up to date with daylight saving

// Make sure not to exceed maxNumTimeZones (nom 25) in GPSClock.ino 
//
// https://github.com/JChristensen/Timezone
// https://github.com/JChristensen/Timezone/blob/master/examples/WorldClock/WorldClock.ino
// TimeChangeRule myRule = {abbrev, week, dayOfWeek, month, hour, offset};

   int iZone = 0;  // counter for entering zone data

// Central European Time (Frankfurt, Paris)
   TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     //Central European Summer Time, UTC + 120 min = 2 hrs
   TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};       //Central European Time. UTC + 60 min = 1 hr
   Timezone CE(CEST, CET);
   local[iZone] = CE.toLocal(utc, &tcr[iZone]); // timezone library: &tcr gives pointer to time zone name
   
//   Serial.print(iZone); Serial.print(" "); Serial.print(tcr[iZone] -> abbrev);
//   utcOffset = local[iZone] / long(60) - utc / long(60); 
//   Serial.print(" ");Serial.println(float(utcOffset)/60);
   iZone = iZone+1;

// iZone==1: problems showing  -> abbrev in setup menu - why???  

// United Kingdom (London, Belfast) + Ireland
  TimeChangeRule BST = {"BST", Last, Sun, Mar, 1, 60};        // British Summer Time
  TimeChangeRule GMT = {"GMT", Last, Sun, Oct, 2, 0};         // Standard Time
  Timezone UK(BST, GMT);
  local[iZone] = UK.toLocal(utc, &tcr[iZone]);

//  Serial.print(iZone); Serial.print(" "); Serial.print(tcr[iZone] -> abbrev);
//  utcOffset = local[iZone] / long(60) - utc / long(60); 
//  Serial.print(" ");Serial.println(float(utcOffset)/60);   
   iZone = iZone+1;   


// Canadian Atlantic
  TimeChangeRule ADT = {"ADT", Second, Sun, Mar, 2, -180};  //UTC - 3 hours
  TimeChangeRule AST = {"AST", First, Sun, Nov, 2, -240};   //UTC - 4 hours
  Timezone Atlantic(ADT, AST);
  local[iZone] = Atlantic.toLocal(utc, &tcr[iZone]);
  iZone = iZone+1;
     
// Eastern US 
  TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, -240};  //UTC - 4 hours
  TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, -300};   //UTC - 5 hours
  Timezone usEastern(usEDT, usEST);
  local[iZone] = usEastern.toLocal(utc, &tcr[iZone]);
  iZone = iZone+1;

// US Central Time Zone (Chicago, Houston)
  TimeChangeRule usCDT = {"CDT", Second, Sun, Mar, 2, -300};
  TimeChangeRule usCST = {"CST", First, Sun, Nov, 2, -360};
  Timezone usCT(usCDT, usCST);
  local[iZone] = usCT.toLocal(utc, &tcr[iZone]);
  iZone = iZone+1;

// US Mountain Time Zone (Denver, Salt Lake City)
  TimeChangeRule usMDT = {"MDT", Second, Sun, Mar, 2, -360};
  TimeChangeRule usMST = {"MST", First, Sun, Nov, 2, -420};
  Timezone usMT(usMDT, usMST);
  local[iZone] = usMT.toLocal(utc, &tcr[iZone]);
  iZone = iZone+1;
 
// Arizona is US Mountain Time Zone but does not use DST
  TimeChangeRule AZ = {"AZ", First, Sun, Nov, 2, -420};
  Timezone usAZ(AZ);
  local[iZone] = usAZ.toLocal(utc, &tcr[iZone]);
  iZone = iZone+1;

// Pacific US 
  TimeChangeRule usPDT = {"PDT", Second, Sun, Mar, 2, -420};
  TimeChangeRule usPST = {"PST", First, Sun, Nov, 2, -480};
  Timezone usPacific(usPDT, usPST);
  local[iZone] = usPacific.toLocal(utc, &tcr[iZone]);
  iZone = iZone+1;

// Alaskan US
  TimeChangeRule usAKDT = {"AKDT", Second, Sun, Mar, 2, -480}; // UTC  - 8
  TimeChangeRule usAKST = {"AKST", First, Sun, Nov, 2, -540}; 
  Timezone usAlaska(usAKDT, usAKST);
  local[iZone] = usAlaska.toLocal(utc, &tcr[iZone]);
  iZone = iZone+1;


// Hawaii US
  TimeChangeRule usHK = {"HST", Second, Sun, Mar, 2, -600}; // UTC - 10 hrs
  Timezone usHawaii(usHK);
  local[iZone] = usHawaii.toLocal(utc, &tcr[iZone]);
  iZone = iZone+1;


/// New day! ////////

// Australia time zones: https://forum.arduino.cc/t/an-array-of-time-zones/406030

// Australia Eastern Time Zone (Sydney, Melbourne): OK
  TimeChangeRule aEDT = {"AEDT", First, Sun, Oct, 2, 660};    // UTC + 11 hours
  TimeChangeRule aEST = {"AEST", First, Sun, Apr, 3, 600};    // UTC + 10 hours
  Timezone ausNSW(aEDT, aEST);
  local[iZone] = ausNSW.toLocal(utc, &tcr[iZone]); 
  iZone = iZone+1;
  
  Timezone ausQLD(aEST, aEST); // no daylight saving
  local[iZone] = ausQLD.toLocal(utc, &tcr[iZone]); 
  iZone = iZone+1;

  //Australia Central Time Zone (Darwin)
  TimeChangeRule aCDT = {"ACDT", First, Sun, Oct, 2, 630};    //UTC + 10.5 hours
  TimeChangeRule aCST = {"ACST", First, Sun, Apr, 3, 570};    //UTC +  9.5 hours
  Timezone ausNT(aCST, aCST); // no daylight saving
  local[iZone] = ausNT.toLocal(utc, &tcr[iZone]); 
  iZone = iZone+1;
  
  Timezone ausSA(aCDT, aCST);
  local[iZone] = ausSA.toLocal(utc, &tcr[iZone]); 
  iZone = iZone+1;
 
// Japan standard time, no daylight saving
  TimeChangeRule JAP = {"JST", Second, Sun, Mar, 2, 540};  // Japan  Time = UTC + 9 hours
  Timezone Japan(JAP, JAP);
  local[iZone]  = Japan.toLocal(utc, &tcr[iZone]);
  iZone = iZone+1;

//Australia Western Time Zone (Perth)
  TimeChangeRule aWST = {"AWST", First, Sun, Apr, 3, 480};    //UTC + 8 hours
  Timezone ausWA(aWST, aWST); 
  local[iZone]  = ausWA.toLocal(utc, &tcr[iZone]);
  iZone = iZone+1;
 
// China
  TimeChangeRule CN = {"CHN", Second, Sun, Mar, 2, 480};  // China  Time = UTC + 8 hours
  Timezone China(CN, CN);
  local[iZone]  = China.toLocal(utc, &tcr[iZone]);
  iZone = iZone+1;

// Indian Standard Time
  TimeChangeRule inIST = {"IND", Second, Sun, Mar, 2, 330};  // Indian Standard Time = UTC - 5 hours 30 min
  Timezone India(inIST, inIST);
  local[iZone] = India.toLocal(utc, &tcr[iZone]);
  iZone = iZone+1;

// Turkey  Time
   TimeChangeRule TT = {"TUR", Second, Sun, Mar, 2, 180};  // Turkey  Time = UTC + 3 hours, no DST
   Timezone Turkey(TT, TT);
   local[iZone] = Turkey.toLocal(utc, &tcr[iZone]);
   iZone = iZone+1;


//Eastern European Time (Helsinki +)
   TimeChangeRule EEST = {"EEST", Last, Sun, Mar, 2, 180};     //Central European Summer Time, UTC + 120 min = 2 hrs
   TimeChangeRule EET = {"EET ", Last, Sun, Oct, 3, 120};       //Central European Time. UTC + 60 min = 1 hr
   Timezone EasternEurope(EEST, EET);
   local[iZone] = EasternEurope.toLocal(utc, &tcr[iZone]); // timezone library: &tcr gives pointer to time zone name
   
//   Serial.print(iZone); Serial.print(" "); Serial.print(tcr[iZone] -> abbrev);
//   utcOffset = local[iZone] / long(60) - utc / long(60); 
//   Serial.print(" ");Serial.println(float(utcOffset)/60);
//   Serial.println(" ");
   
   numTimeZones = iZone;

/// END ////
