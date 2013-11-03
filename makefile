#!/bin/sh

CXX=llvm-g++-4.2
INCLUDES=

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


OBJS=  RtAudio.o DigitalFilter.o UnitGenerator.o Disc.o World.o UGenChain.o audiohockeytable.o 

audiohockeytable: $(OBJS)
	$(CXX) -o audiohockeytable $(OBJS) $(LIBS) 

audiohockeytable.o: audiohockeytable.cpp graphics.h DigitalFilter.h
	$(CXX) $(FLAGS) audiohockeytable.cpp

UGenChain.o: UGenChain.cpp UGenChain.h
	$(CXX) $(FLAGS) UGenChain.cpp

UnitGenerator.o: UnitGenerator.cpp UnitGenerator.h
	$(CXX) $(FLAGS) UnitGenerator.cpp

Disc.o: Disc.cpp Disc.h
	$(CXX) $(FLAGS) Disc.cpp

DigitalFilter.o: DigitalFilter.cpp DigitalFilter.h
	$(CXX) $(FLAGS) DigitalFilter.cpp

World.o: World.cpp World.h
	$(CXX) $(FLAGS) World.cpp


RtAudio.o: RtAudio.h RtError.h RtAudio.cpp
	$(CXX) $(FLAGS) RtAudio.cpp



clean:
	rm -f *~ *# *.o smellovision