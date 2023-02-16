# NEC PC-6601SR/Mr.PC Keyboard Adapter
The NEC PC-6601SR/Mr.PC is an 8-bit home computer released by NEC in 1984. Among many ostentatious features, it included a wireless (infrared) keyboard that could also be used to control TV functions using the companion monitor, the PC-TV151.

Unfortunately, this keyboard is difficult to find in the modern day, and many have been damaged by battery leakage or simply becoming separated from their computers.

As part of a [Leaded Solder](https://www.leadedsolder.com) blog entry, this keyboard adapter was developed so that these computers would not be rendered useless. At current, it connects through a wired 4p4c (RJ9, or telephone handset) connection.

If it is useful to you, and you wish to support the development of more things like this, [please consider supporting the blog on Patreon](https://www.patreon.com/leadedsolder). Every cent goes directly into new projects.

## Known Issues
 - Infrared is not yet supported.
 - Due to lack of testing equipment, TV mode signals are not supported.
 - "Game Mode" input mode is not yet supported.

## Hardware
You will require the following parts to assemble this project:
 - An Arduino Mini or clone;
 - A 4p4c connector;
 - An RJ9 telephone _handset_ cable;
 - A mini-DIN6 (PS/2) connector;
 - Several DuPont jumper leads;
 - 33-ohm through-hole resistor, or thereabouts;

To assemble:
 1. Connect PS/2 data to pin 5 of the Arduino.
 2. Connect PS/2 clock to pin PD2 of the Arduino.
 3. Connect the serial output pin, pin 3, of the 4p4c connector to pin 13 of the Arduino. Place the resistor in series so that it protects the Arduino.
 4. Connect pins 2 and 4 of the 4p4c connector to ground, sharing it with the Arduino's ground.
 5. (Optional) Connect pin 1 of the 4p4c connector to the Arduino's 5-volt input power pin. If you don't do this, you will have to power the Arduino over USB, which may be appropriate if you are updating the firmware at the same time.

A PCB version is expected in the future, but it is not a priority at this time.

## Software
Compile the *.ino file using an Arduino IDE and upload it to the board. You may have to install a package for the PS2KeyAdvanced library.
