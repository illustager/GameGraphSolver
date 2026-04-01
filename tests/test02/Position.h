#pragma once

#include <iostream>

template<unsigned N, unsigned M>
class TPosition {
public:
	TPosition() = delete;
	TPosition(int curr) : curr(curr) {}

	~TPosition() = default;

	std::vector<std::unique_ptr<TPosition<N, M>>> get_next_positions() const {
		std::vector<std::unique_ptr<TPosition<N, M>>> results;
		for (unsigned i = 1; i <= N; ++i) {
			unsigned next = curr + i;
			if (next > M + 1) {
				break;
			}
			results.emplace_back(std::make_unique<TPosition<N, M>>(next));
		}
		return results;
	}

	bool is_terminal() const {
		return curr >= M + 1;
	}

	bool operator<(const TPosition& other) const {
		return curr < other.curr;
	}

	static std::vector<std::unique_ptr<TPosition<N, M>>> get_starting_positions() {
		std::vector<std::unique_ptr<TPosition<N, M>>> results;
		results.emplace_back(std::make_unique<TPosition<N, M>>(1));
		return results;
	}

	friend std::ostream& operator<<(std::ostream& os, const TPosition<N, M>& pos) {
		os << pos.curr;
		return os;
	}

private:
	unsigned	curr;
};

using Position = TPosition<5, 30>;
