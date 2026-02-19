ID=00

.PHONY: clean run

run: main
	./main

main: main.cpp example*/* ../*
	g++ main.cpp ../GameGraphSolver.cpp -std=c++17 -O2 -Wall -I.. -Iexample$(ID) -o main

clean:
	rm ./main
