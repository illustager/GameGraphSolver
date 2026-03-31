#include "GameGraphSolver.h"

#ifdef NDEBUG
#define AT(v, i) (v)[i]
#else
#define AT(v, i) (v).at(i)
#endif

using PT					= GameGraphSolver::PositionType;
using PositionBase			= GameGraphPositionBase;
using PositionBaseUniquePtr	= std::unique_ptr<PositionBase>;

using namespace std;

GameGraphSolver::PT GameGraphSolver::get_position_type(const PositionBase& pos) const {
	return get_position_type(&pos);
}

GameGraphSolver::PT GameGraphSolver::get_position_type(const PositionBase* pos) const {
	auto idx = index_of(pos);
	if (idx.has_value()) {
		return AT(types_, idx.value());
	}
	else {
		return PositionType::INVALID;
	}
}

vector<const PositionBase*> GameGraphSolver::get_positions() const {
	vector<const PositionBase*> results;
	for (size_t u = 0; u < num_nodes_; ++u) {
		results.emplace_back(AT(nodes_, u).get());
	}
	return results;
}

vector<const PositionBase*> GameGraphSolver::get_terminals() const {
	vector<const PositionBase*> results;
	for (size_t u : terminals_) {
		results.emplace_back(AT(nodes_, u).get());
	}
	return results;
}

vector<const PositionBase*> GameGraphSolver::get_adjacent_positions(const PositionBase* pos) const {
	auto idx = index_of(pos);
	if (!idx.has_value()) {
		return {};
	}
	
	vector<const PositionBase*> results;
	const auto& neighbors = AT(adj_, idx.value());
	for (size_t u : neighbors) {
		results.emplace_back(AT(nodes_, u).get());
	}
	return results;
}

vector<const PositionBase*> GameGraphSolver::get_adjacent_positions(const PositionBase& pos) const {
	return get_adjacent_positions(&pos);
}

bool GameGraphSolver::is_draw(const PositionBase* pos) const {
	auto idx = index_of(pos);
	if (!idx.has_value()) {
		return false;
	}

	return AT(is_draw_, idx.value());
}

bool GameGraphSolver::is_draw(const PositionBase& pos) const {
	return is_draw(&pos);
}

optional<size_t> GameGraphSolver::index_of(const PositionBase* pos) const {
	auto it = node_indices_.find(pos);
	if (it == node_indices_.end()) {
		return nullopt;
	}
	return it->second;
}

void GameGraphSolver::build_graph(vector<PositionBaseUniquePtr> starting_positions) {
	queue<size_t> q;

	for (auto& pos : starting_positions) {
		bool is_new_node;
		size_t u = add_node(move(pos), is_new_node);
		q.emplace(u);
	}

	while (!q.empty()) {
		size_t u = q.front();
		q.pop();

		auto next_positions = AT(nodes_, u)->get_next_positions();

		for (auto& next : next_positions) {
			bool is_new_node;
			size_t v = add_node(move(next), is_new_node);
			add_edge(u, v);
			
			if (is_new_node) {
				if (AT(nodes_, v)->is_terminal()) {
					terminals_.emplace_back(v);
				}
				else {
					q.emplace(v);
				}
			}
		}
	}

	num_nodes_ = nodes_.size();
	adj_.resize(num_nodes_);
	rev_adj_.resize(num_nodes_);
	num_n_children.resize(num_nodes_, 0);
	num_t_children.resize(num_nodes_, 0);
	num_nt_children.resize(num_nodes_, 0);
	has_p_child.resize(num_nodes_, false);
	has_pt_child.resize(num_nodes_, false);
	types_.resize(num_nodes_, PT::UNDETERMINED);
	is_draw_.resize(num_nodes_, false);
	has_self_loop_.resize(num_nodes_, false);
}

void GameGraphSolver::color_graph() {
	color_by_extended_rules(true);
	color_by_basic_rules();
	while (color_by_extended_rules(false)) {
		;
	}
}

