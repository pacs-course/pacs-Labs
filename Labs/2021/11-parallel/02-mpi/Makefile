CXX       = mpic++
CXXFLAGS ?= -std=c++17
CPPFLAGS ?= -fopenmp -O3 -Wall -pedantic -I.
LDLIBS   ?= 
LINK.o := $(LINK.cc) # Use C++ linker.

DEPEND = make.dep

EXEC = 01-hello_world 02-ping_pong 03-probe 04-deadlock 04-non_blocking 05-pi 06-matrix_vector_product
SRCS = # $(wildcard *.cpp)
OBJS = # $(SRCS:.cpp=.o)

.PHONY = all $(EXEC) $(OBJS) clean distclean $(DEPEND)

all: $(DEPEND) $(EXEC)

$(EXEC): $(OBJS)

$(OBJS): %.o: %.cpp

clean:
	$(RM) $(DEPEND)
	$(RM) *.o

distclean: clean
	$(RM) $(EXEC)
	$(RM) *.csv *.out *.bak *~

$(DEPEND): $(SRCS)
	@$(RM) $(DEPEND)
	@for file in $(SRCS); \
	do \
	  $(CXX) $(CPPFLAGS) $(CXXFLAGS) -MM $${file} >> $(DEPEND); \
	done

-include $(DEPEND)
