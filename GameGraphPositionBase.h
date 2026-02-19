#pragma once

#include <vector>
#include <memory>
#include <functional>

class GameGraphPositionBase {
public:
	using unique_ptr = std::unique_ptr<GameGraphPositionBase>;
	
	virtual ~GameGraphPositionBase() = default;

	virtual std::vector<unique_ptr> get_next_positions() const = 0;
	virtual bool is_terminal() const = 0;
	virtual bool less(const GameGraphPositionBase* rhs) const = 0;

	struct Less {
		bool operator()(const GameGraphPositionBase* lhs, const GameGraphPositionBase* rhs) const {
			return lhs->less(rhs);
		}
	};
};
