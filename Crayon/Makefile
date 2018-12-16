#------------------------------------------------------------------------------
# Crayon Makefile
#------------------------------------------------------------------------------
$(if $(PLATFORM),,$(error Please specify the target platform to compile Crayon\
  for, or to clean, on the command line (eg. `make PLATFORM=dreamcast`). Run\
  `ls code` for a list of platform names.))

LIB_NAME  := crayon
CFLAGS    := $(KOS_CFLAGS)

# Directories
SRC       := code/$(PLATFORM)
LIB       := lib/$(PLATFORM)
BUILD     := build

# Sources
CFILES    := $(shell find $(SRC) -type f -name "*.c")

# Intermediate outputs
OBJS      := $(CFILES:%.c=$(BUILD)/%.o)

#------------------------------------------------------------------------------
# Targets
#------------------------------------------------------------------------------

.PHONY: all
all: $(LIB)/lib$(LIB_NAME).a

.PHONY: clean
clean:
	rm -rf $(BUILD) $(LIB)

#------------------------------------------------------------------------------
# Build rules
#------------------------------------------------------------------------------

$(LIB)/lib$(LIB_NAME).a: $(OBJS)
	@mkdir -p $(dir $@)
	rm -f $@
	$(KOS_AR) cs $@ $^

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(KOS_CC) $(CFLAGS) -c -o $@ $<
