# flags
#---------------------------------------------------------------------------------

CFLAGS = $(KOS_CFLAGS)
# CFLAGS += -I$(KOS_BASE)/addons/zlib	#Might need to add this to the bash script
cdiName = $(shell pwd | rev | cut -d'/' -f1 | rev).cdi

all:
	./../Crayon/make.sh -preprocess -dreamcast -cd

dreamcastCD:
	./../Crayon/make.sh -preprocess -dreamcast -cd

dreamcastSD:
	./../Crayon/make.sh -preprocess -dreamcast -sd

clean:
	./../Crayon/make.sh -clean

help:
	./../Crayon/make.sh -h

check:
	./../Crayon/make.sh -preprocess -noRM

# I should probs remove buildRun later on or use a better emulator
buildRun: clean all
	lxdream $(cdiName)
