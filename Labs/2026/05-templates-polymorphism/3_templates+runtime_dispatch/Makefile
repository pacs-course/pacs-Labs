RUNTIME_CXX      ?= nvcc
RUNTIME_CXXFLAGS ?= -std=c++20 --extended-lambda -O3 -I.
RUNTIME_LDLIBS   ?= -ltbb

EXEC = test_matrix_mult
SRCS = matrix.cpp test_matrix_mult.cpp
OBJS = $(SRCS:.cpp=.o)

.PHONY: all clean distclean

all: CXX = $(RUNTIME_CXX)
all: CXXFLAGS = $(RUNTIME_CXXFLAGS)
all: LDLIBS = $(RUNTIME_LDLIBS)
all: clean $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(LDLIBS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(if $(findstring nvcc,$(notdir $(CXX))),-x cu,) -c $< -o $@

clean:
	$(RM) *.o

distclean: clean
	$(RM) $(EXEC)
	$(RM) *.csv *.out *.bak *~
