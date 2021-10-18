# Arduino Mega GPS clock

Arduino Mega with 20 x 4 LCD taking data from a GPS. Here showing local and UTC time. ![Image](GPSClock-00-EU.jpg)

Blog post with documentation of hardware and some 21 different screens showing solar and lunar position, their rise and set times, moon phase, and more: 
* GPS Clock published, 29.09.2021, https://la3za.blogspot.com/2021/09/gps-clock.html

Blog posts detailing the development:
* Finally figured out the moon,  23.08.2021, https://la3za.blogspot.com/2021/08/finally-figured-out-moon.html
* GPS Clock with Binary Display, 27.04.2020, https://la3za.blogspot.com/2020/04/gps-clock-with-binary-display.html
* Yet another Arduino GPS clock,  6.11.2016, https://la3za.blogspot.com/2016/11/yet-another-arduino-clock.html

Hardware schematic: ![Image](2021-10-18-GPSClock.png)

The code starts default with 
* Central European Time for the local time zone
* European date format, dd.mm.yyyy
* All day names, full and abbreviated, are in English
* GPS baud rate is 9600

See [wiki](https://github.com/la3za/GPSClock/wiki) for more details on hardware and software setup.

