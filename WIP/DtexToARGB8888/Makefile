
EXT =
CPPFLAGS = -std=gnu++11 -lpng

CC = g++
EXE = DtexToARGB8888

CPPFILES = $(wildcard *.cpp)
OBJS = $(CPPFILES:%.cpp=%.o)

.PHONY: all clean

# If you haven't seen this before, notice how the rules
# of the Makefile build .o files from c++ source, and then
# build the executable from the .o files. Files are only
# re-made if they've been modified, or something they depend
# on has been modified.

all: clean $(EXE)

$(EXE): $(OBJS)
	$(CC) -o $(EXE) $(OBJS) -std=gnu++11 -lpng

%.o: %.cpp
	$(CC) $(CPPFLAGS) -c $<

clean:
	rm -f *.o $(EXE)$(EXT)

run:
	./$(EXE)

dtex_examples:
	texconv -i "textures/dwarfy.png" -o "textures/dwarfy_ARGB1555.dtex" -f "ARGB1555"
	texconv -i "textures/dwarfy.png" -o "textures/dwarfy_RGB565.dtex" -f "RGB565"
	texconv -i "textures/dwarfy.png" -o "textures/dwarfy_ARGB4444.dtex" -f "ARGB4444"
	texconv -i "textures/dwarfy.png" -o "textures/dwarfy_YUV422.dtex" -f "YUV422"
	texconv -i "textures/dwarfy.png" -o "textures/dwarfy_BUMPMAP.dtex" -f "BUMPMAP"
	
	texconv -i "textures/insta.png" -o "textures/insta_PAL4BPP.dtex" -f "PAL4BPP"
	texconv -i "textures/insta.png" -o "textures/insta_PAL8BPP.dtex" -f "PAL8BPP"
