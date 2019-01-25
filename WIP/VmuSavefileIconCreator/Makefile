
EXT =
CPPFLAGS = -std=gnu++11

CC = g++
EXE = PngToVmuIcon

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
	$(CC) -o $(EXE) $(OBJS) -std=gnu++11

%.o: %.cpp
	$(CC) $(CPPFLAGS) -c $<

clean:
	rm -f *.o $(EXE)$(EXT)

run_LCD:
	./$(EXE) "icon_VMU_LCD.png" "LCD.bin" 1

# run_Save:
# $(EXE) "icon_Savefile.png" "palette.bin" 0
