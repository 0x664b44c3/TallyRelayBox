#ifndef TSL_H
#define TSL_H
#include <inttypes.h>

void tsl_onChar(unsigned char chr);
extern uint8_t tsl_wasUpdated;
void tsl_setCallback(void(*cb_ptr)(uint8_t, uint8_t, uint8_t, unsigned char*));
#endif // TSL_H
