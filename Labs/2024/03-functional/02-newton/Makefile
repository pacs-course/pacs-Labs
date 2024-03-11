CXX      ?= g++
CXXFLAGS ?= -std=c++20
CPPFLAGS ?= -O3 -Wall -I. -Wno-conversion-null -Wno-deprecated-declarations -I../../../../../pacs-examples/Examples/include

#EXEC     = mainMuParserGetPot
#EXEC     = mainGetPotLambda
EXEC     = main
#LDFLAGS ?= -L../../../../../pacs-examples/Examples/lib
#LIBS  ?= -lmuparser

all: $(EXEC)

%.o: %.cpp NewtonSolver.hpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<

$(EXEC): %: %.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $< $(LIBS) -o $@

clean:
	$(RM) *.o

distclean: clean
	$(RM) *~
