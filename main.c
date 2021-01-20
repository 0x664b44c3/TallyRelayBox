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
//#define ENABLE_DEBUG
typedef union {
	uint16_t u16;
	struct {
		uint8_t u8l;
		uint8_t u8h;
	};
} accu_t;

uint8_t swap_bits_dip(uint8_t in)
{
	uint8_t ret = (in&1)?2:0;
	ret        |= (in&2)?8:0;
	ret        |= (in&4)?1:0;
	ret        |= (in&8)?4:0;
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




uint8_t tmr_msg=0;
uint8_t tslAddr=0;
unsigned char hex[]="0123456789abcdef";
void tslCallback(uint8_t addr, uint8_t bright, uint8_t tally, unsigned char * text)
{
	timer_set(tmr_msg, 10);
//	uart_puts_p(PSTR("TSL Frame: "));
//	uart_putc(hex[addr>>4]);
//	uart_putc(hex[addr&15]);
//	uart_putc(' ');
//	uart_putc(hex[bright&15]);
//	uart_putc(' ');
//	uart_putc(hex[tally&15]);
//	uart_putc(' ');
//	uart_putc('"');
//	for(uint8_t i=0;i<15;++i)
//		uart_putc(text[i]);
//	uart_puts_p(PSTR("\"\r\n"));
	if ((addr>=tslAddr) && (addr<tslAddr+8))
	{
		uint8_t channel = addr - tslAddr;
		PORTA&=~_BV(channel);
		if (tally&1)
			PORTA|=_BV(channel);
	}
}

ISR(USART0_RX_vect)
{
	    unsigned char data = UDR0;
		tsl_onChar(data);
}

void initUart(void)
{
	    UCSR0B = 0;
		uint16_t ubrr = (((F_CPU) + 8UL * (baud)) / (16UL * (baud)) -1UL);

		UBRR0L = ubrr & 0xff;
		UBRR0H = ubrr >> 8;
		UCSR0C = _BV(UCSZ00)|_BV(UCSZ01);
		UCSR0A = 0;                                              // clear error flags, disable U2X and MPCM
		UCSR0B = (1<<RXEN0)|(1<<TXEN0);
}

int main(void) {
	timer_init();
	init_hw();
	initUart();
	UCSR0B |= _BV(RXCIE0);
	for(uint8_t d=0;d<250;++d)
		_delay_ms(1);
	PORTD|=0xc0;
	//enable receive on 3+8
	PORTD&=~_BV(2);
#ifdef ENABLE_DEBUG
	//enable transmit on 2+7 (debug)
	PORTD|=_BV(5);
#endif
	tsl_setCallback(&tslCallback);

	timer_reg();
	uint8_t tmr_blink = timer_reg();
	tmr_msg = timer_reg();

	uart_puts_p(PSTR("TSL Adapter. Enter main loop.\r\n"));
	sei();
	while(1)
	{

		uint8_t addr_low = swap_bits_dip(((~PINC)&0x3c)>>2);
		uint8_t addr_high = swap_bits_dip((~PINB)&0x0f);
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
			uart_puts_p(PSTR(".\r\n"));
#endif
		}

		if (timer_expired(tmr_msg))
			PORTD|=_BV(7);
		else
			PORTD&=~_BV(7);
		if (timer_expired(tmr_blink))
		{
			timer_set(tmr_blink, 500);
			PORTD^=0x40;
		}
	}
}
