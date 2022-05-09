#pragma once

#include "Game.hpp"
#include "math/random.hpp"

struct Bonobo_Agent : public Agent {
	size_t weight[1] = { 1 };

	virtual Action act(const Player& player, const Game& game) noexcept;
	virtual const char* type_name() const noexcept { return "Bonobo"; }
};

struct Table_Agent : public Agent {
	double weight[(size_t)Value::Size * (size_t)Value::Size] = { 1 };

	void init_random(RNG_State& rng) noexcept;

	virtual void offspring(Agent& out, RNG_State& rng) noexcept;
	virtual Action act(const Player& player, const Game& game) noexcept;
	virtual const char* type_name() const noexcept { return "Table"; }
};