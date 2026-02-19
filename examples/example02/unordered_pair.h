#pragma once

#include <initializer_list>
#include <array>
#include <utility>

template<typename T>
class unordered_pair {
public:
	unordered_pair() = default;

	unordered_pair(std::initializer_list<T> init) {
		std::copy(init.begin(), init.end(), items_.begin());
	}

	T& operator[](bool index) {
		if (index == 0) {
			return items_[0] < items_[1] ? items_[0] : items_[1];
		}
		else {
			return items_[0] > items_[1] ? items_[0] : items_[1];
		}
	}

	const T& operator[](bool index) const {
		if (index == 0) {
			return items_[0] < items_[1] ? items_[0] : items_[1];
		}
		else {
			return items_[0] > items_[1] ? items_[0] : items_[1];
		}
	}

	std::pair<T, T> to_pair() const {
		return {(*this)[0], (*this)[1]};
	}

private:
	std::array<T, 2> items_;
};
