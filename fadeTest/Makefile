OBJS = fade.o

SCRAMBLED = 1st_read.bin

KOS_LOCAL_CFLAGS = -I$(KOS_BASE)/addons/zlib

all: rm-1st $(SCRAMBLED)

include $(KOS_BASE)/Makefile.rules
	
clean:
	-rm -f fade.elf $(OBJS)
	-rm -f fade.bin 1st_read.bin
	-rm -f romdisk_boot.*
	
rm-1st:
	-rm -f $(SCRAMBLED)
	-rm -f romdisk_boot.*

$(SCRAMBLED): fade.elf
	sh-elf-objcopy -R .stack -O binary fade.elf fade.bin
	$(KOS_BASE)/utils/scramble/scramble fade.bin $(SCRAMBLED)

fade.elf: $(OBJS) romdisk_boot.o 
	$(KOS_CC) $(KOS_CFLAGS) $(KOS_LDFLAGS) -o $@ $(KOS_START) $^ -lpng -lz -lm $(KOS_LIBS)

romdisk_boot.img:
	$(KOS_GENROMFS) -f $@ -d romdisk_boot -v

romdisk_boot.o: romdisk_boot.img
	$(KOS_BASE)/utils/bin2o/bin2o $< romdisk_boot $@