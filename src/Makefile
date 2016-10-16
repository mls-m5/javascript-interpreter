
objects = main.o virtualmachine.o jsfunctions.o ast.o
CXXFLAGS = -g  -std=c++11
target = ../mjavascript
all: $(objects)
	g++ $(objects) -o $(target) $(CXXFLAGS)
	make -C tests/ tests

%.o: %.cpp virtualmachine.h value.h lexer.h ast.h compiler.h
	g++ -c $< -o $@ $(CXXFLAGS)
	
clean:
	rm -f *.o
	rm $(target)