
objects = main.o virtualmachine.o jsfunctions.o ast.o compiler.o object.o
CXXFLAGS = -g  -std=c++11 -Isrc/
target = ../mjavascript
all: $(objects)
	g++ $(objects) -o $(target) $(CXXFLAGS)
	make -C tests/ tests

%.o: %.cpp virtualmachine.h value.h lexer.h ast.h compiler.h statement.h nativefunction.h
	g++ -c $< -o $@ $(CXXFLAGS)
	
clean:
	rm -f *.o
	rm $(target)