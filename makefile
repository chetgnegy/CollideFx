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


OBJS=  vmath.o RtAudio.o Physics.o DigitalFilter.o UnitGenerator.o GraphicsBox.o Disc.o World.o UGenChain.o audiohockeytable.o

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

Disc.o: Disc.cpp Disc.h Drawable.h Moveable.h Physical.h
	$(CXX) $(FLAGS) $(INC) Disc.cpp

DigitalFilter.o: DigitalFilter.cpp DigitalFilter.h
	$(CXX) $(FLAGS) $(INC) DigitalFilter.cpp

World.o: World.cpp World.h Drawable.h graphicsutil.h
	$(CXX) $(FLAGS) $(INC) World.cpp

Physics.o: Physics.cpp Physics.h Physical.h
	$(CXX) $(FLAGS) $(INC) Physics.cpp

RtAudio.o: RtAudio.h RtError.h RtAudio.cpp
	$(CXX) $(FLAGS) $(INC) RtAudio.cpp

vmath.o: vmath.cpp vmath.h
	$(CXX) $(FLAGS) $(INC) vmath.cpp


clean:
	rm -f *~ *# *.o smellovision