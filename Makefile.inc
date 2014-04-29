F_OSC	= $(F_CPU)

MCUCFLAGS=-mmcu=$(MCU_TARGET) -DF_CPU=$(F_CPU)UL -DF_OSC=$(F_OSC)
OPTIMIZE= -Os -Wall -funsigned-char -funsigned-bitfields -ffunction-sections -fpack-struct -fshort-enums -fno-inline-small-functions -fno-split-wide-types -fno-tree-scev-cprop
LIBOPTIMIZE= -Os -Wall -funsigned-char -funsigned-bitfields -ffunction-sections -fpack-struct -fshort-enums -fno-inline-small-functions -fno-split-wide-types -fno-tree-scev-cprop
LDOPTIMIZE=-fwhole-program -Wl,--relax,--gc-sections 

CC = avr-gcc
AR = avr-ar

OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump

AVRSIZE = avr-size -C -t --mcu $(MCU_TARGET)

#AVRRESET =sudo -S ./bin/avr-reset.py
AVRDUDE = sudo -S avrdude -v
AVRDUDEFLAGS	=-p $(UPLOADCHIP) -c $(PROGRAMMER) -P $(PORT) $(AVRDUDE_EXTRAFLAGS) -F -u

%.lst: %.elf
	$(OBJDUMP) -h -S $< > $@

%.hex: %.elf
	$(AVRSIZE) $< ; $(OBJCOPY) -j .text -j .data -O ihex $< $@

%.srec: %.elf
	$(OBJCOPY) -j .text -j .data -O srec $< $@

%.bin: %.elf
	$(OBJCOPY) -j .text -j .data -O binary $< $@

# Rules for building the .eeprom rom images

%_eeprom.hex: %.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O ihex $< $@ \
	|| { echo empty $@ not generated; exit 0; }

%_eeprom.srec: %.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O srec $< $@ \
	|| { echo empty $@ not generated; exit 0; }

%_eeprom.bin: %.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O binary $< $@ \
	|| { echo empty $@ not generated; exit 0; }
