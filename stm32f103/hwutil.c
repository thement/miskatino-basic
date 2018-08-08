#include "stm32f103.h"
#include "stdlib.h"
#include "params.h"
#include "../core/utils.h"
#include "../core/textual.h"
#include "../core/tokens.h"

#define UART_RX_BUF_SIZE 8

#define STORAGE_START ((64 - 16) * 1024)
#define STORAGE_PAGE_SIZE 1024

#define STORAGE_READ(p) (REG_B(FLASH_START + STORAGE_START, (p)))

#define uchar unsigned char

char hex[] = "0123456789ABCDEF";
unsigned char uartRxBuf[UART_RX_BUF_SIZE];
char uartRxBufStart, uartRxBufEnd;

uchar writeOddChar;
unsigned short storagePos;

char extraCmdArgCnt[] = {2, 2, 2, 2};

char extraFuncArgCnt[] = {1, 1, 1, 1, 1};

static char* commonStrings = CONST_COMMON_STRINGS;
static char * parsingErrors = CONST_PARSING_ERRORS;

void enableInterrupts(void) {
    __asm("cpsie i");
}

void irqEnable(unsigned char n) {
    REG_L(NVIC_BASE, NVIC_ISER + (n / 32) * 4) |= (1 << (n % 32));
}

void pinMode(int base, char num, char mode, char cnf) {
    int* p = (int*) (void*) (base + (num < 8 ? GPIO_CRL : GPIO_CRH));
    num &= 0x7;
    char offs = num * 4;
    int v = *p;
    v &= ~(0xF << offs);
    v |= (mode | (cnf << 2)) << offs;
    *p = v;
}

void pinOutput(int base, char num, char v) {
    if (v == 0) {
        num += 16;
    }
    REG_L(base, GPIO_BSRR) |= 1 << num;
}

char pinInput(int base, char num) {
    return (REG_L(base, GPIO_IDR) & (1 << num)) ? 1 : 0;
}

char uartPinsSetup(void) {
    char remap;
    pinMode(GPIOB_BASE, 7, PIN_MODE_IN, PIN_CNF_I_PULL);
    REG_L(GPIOB_BASE, GPIO_ODR) &= ~(1 << 7); // pull-down on input
    remap = pinInput(GPIOB_BASE, 7);
    pinMode(GPIOB_BASE, 7, PIN_MODE_IN, PIN_CNF_I_FLT);
    if (remap) {
        pinMode(GPIOB_BASE, 6, PIN_MODE_OUT, PIN_CNF_O_APP);
        REG_L(AFIO_BASE, AFIO_MAPR) |= (1 << 2); // UART1 remap
    } else {
        pinMode(GPIOA_BASE, 9, PIN_MODE_OUT, PIN_CNF_O_APP);
        REG_L(AFIO_BASE, AFIO_MAPR) &= ~(1 << 2); // UART1 no remap
    }
    return remap;
}

void uartEnable(int divisor) {
    REG_L(RCC_BASE, RCC_APB2ENR) |= (1 << 3); // ports B
    REG_L(RCC_BASE, RCC_APB2ENR) |= (1 << 0); // AFIO clock
    char uartRemap = uartPinsSetup();
    if (!uartRemap) {
        REG_L(RCC_BASE, RCC_APB2ENR) &= ~(1 << 3); // ports B off
    }
    REG_L(RCC_BASE, RCC_APB2ENR) |= (1 << 14); // UART clock
    if (REG_L(RCC_BASE, RCC_CFGR) & (4 << 11)) {
        divisor /= 2;
    }
    REG_L(USART_BASE, USART_BRR) |= divisor;
    REG_L(USART_BASE, USART_CR1) |= (1 << 13); // UART enable
    REG_L(USART_BASE, USART_CR1) |= (3 << 2); // UART transmit/receive enable
    REG_L(USART_BASE, USART_CR1) |= (1 << 5); // UART receive interrupt enable
    uartRxBufStart = 0;
    uartRxBufEnd = 0;
    irqEnable(IRQ_UART);
}

int uartRead(void) {
    unsigned char c;
    if (uartRxBufStart == uartRxBufEnd) {
        return -1;
    }
    c = uartRxBuf[uartRxBufStart];
    uartRxBufStart = (uartRxBufStart + 1) % UART_RX_BUF_SIZE;
    return c;
}

void uartIrqHandler(void) {
    char next = (uartRxBufEnd + 1) % UART_RX_BUF_SIZE;
    uartRxBuf[uartRxBufEnd] = REG_B(USART_BASE, USART_DR);
    if (next != uartRxBufStart) {
        uartRxBufEnd = next;
    }
}

void uartSend(int c) {
    while ((REG_L(USART_BASE, USART_SR) & (1 << 7)) == 0) {
        __asm("nop");
    }
    REG_L(USART_BASE, USART_DR) = c;
}

void uartSends(char* s) {
    while (*s) {
        uartSend(*(s++));
    }
}

void uartSendHex(int x, int d) {
    while (d-- > 0) {
        uartSend(hex[(x >> (d * 4)) & 0xF]);
    }
}

void uartSendDec(int x) {
    static char s[10];
    int i, x1;
    i = 0;
    while (x > 0) {
        x1 = x / 10;
        s[i++] = x - x1 * 10;
        x = x1;
    }
    if (i == 0) {
        s[i++] = 0;
    }
    while (i > 0) {
        uartSend('0' + s[--i]);
    }
}

void adcEnable(void) {
    REG_L(RCC_BASE, RCC_APB2ENR) |= (1 << 9); // ADC1 clock
    REG_L(RCC_BASE, RCC_CFGR) &= ~(3 << 14);
    REG_L(RCC_BASE, RCC_CFGR) |= (1 << 14); // ADC1 prescaler ABP2clk/4
    REG_L(ADC1_BASE, ADC_CR2) = 1; // enable ADC1
    REG_L(ADC1_BASE, ADC_SMPR2) = 044444444;
    REG_L(ADC1_BASE, ADC_SQR1) = 0; // sequence of 1 measure
}

void setupPll(int mhz) {
    int boost = mhz / 4 - 2;
    REG_L(RCC_BASE, RCC_CR) &= ~(1 << 24);
    while ((REG_L(RCC_BASE, RCC_CR) & (1 << 25)) != 0) {
        __asm("nop");
    }
    REG_L(RCC_BASE, RCC_CFGR) = (boost & 0xF) << 18;
    REG_L(RCC_BASE, RCC_CFGR) |= (4 << 11); // APB2 / 2
    REG_L(RCC_BASE, RCC_CFGR) |= (4 << 8); // APB1 / 2
    REG_L(RCC_BASE, RCC_CR) |= (1 << 24);
    while ((REG_L(RCC_BASE, RCC_CR) & (1 << 25)) == 0) {
        __asm("nop");
    }
    REG_L(RCC_BASE, RCC_CFGR) |= (1 << 1);
    while (((REG_L(RCC_BASE, RCC_CFGR) >> 2) & 0x3) != 2) {
        __asm("nop");
    }
}

void waitRtcWritable() {
    while ((REG_L(RTC_BASE, RTC_CRL) & (1 << 5)) == 0) {
        __asm("nop");
    }
}

void setupRealTimeClock(void) {
    REG_L(RCC_BASE, RCC_APB1ENR) |= (1 << 27) | (1 << 28);
    REG_L(PWR_BASE, PWR_CR) |= (1 << 8);
    REG_L(RCC_BASE, RCC_CSR) |= 1;
    while ((REG_L(RCC_BASE, RCC_CSR) & 2) != 2) {
        __asm("nop");
    }
    REG_L(RCC_BASE, RCC_BDCR) |= (2 << 8) | (1 << 15);
    while ((REG_L(RTC_BASE, RTC_CRL) & (1 << 3)) == 0) {
        __asm("nop");
    }
    waitRtcWritable();
    REG_L(RTC_BASE, RTC_CRL) |= (1 << 4);
    REG_L(RTC_BASE, RTC_PRLL) = 40;
    REG_L(RTC_BASE, RTC_CNTH) = 0;
    REG_L(RTC_BASE, RTC_CNTL) = 0;
    REG_L(RTC_BASE, RTC_CRL) &= ~(1 << 4);
    waitRtcWritable();
    REG_L(PWR_BASE, PWR_CR) &= ~(1 << 8);
}

void setupClocks(int mhz) {
    setupPll(mhz);
    setupRealTimeClock();
}

int strlen(const char* s) {
    int i;
    for (i = 0; s[i]; i++) {
    }
    return i;
}

void* memcpy(void* dst, const void* src, int sz) {
    char* d = (char*) dst;
    char* s = (char*) src;
    while (sz-- > 0) {
        *(d++) = *(s++);
    }
    return dst;
}

int memcmp(const void* dst, const void* src, int sz) {
    uchar* d = (uchar*) dst;
    uchar* s = (uchar*) src;
    int i, v;
    for (i = 0; i < sz; i++) {
        v = *(d++) - *(s++);
        if (v != 0) {
            return v;
        }
    }
    return 0;
}

void* memmove(void* dst, const void* src, int sz) {
    uchar* d = (uchar*) dst;
    uchar* s = (uchar*) src;
    int i;
    if (d < s) {
        for (i = 0; i < sz; i++) {
            *(d++) = *(s++);
        }
    } else {
        d += sz;
        s += sz;
        for (i = 0; i < sz; i++) {
            *(--d) = *(--s);
        }
    }
    return dst;
}

void sysPutc(char c) {
    if (c == '\n') {
        uartSend('\r');
    } else if (c == '\b') {
        uartSend('\b');
        uartSend(' ');
    }
    uartSend(c);
}

void sysEcho(char c) {
    sysPutc(c);
}

short adcRead(char channel) {
    pinMode(GPIOA_BASE, channel, PIN_MODE_IN, PIN_CNF_I_ANA);
    REG_L(ADC1_BASE, ADC_SQR3) = channel;
    REG_L(ADC1_BASE, ADC_CR2) = 1;
    while ((REG_L(ADC1_BASE, ADC_SR) & 2) == 0) {
        __asm("nop");
    }
    return (short) (REG_L(ADC1_BASE, ADC_DR) & 0xFFF);
}

short pinRead(char pin) {
    return pinInput(GPIOA_BASE, pin);
}

void pinOut(char pin, schar state) {
    if (state >= 0) {
        pinMode(GPIOA_BASE, pin, PIN_MODE_OUT, PIN_CNF_O_PP);
        pinOutput(GPIOA_BASE, pin, state);
    } else {
        pinMode(GPIOA_BASE, pin, PIN_MODE_IN, state == -1 ? PIN_CNF_I_FLT : PIN_CNF_I_PULL);
        if (state < -1) {
            pinOutput(GPIOA_BASE, pin, state == -2 ? 1 : 0);
        }
    }
}

numeric sysMillis(numeric div) {
    unsigned short h1 = REG_L(RTC_BASE, RTC_CNTH);
    unsigned short lo = REG_L(RTC_BASE, RTC_CNTL);
    unsigned short h2 = REG_L(RTC_BASE, RTC_CNTH);
    long v;
    if (h1 == h2) {
        v = (((long) h1) << 16) | lo;
    } else {
        v = ((long) h2) << 16;
    }
    return (div <= 1) ? v : v / div;
}

void outputConstStr(char strId, char index, char* w) {
    char* s;
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
        while (*s++ != '\n') {
        }
        index -= 1;
    }
    while (*s != '\n') {
        if (w) {
            *(w++) = (*s++);
        } else {
            sysPutc(*s++);
        }
    }
    if (w) {
        *w = 0;
    }
}

short extraCommandByHash(numeric h) {
    switch (h) {
        case 0x036F: // POKE
            return CMD_EXTRA + 0;
        case 0x06EC: // POKE2
            return CMD_EXTRA + 1;
        case 0x06EA: // POKE4
            return CMD_EXTRA + 2;
        case 0x019C: // PIN
            return CMD_EXTRA + 3;
        default:
            return -1;
    }
}

short extraFunctionByHash(numeric h) {
    switch (h) {
        case 0x019C: // PIN
            return 3;
        case 0x01CF: // ADC
            return 4;
        case 0x0355: // PEEK
            return 0;
        case 0x0698: // PEEK2
            return 1;
        case 0x069E: // PEEK4
            return 2;
        default:
            return -1;
    }
}

void extraCommand(char cmd, numeric args[]) {
    switch (cmd) {
        case 0:
            *((unsigned char*)(args[0])) = (unsigned char) args[1];
            break;
        case 1:
            *((unsigned short*)(args[0])) = (unsigned short) args[1];
            break;
        case 2:
            *((unsigned long*)(args[0])) = (unsigned long) args[1];
            break;
        case 3:
            pinOut(args[0], args[1]);
            break;
    }
}

numeric extraFunction(char cmd, numeric args[]) {
    switch (cmd) {
        case 0:
            return *((unsigned char*)(args[0]));
        case 1:
            return *((unsigned short*)(args[0]));
        case 2:
            return *((unsigned long*)(args[0]));
        case 3:
            return pinRead(args[0]);
        case 4:
            return adcRead(args[0]);
    }
    return 0;
}

static void storageSend(uchar c) {
    if ((storagePos & 1) == 0) {
        writeOddChar = c;
        if (storagePos % STORAGE_PAGE_SIZE == 0) {
            REG_L(FLASH_BASE, FLASH_CR) = (1 << 1);
            REG_L(FLASH_BASE, FLASH_AR) = (FLASH_START + STORAGE_START + storagePos);
            REG_L(FLASH_BASE, FLASH_CR) |= (1 << 6);
            while ((REG_L(FLASH_BASE, FLASH_SR) & 1) != 0) {
                __asm("nop");
            }
            REG_L(FLASH_BASE, FLASH_CR) &= ~((1 << 1) | (1 << 6));
        }
    } else {
        REG_L(FLASH_BASE, FLASH_CR) = (1 << 0);
        REG_S(FLASH_START + STORAGE_START, storagePos & ~1) = (((unsigned short) c) << 8) | writeOddChar;
        while ((REG_L(FLASH_BASE, FLASH_SR) & 1) != 0) {
            __asm("nop");
        }
        REG_L(FLASH_BASE, FLASH_CR) &= ~(1 << 0);
    }
    storagePos += 1;
}

unsigned char storageChecksum(short size) {
    unsigned char res = 0;
    while (size > 0) {
        size -= 1;
        res ^= STORAGE_READ(size);
    }
    return res;
}

char storageOperation(void* data, short size) {
    short i;
    if (data == NULL) {
        if (size) {
            storagePos = 0;
            if (size > 0) {
                return 1;
            }
            size = (((short) STORAGE_READ(1)) << 8) | STORAGE_READ(0);
            if (size <= 0 || size >= PROG_SPACE_SIZE) {
                return 0;
            }
            size += 2;
            return storageChecksum(size + 1) == 0;
        } else {
            i = storageChecksum(storagePos & ~1);
            if (storagePos & 1) {
                i ^= writeOddChar;
            }
            storageSend(i);
            if ((storagePos & 1) != 0) {
                storageSend(0xFF);
            }
            return 1;
        }
    } else {
        if (size > 0) {
            for (i = 0; i < size; i++) {
                storageSend(((uchar*)data)[i]);
            }
        } else {
            for (i = -size -1; i >= 0; i--) {
                ((uchar*)data)[i] = STORAGE_READ(storagePos + i);
            }
            storagePos += -size;
        }
    }
    return 1;
}

