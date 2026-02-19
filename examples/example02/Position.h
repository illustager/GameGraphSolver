#pragma once

#include "GameGraphPositionBase.h"
#include "unordered_pair.h"

#include <array>
#include <iostream>
#include <utility>

class Position : public GameGraphPositionBase {
public:
	using PlayerState = unordered_pair<int>;

	Position() = delete;

	Position(const PlayerState &p, const PlayerState &o)
		: player(p), opponent(o) {}

	Position(PlayerState&& p, PlayerState&& o)
		: player(std::forward<PlayerState>(p)), opponent(std::forward<PlayerState>(o)) {}

	std::vector<unique_ptr> get_next_positions() const override {
		std::vector<unique_ptr> results;
		for (int move_id = 0; move_id < 4; ++move_id) {
			bool is_valid;
			unique_ptr r = make_move(move_id, is_valid);
			if (is_valid) {
				results.emplace_back(std::move(r));
			}
		}
		return results;
	}

	bool is_terminal() const override {
		return opponent.to_pair() == std::pair<int, int>{0, 0};
	}

	bool less(const GameGraphPositionBase* rhs) const override {
		const Position* pos = dynamic_cast<const Position*>(rhs);
		if (player.to_pair() != pos->player.to_pair()) {
			return player.to_pair() < pos->player.to_pair();
		}
		else {
			return opponent.to_pair() < pos->opponent.to_pair();
		}
	}

	static std::vector<unique_ptr> get_initial_positions() {
		std::vector<unique_ptr> results;
		results.emplace_back(new Position{{1, 1}, {1, 1}});
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
	unique_ptr make_move(int move_id, bool& is_valid) const {
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
