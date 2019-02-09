
EXT =
CPPFLAGS = -std=gnu++11 -lpng

CC = g++
EXE = DtexToRGBA8888

CPPFILES = $(wildcard *.cpp)
OBJS = $(CPPFILES:%.cpp=%.o)

.PHONY: all clean

# If you haven't seen this before, notice how the rules
# of the Makefile build .o files from c++ source, and then
# build the executable from the .o files. Files are only
# re-made if they've been modified, or something they depend
# on has been modified.

all: clean dtex_examples $(EXE)

$(EXE): $(OBJS)
	$(CC) -o $(EXE) $(OBJS) -std=gnu++11 -lpng

%.o: %.cpp
	$(CC) $(CPPFLAGS) -c $<

clean:
	rm -f *.o $(EXE)$(EXT)
	rm -f textures/*.png
	rm -f textures/*.dtex
	rm -f textures/*.dtex.pal
	rm -f *.bin

run:
	./DtexToRGBA8888 textures/cow_RGB565.dtex lol.bin --preview
	./DtexToRGBA8888 textures/cow_YUV422.dtex lol.bin --preview
	./DtexToRGBA8888 textures/cow_YUV422_s.dtex lol.bin --preview

	./DtexToRGBA8888 textures/dwarfy_ARGB1555.dtex lol.bin --preview
	./DtexToRGBA8888 textures/dwarfy_RGB565.dtex lol.bin --preview
	./DtexToRGBA8888 textures/dwarfy_ARGB4444.dtex lol.bin --preview
	./DtexToRGBA8888 textures/dwarfy_YUV422.dtex lol.bin --preview
	#./DtexToRGBA8888 textures/dwarfy_BUMPMAP.dtex lol.bin --preview

	./DtexToRGBA8888 textures/insta_RGB565.dtex lol.bin --preview
	./DtexToRGBA8888 textures/insta_PAL4BPP.dtex lol.bin --preview
	./DtexToRGBA8888 textures/insta_PAL8BPP.dtex lol.bin --preview

	./DtexToRGBA8888 textures/lines_PAL8BPP.dtex lol.bin --preview

	./DtexToRGBA8888 textures/simpleRect_RGB565.dtex lol.bin --preview
	./DtexToRGBA8888 textures/simpleRect_PAL4BPP.dtex lol.bin --preview

	./DtexToRGBA8888 textures/strideTest_RGB565.dtex lol.bin --preview

dtex_examples:
	texconv -i "textures/source/cow.png" -o "textures/cow_RGB565.dtex" -f "RGB565"
	texconv -i "textures/source/cow.png" -o "textures/cow_YUV422.dtex" -f "YUV422"
	texconv -i "textures/source/cow.png" -o "textures/cow_YUV422_s.dtex" -f "YUV422" -stride 4	#128/32 = 4

	texconv -i "textures/source/dwarfy.png" -o "textures/dwarfy_ARGB1555.dtex" -f "ARGB1555"
	texconv -i "textures/source/dwarfy.png" -o "textures/dwarfy_RGB565.dtex" -f "RGB565"
	texconv -i "textures/source/dwarfy.png" -o "textures/dwarfy_ARGB4444.dtex" -f "ARGB4444"
	texconv -i "textures/source/dwarfy.png" -o "textures/dwarfy_YUV422.dtex" -f "YUV422"
	#texconv -i "textures/source/dwarfy.png" -o "textures/dwarfy_BUMPMAP.dtex" -f "BUMPMAP"

	texconv -i "textures/source/insta.png" -o "textures/insta_RGB565.dtex" -f "RGB565"
	texconv -i "textures/source/insta.png" -o "textures/insta_PAL4BPP.dtex" -f "PAL4BPP"
	texconv -i "textures/source/insta.png" -o "textures/insta_PAL8BPP.dtex" -f "PAL8BPP"

	texconv -i "textures/source/lines.png" -o "textures/lines_PAL8BPP.dtex" -f "PAL8BPP"

	texconv -i "textures/source/simpleRect.png" -o "textures/simpleRect_RGB565.dtex" -f "RGB565"	#Showing non-square textures work
	texconv -i "textures/source/simpleRect.png" -o "textures/simpleRect_PAL4BPP.dtex" -f "PAL4BPP"

	texconv -i "textures/source/strideTest.png" -o "textures/strideTest_RGB565.dtex" -f "RGB565" -stride 3	#96/32 = 3. Note stride only applies to width (Height of 96 doesn't work)
