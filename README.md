# RPi-I2C

Background
----------
With the Raspberry Pi lacking analog pins, there is currently no way to actively monitor battery voltage in the GameBoy Zero (or any other Pi projects for that matter). Using an ATTiny85 chip or similar chip and the I²C bus, it is possible to request analog data using only two GPIO pins.

Required Hardware and Components
--------------------------------
- [Raspberry Pi Zero](https://www.raspberrypi.org/products/pi-zero/) (or Model B+, Raspberry Pi 2 and Pi 3)
- [ATTiny85 Development Board](http://digistump.com/products/1) (or an alternative Arduino compatible board with ADC pins)

Current State
-------------
This repo is currently for experimentation and testing.

Going Forward
-------------
The end result will be a script which periodically polls the ATTiny85 for analog data. This data can beused to display a frame buffer overlay on the screen, such as a battery indicator as an icon and/or percentage.

Links
-----
More detail can be found on this thread:
http://sudomod.com/forum/viewtopic.php?f=20&t=1768

Feel free to contact me on the [Sudomod forums] (www.sudomod.com/forum) or on the [Sudomod Discord channel] (https://discordapp.com/channels/188359728454303744/188359728454303744)
