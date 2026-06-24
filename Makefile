# Use the system compiler
CXX = g++

# Get flags from fltk-config
# We filter out -march and -mtune to avoid CachyOS specific CPU errors
CXXFLAGS_RAW = $(shell fltk-config --cxxflags)
CXXFLAGS = $(filter-out -march=% -mtune=%,$(CXXFLAGS_RAW)) -march=x86-64

# Get linker flags
LDFLAGS = $(shell fltk-config --ldflags)

# Target executable name
TARGET = hello

# Source files
SRC = hello.cpp

# Default rule
all: $(TARGET)

# Link rule: Ensure source is compiled and linked with flags AFTER the object
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

# Clean rule
clean:
	rm -f $(TARGET)

# Phony targets
.PHONY: all clean   