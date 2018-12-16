#------------------------------------------------------------------------------
# DemoCrayon Makefile
#------------------------------------------------------------------------------
$(if $(CRAYON_BASE),,$(error Cannot locate Crayon. Please set CRAYON_BASE to\
  Crayon's root directory, either in your environment, or in a command line\
  argument to make. (eg. $$ make CRAYON_BASE=path/to/Crayon)))

PROJ_NAME := $(notdir $(shell pwd))
CFLAGS    := $(KOS_CFLAGS)
LDFLAGS   := $(KOS_LDFLAGS) -L$(CRAYON_BASE)/lib/dreamcast
LIBS      := -lcrayon -lkosext2fs -lz -lm $(KOS_LIBS)
IP_BIN    := $(CRAYON_BASE)/IP.BIN
PPFLAGS   := # Crayon asset preprocessor options

# Directories
SRC       := code
BUILD     := build

# Sources
CFILES    := $(shell find $(SRC) -type f -name "*.c")

# Intermediate outputs
OFILES    := $(CFILES:%.c=$(BUILD)/%.o)
SCRAMBLED := cdfs/1st_read.bin

#------------------------------------------------------------------------------
# Targets
#------------------------------------------------------------------------------

.PHONY: all
all: $(PROJ_NAME).cdi

.PHONY: preprocess
preprocess:
	@mkdir -p cdfs
	$(CRAYON_BASE)/preprocess.sh $(PPFLAGS)

.PHONY: clean
clean:
	rm -f $(PROJ_NAME).cdi
	rm -rf $(BUILD) cdfs

.PHONY: run
run: $(PROJ_NAME).cdi
	@mkdir -p .redream
	cd .redream; redream ../$<

#------------------------------------------------------------------------------
# Build rules
#------------------------------------------------------------------------------

$(PROJ_NAME).cdi: preprocess $(SCRAMBLED)
	mkisofs -G $(IP_BIN) -C 0,11702 -J -l -r -o $(BUILD)/$(PROJ_NAME).iso cdfs
	cdi4dc $(BUILD)/$(PROJ_NAME).iso $@

$(SCRAMBLED): $(BUILD)/$(PROJ_NAME).bin
	@mkdir -p $(dir $@)
	$(KOS_BASE)/utils/scramble/scramble $< $@

$(BUILD)/$(PROJ_NAME).bin: $(BUILD)/$(PROJ_NAME).elf
	@mkdir -p $(dir $@)
	sh-elf-objcopy -R .stack -O binary $< $@

$(BUILD)/$(PROJ_NAME).elf: $(OFILES)
	@mkdir -p $(dir $@)
	$(KOS_CC) $(CFLAGS) $(LDFLAGS) -o $@ $(KOS_START) $^ $(LIBS)

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(KOS_CC) $(CFLAGS) -c -o $@ $<
