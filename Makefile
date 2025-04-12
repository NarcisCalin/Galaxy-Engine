# Compiler
CXX = clang++
CXXFLAGS = -std=c++17 -Iinclude

# Src files
SRC = $(wildcard src/*.cpp src/UI/*.cpp src/Particles/*.cpp)

# Output
OUT = GalaxyEngine.exe

# Build rule
$(OUT): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT)

# Clean rule
clean:
	del $(OUT)
