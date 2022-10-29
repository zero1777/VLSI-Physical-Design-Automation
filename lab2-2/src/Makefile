# g++ -std=c++11 main.cpp LCN.cpp -o main
# sample Makefile
RES = main.cpp LCN.cpp Partition.cpp
EXE = hw2
CFLAGS = -std=c++11 -O3 --optimize
EXEDIR = ../bin
all:main.cpp LCN.cpp Partition.cpp LCN.hpp Partition.hpp
	g++ ./$(RES) $(CFLAGS) -o $(EXEDIR)/$(EXE)
clean:
	rm $(EXEDIR)/$(EXE)