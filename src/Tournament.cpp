#include "Tournament.hpp"

#include <algorithm>
#include <assert.h>

#include "Macros.hpp"


Tournament::Tournament() noexcept {
	rounds.push_back({});
}

void Tournament::shuffle(RNG_State& state) noexcept {
	std::shuffle(BEG_END(rounds.back()), Uniform_uint32_t{ state });
}

void Tournament::round(RNG_State& state) noexcept {
	std::vector<Agent*>& agents = rounds.back();
	std::vector<Agent*> next_round;

	assert((agents.size() % 3) == 0);

	for (size_t i = 0; i < agents.size(); i += 3) {
		Game game;
		game.agents[0] = agents[i + 0];
		game.agents[1] = agents[i + 1];
		game.agents[2] = agents[i + 2];

		while (!game.is_over()) {
			game.step(state);
		}

		size_t best = game.best();

		next_round.push_back(game.agents[best]);
	}

	rounds.emplace_back(std::move(next_round));
}
