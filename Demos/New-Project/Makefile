#------------------------------------------------------------------------------
# Template Makefile
#------------------------------------------------------------------------------
$(if $(CRAYON_BASE),,$(error Cannot locate Crayon. Please set CRAYON_BASE to\
  Crayon's root directory, either in your environment, or in a command line\
  argument to make (eg. `make CRAYON_BASE=path/to/Crayon`)))

CRAYON_PROJ_NAME   := $(notdir $(shell pwd))
CRAYON_PROJ_LIBS   := -lkosext2fs -lz
CRAYON_PROJ_SRC_C  := $(shell find code -type f -name "*.c")
CRAYON_BUILD_DIR   := build
# CRAYON_IP_BIN      := $(shell pwd)/IP.BIN

### TODO: Write a cleaner set of shortcut rules in this example makefile.
###     The following rules emulate the previous version of this makefile.

all: crayon-cdi

dc-cdi: crayon-cdi

dc-sd: crayon-sd

dc-pc: crayon-pc

clean: crayon-clean

help: crayon-pp-help

check:
	make crayon-pp CRAYON_PP_FLAGS=-noRM

buildRun: crayon-run

crayon: crayon-rebuild-crayon

crayon_clean:
	cd $(CRAYON_BASE); make clean PLATFORM=dreamcast

preprocess: crayon-pp

include $(CRAYON_BASE)/rules/dreamcast.mk
