QT+=core
INCLUDEPATH = /usr/avr/include
SOURCES += \
	 main.c \
	 gptimer.c \
	 tsl.c \
	 uart.c

DEFINES += __AVR_ATmega324PB__

HEADERS += \
	 gptimer.h \
	 tsl.h \
	 uart.h

OTHER_FILES += \
	Makefile


