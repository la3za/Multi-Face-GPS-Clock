# Multi Face GPS Clock for the Arduino Mega

The purpose of this project is to make available as many clock and clock related functions as possible given the constraint of a 20 x 4 LCD. The number of different screens are now more than forty, see [Wiki](https://github.com/la3za/Multi-Face-GPS-Clock/wiki) for pictures of them all. Some are useful, some are very specialized, and some are there just because they can be made. All the screens have been laid out with a view to good esthetics and easy readability.

Please join the [groups.io discussion group](https://groups.io/g/Multi-Face-GPS-Clock/) for the Multi Face GPS Clock and related projects.

The clock is inspired by the [Clock Kit from QRPLabs](https://qrp-labs.com/clockn.html). It is based on an Arduino Mega taking time and position data from a GPS, and the latest Arduino IDE (presently 2.3.3). It supports multiple languages in addition to English (default, and always used for UTC) for day names when local time is shown: Norwegian, German, French and Spanish, and Icelandic, Danish, and Swedish and more. 

The new version 2 software makes setup much easier as most, if not all, parameters now can be entered via the rotary control, and there is minimal need for editing of the software.

Below the clock shows local time and UTC time: 

<img src="Images/GPSClock-00-EU.jpg" width="50%">

## YouTube videos
* Here's a YouTube video showing all the 24 screens in the Favorites subset of clock faces, using the Demo mode with 10 seconds per screen: https://youtu.be/AJEWBOJmE1E

* Here's a Youtube video showing how to set up the clock via the new menu system introduced in ver 2 of the software - all engaged by a short press on the rotary control. The clock starts with incorrect GPS baud rate so it won't read the GPS. That is changed and then it shows time in EU date format, Central European Daylight Saving time zone, Spanish local language. This is then changed to US date format, Eastern Daylight Time, and English as local language: https://www.youtube.com/watch?v=mNIamsUlenw

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

The user interface for choosing clock face and for setup is the rotary encoder with push button. Use of the PPS (Pulse per second) output of the GPS for interrupt-driven timing of second pulse is optional from v2.1.0.

![Image](Images/2023-12-25-GPS-Clock.png)

See [wiki](https://github.com/la3za/Multi-Face-GPS-Clock/wiki) for more details on hardware and software setup.

## Other users

Mitch, W4OA, made a nice 3D case for the clock. Click for [build notes](https://github.com/la3za/Multi-Face-GPS-Clock/tree/master/Builds/W4OA/W4OA_Multi_Face_GPS_Clock.pdf). STL-files can be found in the [W4OA directory](https://github.com/la3za/Multi-Face-GPS-Clock/tree/master/Builds/W4OA):

<img src="Builds/W4OA/Multi_Face_GPS_Clock_W4OA.jpg" width="50%">

The 3D-printed case of John, AA7US. Click for [build notes](https://github.com/la3za/Multi-Face-GPS-Clock/tree/master/Builds/AA7US/AA7US-Multi-Face-GPS-Clock-Enclosure-Build-Notes.pdf). STL-files can be found in the [AA7US directory](https://github.com/la3za/Multi-Face-GPS-Clock/tree/master/Builds/AA7US):

<img src="Builds/AA7US/AA7US_IMG_5007.JPG" width="50%">

<img src="Builds/AA7US/AA7US_IMG_5011.JPG" width="50%">

The breadboard of Michael, DK7UX, with GPS from QRPLabs QLG1 or QLG2. It shows how simple the hardware is:

<img src="Builds/DK7UX.jpg" width="50%">

Here's the clock made by Stef, DM5TU. Note the support for the German language in the clock:

<img src="Builds/DM5TU/photo-5.jpg" width="50%">

<img src="Builds/DM5TU/photo-4.jpg" width="50%">

<img src="Builds/DM5TU/photo-2.jpg" width="50%">

<img src="Builds/DM5TU/photo-1.jpg" width="50%">

Stef's clock is built in a [Bahar enclosure 150 x 70 z 100 mm](https://bahar-enclosure.com/products/bda-40004-w100-150-x-70-x-100-mm), also found on [AliExpress(https://de.aliexpress.com/item/1005006813663747.html?gps-id=pcFullPieceDiscountItemList&scm=1007.24625.271276.0&scm_id=1007.24625.271276.0&scm-url=1007.24625.271276.0&pvid=dd13dbc5-070c-4c98-bf19-d16a37ea5364&_t=gps-id%3ApcFullPieceDiscountItemList%2Cscm-url%3A1007.24625.271276.0%2Cpvid%3Add13dbc5-070c-4c98-bf19-d16a37ea5364%2Ctpp_buckets%3A668%232846%238115%232000&pdp_ext_f=%7B%22order%22%3A%2237%22%2C%22eval%22%3A%221%22%2C%22sceneId%22%3A%2214625%22%7D&pdp_npi=4%40dis%21EUR%2113.29%2112.79%21%21%2114.74%2114.18%21%402103956b17487939551564870e284e%2112000038391338091%21rec%21DE%21%21ABXZ&spm=a2g0o.store_pc_home.fullPieceDiscountItemList_2008863407519.1005006813663747&gatewayAdapt=glo2deu)

Please get in touch if you want to display your implementation here!



