CXX       = mpic++
CXXFLAGS ?= -std=c++17
CPPFLAGS ?= -fopenmp -O3 -Wall -pedantic

LDFLAGS ?=
LIBS    ?=

EXEC = 01-helloWorld 02-Matrix

.PHONY = all $(EXEC) clean distclean $(DEPEND)

all: $(EXEC)

$(EXEC): %: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $< $(LIBS) -o $@

clean:
	$(RM) *.o

distclean: clean
	$(RM) $(EXEC)
	$(RM) *.csv *.out *.bak *~
