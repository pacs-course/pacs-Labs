CXX      ?= g++
CXXFLAGS ?= -std=c++17
CPPFLAGS ?= -Wall -pedantic

LDFLAGS ?=
LIBS    ?=

EXEC = assert1 assert2 debug_macro defensive \
       pause print student1 student2 \
       undefined_behaviour undefined_behaviour2

DEBUG_SUFFIX = debug
RELEASE_SUFFIX = release

.PHONY = all $(EXEC) clean distclean

all: $(EXEC)

$(EXEC): %: %.cpp
	$(CXX) $(CPPFLAGS) -O0 $(CXXFLAGS) $(LDFLAGS) $< $(LIBS) -o $@.$(DEBUG_SUFFIX)
	$(CXX) $(CPPFLAGS) -O3 -DNDEBUG $(CXXFLAGS) $(LDFLAGS) $< $(LIBS) -o $@.$(RELEASE_SUFFIX)
	

clean:
	$(RM) *.o

distclean: clean
	$(RM) $(addsuffix .$(DEBUG_SUFFIX), $(EXEC))
	$(RM) $(addsuffix .$(RELEASE_SUFFIX), $(EXEC))
	$(RM) *~
