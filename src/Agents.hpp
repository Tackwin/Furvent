#pragma once

#include <array>

#include "Game.hpp"
#include "math/random.hpp"

struct Bonobo_Agent : public Agent {
	size_t weight[1] = { 1 };

	virtual Action act(const Player& player, const Game& game, RNG_State& rng) noexcept;
	virtual const char* type_name() const noexcept { return "Bonobo"; }
};

struct Table_Agent : public Agent {
	std::array<float, (size_t)Value::Size * (size_t)Value::Size> weight = { 1 };

	void init_random(RNG_State& rng) noexcept;

	virtual void offspring(Agent& out, RNG_State& rng) noexcept;
	virtual Action act(const Player& player, const Game& game, RNG_State& rng) noexcept;
	virtual const char* type_name() const noexcept { return "Table"; }
};