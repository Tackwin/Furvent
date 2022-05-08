#pragma once

#include <vector>

#include "math/random.hpp"
#include "Agents.hpp"

struct Tournament {
	std::vector<std::vector<Agent*>> rounds;

	Tournament() noexcept;

	template<typename T>
	void append(std::vector<T>& to_append) noexcept {
		for (auto& x : to_append) rounds.back().push_back(dynamic_cast<Agent*>(&x));
	}
	void shuffle(RNG_State& state) noexcept;

	void round(RNG_State& state) noexcept;
};