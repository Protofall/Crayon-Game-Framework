OBJS = swap.o

BINS = swap.bin 1st_read.bin

KOS_LOCAL_CFLAGS = -I$(KOS_BASE)/addons/zlib
#For zlib

TARGET = swap

SCRAMBLED = 1st_read.bin

ROMDISKS = $(addsuffix .img,$(notdir $(wildcard romdisks/*)))

#$(wildcard romdisks/*) # romdisks/level1 romdisks/level2 romdisks/level3 (find the folders)
#$(notdir $(wildcard romdisks/*)) # level1 level2 level3 (get the part after the last slash of each)
#$(addsuffix .img,$(notdir $(wildcard romdisks/*))) # level1.img level2.img level3.img (stick .img on the end of each)

all: $(SCRAMBLED) $(ROMDISKS)

include $(KOS_BASE)/Makefile.rules

clean:
	-rm -f $(TARGET).elf $(OBJS) $(BINS) *.img

$(SCRAMBLED): $(TARGET).elf
	sh-elf-objcopy -R .stack -O binary $(TARGET).elf $(TARGET).bin
	$(KOS_BASE)/utils/scramble/scramble $(TARGET).bin $(SCRAMBLED)

$(TARGET).elf: $(OBJS)
	$(KOS_CC) $(KOS_CFLAGS) $(KOS_LDFLAGS) -o $@ $(KOS_START) $^ -lpng -lz -lm $(KOS_LIBS)

%.img: romdisks/%
	$(KOS_GENROMFS) -f $@ -d $^ -v