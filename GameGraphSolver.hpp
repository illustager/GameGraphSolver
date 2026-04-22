#pragma once

#include <map>
#include <memory>
#include <vector>
#include <stack>
#include <optional>
#include <iostream>
#include <queue>
#include <type_traits>
#include <concepts>
#include <unordered_map>

template<typename T>
concept LessComparable = requires(const T& a, const T& b) {
	{ a < b } -> std::convertible_to<bool>;
};

template<typename T>
concept Hashable = requires(const T& a, const T& b) {
	{ a == b } -> std::convertible_to<bool>;
	{ a.hash() } -> std::convertible_to<std::size_t>;
};

template<typename T>
concept PositionConcept = requires(const T& pos) {
	{ T::get_starting_positions() } -> std::convertible_to<std::vector<std::unique_ptr<T>>>;
	{ pos.get_next_positions() } -> std::convertible_to<std::vector<std::unique_ptr<T>>>;
	{ pos.is_terminal() } -> std::convertible_to<bool>;
};

template<typename Position>
	requires PositionConcept<Position> && (LessComparable<Position> || Hashable<Position>)
class GameGraphSolver final {
	struct Less {
		bool operator()(const Position* lhs, const Position* rhs) const {
			return *lhs < *rhs;
		}
	};

	struct Hash {
		std::size_t operator()(const Position* pos) const {
			return pos->hash();
		}
	};

	struct Equal {
		bool operator()(const Position* lhs, const Position* rhs) const {
			return *lhs == *rhs;
		}
	};

	using PositionUniquePtr = std::unique_ptr<Position>;
	template<typename T>
	using PositionMap = std::conditional_t<Hashable<Position>,
										   std::unordered_map<const Position*, T, Hash, Equal>,
										   std::map<const Position*, T, Less>>;
public:
	enum class PositionType {
		P_POSITION,
		PT_POSITION,
		T_POSITION,
		NT_POSITION,
		N_POSITION,
		UNDETERMINED,
		INVALID
	};
	using PT = PositionType;

	friend std::ostream& operator<<(std::ostream& os, PT pt) {
		switch (pt) {
			case PT::P_POSITION:	os << "P-POSITION"; break;
			case PT::PT_POSITION:	os << "PT-POSITION"; break;
			case PT::T_POSITION:	os << "T-POSITION"; break;
			case PT::NT_POSITION:	os << "NT-POSITION"; break;
			case PT::N_POSITION:	os << "N-POSITION"; break;
			case PT::UNDETERMINED:	os << "UNDETERMINED"; break;
			case PT::INVALID:		os << "INVALID"; break;

			default: break;
		}
		return os;
	}

public:
	GameGraphSolver() = default;
	~GameGraphSolver() = default;

	void build_graph();
	void color_graph();

	PT get_position_type(const Position& pos) const;
	PT get_position_type(const Position* pos) const;

	std::vector<const Position*> get_positions() const;
	std::vector<const Position*> get_terminals() const;
	std::vector<const Position*> get_adjacent_positions(const Position& pos) const;
	std::vector<const Position*> get_adjacent_positions(const Position* pos) const;

	bool is_draw(const Position& position) const;
	bool is_draw(const Position* position) const;
	
private:
	std::optional<std::size_t> index_of(const Position* pos) const;

	void color_terminals(std::queue<std::size_t>& q);
	void color_by_basic_rules();
	void color_node_in_trivial_sink_scc(std::size_t node, std::queue<std::size_t>& q);
	void color_sink_sccs(std::queue<std::size_t>& q);
	bool color_by_extended_rules(bool t_positions_are_draw);

	std::size_t add_node(PositionUniquePtr node, bool& is_new_node);
	void add_edge(std::size_t u, std::size_t v);
	void color_node(std::size_t node, PT type);

	bool has_self_loop(std::size_t node) const;

	void tarjan_helper(std::size_t node,
	                   std::vector<int>& index_map,
	                   std::vector<int>& lowlink_map,
	                   std::stack<std::size_t>& S,
	                   std::vector<bool>& on_stack,
	                   int& index,
					   std::size_t& scc_count,
					   std::vector<std::size_t>& scc_map,
					   std::vector<std::vector<std::size_t>>& scc_nodes) const;

	void tarjan_undetermined_nodes(std::size_t& scc_count,
								   std::vector<std::size_t>& scc_map,
								   std::vector<std::vector<std::size_t>>& scc_nodes,
								   std::vector<std::vector<std::size_t>>& scc_adj) const;

private:
	std::size_t									num_nodes_;
	std::vector<PositionUniquePtr>				nodes_;
	PositionMap<std::size_t>					node_indices_;
	std::vector<std::size_t>					terminals_;
	std::vector<std::vector<std::size_t>>		adj_;
	std::vector<std::vector<std::size_t>>		rev_adj_;
	std::vector<std::size_t>					num_n_children_;
	std::vector<std::size_t>					num_t_children_;
	std::vector<std::size_t>					num_nt_children_;
	std::vector<bool>							has_p_child_;
	std::vector<bool>							has_pt_child_;
	std::vector<PositionType>					types_;
	std::vector<bool>							is_draw_;
	std::vector<bool>							has_self_loop_;
};

