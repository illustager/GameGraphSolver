#include "GameGraphSolver.h"

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
		return types_.at(idx.value());
	}
	else {
		return PositionType::INVALID;
	}
}

vector<const PositionBase*> GameGraphSolver::get_positions() const {
	vector<const PositionBase*> results;
	for (size_t u = 0; u < num_nodes_; ++u) {
		results.emplace_back(nodes_.at(u).get());
	}
	return results;
}

vector<const PositionBase*> GameGraphSolver::get_terminals() const {
	vector<const PositionBase*> results;
	for (size_t u : terminals_) {
		results.emplace_back(nodes_.at(u).get());
	}
	return results;
}

vector<const PositionBase*> GameGraphSolver::get_adjacent_positions(const PositionBase* pos) const {
	auto idx = index_of(pos);
	if (!idx.has_value()) {
		return {};
	}
	
	vector<const PositionBase*> results;
	const auto& neighbors = adj_.at(idx.value());
	for (size_t u : neighbors) {
		results.emplace_back(nodes_.at(u).get());
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

	return is_draw_.at(idx.value());
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

		auto next_positions = nodes_.at(u)->get_next_positions();

		for (auto& next : next_positions) {
			bool is_new_node;
			size_t v = add_node(move(next), is_new_node);
			add_edge(u, v);
			
			if (is_new_node) {
				if (nodes_.at(v)->is_terminal()) {
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
	adj_.at(u).emplace_back(v);

	if (rev_adj_.size() <= v) {
		rev_adj_.resize(v + 1);
	}
	rev_adj_.at(v).emplace_back(u);

	if (has_self_loop_.size() <= u) {
		has_self_loop_.resize(u + 1, false);
	}
	if (u == v) {
		has_self_loop_.at(u) = true;
	}
}

void GameGraphSolver::color_node(size_t node, PT type) {
	types_.at(node) = type;
}

bool GameGraphSolver::has_self_loop(size_t node) const {
	return has_self_loop_.at(node);
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

		for (size_t parent : rev_adj_.at(node)) {
			if (types_.at(parent) != PT::UNDETERMINED) {
				continue;
			}

			if (types_.at(node) == PT::P_POSITION) {
				color_node(parent, PT::N_POSITION);
				q.emplace(parent);
			}
			else { // node is N-POSITION
				num_n_children.at(parent) += 1;

				if (num_n_children.at(parent) == adj_.at(parent).size()) {
					color_node(parent, PT::P_POSITION);
					q.emplace(parent);
				}
			}
		}
	}
}

void GameGraphSolver::color_node_in_trivial_sink_scc(size_t node, queue<size_t>& q) {
	if (adj_.at(node).empty()) {
		;
	}
	else if (num_t_children.at(node) == adj_.at(node).size()) {
		color_node(node, PT::T_POSITION);
		q.emplace(node);
	}
	else if (num_t_children.at(node)
	       + num_nt_children.at(node)
		   + num_n_children.at(node) == adj_.at(node).size()) {
		color_node(node, PT::PT_POSITION);
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
		if (scc_adj.at(scc_id).empty()) {
			const auto& nodes_in_scc = scc_nodes.at(scc_id);
			
			if (nodes_in_scc.size() == 1) {
				if (!has_self_loop(nodes_in_scc[0])) {
					color_node_in_trivial_sink_scc(nodes_in_scc[0], q);
					continue;
				}
			}

			for (size_t node : nodes_in_scc) {
				color_node(node, PT::T_POSITION);
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
			is_draw_.at(node) = true;
		}

		for (size_t parent : rev_adj_.at(node)) {
			if (types_.at(parent) != PT::UNDETERMINED) {
				continue;
			}

			if (types_.at(node) == PT::PT_POSITION) {
				color_node(parent, PT::NT_POSITION);
				q.emplace(parent);
				changed = true;
			}
			else {	// node is T-POSITION or NT-POSITION
				if (types_.at(node) == PT::T_POSITION) {
					num_t_children.at(parent) += 1;
				}
				else {
					num_nt_children.at(parent) += 1;
				}

				if (num_t_children.at(parent) == adj_.at(parent).size()) {
					color_node(parent, PT::T_POSITION);
					q.emplace(parent);
					changed = true;
				}
				else if (num_t_children.at(parent)
				       + num_nt_children.at(parent)
					   + num_n_children.at(parent) == adj_.at(parent).size()) {
					color_node(parent, PT::PT_POSITION);
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
	index_map.at(node) = index;
	lowlink_map.at(node) = index;
	++index;
	S.push(node);
	on_stack.at(node) = true;

	for (size_t neighbor : adj_.at(node)) {
		if (types_.at(neighbor) != PT::UNDETERMINED) {
			continue;
		}

		if (index_map.at(neighbor) < 0) {
			tarjan_helper(neighbor, index_map, lowlink_map, S, on_stack, index, scc_count, scc_map, scc_nodes);
			lowlink_map.at(node) = min(lowlink_map.at(node), lowlink_map.at(neighbor));
		}
		else if (on_stack.at(neighbor)) {
			lowlink_map.at(node) = min(lowlink_map.at(node), index_map.at(neighbor));
		}
	}

	if (lowlink_map.at(node) == index_map.at(node)) {
		scc_nodes.emplace_back(vector<size_t>{});

		size_t w = S.top();
		do {
			w = S.top();
			S.pop();
			on_stack.at(w) = false;

			scc_map.at(w) = scc_count;
			scc_nodes.at(scc_count).emplace_back(w);
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
		if (types_.at(node) != PT::UNDETERMINED) {
			continue;
		}

		if (index_map.at(node) < 0) {
			tarjan_helper(node, index_map, lowlink_map, S, on_stack, index, scc_count, scc_map, scc_nodes);
		}
	}

	scc_adj.resize(scc_count);

	for (size_t node = 0; node < num_nodes_; ++node) {
		if (types_.at(node) != PT::UNDETERMINED) {
			continue;
		}

		size_t node_scc = scc_map.at(node);
		for (size_t neighbor : adj_.at(node)) {
			if (types_.at(neighbor) != PT::UNDETERMINED) {
				continue;
			}

			size_t neighbor_scc = scc_map.at(neighbor);
			if (node_scc != neighbor_scc) {
				scc_adj.at(node_scc).emplace_back(neighbor_scc);
			}
		}
	}
}
