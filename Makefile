CXX      := g++
CXXFLAGS := -Wall -Wextra -O3 -march=native -flto -funroll-loops -std=c++17 -I./src

SRCDIR   := src
SOURCES  := $(wildcard $(SRCDIR)/*.cpp)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.cpp=%.o)
TARGET   := chess_engine.exe  

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ -flto

%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean
