#include <inttypes.h>

enum parserState
{
	tslInit=0,
	tslControl=1,
	tslData=2
};

#define TSL_COUNT 1

uint8_t _tslParserState = tslInit;
uint8_t _tslDataCtr = 0;
uint8_t _tslDataPtr = 0;


static unsigned char tsl_addr;
static unsigned char tsl_tally;
static unsigned char tsl_bright;
static unsigned char tsl_displayData[16];
uint8_t tsl_wasUpdated=0;

static void (*tsl_callback)(uint8_t /*addr*/, uint8_t /*bright*/, uint8_t /*tally*/, unsigned char * /*text*/)=0;

void tsl_setCallback(void(*cb_ptr)(uint8_t, uint8_t, uint8_t, unsigned char*))
{
	tsl_callback = cb_ptr;
}
void tsl_onChar(unsigned char chr)
{
	if (chr & 0x80)
	{
		tsl_addr = chr & 0x7f;
		_tslParserState = tslControl;
		_tslDataPtr = 0;
		return;
	}
	else
	{
		switch(_tslParserState)
		{
			case tslControl:
				tsl_tally  = (chr & 0x0f);
				tsl_bright = (chr & 0x30) >> 4;
				_tslParserState = tslData;
				break;
			case tslData:
				tsl_displayData[_tslDataPtr++] = chr;
				if (_tslDataPtr==16)
				{
					_tslParserState = tslInit;
					if (tsl_callback)
						tsl_callback(tsl_addr, tsl_bright, tsl_tally, tsl_displayData);
//					tsl_wasUpdated =1;
				}
				break;
			default:
				_tslParserState = tslInit;
				break;
		}
	}
}
