#------------------------------------------------------------------------------
# Include this file at the end of your dreamcast project's Makefile, after
# defining the configuration variables listed below. KOS' environment variables
# are also expected to be set.
#------------------------------------------------------------------------------

# CRAYON_PROJ_NAME:     Name of the project, used in output file names
# CRAYON_PROJ_SRC_C:    List of C source files to compile into the executable
# CRAYON_PROJ_IP_BIN:   (optional) IP.BIN path, if you wish to use your own
# CRAYON_PROJ_CFLAGS:   (optional) Extra compilation options
# CRAYON_PROJ_LDFLAGS:  (optional) Extra linking options
# CRAYON_PROJ_LIBS:     (optional) Extra '-l' library linkage options

# CRAYON_BASE:          Root directory of the Crayon repository
# CRAYON_BUILD_DIR:     Directory for intermediate outputs (created for you)
# CRAYON_PP_FLAGS:      (optional) Options to pass to the asset preprocessor

PROJECT  := $(CRAYON_PROJ_NAME)
CFLAGS   := $(CRAYON_PROJ_CFLAGS) $(KOS_CFLAGS) \
	-I$(CRAYON_BASE)/include
LDFLAGS  := $(CRAYON_PROJ_LDFLAGS) $(KOS_LDFLAGS) \
	-L$(CRAYON_BASE)/lib/dreamcast
LIBS     := $(CRAYON_PROJ_LIBS) -lcrayon -lm $(KOS_LIBS)
IP_BIN   := $(CRAYON_IP_BIN)$(if $(CRAYON_IP_BIN),,$(CRAYON_BASE)/IP.BIN)
BUILD    := $(CRAYON_BUILD_DIR)
OFILES   := $(CRAYON_PROJ_SRC_C:%.c=$(BUILD)/%.o)

#------------------------------------------------------------------------------
# crayon-cdi: Produce a burnable CDI image in the current directory
.PHONY: crayon-cdi
crayon-cdi: $(PROJECT).cdi

#------------------------------------------------------------------------------
# crayon-pp: Preprocess ./assets and store products in ./cdfs
### TODO: Make this behaviour more configurable
.PHONY: crayon-pp
crayon-pp:
	@mkdir -p cdfs
	$(CRAYON_BASE)/preprocess.sh -noRM $(CRAYON_PP_FLAGS)

#------------------------------------------------------------------------------
# crayon-pp-help: Show help for the asset preprocessor
.PHONY: crayon-pp-help
crayon-pp-help:
	$(CRAYON_BASE)/preprocess.sh -h

#------------------------------------------------------------------------------
# crayon-clean: Remove all outputs
.PHONY: crayon-clean
crayon-clean:
	rm -f $(PROJECT).cdi
	rm -rf $(BUILD) cdfs

#------------------------------------------------------------------------------
# crayon-run: Build and run the project in redream
.PHONY: crayon-run
crayon-run: $(PROJECT).cdi
	@mkdir -p .redream
	cd .redream; redream ../$<

#------------------------------------------------------------------------------
# crayon-rebuild-crayon: Rebuild crayon and force relink
.PHONY: crayon-rebuild-crayon
crayon-rebuild-crayon:
	cd $(CRAYON_BASE); make PLATFORM=dreamcast
	rm -f $(PROJECT).cdi $(BUILD)/$(PROJECT).*

#------------------------------------------------------------------------------
# Targets
#------------------------------------------------------------------------------

$(PROJECT).cdi: crayon-pp cdfs/1st_read.bin
	mkisofs -G $(IP_BIN) -C 0,11702 -J -l -r -o $(BUILD)/$(PROJECT).iso cdfs
	cdi4dc $(BUILD)/$(PROJECT).iso $@

cdfs/1st_read.bin: $(BUILD)/$(PROJECT).bin
	@mkdir -p $(dir $@)
	$(KOS_BASE)/utils/scramble/scramble $< $@

$(BUILD)/$(PROJECT).bin: $(BUILD)/$(PROJECT).elf
	@mkdir -p $(dir $@)
	sh-elf-objcopy -R .stack -O binary $< $@

$(BUILD)/$(PROJECT).elf: $(OFILES)
	@mkdir -p $(dir $@)
	$(KOS_CC) $(CFLAGS) $(LDFLAGS) -o $@ $(KOS_START) $^ $(LIBS)

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(KOS_CC) $(CFLAGS) -c -o $@ $<
