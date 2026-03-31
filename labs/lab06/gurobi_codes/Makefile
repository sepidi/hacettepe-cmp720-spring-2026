# Makefile for Application Mapping Optimizer
# ============================================
#
# This Makefile compiles the C++ implementation of the application mapping problem.
#
# Usage:
#   make              # Compile the program
#   make clean        # Remove compiled files
#   make run          # Compile and run with default data file

# Compiler settings
CXX = g++
CXXFLAGS = -std=c++11 -Wall -O2

# Gurobi settings
# Set GUROBI_HOME to your Gurobi installation directory
# Example: GUROBI_HOME = /opt/gurobi1301/linux64
ifndef GUROBI_HOME
    $(error GUROBI_HOME is not set. Please set it to your Gurobi installation directory)
endif

GUROBI_INC = $(GUROBI_HOME)/include
GUROBI_LIB = $(GUROBI_HOME)/lib

# Gurobi version (adjust if using a different version)
GUROBI_VERSION = 130

# Include and library flags
INCLUDES = -I$(GUROBI_INC)
LDFLAGS = -L$(GUROBI_LIB)
LIBS = -lgurobi_c++ -lgurobi$(GUROBI_VERSION) -lm -lpthread

# Target executable
TARGET = application_mapping

# Source files
SOURCES = application_mapping_cpp.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Default data file
DATA_FILE = application_mapping_data.txt

# Build target
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) $(LDFLAGS) $(LIBS) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Run the program
run: $(TARGET)
	./$(TARGET) $(DATA_FILE)

# Clean build files
clean:
	rm -f $(OBJECTS) $(TARGET)

# Help
help:
	@echo "Available targets:"
	@echo "  make          - Compile the program"
	@echo "  make run      - Compile and run with default data file"
	@echo "  make clean    - Remove compiled files"
	@echo "  make help     - Show this help message"
	@echo ""
	@echo "Environment variables:"
	@echo "  GUROBI_HOME   - Path to Gurobi installation (required)"
	@echo ""
	@echo "Example:"
	@echo "  export GUROBI_HOME=/opt/gurobi1301/linux64"
	@echo "  make"
	@echo "  ./application_mapping application_mapping_data.txt"

.PHONY: all run clean help
