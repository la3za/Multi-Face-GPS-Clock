// Program that puts birthday data into EEPROM for use in Reminder() function of GPSClock
// starting from address EEPROM_OFFSET2 which must have the same value as in GPSClock
//
// Program writes data in struct person[], 
// then reads it and prints it on serial port for verification
//
// Sverre Holm, 21.01.2024
//

#include <EEPROM.h>

#define EEPROM_OFFSET2 100
#define MAX_NO_OF_PERSONS 16
#define LENGTH_NAME     10  // min 9 + 1, was 11
int addrOffset;
typedef struct
  {
      char Name[LENGTH_NAME];
      byte Day; // was int
      byte Month;
      int Year;
  }  person_type;


// LENGTH_NAME-1 long field for name, then day, month, year

person_type person[6] = 
                   { {"Father   ", 18,  6, 1972}, 
                     {"Mother   ",  7,  7, 1970},
                     {"Son      ", 25,  2, 1993},
                     {"Son      ",  9, 10, 2000},                  
                     {"Daughter ", 26,  7, 2003},  
                     {"Daughter ", 18, 12, 2005}}; 

// person_type person[8] = 
//                    { {"Paul     ", 18,  6, 1942},  // Beatles
//                      {"Ringo    ",  7,  7, 1940},
//                      {"George   ", 25,  2, 1943},   // 29.11.2001
//                      {"John     ",  9, 10, 1940},   //  8.12.1980                 
//                      {"Mick     ", 26,  7, 1943},  // Rolling Stones   
//                      {"Keith    ", 18, 12, 1943},
//                      {"Ron      ",  1,  6, 1947},
//                      {"Charlie  ",  2,  6, 1941} }; // 24.8.2021 





void writeStringToEEPROM(int addrOffset, const String &strToWrite)
{
  byte len = strToWrite.length();
  EEPROM.write(addrOffset, len);
  for (int i = 0; i < len; i++)
  {
    EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
  }
}
String readStringFromEEPROM(int addrOffset)
{
  int newStrLen = EEPROM.read(addrOffset);
  char data[newStrLen + 1];
  for (int i = 0; i < newStrLen; i++)
  {
    data[i] = EEPROM.read(addrOffset + 1 + i);
  }
  data[newStrLen] = '\ 0'; // !!! NOTE !!! Remove the space between the slash "/" and "0" (I've added a space because otherwise there is a display bug)
  return String(data);
}

//////////////////////////////////////////////////////////////////////////////////

int readIntFromEEPROM(int address)
// from https://roboticsbackend.com/arduino-store-int-into-eeprom/
{
  return (EEPROM.read(address) << 8) + EEPROM.read(address + 1);
}

void updateIntIntoEEPROM(int address, int number)
// modified from writeIntIntoEEPROM from https://roboticsbackend.com/arduino-store-int-into-eeprom/
{ 
  EEPROM.update(address, number >> 8);
  EEPROM.update(address + 1, number & 0xFF);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void writePersonEEPROM()
{
  byte lengthData = sizeof(person)/sizeof(person[0]);
  EEPROM.write(EEPROM_OFFSET2, lengthData);               // position 0, write length of struct 
  
  for (int i=0; i<lengthData; ++i)
  {
    addrOffset = EEPROM_OFFSET2 + 1 + i*20;
    writeStringToEEPROM(addrOffset, person[i].Name);      // positions 1, ... , 13 (1 more than LENGTH_NAME)
    EEPROM.write(addrOffset + 14, person[i].Day);
    EEPROM.write(addrOffset + 15, person[i].Month);
    updateIntIntoEEPROM(addrOffset + 16, person[i].Year); // positions 15, 16
    Serial.println(i);
  }
}


void readPersonEEPROM()
{
  byte lengthData;
  lengthData = EEPROM.read(EEPROM_OFFSET2);      // position 0,  length of struct 
  Serial.print("Number of records: ");Serial.println(lengthData);
  for (int i=0; i<lengthData; ++i)
  {
    addrOffset = EEPROM_OFFSET2 + 1 + i*20;
    String Nan = readStringFromEEPROM(addrOffset);      // positions 1, ... , 13
      Nan.toCharArray(person[i].Name,LENGTH_NAME);
    person[i].Day = EEPROM.read(addrOffset + 14);
    person[i].Month = EEPROM.read(addrOffset + 15);
    int Yr = readIntFromEEPROM(addrOffset + 16); // positions 16, 17
      person[i].Year = Yr;

   Serial.print(i); Serial.print(", "); Serial.print(addrOffset); Serial.print(": "); Serial.print(person[i].Name); Serial.print(": ");Serial.print(person[i].Day);Serial.print("-");Serial.print(person[i].Month);Serial.print("-");Serial.println(person[i].Year);
  }  
}

void setup()
{
  Serial.begin(115200);
  Serial.println(F("EEPROM write"));
  writePersonEEPROM();
  readPersonEEPROM();
}
///////////////////////////////////////////////////////
void loop()
{

}
