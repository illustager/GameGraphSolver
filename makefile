CXX = g++
CXXFLAGS = -Wall -Wextra -O2 -std=c++17 -fPIC -DNDEBUG

.PHONY: all clean

all: libGameGraphSolver.a libGameGraphSolver.so

clean:
	rm -f *.a *.so *.o

libGameGraphSolver.a: GameGraphSolver.o
	ar rcs $@ $^

libGameGraphSolver.so: GameGraphSolver.o
	$(CXX) $(CXXFLAGS) -shared -o $@ $^

GameGraphSolver.o: GameGraphSolver.cpp *.h
	$(CXX) $(CXXFLAGS) -c $< -o $@
