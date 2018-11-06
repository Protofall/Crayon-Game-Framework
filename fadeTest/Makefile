OBJS = fade.o

SCRAMBLED = 1st_read.bin

KOS_LOCAL_CFLAGS = -I$(KOS_BASE)/addons/zlib

all: rm fade.cdi

include $(KOS_BASE)/Makefile.rules

clean:
	-rm -f $(SCRAMBLED)
	-rm -f fade.bin
	-rm -f fade.cdi
	-rm -f fade.iso
	-rm -f fade.elf $(OBJS)
	-rm -f romdisk_boot.*

rm:
	-rm -f $(SCRAMBLED)
	-rm -f fade.bin
	-rm -f fade.cdi
	-rm -f fade.iso
	-rm -f fade.elf
	-rm -f romdisk_boot.*

fade.cdi: $(SCRAMBLED)
	sh-elf-objcopy -R .stack -O binary fade.elf fade.bin
	$(KOS_BASE)/utils/scramble/scramble fade.bin 1st_read.bin
	mkisofs -G $(KOS_BASE)/../IP.BIN -C 0,11702 -J -l -r -o fade.iso .
	cdi4dc fade.iso fade.cdi

$(SCRAMBLED): fade.elf
	sh-elf-objcopy -R .stack -O binary fade.elf fade.bin
	$(KOS_BASE)/utils/scramble/scramble fade.bin $(SCRAMBLED)

fade.elf: $(OBJS) romdisk_boot.o 
	$(KOS_CC) $(KOS_CFLAGS) $(KOS_LDFLAGS) -o $@ $(KOS_START) $^ -lpng -lz -lm $(KOS_LIBS)

romdisk_boot.img:
	$(KOS_GENROMFS) -f $@ -d romdisk_boot -v

romdisk_boot.o: romdisk_boot.img
	$(KOS_BASE)/utils/bin2o/bin2o $< romdisk_boot $@
