# flags
#---------------------------------------------------------------------------------

CFLAGS = $(KOS_CFLAGS)
# CFLAGS += -I$(KOS_BASE)/addons/zlib	#Might need to add this to the bash script

all:
	./make.sh -preprocess -dreamcast -cd

dreamcastCD:
	./make.sh -preprocess -dreamcast -cd

dreamcastSD:
	./make.sh -preprocess -dreamcast -sd

clean:
	./make.sh -clean

help:
	./make.sh -h

check:
	./make.sh -preprocess -noRM

# I should probs remove buildRun later on
buildRun: clean all
	lxdream Minesweeper.cdi
