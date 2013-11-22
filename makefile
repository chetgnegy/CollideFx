#!/bin/sh

CXX=g++ -w
INC=-I include/ -I include/audio -I include/physics/ -I include/visual/ -I include/visual/ui/

INCDIR=./include/
A_INCDIR=./include/audio/
P_INCDIR=./include/physics/
V_INCDIR=./include/visual/
U_INCDIR=./include/visual/ui/
VPATH=$(INCDIR) $(A_INCDIR) $(P_INCDIR) $(V_INCDIR) $(U_INCDIR)

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
FLAGS=-D__UNIX_JACK__ -c 
LIBS=-lasound -lpthread -ljack -lstdc++ -lm
endif
ifeq ($(UNAME), Darwin)
FLAGS=-D__MACOSX_CORE__ -c -g
LIBS=-framework CoreAudio -framework CoreMIDI -framework CoreFoundation \
	-framework IOKit -framework Carbon -framework OpenGL \
	-framework GLUT -framework Foundation -framework AppKit \
	-lstdc++ -lm
endif


A_OBJS = ClassicWaveform.o DigitalFilter.o fft.o RtAudio.o RtMidi.o Thread.o Stk.o UGenChain.o UGenGraphBuilder.o UnitGenerator.o
P_OBJS = Physics.o vmath.o 
V_OBJS = Disc.o Graphics.o Orb.o World.o 
U_OBJS = Menu.o RgbImage.o

AudioHockeyTable: $(A_OBJS) $(P_OBJS) $(V_OBJS) $(U_OBJS) AudioHockeyTable.o
	$(CXX) -o AudioHockeyTable $(INC) $(A_OBJS) $(P_OBJS) $(V_OBJS) $(U_OBJS) AudioHockeyTable.o $(LIBS)

AudioHockeyTable.o: AudioHockeyTable.cpp DigitalFilter.h
	$(CXX) $(FLAGS) $(INC) AudioHockeyTable.cpp

#------------------Audio modules-----------------#

ClassicWaveform.o: ClassicWaveform.cpp ClassicWaveform.h
	$(CXX) $(FLAGS) $(INC) $(A_INCDIR)ClassicWaveform.cpp

DigitalFilter.o: DigitalFilter.cpp DigitalFilter.h
	$(CXX) $(FLAGS) $(INC) $(A_INCDIR)DigitalFilter.cpp

fft.o: fft.cpp fft.h
	$(CXX) $(FLAGS) $(INC) $(A_INCDIR)fft.cpp

RtAudio.o: RtAudio.h RtError.h RtAudio.cpp
	$(CXX) $(FLAGS) $(INC) $(A_INCDIR)RtAudio.cpp

RtMidi.o: RtMidi.h RtError.h RtMidi.cpp
	$(CXX) $(FLAGS) $(INC) $(A_INCDIR)RtMidi.cpp

Stk.o: Stk.h Stk.cpp
	$(CXX) $(FLAGS) $(INC) $(A_INCDIR)Stk.cpp

Thread.o: Thread.h Thread.cpp
	$(CXX) $(FLAGS) $(INC) $(A_INCDIR)Thread.cpp

UGenChain.o: UGenChain.cpp UGenChain.h
	$(CXX) $(FLAGS) $(INC) $(A_INCDIR)UGenChain.cpp

UGenGraphBuilder.o: UGenGraphBuilder.cpp UGenGraphBuilder.h
	$(CXX) $(FLAGS) $(INC) $(A_INCDIR)UGenGraphBuilder.cpp

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

Graphics.o: Graphics.cpp Graphics.h
	$(CXX) $(FLAGS) $(INC) $(V_INCDIR)Graphics.cpp

Orb.o: Orb.cpp Orb.h
	$(CXX) $(FLAGS) $(INC) $(V_INCDIR)Orb.cpp

World.o: World.cpp World.h Drawable.h graphicsutil.h
	$(CXX) $(FLAGS) $(INC) $(V_INCDIR)World.cpp

#------------------UI modules----------------#

Menu.o: Menu.cpp Menu.h
	$(CXX) $(FLAGS) $(INC) $(U_INCDIR)Menu.cpp

RgbImage.o: RgbImage.cpp RgbImage.h
	$(CXX) $(FLAGS) $(INC) $(U_INCDIR)RgbImage.cpp

clean:
	rm -f *~ *# *.o AudioHockeyTable