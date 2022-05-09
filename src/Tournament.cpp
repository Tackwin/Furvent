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

	std::array<size_t, 3> n_wins;

	constexpr size_t BOX = 1;

	for (size_t i = 0; i < agents.size(); i += 3) {
		for (auto& x : n_wins) x = 0;


		for (size_t n = 0; n < BOX; ++n) {
			Game game;
			game.agents[0] = agents[i + 0];
			game.agents[1] = agents[i + 1];
			game.agents[2] = agents[i + 2];

			while (!game.is_over()) {
				game.step(state);
			}

			size_t best = game.best();
			n_wins[best]++;

			if (n_wins[best] * 3 > BOX) break;
		}

		size_t best = 0;
		for (size_t j = 0; j < 3; ++j) if (n_wins[best] < n_wins[j]) best = j;
		next_round.push_back(agents[i + best]);
	}

	rounds.emplace_back(std::move(next_round));
}

void Tournament::at_least_n_best(size_t n, RNG_State& state) noexcept {
	while (rounds.back().size() >= n * 3) {
		round(state);
	}
}