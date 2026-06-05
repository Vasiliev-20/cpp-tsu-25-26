CXX = g++
CXXFLAGS = -std=c++17 -Wall

TARGET = terminal
SRCS = main.cpp Terminal.cpp Directory.cpp Entry.cpp FileClasses.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