size_t GameGraphSolver::add_node(PositionBaseUniquePtr node, bool& is_new_node) {
	auto it = node_indices_.find(node.get());
	if (it == node_indices_.end()) {
		is_new_node = true;
		nodes_.emplace_back(move(node));
		size_t u = nodes_.size() - 1;
		node_indices_[nodes_.back().get()] = u;
		return u;
	}
	else {
		is_new_node = false;
		return it->second;
	}
}

void GameGraphSolver::add_edge(size_t u, size_t v) {
	if (adj_.size() <= u) {
		adj_.resize(u + 1);
	}
	AT(adj_, u).emplace_back(v);

	if (rev_adj_.size() <= v) {
		rev_adj_.resize(v + 1);
	}
	AT(rev_adj_, v).emplace_back(u);

	if (has_self_loop_.size() <= u) {
		has_self_loop_.resize(u + 1, false);
	}
	if (u == v) {
		AT(has_self_loop_, u) = true;
	}
}

void GameGraphSolver::color_node(size_t node, PT type) {
	AT(types_, node) = type;
}

bool GameGraphSolver::has_self_loop(size_t node) const {
	return AT(has_self_loop_, node);
}

void GameGraphSolver::color_terminals(queue<size_t>& q) {
	for (size_t t : terminals_) {
		color_node(t, PT::P_POSITION);
		q.emplace(t);
	}
}

void GameGraphSolver::color_by_basic_rules() {
	std::queue<std::size_t> q;

	color_terminals(q);

	while (!q.empty()) {
		size_t node = q.front();
		q.pop();

		for (size_t parent : AT(rev_adj_, node)) {
			if (AT(types_, parent) != PT::UNDETERMINED) {
				continue;
			}

			if (AT(types_, node) == PT::P_POSITION) {
				AT(types_, parent) = PT::N_POSITION;
				q.emplace(parent);
			}
			else { // node is N-POSITION
				AT(num_n_children, parent) += 1;

				if (AT(num_n_children, parent) == AT(adj_, parent).size()) {
					AT(types_, parent) = PT::P_POSITION;
					q.emplace(parent);
				}
			}
		}
	}
}

void GameGraphSolver::color_node_in_trivial_sink_scc(size_t node, queue<size_t>& q) {
	if (AT(adj_, node).empty()) {
		;
	}
	else if (AT(num_t_children, node) == AT(adj_, node).size()) {
		AT(types_, node) = PT::T_POSITION;
		q.emplace(node);
	}
	else if (AT(num_t_children, node)
	       + AT(num_nt_children, node)
		   + AT(num_n_children, node) == AT(adj_, node).size()) {
		AT(types_, node) = PT::PT_POSITION;
		q.emplace(node);
	}
}

void GameGraphSolver::color_sink_sccs(queue<size_t>& q) {
	size_t					scc_count = 0;
	vector<size_t>			scc_map(num_nodes_);
	vector<vector<size_t>>	scc_nodes;
	vector<vector<size_t>>	scc_adj;

	tarjan_undetermined_nodes(scc_count, scc_map, scc_nodes, scc_adj);

	for (size_t scc_id = 0; scc_id < scc_count; ++scc_id) {
		if (AT(scc_adj, scc_id).empty()) {
			const auto& nodes_in_scc = AT(scc_nodes, scc_id);
			
			if (nodes_in_scc.size() == 1) {
				if (!has_self_loop(nodes_in_scc[0])) {
					color_node_in_trivial_sink_scc(nodes_in_scc[0], q);
					continue;
				}
			}

			for (size_t node : nodes_in_scc) {
				AT(types_, node) = PT::T_POSITION;
				q.emplace(node);
			}
		}
	}
}

