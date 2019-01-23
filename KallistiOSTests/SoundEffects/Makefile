#Note that the cdi option uses Kazade's cdi4dc program which is a terminal/Unix
#equivalent of bootdreams from Windows

OBJS = sound.o

SCRAMBLED = 1st_read.bin

KOS_LOCAL_CFLAGS = -I$(KOS_BASE)/addons/zlib

all: rm-1st $(SCRAMBLED)

include $(KOS_BASE)/Makefile.rules
	
clean:
	-rm -f sound.elf $(OBJS)
	-rm -f sound.bin 1st_read.bin sound.iso sound.cdi
	-rm -f romdisk_boot.*
	
rm-1st:
	-rm -f $(SCRAMBLED)
	-rm -f romdisk_boot.*

cdi: clean $(SCRAMBLED)
	mkisofs -G $(KOS_BASE)/../IP.BIN -C 0,11702 -J -l -r -o sound.iso .
	cdi4dc sound.iso sound.cdi

$(SCRAMBLED): sound.elf
	sh-elf-objcopy -R .stack -O binary sound.elf sound.bin
	$(KOS_BASE)/utils/scramble/scramble sound.bin $(SCRAMBLED)

sound.elf: $(OBJS) romdisk_boot.o 
	$(KOS_CC) $(KOS_CFLAGS) $(KOS_LDFLAGS) -o $@ $(KOS_START) $^ -lz -lm $(KOS_LIBS)

romdisk_boot.img:
	$(KOS_GENROMFS) -f $@ -d romdisk_boot -v

romdisk_boot.o: romdisk_boot.img
	$(KOS_BASE)/utils/bin2o/bin2o $< romdisk_boot $@
