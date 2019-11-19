#------------------------------------------------------------------------------
# Crayon Makefile
#------------------------------------------------------------------------------
$(if $(PLATFORM),,$(error Please specify the target platform to compile Crayon\
  for, or to clean, on the command line (eg. `make PLATFORM=dreamcast`). Run\
  `ls code` for a list of platform names))

LIB_NAME  := crayon
CFLAGS    := $(KOS_CFLAGS) -Wshadow -Iinclude/crayon

SRC       := code/$(PLATFORM)
LIB       := lib/$(PLATFORM)
BUILD     := build

CFILES    := $(shell find $(SRC) -type f -name "*.c")
OBJS      := $(CFILES:%.c=$(BUILD)/%.o)

#------------------------------------------------------------------------------
.PHONY: all
all: $(LIB)/lib$(LIB_NAME).a

#------------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -rf $(BUILD) $(LIB)

#------------------------------------------------------------------------------
# Targets
#------------------------------------------------------------------------------

$(LIB)/lib$(LIB_NAME).a: $(OBJS)
	@mkdir -p $(dir $@)	#Dir is not same as bash's dir. This is makefile specific
						#And it takes a filename and returns the path section
						#So here thats just "$(LIB)"
	rm -f $@
	$(KOS_AR) rcs $@ $^

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(KOS_CC) $(CFLAGS) -c -o $@ $<
