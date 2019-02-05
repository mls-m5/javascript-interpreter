tests = $(patsubst %_test.cpp,%_test,$(wildcard *_test.cpp))
CPPFLAGS = -g -std=c++11
objects = ../virtualmachine.o ../jsfunctions.o ../ast.o ../compiler.o ../object.o

.PHONY: build_objects
build_objects:
	make -C ../

all: build_objects

tests: ${tests} 

%_test: %_test.o $(objects)
	g++ $< -o $@ ${CPPFLAGS} ${objects}

vm_test.o: vm_test.cpp ../virtualmachine.h ../value.h ../statement.h ../compiler.h ../ast.h
	g++ -c $< -o $@ ${CPPFLAGS}
	
	
gc_test.o: gc_test.cpp ../virtualmachine.h ../value.h ../statement.h
	g++ -c $< -o $@ ${CPPFLAGS}
	

lexer_test.o: lexer_test.cpp ../lexer.h
	g++ -c $< -o $@ ${CPPFLAGS}
	
ast_test.o: ast_test.cpp ../lexer.h ../ast.h
	g++ -c $< -o $@ ${CPPFLAGS}

compiler_test.o: compiler_test.cpp ../value.h ../lexer.h ../ast.h ../compiler.h ../statement.h
	g++ -c $< -o $@ ${CPPFLAGS}
	