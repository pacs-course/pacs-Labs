CXX      ?= g++
CXXFLAGS ?= -std=c++17
LINK.o := $(LINK.cc) # Use C++ linker (for implicit rule in line 21).

CPPFLAGS += -O3 -I. -I../../../../Examples/include
LDLIBS += -L../../../../Examples/lib -Wl,-rpath=../../../../Examples/lib -lpacs

SRCS=$(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
HEADERS=$(wildcard *.hpp)

exe_sources=$(filter main%.cpp,$(SRCS))
EXEC=$(exe_sources:.cpp=)

.phony= all clean distclean

.DEFAULT_GOAL = all

all: $(EXEC)

$(EXEC): $(OBJS)

$(OBJS): $(SRCS) $(HEADERS)

clean:
	$(RM) -f $(OBJS)

distclean: clean
	$(RM) -f $(EXEC)
	$(RM) *.out *.bak *~
