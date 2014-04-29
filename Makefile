include Makefile.inc

PRG           	= reflow
MCU_TARGET		= atmega328p
F_CPU			= 16000000
PROGRAMMER		= arduino
UPLOADCHIP		= m328p
PORT			= /dev/ttyACM0
AVRDUDE_EXTRAFLAGS =

SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))
#EEPROMS=$(patsubst %.c,%_eeprom.hex,$(SRCS))
UPLOAD_FLASH	= $(PRG).hex

override CFLAGS  =$(OPTIMIZE) $(MCUCFLAGS) $(DEFS)
override LDFLAGS =$(LDOPTIMIZE)

all: $(OBJS)

$(PRG).elf: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

upload: $(UPLOAD_FLASH)
	sudo $(AVRDUDE) $(AVRDUDEFLAGS) -U flash:w:$(UPLOAD_FLASH)

clean:
	rm -rf *.o *.a *.elf *.eps *.png *.pdf *.bak *.hex
	rm -rf *.lst *.map $(EXTRA_CLEAN_FILES)
	rm -rf *.Rout *.bin
	rm -rf *.srec
	rm -rf $(PRG)
