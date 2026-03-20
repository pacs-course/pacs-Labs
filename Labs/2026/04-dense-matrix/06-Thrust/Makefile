CXX       = nvcc
NVCCFLAGS ?= -std=c++17 --extended-lambda -O3 -I.
LDLIBS    ?=

EXEC = test_matrix_mult
SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

.PHONY: all clean distclean

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(NVCCFLAGS) $(OBJS) $(LDLIBS) -o $@

%.o: %.cpp
	$(CXX) $(NVCCFLAGS) -x cu -c $< -o $@

clean:
	$(RM) *.o

distclean: clean
	$(RM) $(EXEC)
	$(RM) *.csv *.out *.bak *~