#ifdef NDEBUG
#define AT(v, i) (v)[i]
#else
#define AT(v, i) (v).at(i)
#endif

template<typename Position>
GameGraphSolver<Position>::PT GameGraphSolver<Position>::get_position_type(const Position& pos) const {
	return get_position_type(&pos);
}

template<typename Position>
GameGraphSolver<Position>::PT GameGraphSolver<Position>::get_position_type(const Position* pos) const {
	auto idx = index_of(pos);
	if (idx.has_value()) {
		return AT(types_, idx.value());
	}
	else {
		return PositionType::INVALID;
	}
}

template<typename Position>
std::vector<const Position*> GameGraphSolver<Position>::get_positions() const {
	std::vector<const Position*> results;
	for (std::size_t u = 0; u < num_nodes_; ++u) {
		results.emplace_back(AT(nodes_, u).get());
	}
	return results;
}

template<typename Position>
std::vector<const Position*> GameGraphSolver<Position>::get_terminals() const {
	std::vector<const Position*> results;
	for (std::size_t u : terminals_) {
		results.emplace_back(AT(nodes_, u).get());
	}
	return results;
}

template<typename Position>
std::vector<const Position*> GameGraphSolver<Position>::get_adjacent_positions(const Position* pos) const {
	auto idx = index_of(pos);
	if (!idx.has_value()) {
		return {};
	}
	
	std::vector<const Position*> results;
	const auto& neighbors = AT(adj_, idx.value());
	for (std::size_t u : neighbors) {
		results.emplace_back(AT(nodes_, u).get());
	}
	return results;
}

template<typename Position>
std::vector<const Position*> GameGraphSolver<Position>::get_adjacent_positions(const Position& pos) const {
	return get_adjacent_positions(&pos);
}

template<typename Position>
bool GameGraphSolver<Position>::is_draw(const Position* pos) const {
	auto idx = index_of(pos);
	if (!idx.has_value()) {
		return false;
	}

	return AT(is_draw_, idx.value());
}

template<typename Position>
bool GameGraphSolver<Position>::is_draw(const Position& pos) const {
	return is_draw(&pos);
}

template<typename Position>
std::optional<std::size_t> GameGraphSolver<Position>::index_of(const Position* pos) const {
	auto it = node_indices_.find(pos);
	if (it == node_indices_.end()) {
		return std::nullopt;
	}
	return it->second;
}

template<typename Position>
void GameGraphSolver<Position>::build_graph() {
	std::queue<std::size_t> q;
	std::vector<PositionUniquePtr> starting_positions = Position::get_starting_positions();

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
	num_n_children_.resize(num_nodes_, 0);
	num_t_children_.resize(num_nodes_, 0);
	num_nt_children_.resize(num_nodes_, 0);
	has_p_child_.resize(num_nodes_, false);
	has_pt_child_.resize(num_nodes_, false);
	types_.resize(num_nodes_, PT::UNDETERMINED);
	is_draw_.resize(num_nodes_, false);
	has_self_loop_.resize(num_nodes_, false);
}

template<typename Position>
void GameGraphSolver<Position>::color_graph() {
	color_by_extended_rules(true);
	color_by_basic_rules();
	while (color_by_extended_rules(false)) {
		;
	}
}

template<typename Position>
size_t GameGraphSolver<Position>::add_node(PositionUniquePtr node, bool& is_new_node) {
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

template<typename Position>
void GameGraphSolver<Position>::add_edge(std::size_t u, std::size_t v) {
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

template<typename Position>
void GameGraphSolver<Position>::color_node(std::size_t node, PT type) {
	AT(types_, node) = type;
}

template<typename Position>
bool GameGraphSolver<Position>::has_self_loop(std::size_t node) const {
	return AT(has_self_loop_, node);
}

template<typename Position>
void GameGraphSolver<Position>::color_terminals(std::queue<std::size_t>& q) {
	for (std::size_t t : terminals_) {
		color_node(t, PT::P_POSITION);
		q.emplace(t);
	}
}

template<typename Position>
void GameGraphSolver<Position>::color_by_basic_rules() {
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
				AT(num_n_children_, parent) += 1;

				if (AT(num_n_children_, parent) == AT(adj_, parent).size()) {
					AT(types_, parent) = PT::P_POSITION;
					q.emplace(parent);
				}
			}
		}
	}
}

