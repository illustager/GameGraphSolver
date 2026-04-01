#pragma once

#include <iostream>

class Position {
public:
	Position() = delete;
	Position(int curr) : curr(curr) {}

	~Position() = default;

	std::vector<std::unique_ptr<Position>> get_next_positions() const {
		switch (curr) {
			case 0: return make_position_vector(1, 2, 3);
			case 1: return make_position_vector(4, 5, 6);
			case 2: return make_position_vector(2, 6);
			case 3: return make_position_vector(3, 7);
			case 4: return make_position_vector(4);
			case 5: return make_position_vector(8);
			case 6: return make_position_vector(9, 10);
			case 7: return make_position_vector(6);
			case 8: return make_position_vector(9);
			case 9: return make_position_vector(11);
			case 10: return make_position_vector(9, 12);
			case 11: return {};
			case 12: return make_position_vector(13);
			case 13: return make_position_vector(14);
			case 14: return make_position_vector(13);
						
			default: return {};
		}
	}

	bool is_terminal() const {
		return curr == 11;
	}

	bool operator==(const Position& other) const {
		return curr == other.curr;
	}

	std::size_t hash() const {
		return std::hash<unsigned>{}(curr);
	}

	static std::vector<std::unique_ptr<Position>> get_starting_positions() {
		std::vector<std::unique_ptr<Position>> results;
		results.emplace_back(std::make_unique<Position>(0));
		return results;
	}

	friend std::ostream& operator<<(std::ostream& os, const Position& pos) {
		os << pos.curr;
		return os;
	}

private:
	template<typename... Args>
	static std::vector<std::unique_ptr<Position>> make_position_vector(Args&&... args) {
		std::vector<std::unique_ptr<Position>> v;
		v.reserve(sizeof...(Args));
		(v.push_back(std::make_unique<Position>(std::forward<Args>(args))), ...);
		return v;
	}

private:
	unsigned	curr;
};
