
objects = main.o
CXXFLAGS = -g  -std=c++11
all: $(objects)
	g++ $(objects) -o mjavascript
	make -C tests/

%.o: %.cpp
	g++ -c $< -o $@ $(CXXFLAGS)