CXX      ?= g++
CXXFLAGS ?= -std=c++17

# flags to handle Release and Debug build
BUILD ?= debug
CPPFLAGS_DEBUG := -O0 -g -DDEBUG --coverage
CPPFLAGS_RELEASE := -O3 -DNDEBUG
ifeq ($(BUILD), release)
	CPPFLAGS_EXTRA := $(CPPFLAGS_RELEASE)
	BUILD_TYPE = "Release"
else
	ifeq ($(BUILD), debug)
		CPPFLAGS_EXTRA := $(CPPFLAGS_DEBUG)
		BUILD_TYPE = "Debug"
	else 
		CPPFLAGS_EXTRA := $(CPPFLAGS_DEBUG)
		BUILD_TYPE = "Not recognized. Default to Debug"
	endif
endif

CPPFLAGS ?= $(CPPFLAGS_EXTRA) -Wall -pedantic -I. -I${mkEigenInc} -I${mkOpenblasInc}
LDLIBS   ?= -L${mkOpenblasLib} -lopenblas
LINK.o := $(LINK.cc) # Use C++ linker.

DEPEND = make.dep

EXEC = test_matrix_mult
SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

.PHONY = all echo_build_type $(EXEC) $(OBJS) coverage memcheck profile clean distclean $(DEPEND)

all: echo_build_type $(DEPEND) $(EXEC)

echo_build_type:
	@echo "================================================"
	@echo "Build type:" $(BUILD_TYPE)
	@echo "================================================"

$(EXEC): $(OBJS)

$(OBJS): %.o: %.cpp

coverage: all
	lcov --directory . --zerocounters
	./$(EXEC) 200
	lcov --directory . --capture --no-external --output test_coverage.info
	genhtml test_coverage.info --output test_coverage

memcheck: all
	valgrind --tool=memcheck ./$(EXEC) 200

profile: all
	valgrind --tool=callgrind ./$(EXEC) 200

clean:
	$(RM) $(DEPEND)
	$(RM) -r *.o *.gcda *.gcno test_coverage* callgrind*

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
