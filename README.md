# Miskatino Basic

Simple implementation of [**BASIC**](https://en.wikipedia.org/wiki/BASIC) programming language to control **Arduino Mini (2k)** or **STM32F103** microcontrollers. It is exposed via serial interface (e.g. UART) and so MCU could be programmed either from computer or smartphone with the help of bluetooth module.

### Supported MCUs
- Arduino Mini 2k - 700 bytes for code, 300 for variables (also Arduino Micro / Leonardo / Duemilanove etc.) - for demo purpose
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
- assignment to variables via `X = 5*Y+ADC(3)` form, two first letters of variable are recognized

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
