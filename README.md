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
At the moment, the C code creates two independent tasks, one to check the power switch and one to read and store the battery voltage. The timing of these can be modified. I have debugging through the DigiKeyboard library, which allows output via USB to a notepad window.

Next Up...
-------------
I'll be looking to change how I've implemented I²C. Information stored in a struct can be written to a byte array. The python script will then be able to request this information (around 4 bytes) on a regular basis. More information can probably be added, but the ATTiny85 is limited. For additional functionality, an ATTiny67 may be required.

Links
-----
More detail can be found on this thread:
http://sudomod.com/forum/viewtopic.php?f=20&t=1768

Feel free to contact me on the [Sudomod forums] (www.sudomod.com/forum) or on the [Sudomod Discord channel] (https://discordapp.com/channels/188359728454303744/188359728454303744)
