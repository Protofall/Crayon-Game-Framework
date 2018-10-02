# flags
#---------------------------------------------------------------------------------

CFLAGS = $(KOS_CFLAGS)
# CFLAGS += -I$(KOS_BASE)/addons/zlib	#Might need to add this to the bash script

#Change this so the path isn't hardcoded (And add a "if not defined")
crayon_path = ../Crayon
crayon_build = dreamcast

#A list of all of Crayon's .o file locations
crayon_source_files = $(wildcard $(crayon_path)/code/$(crayon_build)/*.c)
crayon_o_files = $(patsubst $(crayon_path)/code/$(crayon_build)/%.c,$(crayon_path)/code/$(crayon_build)/compiled/%.o,$(crayon_source_files))

scrambled_name = 1st_read.bin
project_name = $(shell pwd | sed -e 's/.*\///')


# project_o_files = $(wildcard */*.c)	#Can't do subfolders?
# project_source_files = $(shell find . -name '*.c')	#For example it can return ./code/subfolder/file.c
# test = lol/lol2/name.c
# test_o_file = $(shell echo $(test) | sed -e 's/.*\///' | sed -e 's/\.[^\.]*$//.o/')	#Will convert a path into the correct o file

project_source_files := $(shell find . -type f -name "*.c")
project_o_files := $(patsubst %.c,%.o,$(notdir $(project_source_files)))

ifneq ($(words $(project_o_files)),$(words $(sort $(project_o_files))))
$(error Multiple code files share the same name. Please make sure there is only 1 instance of every name)
endif

objs = $(project_o_files)
objs += $(crayon_o_files)


cdfs = cdfs
IPBIN = $(crayon_path)/IP.BIN	#Change this depending on where your IP.BIN file is located

.PHONY: all
all: dreamcast_cdi

.PHONY: dreamcast_cdi
dreamcast_cdi: preprocess $(scrambled_name)
	mkisofs -G $(IPBIN) -C 0,11702 -J -l -r -o $(project_name).iso $(cdfs)
	cdi4dc $(project_name).iso $(project_name).cdi

.PHONY: dreamcast_sd
dreamcast_sd: preprocess $(project_name).bin
	cp $(project_name).bin $(cdfs)/$(project_name).bin

.PHONY: clean
clean:
	rm -f *.o
	rm -f $(project_name).elf
	rm -f $(project_name).bin
	rm -f $(project_name).iso
	rm -f $(project_name).cdi
	rm -rf $(cdfs)/*

.PHONY: help
help:
	./$(crayon_path)/preprocess.sh -h

.PHONY: check
check:
	./$(crayon_path)/preprocess.sh -noRM

# I should probs remove buildRun later
.PHONY: buildRun
buildRun: dreamcast_cdi
	redream $(project_name).cdi

.PHONY: test
test: $(objs)

# include $(KOS_BASE)/Makefile.rules

#These two are needed for re-compiling Crayon
.PHONY: crayon
crayon: $(crayon_o_files)

.PHONY: crayon_clean
crayon_clean:
	rm -f ./$(crayon_path)/code/$(crayon_build)/compiled/*.o

#------------------Builders--------------------

#Crayon code compilation
$(crayon_path)/code/$(crayon_build)/compiled/%.o: $(crayon_path)/code/$(crayon_build)/%.c
	kos-cc $(KOS_CFLAGS) -c $< -o $@

#Project code compilation (This will always rerun every time its called)
define COMPILE_rule
$$(patsubst %.c,%.o,$$(notdir $(1))): $(1)
	kos-cc $$(KOS_CFLAGS) -c -o $$@ $$<
endef
$(foreach s,$(project_source_files),$(eval $(call COMPILE_rule,$(s))))

#Assets preprocessing
.PHONY: preprocess
preprocess:
	./$(crayon_path)/preprocess.sh

#Linking all .o files into an elf
$(project_name).elf: $(objs)
	$(KOS_CC) $(KOS_CFLAGS) $(KOS_LDFLAGS) -o $@ $(KOS_START) $^ -lkosext2fs -lz -lm $(KOS_LIBS)

#Convert the elf into a binary
$(project_name).bin: $(project_name).elf
	sh-elf-objcopy -R .stack -O binary $(project_name).elf $(project_name).bin

#Scramble the elf
$(scrambled_name): $(project_name).bin
	$(KOS_BASE)/utils/scramble/scramble $(project_name).bin $(cdfs)/$(scrambled_name)
