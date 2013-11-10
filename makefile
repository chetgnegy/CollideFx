#!/bin/sh

CXX=llvm-g++-4.2 
INC=-I include/ -I include/audio -I include/physics -I include/visual

INCDIR=./include/
A_INCDIR=./include/audio/
P_INCDIR=./include/physics/
V_INCDIR=./include/visual/
VPATH=$(INCDIR) $(A_INCDIR) $(P_INCDIR) $(V_INCDIR)

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


A_OBJS = DigitalFilter.o RtAudio.o UGenChain.o UnitGenerator.o
P_OBJS = Disc.o GraphicsBox.o World.o
V_OBJS = Physics.o vmath.o 

AudioHockeyTable: $(A_OBJS) $(P_OBJS) $(V_OBJS) AudioHockeyTable.o
	$(CXX) -o  AudioHockeyTable $(INC) $(A_OBJS) $(P_OBJS) $(V_OBJS) AudioHockeyTable.o $(LIBS)

AudioHockeyTable.o: AudioHockeyTable.cpp DigitalFilter.h
	$(CXX) $(FLAGS) $(INC) AudioHockeyTable.cpp

#------------------Audio modules-----------------#

DigitalFilter.o: DigitalFilter.cpp DigitalFilter.h
	$(CXX) $(FLAGS) $(INC) $(A_INCDIR)DigitalFilter.cpp

RtAudio.o: RtAudio.h RtError.h RtAudio.cpp
	$(CXX) $(FLAGS) $(INC) $(A_INCDIR)RtAudio.cpp

UGenChain.o: UGenChain.cpp UGenChain.h
	$(CXX) $(FLAGS) $(INC) $(A_INCDIR)UGenChain.cpp

UnitGenerator.o: UnitGenerator.cpp UnitGenerator.h
	$(CXX) $(FLAGS) $(INC) $(A_INCDIR)UnitGenerator.cpp

#-----------------Physics modules----------------#

Physics.o: Physics.cpp Physics.h Physical.h
	$(CXX) $(FLAGS) $(INC) $(P_INCDIR)Physics.cpp

vmath.o: vmath.cpp vmath.h
	$(CXX) $(FLAGS) $(INC) $(P_INCDIR)vmath.cpp

#------------------Visual modules----------------#

Disc.o: Disc.cpp Disc.h Drawable.h Moveable.h Physical.h
	$(CXX) $(FLAGS) $(INC) $(V_INCDIR)Disc.cpp

GraphicsBox.o: GraphicsBox.cpp GraphicsBox.h
	$(CXX) $(FLAGS) $(INC) $(V_INCDIR)GraphicsBox.cpp

World.o: World.cpp World.h Drawable.h graphicsutil.h
	$(CXX) $(FLAGS) $(INC) $(V_INCDIR)World.cpp


clean:
	rm -f *~ *# *.o AudioHockeyTable