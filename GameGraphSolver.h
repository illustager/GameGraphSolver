#pragma once

#include "GameGraphPositionBase.h"

#include <map>
#include <vector>
#include <stack>
#include <optional>
#include <iostream>

class GameGraphSolver {
	using PositionBase			= GameGraphPositionBase;
	using PositionBaseUniquePtr	= std::unique_ptr<PositionBase>;
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

	void build_graph(std::vector<PositionBaseUniquePtr> starting_positions);
	void color_graph();

	PT get_position_type(const PositionBase& pos) const;
	PT get_position_type(const PositionBase* pos) const;

	std::vector<const PositionBase*> get_positions() const;
	std::vector<const PositionBase*> get_terminals() const;
	std::vector<const PositionBase*> get_adjacent_positions(const PositionBase& pos) const;
	std::vector<const PositionBase*> get_adjacent_positions(const PositionBase* pos) const;

	bool is_draw(const PositionBase& position) const;
	bool is_draw(const PositionBase* position) const;
	
private:
	std::optional<std::size_t> index_of(const PositionBase* pos) const;

	bool color_well_defined_nodes();
	bool color_sink_sccs();
	bool color_undetermined_nodes();

	std::size_t add_node(PositionBaseUniquePtr node, bool& is_new_node);
	void add_edge(std::size_t u, std::size_t v);
	void color_node(std::size_t u, PT type);

	bool has_self_loop(std::size_t u) const;

	void tarjan_helper(std::size_t node,
	                   std::vector<int>& index_map,
	                   std::vector<int>& lowlink_map,
	                   std::stack<std::size_t>& S,
	                   std::vector<bool>& on_stack,
	                   int& index,
					   size_t& scc_count,
					   std::vector<std::size_t>& scc_map,
					   std::vector<std::vector<std::size_t>>& scc_nodes) const;

	void tarjan_undetermined_nodes(size_t& scc_count,
								   std::vector<std::size_t>& scc_map,
								   std::vector<std::vector<std::size_t>>& scc_nodes,
								   std::vector<std::vector<std::size_t>>& scc_adj) const;

private:
	std::size_t														num_nodes_;
	std::vector<PositionBaseUniquePtr>								nodes_;
	std::map<const PositionBase*, std::size_t, PositionBase::Less>	node_indices_;
	std::vector<size_t>												terminals_;
	std::vector<std::vector<std::size_t>>							adj_;
	std::vector<PositionType>										types_;
	std::vector<bool>												is_draw_;
	std::vector<bool>												has_self_loop_;
};
