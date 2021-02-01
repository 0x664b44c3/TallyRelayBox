#include <string.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include "gptimer.h"
#include "tsl.h"
#include "uart.h"

uint32_t baud = 38400;

//comment out if you use hexadecimal address switches
#define ADDR_BCD


//enable debug output un RS-422
//#define ENABLE_DEBUG


//if commented out, uses Pair 3+8 for Rx, 2+7 for Tx
//if set: uses Pair 2+7 for Rx, 3+8 for Tx

//#define RS422_SWAP ///< use Pair 2+7 for Rx, 3+8 for Rx

//enable EVEN parity
#define ENABLE_PARITY

typedef union {
	uint16_t u16;
	struct {
		uint8_t u8l;
		uint8_t u8h;
	};
} accu_t;

uint8_t swap_bits_dip1(uint8_t in)
{
	uint8_t ret = (in&1)?2:0;
	ret        |= (in&2)?8:0;
	ret        |= (in&4)?1:0;
	ret        |= (in&8)?4:0;
	return ret;
}
uint8_t swap_bits_dip2(uint8_t in)
{
	uint8_t ret = (in&8)?2:0;
	ret        |= (in&4)?8:0;
	ret        |= (in&2)?1:0;
	ret        |= (in&1)?4:0;
	return ret;
}

void init_hw(void)
{
	DDRA=0xff;
	DDRB=0xb0;
	DDRC=0x00;
	PORTD=0x14;
	DDRD=0xfe;
	PORTB=0x0f;
	PORTC=0x3c;
}


unsigned char hex[]="0123456789abcdef";
uint16_t lastDBG=0;
void show_u16(uint16_t val)
{
	uart_putc(hex[val>>12]);
	uart_putc(hex[(val>>8)&15]);
	uart_putc(hex[(val>>4)&15]);
	uart_putc(hex[val&15]);
}
void show_timestamp(const char* pstr)
{
	uint16_t tmp=systick;
	uint16_t diff = tmp-lastDBG;
	lastDBG = tmp;
	if (pstr)
	{
		uart_puts_p(pstr);
		uart_putc(' ');
	}
	uart_puts_p(PSTR("TS 0x"));
	show_u16(tmp);
	uart_puts_p(PSTR(" +"));
	show_u16(diff);
	uart_puts_p(PSTR("\r\n"));
}

uint8_t tmr_msg=0;
uint8_t tslAddr=0;
void tslCallback(uint8_t addr, uint8_t bright, uint8_t tally, unsigned char * text)
{
	timer_set(tmr_msg, 10);
	if ((addr>=tslAddr) && (addr<tslAddr+8))
	{
		uint8_t channel = addr - tslAddr;
		PORTA&=~_BV(channel);
		if (tally&1)
			PORTA|=_BV(channel);
	}
}

const unsigned char error_mask = _BV(FE0)  |
                                 _BV(DOR0) |
                                 _BV(UPE0);
ISR(USART0_RX_vect)
{
	unsigned char status = UCSR0A;
	unsigned char data   = UDR0;
#ifndef IGNORE_UART_ERRORS
	if (status & error_mask)
	{
		tsl_reset();
	}
	else
	{
		tsl_onChar(data);
	}
#else
	tsl_onChar(data);
#endif
}

void initUart(void)
{
	    UCSR0B = 0;
		uint16_t ubrr = (((F_CPU) + 8UL * (baud)) / (16UL * (baud)) -1UL);

		UBRR0L = ubrr & 0xff;
		UBRR0H = ubrr >> 8;
#ifdef ENABLE_PARITY
		UCSR0C = _BV(UCSZ00)|_BV(UCSZ01) |
		         _BV(UPM01);
#else
		UCSR0C = _BV(UCSZ00)|_BV(UCSZ01);
#endif
		UCSR0A = 0;                                              // clear error flags, disable U2X and MPCM
		UCSR0B = (1<<RXEN0)|(1<<TXEN0);
}

int main(void) {
	timer_init();
	tsl_setCallback(&tslCallback);
	init_hw();
	initUart();
	for(uint8_t d=0;d<250;++d)
		_delay_ms(1);
	PORTD|=0xc0;

#ifdef RS422_SWAP
	//enable receive on 2+7
	PORTD&=~_BV(4);
#else
	//enable receive on 3+8
	PORTD&=~_BV(2);
#endif

#ifdef ENABLE_DEBUG
#ifdef RS422_SWAP
	//enable transmit on 3+8 (debug)
	PORTD|=_BV(3);
#else
	//enable transmit on 2+7 (debug)
	PORTD|=_BV(5);
#endif
#endif

	uint8_t tmr_blink = timer_reg();
	tmr_msg = timer_reg();

	uart_puts_p(PSTR("TSL Adapter. Enter main loop.\r\n"));

	//enable rx interrupt
	UCSR0B |= _BV(RXCIE0);
	//enable interrupts
	sei();
	while(1)
	{

		uint8_t addr_low  = swap_bits_dip1(((~PINC)&0x3c)>>2);
		uint8_t addr_high = swap_bits_dip2((~PINB)&0x0f);
		uint8_t addr_new=0;
#ifdef ADDR_BCD
		addr_new = addr_low + addr_high*10;
#else
		addr_new = (addr_high<<4) | addr_low;
#endif
		if (addr_new != tslAddr)
		{
			PORTA=0;
			tslAddr = addr_new;
#ifdef ENABLE_DEBUG
			uart_puts_p(PSTR("TSL Addr: 0x"));
			uart_putc(hex[addr_new>>4]);
			uart_putc(hex[addr_new&15]);
			uart_puts_p(PSTR(" ("));
			uart_putc('0' + tslAddr / 100);
			uart_putc('0' + (tslAddr % 100)/ 10);
			uart_putc('0' + tslAddr % 10);
			uart_puts_p(PSTR(").\r\n"));
#endif
		}

		//activity LED
		if (timer_expired(tmr_msg))
			PORTD|=_BV(7);
		else
			PORTD&=~_BV(7);

		//system LED
		if (timer_expired(tmr_blink))
		{
			timer_set(tmr_blink, 500);
			PORTD^=0x40;
		}
	}
}
