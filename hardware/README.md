This folder contains a very rudimentary hardware documentation. Sorry, we did not yet have time to make a circuit diagram. If you make one, please make sure to send us a pull request or the data. Thanks. :)


The cable going to the plotter seen in the images is connected to the 'computer/modem' port on a HP7550A as follows:
|Cable Color | Plotter Pin|
|-----------|-------|
|Orange     | pin 20 (DTR, data terminal ready, from plotter)|
|Green      | pin 3 (RD, received data, to plotter)|
|Red        | pin 2 (TD, transmitted data, from plotter)|
|Black      | pin 7 (signal ground)|


The LEDs/buttons are in standard configuration (Buttons: direct connection to ground when pressed, LEDs: series resistor)

The converter chip is a MAX232. Since it's 5 Volts, the are two resistive dividers on the bottom for level shifting 5V from the MAX232 to 3.3V for the ESP32 input. The level shifter each use 3 resistors instead of two, because we did not have the correct values when building. The connection from ESP 3.3V to MAX232 5V input is just a direct connection. (3.3V high level is OK for 5V inputs)

There are also two pullup resistors for the buttons, because some ESP32 pins do not have internal pullups.

The HP7550A is set to 'Standalone configuration' and 'remote mode', see the manual (https://raw.githubusercontent.com/xHain-hackspace/PlotterFeeder/master/7550A_ProgrammingManual_OCR.pdf) for details.