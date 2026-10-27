# BlockCraft by Md Rejaul Korim Sadi (231-115-349)
CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra
LIB_SRC   = lib/world.cpp lib/textures.cpp lib/player.cpp lib/game.cpp lib/sky.cpp lib/effects.cpp lib/hud.cpp
SRC       = src/main.cpp $(LIB_SRC)
TEST_SRC  = tests/world_test.cpp lib/world.cpp lib/textures.cpp

ifeq ($(OS),Windows_NT)
  EXE    = .exe
  LIBS   = -lfreeglut -lopengl32 -lglu32 -lwinmm -lgdi32
else
  EXE    =
  LIBS   = -lglut -lGLU -lGL
endif

TARGET = bin/blockcraft$(EXE)
TESTS  = bin/world_test$(EXE)

$(TARGET): $(SRC) lib/*.h
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) -o $@ $(SRC) $(LIBS)

$(TESTS): $(TEST_SRC) lib/*.h
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) -o $@ $(TEST_SRC) $(LIBS)

run: $(TARGET)
	cd bin && ./blockcraft$(EXE)

test: $(TESTS)
	cd bin && ./world_test$(EXE)

clean:
	rm -rf bin obj build

.PHONY: run test clean
