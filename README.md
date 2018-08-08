# Miskatino Releases

### Miskatino Basic for Arduino

It can be used with various chips having at least 2k of RAM:

- Arduino Pro Mini / Nano / Duemilanove (AtMega328)
- Arduino Pro Micro / Leonardo (AtMega32u4)

Just open the folder `arduino-src` in your Arduino IDE and upload to your chip.

It will work with default `Serial` connection (i.e. first UART on Mini or USB on Micro).

However you may want to change `Serial` in this case so that MCU
works with UART interface (D0-D1) instead of USB - this can be
helpful when attaching Bluetooth module, for example. Just
change `Serial` to `Serial1` in the `ardubasic.ino`.

Note that default baud rate is 115200. To work with BlueTooth modules it should
be either changed to 9600 or the module itself should be reconfigured to higher speed.

### Miskatino Basic for STM32F103

Upload the hex file to your controller. It will work via first UART. Default
baud rate is 115200.

Pins 0-15 are assigned to PA0-PA15. ADC pins are the same as digital pins 0-7.
