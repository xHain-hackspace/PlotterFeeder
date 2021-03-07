# PlotterFeeder
## Overview 
ESP32 Firmware to feed HPGL (a control language for plotters) to a pen plotter. For info about HPGL see wikipedia and 7550A_ProgrammingManual_OCR.pdf in this repo.

This project uses an ESP32 board in connection with a suitable RS232 level shifter to control HP Plotters, in our case a HP 7550A. See https://www.sparkfun.com/tutorials/215 for details on why you need a level shifter.

Hardware documentation is currently missing, but it basically consists of 5 LEDs, 5 Buttons, and a RS232-to-TTL level shifter which interfaces to the plotters data-in, data-out, and buffer-full signal lines (plus ground connection). See the manual of your specific plotter to figure these out. You might also need to set the relevant configuration options on your plotter.

There are multiple things that the code can do: standalone plot from flash, plot via WiFi, remote control via Wifi, and play games.

'Plot from flash' can send data from flash to the plotter when one of the 5 input buttons is pressed. The data is defined in src/vectors.h and HPGL data for vector graphics can easily be generated from inkscape (Extensions->Export->Plot. For settings, see svgs/plotter-settings.png in this repo, then copy the HPGL data string from the debug output and insert into vectors.h. You may need to break a large string into multiple smaller strings if you experience crashes or unstable behavior.)

'Plot via WiFi' allows a client to send HPGL data via Wifi. The plotter opens its own Wifi accespoint which is defined in src/main.cpp. With standard settings, you should be able to connect to "xHain Plotter" with password "plotterpassword" and then you need to give yourself a static IP of e.g. 192.168.4.42, as there is no DHCP implemented. Then use src/send_HPGL_wifi.py to send the data. The data will simply be handed over to the plotter as-is. Thus you can send any valid HPGL commands. For testing, try absolute positioning commands and see if the plotter moves. E.g. use a text file with "PA 0,0;PA 1000,1000;" as input or generate data from some simple graphic in inkscape. This is also a good way to quickly test your own HPGL code.

'Remote control via Wifi': Similar to 'plot via WiFi', the setup is the same, but instead you can use the script src/remotecontrol_wifi.py to control the plotter pen with a USB joystick over Wifi. Use the joystick to move the pen and hold the button to put the pen down. Have fun akwardly drawing things in a completely overengineered way. :)

'Play games': Buttons can also trigger games if configured so in src/main.cpp. Instead of calling send_buffered() to send data from flash to the plotter, a game can be run when the corresponding button is pressed. dart_game() is a game where the plotter will draw a target and then rapidly move the pen randomly and when the user presses the button again, an arrow is 'shot' (=drawn) at the current position. The user can then 'shoot' a few more arrows and finally the achieved total score will be plotted and the paper ejected.

## Testing Plotters via USB-Serial Adapters
To test your designs via Inkscape plotter plugins or similar scripts, you can use an USB-serial adapter instead of the plotter feeder hardware and make a cable with the following connections. (These are an example, you might have to adapt them to your specific plotter.) Be aware that some cheap adapters will not produce the correct RS232 negative and positive voltages for this to work. E.g. they might produce 0V and +5V instead of -5V and +5V. If unsure, simply measure the voltages on the data lines of your adapter. This cable can be used with RTS/CTS flow control. Make sure your plotter is set up correctly, and also be aware of different pinout on different plotters, check e.g. chapter 16 in the programming manual: https://raw.githubusercontent.com/xHain-hackspace/PlotterFeeder/master/7550A_ProgrammingManual_OCR.pdf or the pinouts in the manual of your specific plotter. A good place to find manuals is the 'product documentation' link on the plotter sites at http://www.hpmuseum.net/exhibit.php?class=4&cat=24 .

| D-sub 9 female connector (USB-serial Adapter side) pin number | D-sub 25 connector (plotter side, male for HP7470A, female for HP7550A) pin number |
|---------------------------------------------------------------|---------------------------------------------------|
| 1                                                             | no connection                                     |
| 2 (RXD, to PC)                                                | 2 (TD, transmitted data, from plotter)            |
| 3 (TXD, from PC)                                              | 3 (RD, received data, to plotter)                 |
| 4                                                             | no connection                                     |
| 5 (Ground)                                                    | 7 (SGND, signal ground)                           |
| 6 (DSR, to PC)                                                | 4 (RTS, request to send, from plotter)            |
| 7                                                             | no connection                                     |
| 8 (CTS, to PC)                                                | 20 (DTR, data terminal ready, from plotter)       |
| 9                                                             | no connection                                     |

For HP7550A, you should use a *female* D-sub 25 connector on the *computer/modem* port. Set the plotter to the following settings:
Standalone/Eavesdrop -> use Standalone, Remote/Local/Standby -> use Remote, use RTS/CTS flow control on the PC.

