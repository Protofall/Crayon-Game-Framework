#------------------------------------------------------------------------------
# Crayon Makefile
#------------------------------------------------------------------------------
# This variable specifies the target platform to build the library for.
PLATFORM  := # Set this on the command line (i.e. $ make PLATFORM=<platform>)
# To see a list of valid platforms list the code subdirectory.

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
	$(KOS_AR) rcs $@ $^

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(KOS_CC) $(CFLAGS) -c -o $@ $<
