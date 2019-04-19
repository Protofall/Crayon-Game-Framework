TARGET = kmgTest

BINS = $(TARGET).bin 1st_read.bin

SCRAMBLED = 1st_read.bin

OBJS = $(TARGET).o romdisk.o

KOS_ROMDISK_DIR = romdisk

all: $(TARGET).cdi

#include $(KOS_BASE)/Makefile.rules	#Used for the C/C++ compile rules

clean:
	-rm -f $(TARGET).cdi $(TARGET).iso $(TARGET).elf $(TARGET).bin 1st_read.bin $(OBJS) romdisk.* romdisk/*.kmg

$(SCRAMBLED): $(TARGET).elf
	sh-elf-objcopy -R .stack -O binary $(TARGET).elf $(TARGET).bin
	$(KOS_BASE)/utils/scramble/scramble $(TARGET).bin $(SCRAMBLED)

$(TARGET).elf: $(OBJS)
	kos-cc -o $(TARGET).elf $(OBJS) -lkmg -lkosutils
#I think -lkmg for the kmg port and -lkosutils for the .kmg

$(TARGET).cdi: $(SCRAMBLED)
	mkisofs -G $(KOS_BASE)/../IP.BIN -C 0,11702 -J -l -r -o $(TARGET).iso .
	cdi4dc $(TARGET).iso $(TARGET).cdi

romdisk.o: romdisk.img
	$(KOS_BASE)/utils/bin2o/bin2o romdisk.img romdisk romdisk.o

romdisk.img: $(patsubst assets/RGB565/%.png,romdisk/%.kmg,$(wildcard assets/RGB565/*.png)) $(patsubst assets/ARGB4444/%.png,romdisk/%.kmg,$(wildcard assets/ARGB4444/*.png))
	$(KOS_GENROMFS) -f romdisk.img -d romdisk -v

romdisk/%.kmg: assets/RGB565/%.png
	$(KOS_BASE)/utils/vqenc/vqenc -v -t -q -k $<
	mv assets/RGB565/$*.kmg romdisk/

romdisk/%.kmg: assets/ARGB4444/%.png
	$(KOS_BASE)/utils/vqenc/vqenc -v -t -q -k -a $<
	mv assets/ARGB4444/$*.kmg romdisk/

#The only stuff we needed from "# include $(KOS_BASE)/Makefile.rules"
%.o: %.c
	kos-cc $(CFLAGS) -c $< -o $@

%.o: %.cc
	kos-c++ $(CFLAGS) $(CXXFLAGS) -c $< -o $@

%.o: %.cpp
	kos-c++ $(CFLAGS) $(CXXFLAGS) -c $< -o $@
