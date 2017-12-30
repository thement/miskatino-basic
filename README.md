# Miskatino Basic

Simple implementation of [**BASIC**](https://en.wikipedia.org/wiki/BASIC) programming language to control **Arduino Mini (2k)** or **STM32F103** microcontrollers. It is exposed via serial interface (e.g. UART) and so MCU could be programmed either from computer or smartphone with the help of bluetooth module.

### Supported MCUs

The target hardware is really 32-bit ARM controllers, preferably STM32F103, which are cheaper than 8-bit Arduinos, but provide over 10 times more memory and 5 (or 10 with quartz) times faster execution, along with 32-bit integers instead of 16-bit. However Arduino port is prepared for demo purposes, while STM32 version is partially incomplete

- Arduino Mini 2k - 700 bytes for code, 300 for variables (also Arduino Micro / Leonardo / Duemilanove etc.)
- STM32F103 - code and data could take up to 20 kBytes, 32-bit variables, 100000 lines per second (work in progress)

### Recognized commands

- `PRINT` - prints integer value or constant string (double-quoted) to serial console
- `INPUT` - allows to read integer value
- `REM` - just a comment line
- `IF` - conditional execution, i.e. `if x<0; goto 50` (note semicolon instead of "then")
- `GOTO` - jump to line number
- `GOSUB` - subroutine call to line number
- `RETURN` - return from subroutine
- `END` - stop execution (e.g. end main code to prevent running into subroutine lines)
- `DIM` - allocate an array (integer or byte)
- `DATA` - load values to previously allocated array
- `PIN` - to control the pin output signal (0, 1) or make it input (-1)
- `POKE` - to put data byte into memory address (register, or io-port)
- assignment to variables via `X = 5*Y+ADC(3)` form, two first letters of variable are recognized

### Recognized functions
- `ABS` - absolute value of argument
- `PIN` - read logical signal (0, 1) from the given pin
- `ADC` - read voltage from given analog input
- `PEEK` - read byte from given memory address (register, or io-port)

### Speed test

(current implementation optimizes interpretation of tokens,
but does not hard link variables and labels yet)

simple code used:

    10 x = 100000
    20 if x%5000=0;print x
    30 x = x-1
    40 if x>=0; goto 20

i.e. about 4 statements per iteration

- on STM32 with -O0: 10 sec
- on STM32 with -O1: 6 sec
- on STM32 with -O2: 5 sec
- on STM32 with -O3: 4 sec
- on Arduino Micro (x=30000): 7 sec (i.e. 23 for 100000)

i.e. roughly 100000 statements per second on STM32 achievable
