STL_CXX      ?= g++
STL_CXXFLAGS ?= -std=c++20 -O3 -I.
STL_LDLIBS   ?= -ltbb

THRUST_CXX      ?= nvcc
THRUST_CXXFLAGS ?= -std=c++20 --extended-lambda -O3 -I. -DUSE_THRUST
THRUST_LDLIBS   ?=

EXEC = test_matrix_mult
SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

.PHONY: all stl thrust clean distclean

all: stl

stl: CXX = $(STL_CXX)
stl: CXXFLAGS = $(STL_CXXFLAGS)
stl: LDLIBS = $(STL_LDLIBS)
stl: clean $(EXEC)

thrust: CXX = $(THRUST_CXX)
thrust: CXXFLAGS = $(THRUST_CXXFLAGS)
thrust: LDLIBS = $(THRUST_LDLIBS)
thrust: clean $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(LDLIBS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(if $(findstring nvcc,$(notdir $(CXX))),-x cu,) -c $< -o $@

clean:
	$(RM) *.o

distclean: clean
	$(RM) $(EXEC)
	$(RM) *.csv *.out *.bak *~
