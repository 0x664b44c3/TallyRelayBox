#ifndef UART_H
#define UART_H

void uart_putc(unsigned char data);
void uart_puts(const char *s );
void uart_puts_p(const char *progmem_s );
#endif // UART_H 

