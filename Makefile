# Compiler and flags
CXX = clang++
CXXFLAGS = -std=c++17 -Iinclude

# Directory for source and object files
SRC_DIR = src
OBJ_DIR = obj

# Find all .cpp files in the src directory
SRC = $(wildcard $(SRC_DIR)/**/*.cpp)

# Convert .cpp files to .o files in the obj directory
OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Output executable
OUT = GalaxyEngine.exe

# Default target to build the executable
$(OUT): $(OBJ)
	$(CXX) $(OBJ) -o $(OUT)

# Rule to compile .cpp files into .o object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up the build (remove object files and the executable)
clean:
	del /f /q $(OBJ) $(OUT)
