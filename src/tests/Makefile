tests = $(patsubst %_test.cpp,%_test,$(wildcard *_test.cpp))
CPPFLAGS = -g -std=c++11
objects = ../virtualmachine.o ../jsfunctions.o ../ast.o

.PHONY: build_objects
build_objects:
	make -C ../

all: build_objects

tests: ${tests} 

vm_test: vm_test.cpp ../virtualmachine.h ../value.h $(objects)
	g++ $< -o $@ ${CPPFLAGS} ${objects}
	
	
gc_test: gc_test.cpp ../virtualmachine.h ../value.h $(objects)
	g++ $< -o $@ ${CPPFLAGS} ${objects}
	

lexer_test: lexer_test.cpp ../lexer.h $(objects)
	g++ $< -o $@ ${CPPFLAGS} ${objects}
	
ast_test: ast_test.cpp ../lexer.h ../ast.h $(objects)
	g++ $< -o $@ ${CPPFLAGS} ${objects}

compiler_test: compiler_test.cpp ../value.h ../lexer.h ../ast.h $(objects)
	g++ $< -o $@ ${CPPFLAGS} ${objects}