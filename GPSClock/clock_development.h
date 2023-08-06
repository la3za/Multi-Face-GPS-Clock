//
// IN DEVELOPMENT


// Reminder()
// bubbleSort()

////////////////////////////////////////////////////////////////
// Menu item ///////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
// for use in Reminder (to be put in GPSClock.ino):
  int indStart = 0;

  typedef struct
  {
      char Name[11];
      int Day;
      int Month;
      int Year;
  }  person_type;



// Data for Reminder(): 
// Set birthdates and initial/name for user in Reminder()

person_type person[16] = 
                     { {"Eline     ",  2, 12, 2014},
                       {"Filip     ", 31,  3, 2016},
                       {"Ada       ",  2,  8, 2017}, // 2.8
                       {"Fredrik   ", 13, 11, 2019},
                       {"Rune      ", 16,  8, 1981},
                       {"Andreas   ",  8,  5, 1984},
                       {"Martin    ", 10,  4, 1987},
                       {"Thomas    ", 10,  4, 1987},
                       {"Kristine  ", 10,  4, 1986},
                       {"Celeste   ",  9,  2, 1990},
                       {"Lise      ", 21, 12, 1954},
                       {"Sverre    ", 29, 12, 1954}, 
                       {"Otto      ", 12,  7, 1960},
                       {"Odd Egil  ", 16,  3, 1950},   
                       {"Bente     ", 10, 11, 1959},
                       {"Marit     ", 12,  8, 1963} }; //?? år                       
char yearSymbol = char(SCAND_aa_SMALL); // Norwegian å


//person_type person[8] = 
// Beatles: 
//                    { {"Paul       ", 18,  6, 1942},  
//                      {"Ringo      ",  7,  7, 1940},
//                      {"George     ", 25,  2, 1943},   // 29.11.2001
//                      {"John       ",  9, 10, 1940},   //  8.12.1980
// Rolling Stones:                      
//                      {"Mick       ", 26,  7, 1943},  
//                      {"Keith      ", 18, 12, 1943},
//                      {"Ron        ",  1,  6, 1947},
//                      {"Charlie    ",  2,  6, 1941} }; // 24.8.2021 
//char yearSymbol = 'y';  // 'English for 'year'
  
  int noOfReminderScreens = 2;
  int holdTimeReminderScreen = 7; // seconds, if more than one screen of names is to be shown
  
//}


///////////////////////////////////////////////////////////////////////////////////////////
// from https://forum.arduino.cc/t/the-order-of-bubble-sorting/657618/5
// modified for float input

void bubbleSort(float a[],int index_array[], int size) {

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



// Menu item //////////////////////////////////////////////////////////////////////////////////////////


/*****
Purpose: 
Display memorable dates and elapsed time since those dates

Argument List: none. Input is from clock_custom_routines

Return value: none

Issues: Follows UNIX time which is referenced to UTC, so changes birthday order according to UTC and not local time
*****/

void Reminder() // 
{
  int32_t diffSec;
  time_t Tsec;
  tmElements_t T;
  int ind;
//  char yearSymbol;
//  person_type person;
// 
//  userNames(&person[0].Name[0]); // read data 

  int lengthData = sizeof(person) / sizeof(person[0]);
  float timeToBirthday[lengthData+2];
  float diffYearsF[lengthData+2];
  int indexArray[lengthData+2];
  float Age1970[lengthData+2];

  //Serial.print("lengthData ");Serial.println(lengthData);

  for (ind=0; ind<lengthData; ind++)
  {
//      Serial.print(ind); Serial.print(": ");Serial.print(person[ind].Year);Serial.print(" ");Serial.print(person[ind].Month);
//                         Serial.print(" ");Serial.print(person[ind].Day);Serial.print(" ");Serial.println(person[ind].Name);
      Age1970[ind] = max(0.0, 1970.0 - person[ind].Year); // non-zero if older than 1970
      T.Year  = max(person[ind].Year - 1970, 0);          // zero if older than 1970, else difference
      T.Month = person[ind].Month;
      T.Day   = person[ind].Day;
      //T.Hour = 0; T.Minute = 0; T.Second = 0; 
      T.Hour = 23; T.Minute = 59; T.Second = 59;  // to ensure that a birhday today is shown first
 
      Tsec = makeTime( T ); //seconds since 1/1/1970
      diffSec = now() - Tsec;         // difference in seconds to now
      // compensate for utcOffset in minutes: local = now() + utcOffset * 60;
      diffSec = diffSec + utcOffset*60.0;
      
      diffYearsF[ind] =  diffSec/31557600.0 + Age1970[ind];  // [no of sec / year = 31,557,600]
      
      timeToBirthday[ind]  = 1-(diffYearsF[ind] - int(diffYearsF[ind])); // fraction of year
       
//      Serial.print(" ");Serial.print(ind);Serial.print(" ");Serial.print(person[ind].Name); 
//                       Serial.print(", "); Serial.print(diffYearsF[ind]); Serial.print(" "); Serial.println(timeToBirthday[ind],4);  
//      float oneDay = 1/365.; // = 0.00274 
//      if (timeToBirthday[ind] > 1.0 - oneDay) // within a day or so of birthday today - trick to show a birthday first even when on the same day 
//      {
//        timeToBirthday[ind] = 0.0;  // for same day birthday
//        Serial.print(" ---> ");Serial.println(timeToBirthday[ind]); 
//      }
      indexArray[ind] = ind; 
  }
  
  // sort: find indices according to first, second, etc birthday relative to today's date
  bubbleSort(timeToBirthday, indexArray, lengthData);
 
  static char outstr[15];  
  
   
  if (secondGPS % holdTimeReminderScreen == 0) // new display every holdTimeReminderScreen seconds
  { 
 //       Serial.println(secondGPS);
        indStart =  indStart + 4;
        if (indStart >= lengthData | (indStart + 1)/4 >= noOfReminderScreens) indStart = 0; // over the top >= or >?
 //       indStart = min(indStart, lengthData-4); // fill up so the last screen always is filled
 }     
  
  //Serial.print("     indStart: ");Serial.println(indStart);
  
  for (ind=indStart; ind<indStart + 4; ind++)
  {
    lcd.setCursor(0,ind-indStart);
  
    if (ind < lengthData)
    {       
      lcd.print(person[indexArray[ind]].Name); lcd.print(" ");
      lcd.setCursor(9,ind-indStart);
      LcdDate(person[indexArray[ind]].Day, person[indexArray[ind]].Month, 0);        
      lcd.print(F("  ")); 
      lcd.setCursor(15,ind-indStart);
      dtostrf(diffYearsF[indexArray[ind]], 4, 1, outstr);
      lcd.print(outstr); lcd.print(yearSymbol); 
    }
    else
    {
      lcd.print(F("                    "));    // blank line when there is no more data to display 
    }
  }
 }

/////////////////////////////////////////////////////////////////


//////////////////// THE END ////////////////////////////////////////
