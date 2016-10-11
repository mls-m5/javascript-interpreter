
objects = main.o virtualmachine.o jsfunctions.o
CXXFLAGS = -g  -std=c++11
target = ../mjavascript
all: $(objects)
	g++ $(objects) -o $(target) $(CXXFLAGS)
	make -C tests/

%.o: %.cpp virtualmachine.h value.h
	g++ -c $< -o $@ $(CXXFLAGS)
	
clean:
	rm -f *.o
	rm $(target)