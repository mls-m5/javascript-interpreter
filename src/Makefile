
cppfiles = ${wildcard *.cpp}
objects = ${cppfiles:.cpp=.o} #main.o virtualmachine.o jsfunctions.o ast.o compiler.o object.o date.o function.o
headers = ${wildcard *.h}
CXXFLAGS = -g  -std=c++11 -Isrc/
target = ../mjavascript
all: $(objects)
	g++ $(objects) -o $(target) $(CXXFLAGS)
	make -C tests/ tests

%.o: %.cpp ${headers} # virtualmachine.h value.h lexer.h ast.h compiler.h statement.h nativefunction.h
	g++ -c $< -o $@ $(CXXFLAGS)
	
clean:
	rm -f *.o
	rm $(target)