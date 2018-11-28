name = vmu_test

all: rm-cdi $(name).cdi

include $(KOS_BASE)/Makefile.rules

OBJS = $(name).o graphics.o memory.o savefile.o

KOS_LOCAL_CFLAGS = -I$(KOS_BASE)/addons/zlib
	
clean:
	-rm -f $(name).elf $(OBJS)
	-rm -f $(name).bin
	-rm -f $(name).iso
	-rm -f $(name).cdi
	-rm -f 1st_read.bin
	-rm -f romdisk_boot.*

dist:
	-rm -f $(OBJS)
	-rm -f romdisk_boot.*
	$(KOS_STRIP) $(name).elf
	
rm-cdi:
	-rm -f $(name).cdi
	-rm -f romdisk_boot.*

$(name).cdi: $(name).elf
	sh-elf-objcopy -R .stack -O binary $(name).elf $(name).bin
	$(KOS_BASE)/utils/scramble/scramble $(name).bin 1st_read.bin
	mkisofs -G $(KOS_BASE)/../IP.BIN -C 0,11702 -J -l -r -o $(name).iso .
	cdi4dc $(name).iso $(name).cdi

$(name).elf: $(OBJS) romdisk_boot.o 
	$(KOS_CC) $(KOS_CFLAGS) $(KOS_LDFLAGS) -o $@ $(KOS_START) $^ -lz -lm $(KOS_LIBS)

romdisk_boot.img:
	$(KOS_GENROMFS) -f $@ -d romdisk_boot -v

romdisk_boot.o: romdisk_boot.img
	$(KOS_BASE)/utils/bin2o/bin2o $< romdisk_boot $@

run: $(name).elf
	$(KOS_LOADER) $<


