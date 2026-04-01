#include <iostream>

using namespace std;

#include "GameGraphSolver.hpp"
#include "Position.h"

int main() {
	GameGraphSolver<Position> graph;

	graph.build_graph(Position::get_starting_positions());
	graph.color_graph();

	auto positions = graph.get_positions();
	for (auto pos : positions) {
		const Position* dpos = dynamic_cast<const Position*>(pos);
		cout << *dpos << " ==> " << graph.get_position_type(dpos)
		     << '\t' << graph.is_draw(dpos) << endl;
	}

	return 0;
}
