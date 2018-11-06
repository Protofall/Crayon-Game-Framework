all: rm example.cdi

include $(KOS_BASE)/Makefile.rules

OBJS = example.o
	
clean:
	-rm -f 1st_read.bin
	-rm -f example.cdi
	-rm -f example.iso
	-rm -f example.elf $(OBJS)
	-rm -f romdisk_boot.*

rm:
	-rm -f 1st_read.bin
	-rm -f example.cdi
	-rm -f example.iso
	-rm -f example.elf
	-rm -f romdisk_boot.*

example.cdi: example.elf
	sh-elf-objcopy -R .stack -O binary example.elf example.bin
	$(KOS_BASE)/utils/scramble/scramble example.bin 1st_read.bin
	mkisofs -G $(KOS_BASE)/../IP.BIN -C 0,11702 -J -l -r -o example.iso .
	cdi4dc example.iso example.cdi

example.elf: $(OBJS) romdisk_boot.o 
	$(KOS_CC) $(KOS_CFLAGS) $(KOS_LDFLAGS) -o $@ $(KOS_START) $^ -lpng -lz -lm $(KOS_LIBS)

romdisk_boot.img:
	$(KOS_GENROMFS) -f $@ -d romdisk_boot -v

romdisk_boot.o: romdisk_boot.img
	$(KOS_BASE)/utils/bin2o/bin2o $< romdisk_boot $@