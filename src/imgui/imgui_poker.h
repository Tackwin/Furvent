#pragma once

class Game;
class Tournament;
class Table_Agent;

namespace ImGui {
	struct Interaction_State {
		size_t n_best = 3*3*3*3*3*3;
		bool run = false;
	};

	struct Interaction_Query {
		bool step       = false;
		bool hand       = false;
		bool round      = false;
		bool toggle_run = false;

		bool did_clicked_agent = false;
		size_t clicked_round = 0;
		size_t clicked_agent = 0;
		size_t n_best = 3*3*3*3*3;
		bool run_season = false;
		bool next_gen = false;

		bool toggle_next_gen = false;
	};
	
	extern Interaction_Query display(Game& game, const Interaction_State& state) noexcept;
	extern Interaction_Query display(Tournament& tourney, const Interaction_State& state) noexcept;

	extern void display(Table_Agent& agent) noexcept;
};