bool GameGraphSolver::color_by_extended_rules(bool t_positions_are_draw) {
	bool changed = false;
	
	queue<size_t> q;

	color_sink_sccs(q);

	while (!q.empty()) {
		size_t node = q.front();
		q.pop();

		if (t_positions_are_draw) {
			AT(is_draw_, node) = true;
		}

		for (size_t parent : AT(rev_adj_, node)) {
			if (AT(types_, parent) != PT::UNDETERMINED) {
				continue;
			}

			if (AT(types_, node) == PT::PT_POSITION) {
				AT(types_, parent) = PT::NT_POSITION;
				q.emplace(parent);
				changed = true;
			}
			else {	// node is T-POSITION or NT-POSITION
				if (AT(types_, node) == PT::T_POSITION) {
					AT(num_t_children, parent) += 1;
				}
				else {
					AT(num_nt_children, parent) += 1;
				}

				if (AT(num_t_children, parent) == AT(adj_, parent).size()) {
					AT(types_, parent) = PT::T_POSITION;
					q.emplace(parent);
					changed = true;
				}
				else if (AT(num_t_children, parent)
				       + AT(num_nt_children, parent)
					   + AT(num_n_children, parent) == AT(adj_, parent).size()) {
					AT(types_, parent) = PT::PT_POSITION;
					q.emplace(parent);
					changed = true;
				}
			}
		}
	}

	return changed;
}

void GameGraphSolver::tarjan_helper(size_t node,
									vector<int>& index_map,
									vector<int>& lowlink_map,
									stack<size_t>& S,
									vector<bool>& on_stack,
									int& index,
									size_t& scc_count,
									vector<size_t>& scc_map,
									vector<vector<size_t>>& scc_nodes) const {
	AT(index_map, node) = index;
	AT(lowlink_map, node) = index;
	++index;
	S.push(node);
	AT(on_stack, node) = true;

	for (size_t neighbor : AT(adj_, node)) {
		if (AT(types_, neighbor) != PT::UNDETERMINED) {
			continue;
		}

		if (AT(index_map, neighbor) < 0) {
			tarjan_helper(neighbor, index_map, lowlink_map, S, on_stack, index, scc_count, scc_map, scc_nodes);
			AT(lowlink_map, node) = min(AT(lowlink_map, node), AT(lowlink_map, neighbor));
		}
		else if (AT(on_stack, neighbor)) {
			AT(lowlink_map, node) = min(AT(lowlink_map, node), AT(index_map, neighbor));
		}
	}

	if (AT(lowlink_map, node) == AT(index_map, node)) {
		scc_nodes.emplace_back(vector<size_t>{});

		size_t w = S.top();
		do {
			w = S.top();
			S.pop();
			AT(on_stack, w) = false;

			AT(scc_map, w) = scc_count;
			AT(scc_nodes, scc_count).emplace_back(w);
		} while (w != node);
		
		++scc_count;
	}
}

void GameGraphSolver::tarjan_undetermined_nodes(size_t& scc_count,
												vector<size_t>& scc_map,
												vector<vector<size_t>>& scc_nodes,
												vector<vector<size_t>>& scc_adj) const {
	vector<int> 	index_map(num_nodes_, -1);
	vector<int> 	lowlink_map(num_nodes_, -1);
	stack<size_t>	S;
	vector<bool>	on_stack(num_nodes_, false);
	int				index = 0;

	scc_count = 0;
	for (size_t node = 0; node < num_nodes_; ++node) {
		if (AT(types_, node) != PT::UNDETERMINED) {
			continue;
		}

		if (AT(index_map, node) < 0) {
			tarjan_helper(node, index_map, lowlink_map, S, on_stack, index, scc_count, scc_map, scc_nodes);
		}
	}

	scc_adj.resize(scc_count);

	for (size_t node = 0; node < num_nodes_; ++node) {
		if (AT(types_, node) != PT::UNDETERMINED) {
			continue;
		}

		size_t node_scc = AT(scc_map, node);
		for (size_t neighbor : AT(adj_, node)) {
			if (AT(types_, neighbor) != PT::UNDETERMINED) {
				continue;
			}

			size_t neighbor_scc = AT(scc_map, neighbor);
			if (node_scc != neighbor_scc) {
				AT(scc_adj, node_scc).emplace_back(neighbor_scc);
			}
		}
	}
}
