TARGET = JamoHTP-ProtofallSwapGZ
IPBIN = $(KOS_BASE)/../IP.BIN

# directories
#------------------------------------------------------------------------------

# CD ROM contents will be placed here for packaging into a cdi
CDFS_DIR = cdfs

# each subdirectory 'part' will produce 'part.img.gz' in CDFS_DIR
ROMDISKS_DIR = romdisks

# files
#------------------------------------------------------------------------------

OFILES = $(patsubst %.c,%.o,$(wildcard *.c))
SCRAMBLED = $(CDFS_DIR)/1st_read.bin
ROMDISK_IMGS = $(patsubst %,$(CDFS_DIR)/%.img.gz,$(notdir $(wildcard $(ROMDISKS_DIR)/*)))

# flags
#------------------------------------------------------------------------------

CFLAGS = $(KOS_CFLAGS)
# CFLAGS += -I$(KOS_BASE)/addons/zlib

LIBS = -lpng -lz -lm $(KOS_LIBS)

# main targets
#------------------------------------------------------------------------------

all: cdi

cdi: rm-cdi $(TARGET).cdi

elf: rm-elf $(TARGET).elf

clean: rm-elf rm-cdi rm-bin rm-iso
	-rm -f $(OFILES) $(SCRAMBLED) $(ROMDISK_IMGS)

rm-%:
	-rm -f $(TARGET).$*

# build rules
#------------------------------------------------------------------------------
# note: asset.gz: gzip -f -9 asset

include $(KOS_BASE)/Makefile.rules

$(TARGET).cdi: $(SCRAMBLED) $(ROMDISK_IMGS)
	mkisofs -G $(IPBIN) -C 0,11702 -J -l -r -o $(TARGET).iso $(CDFS_DIR)
	cdi4dc $(TARGET).iso $(TARGET).cdi

$(SCRAMBLED): $(CDFS_DIR) $(TARGET).elf
	sh-elf-objcopy -R .stack -O binary $(TARGET).elf $(TARGET).bin
	$(KOS_BASE)/utils/scramble/scramble $(TARGET).bin $@

$(TARGET).elf: $(OFILES)
	$(KOS_CC) $(KOS_LDFLAGS) -o $@ $(KOS_START) $^ $(LIBS)

$(CDFS_DIR)/%.img.gz: $(CDFS_DIR) romdisks/%
	$(KOS_GENROMFS) -f $(CDFS_DIR)/$*.img -d romdisks/$* -v
	gzip -f -9 $(CDFS_DIR)/$*.img

$(CDFS_DIR):
	mkdir -p $(CDFS_DIR)