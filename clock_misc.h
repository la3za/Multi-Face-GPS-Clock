//#include "clock_timezone.h"
#define AUTO_UTC_OFFSET
//Central European Time (Frankfurt, Paris):
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     //Central European Summer Time
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};       //Central European Time
Timezone CE(CEST, CET);

#define SUN_LOW -18 // -6, -12, -18, under this value, the solar height changes to peak value at noon rather than actual value
#define MOON_DARK_BACKGROUND

#define LCD_NORSK

//#define FEATURE_SERIAL_OUTPUT
