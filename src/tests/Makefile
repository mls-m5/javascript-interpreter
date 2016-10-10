tests = $(patsubst %_test.cpp,%_test,$(wildcard *_test.cpp))
CPPFLAGS = -g -std=c++11
objects = ../virtualmachine.o ../jsfunctions.o

all: ${tests}

vm_test: vm_test.cpp ../virtualmachine.h ../value.h $(objects)
	g++ $< -o $@ ${CPPFLAGS} ${objects}
	
	
gc_test: gc_test.cpp ../virtualmachine.h ../value.h $(objects)
	g++ $< -o $@ ${CPPFLAGS} ${objects}
	

