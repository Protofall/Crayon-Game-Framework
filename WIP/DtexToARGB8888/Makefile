
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

