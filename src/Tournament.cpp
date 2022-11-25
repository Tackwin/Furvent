#include "Tournament.hpp"

#include <algorithm>
#include <assert.h>

#include "Macros.hpp"

#include "omp.h"


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

	constexpr size_t BOX = 10;

	next_round.resize(agents.size() / 3);

	std::array<RNG_State, 16> thread_rng;
	for (auto& x : thread_rng) x = derive(state);

	#pragma omp parallel for
	for (size_t i = 0; i < agents.size(); i += 3) {
		for (auto& x : n_wins) x = 0;

		size_t tid = omp_get_thread_num();

		for (size_t n = 0; n < BOX; ++n) {
			Game game;
			game.agents[0] = agents[i + 0];
			game.agents[1] = agents[i + 1];
			game.agents[2] = agents[i + 2];

			for (size_t j = 0; j < 100 && !game.is_over(); ++j) game.play_new_hand(thread_rng[tid]);

			size_t best = game.best();
			n_wins[best]++;

			if (n_wins[best] * 2 > BOX) break;
		}

		size_t best = 0;
		for (size_t j = 0; j < 3; ++j) if (n_wins[best] < n_wins[j]) best = j;
		next_round[i / 3] = agents[i + best];
	}

	rounds.emplace_back(std::move(next_round));
}

void Tournament::at_least_n_best(size_t n, RNG_State& state) noexcept {
	while (rounds.back().size() >= n * 3) {
		round(state);
	}
}

void Round_Robin::play_n(size_t players, size_t n, RNG_State& rng) noexcept {
	thread_local std::vector<size_t> scores;
	scores.clear();
	scores.resize(players, 0);

	for (size_t i = 0; i < n; i += 6)
	// A loop of theses loops will make a 6 way round robin
	for (size_t a = 0; a < players; a++)
	for (size_t b = 0; b < players; b++)
	for (size_t c = 0; c < players; c++)
	if (a != b && b != c && c != a)
	{
		if (b == 0 && c == 1) printf("%zu %zu %zu\n", a, b, c);
		Game game;
		game.agents[0] = agents[a];
		game.agents[1] = agents[b];
		game.agents[2] = agents[c];

		while (!game.is_over()) game.step(rng);

		size_t best = game.best();
		if (best == 0) scores[a]++;
		if (best == 1) scores[b]++;
		if (best == 2) scores[c]++;
	}

	std::sort(BEG(agents), BEG(agents) + players, [&, first = agents.data()] (Agent* a, Agent* b) {
		size_t a_idx = a - *first;
		size_t b_idx = b - *first;

		return scores[a_idx] < scores[b_idx];
	});
}
