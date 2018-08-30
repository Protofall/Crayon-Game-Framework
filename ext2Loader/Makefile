#Note that the cdi option uses Kazade's cdi4dc program which is a terminal/Unix
#equivalent of bootdreams from Windows

NAME =  ext2Loader

OFILES = $(patsubst %.c,%.o,$(wildcard *.c))
SCRAMBLED = 1st_read.bin

KOS_LOCAL_CFLAGS = -I$(KOS_BASE)/addons/zlib

LIBS =  -lkosext2fs -lz -lm $(KOS_LIBS)

all: clean $(SCRAMBLED)

include $(KOS_BASE)/Makefile.rules
	
clean:
	-rm -f $(NAME).elf
	-rm -f $(NAME).bin 1st_read.bin $(NAME).iso $(NAME).cdi
	-rm -f $(OFILES)

cdi: clean $(SCRAMBLED)
	mkisofs -G $(KOS_BASE)/../IP.BIN -C 0,11702 -J -l -r -o $(NAME).iso .
	cdi4dc $(NAME).iso $(NAME).cdi

$(SCRAMBLED): $(NAME).elf
	sh-elf-objcopy -R .stack -O binary $(NAME).elf $(NAME).bin
	$(KOS_BASE)/utils/scramble/scramble $(NAME).bin $(SCRAMBLED)

$(NAME).elf: $(OFILES)
	$(KOS_CC) $(KOS_CFLAGS) $(KOS_LDFLAGS) -o $@ $(KOS_START) $^ -lz -lm $(LIBS)
