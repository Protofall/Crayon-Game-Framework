TARGET = paletteTest
IPBIN = $(KOS_BASE)/../IP.BIN

# directories
#------------------------------------------------------------------------------

# CD ROM contents will be placed here for packaging into a cdi
CDFS_DIR = cdfs

# each subdirectory contains assets for conversion. outputs are placed in
# ROMDISKS_DIR/<part>, where <part> is the name of the subdirectory
ASSETS_DIR = assets

# list of directories to be automatically created
SUBDIRS = $(CDFS_DIR) $(ROMDISKS_DIR)

# files
#------------------------------------------------------------------------------

OFILES = $(patsubst %.c,%.o,$(wildcard *.c))
LAZYOFILES = error.o sprite.o paletteTest.o
SCRAMBLED = $(CDFS_DIR)/1st_read.bin

# flags
#------------------------------------------------------------------------------

CFLAGS = $(KOS_CFLAGS)
# CFLAGS += -I$(KOS_BASE)/addons/zlib

LIBS = -lz -lm $(KOS_LIBS)

all:
	./make.sh -preprocess -dc-cd
#$(KOS_CC) $(KOS_LDFLAGS) -o $(TARGET).elf $(KOS_START) $(LAZYOFILES) $(LIBS)
#sh-elf-objcopy -R .stack -O binary $(TARGET).elf $(TARGET).bin
#$(KOS_BASE)/utils/scramble/scramble $(TARGET).bin $(SCRAMBLED)
#mkisofs -G $(IPBIN) -C 0,11702 -J -l -r -o $(TARGET).iso $(CDFS_DIR)
#cdi4dc $(TARGET).iso $(TARGET).cdi

clean:
	./make.sh -clean