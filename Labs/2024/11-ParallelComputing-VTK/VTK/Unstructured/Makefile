CXX = g++

CXXFLAGS = -std=c++17 -Wall -Wextra -O2

TARGET = main

SRCS = $(wildcard *.cpp)

OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)

# Phony targets
.PHONY: all clean

