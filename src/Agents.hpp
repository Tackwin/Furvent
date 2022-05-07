#pragma once

#include "Game.hpp"

struct Bonobo_Agent : public Agent {
	size_t weight[1] = { 1 };

	virtual Action act(const Player& player, const Game& game) noexcept;
	virtual const char* type_name() const noexcept { return "Bonobo"; }
};

struct Table_Agent : public Agent {
	size_t weight[52*52] = { 1 };

	virtual Action act(const Player& player, const Game& game) noexcept;
	virtual const char* type_name() const noexcept { return "Table"; }
};