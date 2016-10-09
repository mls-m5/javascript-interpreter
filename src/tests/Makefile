tests = $(patsubst %_test.cpp,%_test,$(wildcard *_test.cpp))
CPPFLAGS = -g -std=c++11
OBJECS = virtualmachine.o

all: ${tests}

vm_test: vm_test.cpp ../virtualmachine.h ../value.h
	g++ $< -o $@ ${CPPFLAGS} ${OBJECTS}

