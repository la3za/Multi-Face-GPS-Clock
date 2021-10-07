# Arduino Mega GPS clock

Arduino Mega with 20 x 4 LCD taking data from a GPS. Here showing local and UTC time. ![Image](https://1.bp.blogspot.com/-q5jTOZavFj0/YVOEDw-q6PI/AAAAAAAAMHc/-ox9Ai4zouUaZMNwj6tVyzogB-QYDQu2ACLcBGAsYHQ/s2048/GPSClock-00-EU.jpg)

Blog post with documentation of hardware and some 17 different screens showing solar and lunar data and more: 
* GPS Clock published, 29.09.2021, https://la3za.blogspot.com/2021/09/gps-clock.html

Blog posts detailing the development:
* Finally figured out the moon,  23.08.2021, https://la3za.blogspot.com/2021/08/finally-figured-out-moon.html
* GPS Clock with Binary Display, 27.04.2020, https://la3za.blogspot.com/2020/04/gps-clock-with-binary-display.html
* Yet another Arduino GPS clock,  6.11.2016, https://la3za.blogspot.com/2016/11/yet-another-arduino-clock.html

Hardware schematic: ![Image](https://1.bp.blogspot.com/-54JcLxapwBY/YV4Qk6-JCDI/AAAAAAAAMLM/XcpV2mENt_QEhN9Yw1UA-HAWKpKjjZ9xgCLcBGAsYHQ/s920/2021-10-06-GPSClock.png)

The code starts default with 
* Central European Time for the local time zone
* All day names, full and abbreviated, are in English
* GPS baud rate is 9600

The code can easily be adapted to local abbreviated day names for other languages when local time is displayed, i.e. French, Spanish, Swedish, German etc. If you do that, then please share it and make it available.

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version. See source code of GPSClock.ino
