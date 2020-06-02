#------------------------------------------------------------------------------
# Crayon Makefile
#------------------------------------------------------------------------------
$(if $(PLATFORM),,$(error Please specify the target platform to compile Crayon\
  for, or to clean, on the command line (eg. `make PLATFORM=dreamcast`). Run\
  `ls code` for a list of platform names))

LIB_NAME      := crayon
CFLAGS        := $(KOS_CFLAGS) -Wshadow -Wextra -Iinclude/crayon

SRC           := code/$(PLATFORM)
LIB_CRAYON    := lib/$(PLATFORM)
INCLUDED_LIBS := -lkosfat
BUILD         := build

CFILES        := $(shell find $(SRC) -type f -name "*.c")
OBJS          := $(CFILES:%.c=$(BUILD)/%.o)

#------------------------------------------------------------------------------
.PHONY: all
all: $(LIB_CRAYON)/lib$(LIB_NAME).a

#------------------------------------------------------------------------------
.PHONY: clean
clean:
	rm -rf $(BUILD) $(LIB_CRAYON)

#------------------------------------------------------------------------------
# Targets
#------------------------------------------------------------------------------

#NOTE: "dir" is not same as bash's dir. This is makefile specific and it takes
#a filename and returns the path section. So here thats just "$(LIB)"
$(LIB_CRAYON)/lib$(LIB_NAME).a: $(OBJS)
	@mkdir -p $(dir $@)
	rm -f $@
	$(KOS_AR) rcs $@ $^

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(KOS_CC) $(INCLUDED_LIBS) $(CFLAGS) -c -o $@ $<
