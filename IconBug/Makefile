# ------------------------------------------------------------------------------
# Minesweeper Makefile
#------------------------------------------------------------------------------

#Change "-DCRAYON_SD_MODE" to 1 to make an sd executable

TARGET = "Minesweeper"
LIBS =  -lkosext2fs -lz -lm $(KOS_LIBS)

OFILES = $(patsubst %.c,%.o,$(wildcard *.c))
SCRAMBLED = cdfs/1st_read.bin

all: dc-cdi

dc-cdi: $(OFILES) $(TARGET).cdi

clean:
	rm -f $(TARGET).cdi
	rm -f *.o
	rm -f cdfs/*.bin
	rm -f *.bin
	rm -f *.elf
	rm -f *.iso

include $(KOS_BASE)/Makefile.rules

# create cdi image from CDFS_DIR
$(TARGET).cdi: $(SCRAMBLED)
	mkisofs -G IP.BIN -C 0,11702 -J -l -r -o $(TARGET).iso cdfs/
	cdi4dc $(TARGET).iso $(TARGET).cdi

# create SCRAMBLED executable from elf
$(SCRAMBLED): $(TARGET).elf
	sh-elf-objcopy -R .stack -O binary $(TARGET).elf $(TARGET).bin
	$(KOS_BASE)/utils/scramble/scramble $(TARGET).bin $@

# link objects into elf
$(TARGET).elf: $(OFILES)
	$(KOS_CC) $(KOS_LDFLAGS) -DCRAYON_SD_MODE=0 -o $@ $(KOS_START) $^ $(LIBS)
