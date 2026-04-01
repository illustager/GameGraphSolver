#pragma once

#include "unordered_pair.h"

#include <array>
#include <iostream>
#include <utility>

class Position {
public:
	using PlayerState = unordered_pair<int>;

	Position() = delete;

	Position(const PlayerState &p, const PlayerState &o)
		: player(p), opponent(o) {}

	Position(PlayerState&& p, PlayerState&& o)
		: player(std::forward<PlayerState>(p)), opponent(std::forward<PlayerState>(o)) {}

	std::vector<std::unique_ptr<Position>> get_next_positions() const {
		std::vector<std::unique_ptr<Position>> results;
		for (int move_id = 0; move_id < 4; ++move_id) {
			bool is_valid;
			std::unique_ptr<Position> r = make_move(move_id, is_valid);
			if (is_valid) {
				results.emplace_back(std::move(r));
			}
		}
		return results;
	}

	bool is_terminal() const {
		return opponent.to_pair() == std::pair<int, int>{0, 0};
	}

	bool operator<(const Position& other) const {
		if (player.to_pair() != other.player.to_pair()) {
			return player.to_pair() < other.player.to_pair();
		}
		else {
			return opponent.to_pair() < other.opponent.to_pair();
		}
	}

	static std::vector<std::unique_ptr<Position>> get_starting_positions() {
		std::vector<std::unique_ptr<Position>> results;
		results.emplace_back(std::make_unique<Position>(PlayerState{1, 1}, PlayerState{1, 1}));
		return results;
	}

	const PlayerState& get_player_state() const {
		return player;
	}

	const PlayerState& get_opponent_state() const {
		return opponent;
	}

	friend std::ostream& operator<<(std::ostream &os, const Position &status) {
		os << "Player: (" << status.player[0] << ", " << status.player[1] << "), "
		   << "Opponent: (" << status.opponent[0] << ", " << status.opponent[1] << ")";
		return os;
	}

private:
	std::unique_ptr<Position> make_move(int move_id, bool& is_valid) const {
		is_valid = true;
		PlayerState new_player = player;
		PlayerState new_opponent = opponent;

		int& p = new_player[move_id / 2];
		int  o = new_opponent[move_id % 2];

		if (p == 0 || o == 0) {
			is_valid = false;
		}
		else {
			is_valid = true;
			p = (p + o) % 10;
		}

		return std::make_unique<Position>(new_opponent, new_player);
	}

private:
	PlayerState player;
	PlayerState opponent;
};
