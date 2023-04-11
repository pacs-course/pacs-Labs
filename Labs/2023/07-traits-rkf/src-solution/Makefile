CXX      ?= g++
# Wno-deprecated-enum-enum-conversion: is to avoid warinings from Eigen when using c++20
# fconcepts-diagnostics-depth: is to set the depts with which the compiler tells you which
#   concept is not satisfied, e.g. ScalarOrVector is a hierarchy of concepts, if depth=1
#   the compiler tells you just that the given type is not `Scalar<T> || Vector<T>`
#   but with depth=2 you get to know which of `Normable<T> && Addable<T> && ScalarMultiplicable<T>`
#   is not satisfied
CXXFLAGS ?= -std=c++20 -Wno-deprecated-enum-enum-conversion -fconcepts-diagnostics-depth=3
CPPFLAGS ?= -O3 -Wall -pedantic -I. -I${mkEigenInc}
LDLIBS   ?= 
LINK.o := $(LINK.cc) # Use C++ linker.

DEPEND = make.dep

EXEC = main
SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

.PHONY = all $(EXEC) $(OBJS) clean distclean $(DEPEND)

all: $(DEPEND) $(EXEC)

$(EXEC): $(OBJS)

$(OBJS): %.o: %.cpp

clean:
	$(RM) $(DEPEND)
	$(RM) *.o

distclean: clean
	$(RM) $(EXEC)
	$(RM) *.out *.bak *~

$(DEPEND): $(SRCS)
	@$(RM) $(DEPEND)
	@for file in $(SRCS); \
	do \
	  $(CXX) $(CPPFLAGS) $(CXXFLAGS) -MM $${file} >> $(DEPEND); \
	done

-include $(DEPEND)
