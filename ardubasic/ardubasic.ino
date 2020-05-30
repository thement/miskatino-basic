#include <EEPROM.h>
#include <avr/pgmspace.h>
#include <Wire.h>

#include "main.h"
#include "mytypes.h"
#include "textual.h"
#include "tokens.h"
#include "extern.h"

#define SERIAL Serial
#define UART_SPEED 115200

#define PROG_SPACE_SIZE 850
#define VARS_SPACE_SIZE 150
#define LINE_SIZE 40

char extraCmdArgCnt[] = {2, 2};

char extraFuncArgCnt[] = {1, 1, 1, 1, 2, 3};

static const char commonStrings[] PROGMEM = CONST_COMMON_STRINGS;
static const char parsingErrors[] PROGMEM = CONST_PARSING_ERRORS;

char dataSpace[VARS_SPACE_SIZE + PROG_SPACE_SIZE];
char lineSpace[LINE_SIZE * 3];

short filePtr;

char sysGetc(void) {
    return (SERIAL.available() > 0) ? (char) SERIAL.read() : 0;
}

void sysPutc(char c) {
    if (c == '\n') {
      SERIAL.write('\r');
    } else if (c == '\b') {
        SERIAL.write('\b');
        SERIAL.write(' ');
    }
    SERIAL.write(c);
}

void sysEcho(char c) {
    sysPutc(c);
}

short adcRead(char channel) {
    if (channel == -1) {
        analogRead(0);
        ADMUX = (ADMUX & 0xF0) | 0x0E;
        delay(1);
        ADCSRA |= 0x40;
        while (ADCSRA & 0x40);
        short v = ADC;
        return v > 0 ? (1100L * 1023) / v : -1;
    }
    return analogRead(channel);
}

char pinRead(char pin) {
    return (digitalRead(pin) == HIGH) ? 1 : 0;
}

void pinOut(char pin, char state) {
    if (state >= 0) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, state ? HIGH : LOW);
    } else {
        pinMode(pin, (state == -1) ? INPUT : INPUT_PULLUP);
    }
}

void poke(short addr, uchar value) {
    *((uchar*) addr) = value;
}

uchar peek(short addr) {
    return *((uchar*) addr);
}

numeric sysMillis(numeric div) {
    if (div <= 1) {
        (numeric) millis();
    }
    return (numeric) (millis() / div);
}

void outputConstStr(char strId, char index, char* w) {
    const char* s;
    int k = 0;
    char c;
    switch (strId) {
        case ID_COMMON_STRINGS:
            s = commonStrings;
            break;
        case ID_PARSING_ERRORS:
            s = parsingErrors;
            break;
        default:
            return;
    }
    while (index > 0) {
        do {
            c = pgm_read_byte_near(s + k);
            k += 1;
        } while (c != '\n');
        index -= 1;
    }
    while (1) {
        c = pgm_read_byte_near(s + k);
        if (c == '\n') {
            break;
        }
        if (w != NULL) {
            *(w++) = c;
        } else {
            sysPutc(c);
        }
        k += 1;
    }
    if (w != NULL) {
        *w = 0;
    }
}

short extraCommandByHash(numeric h) {
    switch (h) {
        case 0x036F: // POKE
            return CMD_EXTRA + 0;
        case 0x019C: // PIN
            return CMD_EXTRA + 1;
        default:
            return -1;
    }
}

short extraFunctionByHash(numeric h) {
    switch (h) {
        case 0x0355: // PEEK
            return 0;
        case 0x019C: // PIN
            return 1;
        case 0x01CF: // ADC
            return 2;
	case 0x0254: // I2CR
            return 3;
	case 0x0251: // I2CW
            return 4;
	case 0x0490: // I2CW2
            return 5;
        default:
            return -1;
    }
}

void extraCommand(char cmd, numeric args[]) {
    switch (cmd) {
        case 0:
            poke(args[0], args[1]);
            break;
        case 1:
            pinOut(args[0], args[1]);
            break;
    }
}

numeric extraFunction(char cmd, numeric args[]) {
    switch (cmd) {
        case 0:
            return peek(args[0]);
        case 1:
            return pinRead(args[0]);
        case 2:
            return adcRead(args[0]);
        case 3: {
	    Wire.requestFrom(args[0], 1, true);
	    if (!Wire.available()) {
		Serial.println('!');
		return -1;
	    }
	    return Wire.read();
	}
        case 4: {
	    Wire.beginTransmission(args[1]);
	    Wire.write(args[0]);
	    char err = Wire.endTransmission();
	    if (err != 0) {
		Serial.println('!');
	    }
	    return err;
	}
        case 5: {
	    Wire.beginTransmission(args[2]);
	    Wire.write(args[1]);
	    Wire.write(args[0]);
	    char err = Wire.endTransmission();
	    if (err != 0) {
		Serial.println('!');
	    }
	    return err;
	}
    }
    return 0;
}

unsigned char storageChecksum(short size) {
    unsigned char res = 0;
    while (size > 0) {
        res ^= EEPROM.read(--size);
    }
    return res;
}

char storageOperation(void* data, short size) {
    short i;
    if (data == NULL) {
        if (size == 0) {
            EEPROM.write(filePtr, filePtr > 4 ? storageChecksum(filePtr) : 0x55);
            return 1;
        } else {
            filePtr = 0;
            if (size > 0) {
                return 1;
            }
            size = EEPROM.read(1);
            size = ((size << 8) | EEPROM.read(0));
            if (size <= 0 || size >= PROG_SPACE_SIZE) {
                return 0;
            }
            size += 2;
            return storageChecksum(size + 1) == 0;
        }
    }
    if (size > 0) {
        for (i = 0; i < size; i += 1) {
            EEPROM.write(filePtr++, ((unsigned char*)data)[i]);
        }
    } else {
        size = -size;
        for (i = 0; i < size; i += 1) {
            ((unsigned char*)data)[i] = EEPROM.read(filePtr++);
        }
    }
    return 1;
}

void optionalProgramErase() {
    pinMode(0, INPUT_PULLUP);
    delay(50);
    if (digitalRead(0) == LOW) {
        EEPROM.write(0, 0);
        EEPROM.write(0, 1);
    }
}

void setup() {
    optionalProgramErase();
    SERIAL.begin(UART_SPEED);
    Wire.begin();
    pinMode(0, INPUT_PULLUP);
    while (!SERIAL);
    init(VARS_SPACE_SIZE, LINE_SIZE, PROG_SPACE_SIZE);
}

void loop() {
    lastInput = sysGetc();
    dispatch();
}

