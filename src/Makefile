# Compiler and flags
CXX      := g++
CXXFLAGS := -Wall -Wextra -O3 -march=native -flto -funroll-loops -std=c++17 -I.

# Source files and targets
SOURCES  := $(wildcard *.cpp)
OBJECTS  := $(SOURCES:.cpp=.o)
TARGET   := my_chess_engine.exe  # Updated to match Arena's expected executable

# Default rule: build
all: $(TARGET)

# Link all object files into the final executable
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ -flto

# Compile .cpp -> .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean
