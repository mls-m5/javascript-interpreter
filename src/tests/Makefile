tests = $(patsubst %_test.cpp,%_test,$(wildcard *_test.cpp))
CPPFLAGS = -std=c++11

all: ${tests}

vm_test: vm_test.cpp
	g++ $< -o $@ ${CPPFLAGS}

