# RPi-I2C

Background
----------
With the Raspberry Pi lacking analog pins, there is currently no way to actively monitor battery voltage in the GameBoy Zero (or any other Pi projects for that matter). Using an ATTiny85 chip or similar and the I²C bus, it is possible to request analog data using only two GPIO pins.

Required Hardware and Components
--------------------------------
- [Raspberry Pi Zero](https://www.raspberrypi.org/products/pi-zero/) (or Model B+, Raspberry Pi 2 and Pi 3)
- [ATTiny85 Development Board](http://digistump.com/products/1) (or an alternative Arduino compatible board with ADC pins)

Current State
-------------
At the moment, the C code periodically reads the battery voltage and stores an average of the last few values. This version no longer monitors the power switch. I have debugging through the DigiKeyboard library, which allows output via USB to a notepad window.

I've trimmed down the task manager from this version. It's flexible and quite useful, but takes up a lot of space on the MCU and this only needs to do one thing, monitor the battery.

Next Up...
-------------
I'm working on getting the python script to pull back the voltage over I²C.

Links
-----
More detail can be found on this thread:
http://sudomod.com/forum/viewtopic.php?f=20&t=1768

Feel free to contact me on the [Sudomod forums] (www.sudomod.com/forum) or on the [Sudomod Discord channel] (https://discordapp.com/channels/188359728454303744/188359728454303744)
