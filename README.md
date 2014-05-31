reflow
======

This code will control a reflow oven with an Arduino Uno. At this moment, it works with a simple Arduino Uno, using avr-gcc and avrdude for upload.

By default, uses /dev/ttyUSB0 for uploading. You can change it on the Makefile.

```
   make clean && make upload
```

File           | Description
-------------- | ---------------
Makefile 	   | Global defines
Makefile.inc   | Common flags for avr-gcc
reflow.c       | _main_ funcion
heater.c       | This is the important file. Heat/Fan management, reflow program, etc.
pid.c          | PID algorithm
menu.c         | LCD high level menu
temp.c         | Thermocouple K voltage readings
encoder.c      | Code for the encoder PEC11-4215F-S0024
lcd_PCD8544.c  | Low level for the PCD8544 LCD Nokia

