
objects = test.o
CXXFLAGS = -g  -std=c++11
all: $(objects)
	g++ $(objects) -o mjavascript

%.o: %.cpp
	g++ -c $< -o $@ $(CXXFLAGS)