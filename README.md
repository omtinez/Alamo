AVR Template
============

## Toolchain

### Ubuntu

The AVR toolchain, C libraries, and the avrdude downloader/uploader, can be
installed from the Ubuntu repositories:

    apt-get install gcc-avr avr-libc avrdude

## Building

1. Clone the [avr-template](https://github.com/uctools/avr-templates) repository
   (or fork it and clone your own repository).

        git clone git@github.com:uctools/avr-template

2. Modify the Makefile:
    * Set TARGET to the desired name of the output file (eg: TARGET = main)
    * Set SOURCES to a list of your sources (eg: SOURCES = main.c two.c three.c)
    * Set MCU to the AVR part number to build for (eg: MCU = atmega328p)

    By default, the Makefile will build code for the ATmega328p, which is the
    microcontroller on the Arduino

3. Run `make`

4. The output files will be created in the 'build' folder

## Flashing

1. Modify the Makefile
  * Set PROGRAMMER to your programmer. This will be the `-c` option for avrdude
  * Set PORT to the port your programmer is on. This will be the `-P` option for 
    avrdude

    By default, the Makefile will flash an Arduino conenected over USB on
    `/dev/usbACM0` using the Arduino bootloader.

2. Run `make flash`. You may need to run this command as root (`sudo make
   flash`) if your user cannot access the port.
