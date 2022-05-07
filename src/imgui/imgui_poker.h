#pragma once

class Game;

namespace ImGui {
	struct Interaction_State {
		bool run = false;
	};

	struct Interaction_Query {
		bool step       = false;
		bool hand       = false;
		bool toggle_run = false;
	};
	
	extern Interaction_Query display(Game& game, const Interaction_State& state) noexcept;
};