template<typename Position>
void GameGraphSolver<Position>::color_node_in_trivial_sink_scc(std::size_t node, std::queue<std::size_t>& q) {
	if (AT(adj_, node).empty()) {
		;
	}
	else if (AT(num_t_children_, node) == AT(adj_, node).size()) {
		AT(types_, node) = PT::T_POSITION;
		q.emplace(node);
	}
	else if (AT(num_t_children_, node)
	       + AT(num_nt_children_, node)
		   + AT(num_n_children_, node) == AT(adj_, node).size()) {
		AT(types_, node) = PT::PT_POSITION;
		q.emplace(node);
	}
}

template<typename Position>
void GameGraphSolver<Position>::color_sink_sccs(std::queue<std::size_t>& q) {
	std::size_t								scc_count = 0;
	std::vector<std::size_t>				scc_map(num_nodes_);
	std::vector<std::vector<std::size_t>>	scc_nodes;
	std::vector<std::vector<std::size_t>>	scc_adj;

	tarjan_undetermined_nodes(scc_count, scc_map, scc_nodes, scc_adj);

	for (std::size_t scc_id = 0; scc_id < scc_count; ++scc_id) {
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

template<typename Position>
bool GameGraphSolver<Position>::color_by_extended_rules(bool t_positions_are_draw) {
	bool changed = false;
	
	std::queue<std::size_t> q;

	color_sink_sccs(q);

	while (!q.empty()) {
		std::size_t node = q.front();
		q.pop();

		if (t_positions_are_draw) {
			AT(is_draw_, node) = true;
		}

		for (std::size_t parent : AT(rev_adj_, node)) {
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
					AT(num_t_children_, parent) += 1;
				}
				else {
					AT(num_nt_children_, parent) += 1;
				}

				if (AT(num_t_children_, parent) == AT(adj_, parent).size()) {
					AT(types_, parent) = PT::T_POSITION;
					q.emplace(parent);
					changed = true;
				}
				else if (AT(num_t_children_, parent)
				       + AT(num_nt_children_, parent)
					   + AT(num_n_children_, parent) == AT(adj_, parent).size()) {
					AT(types_, parent) = PT::PT_POSITION;
					q.emplace(parent);
					changed = true;
				}
			}
		}
	}

	return changed;
}

template<typename Position>
void GameGraphSolver<Position>::tarjan_helper(std::size_t node,
												  std::vector<int>& index_map,
												  std::vector<int>& lowlink_map,
												  std::stack<std::size_t>& S,
												  std::vector<bool>& on_stack,
												  int& index,
												  std::size_t& scc_count,
												  std::vector<std::size_t>& scc_map,
												  std::vector<std::vector<std::size_t>>& scc_nodes) const {
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
			AT(lowlink_map, node) = std::min(AT(lowlink_map, node), AT(lowlink_map, neighbor));
		}
		else if (AT(on_stack, neighbor)) {
			AT(lowlink_map, node) = std::min(AT(lowlink_map, node), AT(index_map, neighbor));
		}
	}

	if (AT(lowlink_map, node) == AT(index_map, node)) {
		scc_nodes.emplace_back(std::vector<std::size_t>{});

		std::size_t w = S.top();
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

template<typename Position>
void GameGraphSolver<Position>::tarjan_undetermined_nodes(std::size_t& scc_count,
														  std::vector<std::size_t>& scc_map,
														  std::vector<std::vector<std::size_t>>& scc_nodes,
														  std::vector<std::vector<std::size_t>>& scc_adj) const {
	std::vector<int> 	index_map(num_nodes_, -1);
	std::vector<int> 	lowlink_map(num_nodes_, -1);
	std::stack<std::size_t>	S;
	std::vector<bool>	on_stack(num_nodes_, false);
	int				index = 0;

	scc_count = 0;
	for (std::size_t node = 0; node < num_nodes_; ++node) {
		if (AT(types_, node) != PT::UNDETERMINED) {
			continue;
		}

		if (AT(index_map, node) < 0) {
			tarjan_helper(node, index_map, lowlink_map, S, on_stack, index, scc_count, scc_map, scc_nodes);
		}
	}

	scc_adj.resize(scc_count);

	for (std::size_t node = 0; node < num_nodes_; ++node) {
		if (AT(types_, node) != PT::UNDETERMINED) {
			continue;
		}

		std::size_t node_scc = AT(scc_map, node);
		for (std::size_t neighbor : AT(adj_, node)) {
			if (AT(types_, neighbor) != PT::UNDETERMINED) {
				continue;
			}

			std::size_t neighbor_scc = AT(scc_map, neighbor);
			if (node_scc != neighbor_scc) {
				AT(scc_adj, node_scc).emplace_back(neighbor_scc);
			}
		}
	}
}

#undef AT

