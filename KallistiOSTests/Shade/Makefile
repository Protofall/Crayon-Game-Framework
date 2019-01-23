PROG_NAME = shade

OBJS = $(PROG_NAME).o

SCRAMBLED = 1st_read.bin

KOS_LOCAL_CFLAGS = -I$(KOS_BASE)/addons/zlib

all: rm $(PROG_NAME).cdi

include $(KOS_BASE)/Makefile.rules

clean:
	-rm -f $(SCRAMBLED)
	-rm -f $(PROG_NAME).bin
	-rm -f $(PROG_NAME).cdi
	-rm -f $(PROG_NAME).iso
	-rm -f $(PROG_NAME).elf $(OBJS)
	-rm -f romdisk_boot.*

rm:
	-rm -f $(SCRAMBLED)
	-rm -f $(PROG_NAME).bin
	-rm -f $(PROG_NAME).cdi
	-rm -f $(PROG_NAME).iso
	-rm -f $(PROG_NAME).elf
	-rm -f romdisk_boot.*

$(PROG_NAME).cdi: $(SCRAMBLED)
	sh-elf-objcopy -R .stack -O binary $(PROG_NAME).elf $(PROG_NAME).bin
	$(KOS_BASE)/utils/scramble/scramble $(PROG_NAME).bin 1st_read.bin
	mkisofs -G $(KOS_BASE)/../IP.BIN -C 0,11702 -J -l -r -o $(PROG_NAME).iso .
	cdi4dc $(PROG_NAME).iso $(PROG_NAME).cdi

$(SCRAMBLED): $(PROG_NAME).elf
	sh-elf-objcopy -R .stack -O binary $(PROG_NAME).elf $(PROG_NAME).bin
	$(KOS_BASE)/utils/scramble/scramble $(PROG_NAME).bin $(SCRAMBLED)

$(PROG_NAME).elf: $(OBJS) romdisk_boot.o 
	$(KOS_CC) $(KOS_CFLAGS) $(KOS_LDFLAGS) -o $@ $(KOS_START) $^ -lpng -lz -lm $(KOS_LIBS)

romdisk_boot.img:
	$(KOS_GENROMFS) -f $@ -d romdisk_boot -v

romdisk_boot.o: romdisk_boot.img
	$(KOS_BASE)/utils/bin2o/bin2o $< romdisk_boot $@
