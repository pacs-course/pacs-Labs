CXX      ?= g++
CXXFLAGS ?= -std=c++17
CPPFLAGS ?= -O3 -Wall

EXEC     = main
LDFLAGS ?=
LIBS    ?=

all: $(EXEC)

%.o: %.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<

$(EXEC): %: %.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $< $(LIBS) -o $@

clean:
	$(RM) *.o

distclean: clean
	$(RM) $(EXEC)
	$(RM) *~
