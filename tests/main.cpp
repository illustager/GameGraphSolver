#include <iostream>

using namespace std;

#include "GameGraphSolver.hpp"
#include "Position.h"

int main() {
	GameGraphSolver<Position> graph;

	graph.build_graph();
	graph.color_graph();

	auto positions = graph.get_positions();
	for (auto pos : positions) {
		cout << *pos << " ==> " << graph.get_position_type(pos)
		     << '\t' << graph.is_draw(pos) << endl;
	}

	return 0;
}
