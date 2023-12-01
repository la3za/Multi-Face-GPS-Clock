# Multi Face GPS clock for the Arduino Mega

The purpose of this project is to make available as many clock and clock related functions as possible given the constraint of a 20 x 4 LCD. The number of different screens are now forty, see [Wiki](https://github.com/la3za/Multi-Face-GPS-Clock/wiki) for pictures of them all. Some are useful, some are very specialized, and some may be unexpected or outright weird. All the screens have been laid out with a view to good esthetics and easy readability.

The clock is inspired by the [Clock Kit from QRPLabs](https://qrp-labs.com/clockn.html). It is based on an Arduino Mega taking time and position data from a GPS. It supports multiple languages in addition to English (default) for day names when local time is shown: Norwegian, German, French and Spanish, and Icelandic, Danish, and Swedish. 

The new version 2.0.0 software makes setup much easier as most if not all parameters now can be entered via the rotary control, and there is minimal need for editing of the software.

Below the clock shows local time and UTC time. ![Image](Images/GPSClock-00-EU.jpg)

## YouTube videos
* Here's a YouTube video showing all the 24 screens in the Favorites subset of clock faces, using the Demo mode with 10 seconds per screen: https://youtu.be/AJEWBOJmE1E

* Here's a Youtube video showing how to set up the clock via the new menu system introduced in ver 2.0.0 of the software - all engaged by a short press on the rotary control. The clock starts with incorrect GPS baud rate so it won't read the GPS. That is changed and then it shows time in EU date format, Central European Daylight Saving time zone, Spanish local language. This is then changed to US date format, Eastern Daylight Time, and English as local language: https://www.youtube.com/watch?v=mNIamsUlenw

## Examples of clock faces:

Solar rise/set time. Line 2 Actual, Line 3 Civil, and Line 4 Nautical rise and set times:

<img src="Images/SunUpDown.jpg" width="40%">

Position data:

<img src="Images/15b-UTCPosition.jpg" width="40%">

Lunar display with line 2 showing elevation and azimuth for the moon, Line 3 shows next time to set and at what azimuth, and Line 4 distance from earth in % of maximum and in km, and finally lunar phase, that the illumination is falling, and % illumination:

<img src="Images/04-LocalMoon.jpg" width="40%">

Binary, Octal, Decimal and Hex display of time:

<img src="Images/21-BinOctDecHex.jpg" width="40%">

Word clock:

<img src="Images/Word.jpg" width="40%">

Prediction of lunar eclipses:

<img src="Images/LunarEclipses.jpg" width="40%">

Math calculation clock:

<img src="Images/MathSub.jpg" width="40%">

See [the project wiki](https://github.com/la3za/Multi-Face-GPS-Clock/wiki) for images of the various screens as well as screens showing the local language adaptations.

The LA3ZA blog has posts with documentation of hardware and the different screens showing solar and lunar position, their rise and set times, moon phase, date for Easter Sunday in Gregorian and Julian calendars and much more. See keyword ["Multi-face GPS clock"](https://la3za.blogspot.com/search/label/Multi-face%20GPS%20clock), starting with early development in 2016 up to the present. 

## Hardware schematic, supported from v2.0.0 and onwards:

The pot is not longer needed as the new menu system takes care of the backlight control and much more. The v1.1.0 schematics is still valid.

## Hardware schematic, supported from v1.1.0 and onwards: 

Starting with version 1.1.0, a rotary encoder with a push button is supported, in addition to the separate push buttons and the pot of the original code. In order not to clutter the schematic too much, only the rotary encoder is shown in the following schematic. Here the rotary encoder selects screen number. A push on the button lets the rotary encoder control the backlight. Alternatively, the backlight may be controlled directly by a separate trim pot and not by software. In that case, a push on the rotary encoder lets one toggle back and forth to a user-selectable favorite screen. 

![Image](Images/2023-06-05-GPS-Clock.png)

## Hardware schematic, supported from v1.0.0: ![Image](Images/2021-10-18-GPSClock.png)

The two push buttons control whether the screen number increases or decreases by one. The pot controls the backlight of the LCD. These are the only features supported from v1.0.0 to v1.0.4 of the code.

See [wiki](https://github.com/la3za/Multi-Face-GPS-Clock/wiki) for more details on hardware and software setup.

Note new version numbering which follows the semantic versioning standard. Thus previous versions v1.01, 1.02, 1.03, 1.04, 1.10 are now v1.0.1, 1.0.2, 1.0.3, 1.0.4, and v1.1.0 and so on.

