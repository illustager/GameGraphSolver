#pragma once

#include "GameGraphPositionBase.h"

class Position : public GameGraphPositionBase {
public:
	Position() = delete;
	Position(int curr) : curr(curr) {}

	~Position() = default;

	std::vector<unique_ptr> get_next_positions() const override {		
		switch (curr) {
			case 0: return make_position_vector(1, 8);
			case 1: return make_position_vector(1, 2);
			case 2: return make_position_vector(3);
			case 3: return make_position_vector(4, 6);
			case 4: return make_position_vector(5);
			case 5: return make_position_vector(4);
			case 6: return make_position_vector(7);
			case 7: return {};
			case 8: return make_position_vector(3, 8);
			
			default: return {};
		}
	}

	bool is_terminal() const override {
		return curr == 7;
	}

	bool less(const GameGraphPositionBase* rhs) const override {
		const Position* pos = dynamic_cast<const Position*>(rhs);
		return curr < pos->curr;
	}

	static std::vector<unique_ptr> get_initial_positions() {
		std::vector<unique_ptr> results;
		results.emplace_back(std::make_unique<Position>(0));
		return results;
	}

	friend std::ostream& operator<<(std::ostream& os, const Position& pos) {
		os << pos.curr;
		return os;
	}

private:
	template<typename... Args>
	static std::vector<std::unique_ptr<GameGraphPositionBase>> make_position_vector(Args&&... args) {
		std::vector<std::unique_ptr<GameGraphPositionBase>> v;
		v.reserve(sizeof...(Args));
		(v.push_back(std::make_unique<Position>(std::forward<Args>(args))), ...);
		return v;
	} 

private:
	unsigned	curr;
};
