# g++ -std=c++11 main.cpp LCN.cpp -o main
# sample Makefile
RES = main.cpp Reader.cpp Floorplan.cpp
EXE = hw3
CFLAGS = -std=c++11 -O3 --optimize
EXEDIR = ../bin
all:main.cpp Reader.cpp Floorplan.cpp Reader.hpp Floorplan.hpp
	g++ ./$(RES) $(CFLAGS) -o $(EXEDIR)/$(EXE)
clean:
	rm $(EXEDIR)/$(EXE)