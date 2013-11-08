#!/bin/sh

CXX=llvm-g++-4.2 
 

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
FLAGS=-D__UNIX_JACK__ -c 
LIBS=-lasound -lpthread -ljack -lstdc++ -lm
endif
ifeq ($(UNAME), Darwin)
FLAGS=-D__MACOSX_CORE__ -c
LIBS=-framework CoreAudio -framework CoreMIDI -framework CoreFoundation \
	-framework IOKit -framework Carbon -framework OpenGL \
	-framework GLUT -framework Foundation -framework AppKit \
	-lstdc++ -lm
endif


OBJS=  RtAudio.o DigitalFilter.o UnitGenerator.o GraphicsBox.o Disc.o World.o UGenChain.o audiohockeytable.o

audiohockeytable: $(OBJS)
	$(CXX) -o  audiohockeytable $(INC) $(OBJS) $(LIBS)

audiohockeytable.o: audiohockeytable.cpp graphics.h DigitalFilter.h
	$(CXX) $(FLAGS) $(INC) audiohockeytable.cpp

UGenChain.o: UGenChain.cpp UGenChain.h
	$(CXX) $(FLAGS) $(INC) UGenChain.cpp

UnitGenerator.o: UnitGenerator.cpp UnitGenerator.h
	$(CXX) $(FLAGS) $(INC) UnitGenerator.cpp

GraphicsBox.o: GraphicsBox.cpp GraphicsBox.h
	$(CXX) $(FLAGS) $(INC) GraphicsBox.cpp

Disc.o: Disc.cpp Disc.h Drawable.h Moveable.h
	$(CXX) $(FLAGS) $(INC) Disc.cpp

DigitalFilter.o: DigitalFilter.cpp DigitalFilter.h
	$(CXX) $(FLAGS) $(INC) DigitalFilter.cpp

World.o: World.cpp World.h Drawable.h graphicsutil.h
	$(CXX) $(FLAGS) $(INC) World.cpp
	
RtAudio.o: RtAudio.h RtError.h RtAudio.cpp
	$(CXX) $(FLAGS) $(INC) RtAudio.cpp



clean:
	rm -f *~ *# *.o smellovision