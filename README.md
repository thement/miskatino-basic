**Miskatino Basic for Arduino Mini (with 2k RAM) v1.1**

Just open it in your Arduino IDE and upload to your chip.

It could also be compiled for other Arduinos with sufficient RAM,
e.g. for Arduino Pro Micro / Leonardo.

However you may want to change `Serial` in this case so that MCU
works with UART interface (D0-D1) instead of USB - this can be
helpful when attaching Bluetooth module, for example. Just
change `Serial` to `Serial1` in the `ardubasic.ino`.



