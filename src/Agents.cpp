#include "Agents.hpp"

Action Bonobo_Agent::act(const Player& me, const Game& game) noexcept {
	Action action;
	if (me.stack == 0) {
		action.kind = Action::None;
		return action;
	}

	if (game.raised_turn) {
		if (game.current_hand.running_bet > me.stack) action.kind = Action::Fold;
		else action.kind = Action::Follow;
	}
	else {
		bool can_raise{ false };
		size_t to_raise = std::min(game.current_hand.running_bet + weight[0], me.stack);

		for (auto& x : game.players) {
			if (&me == &x) continue;

			if (x.stack + x.current_bet > to_raise + game.current_hand.running_bet) {
				can_raise = true;
				break;
			}
		}

		if (can_raise) {
			action.kind = Action::Raise;
			action.value = to_raise;
		}
		else {
			action.kind = Action::Follow;
		}
	}

	return action;
}

void Table_Agent::init_random(RNG_State& rng) noexcept {
	for (auto& x : weight) x = normal(rng);
}

Action Table_Agent::act(const Player& me, const Game& game) noexcept {
	Action action;

	size_t card1_idx = (size_t)me.hand[0].value;
	size_t card2_idx = (size_t)me.hand[1].value;

	if (card1_idx > card2_idx) {
		auto t = card2_idx;
		card2_idx = card1_idx;
		card1_idx = t;
	}

	if (me.hand[0].color != me.hand[1].color) {
		auto t = card2_idx;
		card2_idx = card1_idx;
		card1_idx = t;
	}

	if (weight[card1_idx + card2_idx * (size_t)Value::Size] > 0) {
		action.kind = Action::Raise;
		action.value = me.stack;
	} else {
		action.kind = Action::Fold;
	}

	return action;
}
