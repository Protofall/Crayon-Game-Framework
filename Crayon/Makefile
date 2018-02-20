TARGET = Crayon
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
LAZYOFILES = debug.o draw.o memory.o main.o
SCRAMBLED = $(CDFS_DIR)/1st_read.bin

# flags
#------------------------------------------------------------------------------

CFLAGS = $(KOS_CFLAGS)
# CFLAGS += -I$(KOS_BASE)/addons/zlib

LIBS = -lz -lm $(KOS_LIBS)

all:
	./make.sh -preprocess -dc-cd

clean:
	./make.sh -clean