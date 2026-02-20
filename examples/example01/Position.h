#pragma once

#include "GameGraphPositionBase.h"

#include <iostream>

template<unsigned N, unsigned M>
class TPosition : public GameGraphPositionBase {
public:
	TPosition() = delete;
	TPosition(int curr) : curr(curr) {}

	~TPosition() = default;

	std::vector<unique_ptr> get_next_positions() const override {
		std::vector<unique_ptr> results;
		for (int i = 1; i <= N; ++i) {
			unsigned next = curr + i;
			if (next > M + 1) {
				break;
			}
			results.emplace_back(std::make_unique<TPosition>(next));
		}
		return results;
	}

	bool is_terminal() const override {
		return curr >= M + 1;
	}

	bool less(const GameGraphPositionBase* rhs) const override {
		const TPosition* pos = dynamic_cast<const TPosition*>(rhs);
		return curr < pos->curr;
	}

	static std::vector<unique_ptr> get_starting_positions() {
		std::vector<unique_ptr> results;
		results.emplace_back(std::make_unique<TPosition>(1));
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
