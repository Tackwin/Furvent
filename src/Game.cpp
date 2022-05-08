#include "Game.hpp"
#include <algorithm>
#include <assert.h>

#include <time.h>
#include <random>
#include "Macros.hpp"

#include "imgui/imgui.h"

Agent Agent::Default_Agent;

std::array<size_t, 3> pick_winners(
	const std::array<Player, 3>& players, std::array<Card, 5> board
) noexcept;

Game::Game() noexcept {
	for (size_t i = 0; i < agents.size(); ++i) {
		agents[i] = &Agent::Default_Agent;
		players[i].stack = 500;
	}
}

bool Game::is_over() noexcept {
	size_t n = 0;
	for (auto& x : players) if (x.stack > 0) n++;
	return n == 1;
}

void Game::step(RNG_State& rng_state) noexcept {
	auto round = [&] (bool force_blind) {
		n_rounds++;

		current_hand.running_bet = 0;
		for (auto& x : players) x.current_bet = 0;

		if (force_blind) {
			auto& big_player = players[big_bling_idx];
			auto& small_player = players[((players.size() + big_bling_idx) - 1) % players.size()];

			current_hand.pot         += std::min(big_player.stack,   big_blind);
			big_player.bet           += std::min(big_player.stack,   big_blind);
			big_player.current_bet   += std::min(big_player.stack,   big_blind);
			big_player.stack         -= std::min(big_player.stack,   big_blind);

			current_hand.pot         += std::min(small_player.stack, big_blind / 2);
			small_player.bet         += std::min(small_player.stack, big_blind / 2);
			small_player.current_bet += std::min(small_player.stack, big_blind / 2);
			small_player.stack       -= std::min(small_player.stack, big_blind / 2);
			
			current_hand.running_bet = big_blind;
		}

		size_t before_running_bet = 0;
		raised_turn = false;
		bool first_turn = true;
		do {
			before_running_bet = current_hand.running_bet;

			for (size_t i = 0; i < players.size(); ++i) {
				auto idx = (i + big_bling_idx + 1) % players.size();
				if (players[idx].folded) continue;

				auto act = agents[idx]->act(players[idx], *this);
				apply(players[idx], act);
			}

			first_turn = false;
		} while (current_hand.running_bet > before_running_bet);
	};

 	switch (phase) {
	case 0: {
		current_hand = {};
		current_hand.draw.reset();
		current_hand.start_seed = rng_state;
		current_hand.draw.shuffle(rng_state);

		for (auto& x : current_hand.won) x = false;
		for (auto& x : players) {
			x.bet = 0;
			x.current_bet = 0;
			x.folded = x.stack == 0;

			for (size_t i = 0; i < x.hand.size(); ++i) x.hand[i] = current_hand.draw.draw();
		}

		big_bling_idx %= players.size();
		break;
	}
	case 1: {
		round(true);
		break;
	}
	case 2: {
		for (size_t i = 0; i < current_hand.flop.size(); ++i)
			current_hand.flop[i] = current_hand.draw.draw();
		round(false);
		break;
	}
	case 3: {
		current_hand.turn = current_hand.draw.draw();
		round(false);
		break;
	}
	case 4: {
		current_hand.river = current_hand.draw.draw();
		round(false);
		auto places = pick_winners(players, {
			current_hand.flop[0],
			current_hand.flop[1],
			current_hand.flop[2],
			current_hand.turn,
			current_hand.river
		});

		std::array<bool, 3> is_out;
		for (size_t i = 0; i < players.size(); ++i) is_out[i] = players[i].folded;
		while (current_hand.pot > 0) {

			// We get what's the smallest side-pot yet to be handled
			size_t running_pot = SIZE_MAX;
			for (size_t i = 0; i < players.size(); ++i) if (!is_out[i])
				running_pot = std::min(running_pot, players[i].bet);

			size_t sum = 0;

			// We determine the size of this pot
			for (auto& x : players) sum   += std::min(running_pot, x.bet);


			// We then take out their participation in this pot to their bet stack.
			for (auto& x : players) x.bet -= std::min(running_pot, x.bet);

			// What is the best place for this pot
			size_t best_place = SIZE_MAX;
			for (size_t i = 0; i < players.size(); ++i) if (!is_out[i])
				best_place = std::min(best_place, places[i]);

			// We count how many ties they were for this place
			size_t split = 0;
			for (size_t i = 0; i < players.size(); ++i) if (!is_out[i] && places[i] == best_place)
				split++;

			// we divide along all the ties the pot
			size_t rest = sum;
			for (size_t i = 0; i < players.size(); ++i) if (!is_out[i] && places[i] == best_place) {
				players[i].stack += sum / split;
				rest -= sum / split;

				// We mark this player has having won something
				current_hand.won[i] = true;
			}
			// Don't forget to award the remainder if it's an non even division !
			for (size_t i = 0; i < players.size(); ++i) if (!is_out[i] && places[i] == best_place) {
				players[i].stack += rest;
				break;
			}

			// We count the participants that are out
			for (size_t i = 0; i < players.size(); ++i) if (players[i].bet == 0) is_out[i] = true;

			// This pot has been handled onto the next one !
			current_hand.pot -= sum;
		}

		big_bling_idx++;
		break;
	}
	case 5: {
		break;
	}
	}
	phase++;
	phase %= N_Phase;
}

void Game::play_new_hand(RNG_State& rng_state) noexcept {
	for (size_t i = 0; i < N_Phase; ++i) step(rng_state);
}

void Game::apply(Player& player, Action action) noexcept {
	switch (action.kind) {
	case Action::Check: {
		assert(!player.folded);
		assert(player.current_bet == current_hand.running_bet);
		break;
	}
	case Action::Fold: {
		assert(!player.folded);
		player.folded = true;
		player.current_bet = 0;
		break;
	}
	case Action::Follow: {
		assert(!player.folded);
		size_t dt = current_hand.running_bet - player.current_bet;
		assert(player.stack >= dt);

		player.stack       -= dt;
		current_hand.pot   += dt;
		player.current_bet += dt;
		player.bet         += dt;
		break;
	}
	case Action::Raise: {
		assert(!player.folded);
		assert(player.stack >= action.value);

		player.stack       -= action.value;
		current_hand.pot   += action.value;
		player.current_bet += action.value;
		player.bet         += action.value;

		current_hand.running_bet = player.current_bet;
		raised_turn = true;
		break;
	}
	case Action::None: {
		assert(player.folded || player.stack == 0);
		break;
	}
	default: assert("Logic error.");
	}
}

Action Agent::act(const Player& me, const Game& game) noexcept {
	Action action;
	if (me.stack == 0) {
		action.kind = Action::None;
		return action;
	}

	if (me.stack < game.current_hand.running_bet + me.bet) {
		action.kind = Action::Fold;
		return action;
	}

	if (game.raised_turn) {
		if (game.current_hand.running_bet > me.stack) action.kind = Action::Fold;
		else action.kind = Action::Follow;
	}
	else {
		bool can_raise{ false };
		size_t to_raise = std::min(game.current_hand.running_bet + 10, me.stack);

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

void Game::render() noexcept {
	for (size_t i = 0; i < players.size(); ++i) {
		ImGui::Text(
			"Player %zu: %s %s with % 5d betting % 5d\n",
			i,
			players[0].hand[0].str().c_str(),
			players[0].hand[1].str().c_str(),
			(int)players[0].stack,
			(int)players[0].bet
		);
	}
}


Deck::Deck() noexcept {
	size_t n = 0;
	for (size_t i = 0; i < (size_t)Color::Size; ++i) {
		for (size_t j = 0; j < (size_t)Value::Size; ++j) {
			deck[n++] = { (Color)i, (Value)j };
		}
	}
}

void Deck::shuffle(RNG_State& state) noexcept {
	std::shuffle(BEG_END(deck), Uniform_uint32_t{state});
}
void Deck::reset() noexcept {
	top = 0;
}
Card Deck::draw() noexcept {
	return deck[top++];
}

std::array<size_t, 3> pick_winners(
	const std::array<Player, 3>& players, std::array<Card, 5> board
) noexcept {
	struct Combo {
		enum Kind {
			High = 0,
			Pair,
			Two_Pair,
			Three_Of_A_Kind,
			Straight,
			Flush,
			Full,
			Four_Of_A_Kind,
			Straight_Flush,
			Royal_Flush,
			Size
		} kind{ High };

		std::array<Card, 5> cards;
		size_t player_idx;

		bool operator==(const Combo& other) const noexcept {
			if (kind != other.kind) return false;
			for (size_t i = 0; i < cards.size(); ++i) {
				if (cards[i].value != other.cards[i].value) return false;
			}
			return true;
		}
		bool operator<(const Combo& other) const noexcept {
			if (kind < other.kind) return true;
			if (kind > other.kind) return false;
			if (cards < other.cards) return true;
			return false;
		}
		bool operator>(const Combo& other) const noexcept {
			if (kind > other.kind) return true;
			if (kind < other.kind) return false;
			if (cards > other.cards) return true;
			return false;
		}
	};

#define E END(hand)
#define X BEG_END(hand)
	auto get_dominant_color = [](const std::array<Card, 7>& hand) -> std::pair<Color, size_t> {
		std::array<size_t, (size_t)Color::Size> n{};
		Color dominant{ Color::Club };

		for (size_t i = 0; i < 4; ++i) ++n[i];
		for (size_t i = 0; i < n.size(); ++i) if (n[(size_t)dominant] < n[i]) dominant = (Color)i;

		return { dominant, n[(size_t)dominant] };
	};

	auto test_four_of_a_kind = [&](const std::array<Card, 7> & hand) -> bool {
		for (size_t i = 0; i < 4; ++i) {
			bool flag =
				std::find(X, Card{ Color::Club, hand[i].value }) != E &&
				std::find(X, Card{ Color::Diamond, hand[i].value }) != E &&
				std::find(X, Card{ Color::Heart, hand[i].value }) != E &&
				std::find(X, Card{ Color::Spade, hand[i].value }) != E;
			if (flag) return true;
		}
		return false;
	};

	auto test_three_of_a_kind = [&](const std::array<Card, 7> & hand) -> bool {
		for (size_t i = 0; i < 5; ++i) {
			size_t n =
				std::count(X, Card{ Color::Club, hand[i].value }) +
				std::count(X, Card{ Color::Diamond, hand[i].value }) +
				std::count(X, Card{ Color::Heart, hand[i].value }) +
				std::count(X, Card{ Color::Spade, hand[i].value });

			if (n >= 3) return true;
		}
		return false;
	};

	auto test_pair = [&](const std::array<Card, 7> & hand) -> bool {
		for (size_t i = 0; i < 6; ++i) {
			size_t n =
				std::count(X, Card{ Color::Club, hand[i].value }) +
				std::count(X, Card{ Color::Diamond, hand[i].value }) +
				std::count(X, Card{ Color::Heart, hand[i].value }) +
				std::count(X, Card{ Color::Spade, hand[i].value });

			if (n >= 2) return true;
		}
		return false;
	};

	auto test_two_pair = [&](const std::array<Card, 7> & hand) -> bool {
		size_t n_pair = 0;

		for (size_t i = 0; i < 6; ++i) {
			size_t n =
				std::count(X, Card{ Color::Club, hand[i].value }) +
				std::count(X, Card{ Color::Diamond, hand[i].value }) +
				std::count(X, Card{ Color::Heart, hand[i].value }) +
				std::count(X, Card{ Color::Spade, hand[i].value });

			if (n >= 2) n_pair++;

			// Every pair is counted twice so we need to go to three pairs to detect a double pair.
			if (n_pair > 2) return true;
		}
		return false;
	};

	auto test_royal_flush = [&](const std::array<Card, 7>& hand) -> bool {
		auto [dominant, n] = get_dominant_color(hand);
		if (n < 5) return false;

		return
			std::find(X, Card{ dominant, Value::As }) != E &&
			std::find(X, Card{ dominant, Value::King }) != E &&
			std::find(X, Card{ dominant, Value::Queen }) != E &&
			std::find(X, Card{ dominant, Value::Jacket }) != E &&
			std::find(X, Card{ dominant, Value::Ten }) != E
		;
	};

	auto test_straight = [&](const std::array<Card, 7>& hand) -> bool {
		for (size_t i = 0; i < 7; ++i) {
			auto any_color = [&](Value value) {
				return std::find_if(X, [&](auto x) {return x.value == value; }) != E;
			};

			size_t start_value = (size_t)hand[i].value;
			if (start_value == (size_t)Value::As) start_value = (size_t)(-1);
			if (start_value > (size_t)Value::Ten) continue;

			for (size_t j = 0; j < 4; ++j) {
				auto to_search = (Value)((start_value + j + 1) % (size_t)Value::Size);
				if (!any_color(to_search)) goto up_continue;
			}

			return true;
			up_continue:
			;
		}
		return false;
	};

	auto test_full = [&](const std::array<Card, 7> & hand) -> bool {
		size_t n_pair = 0;
		size_t n_brelan = 0;


		for (size_t i = 0; i < 6; ++i) {
			size_t n =
				std::count(X, Card{ Color::Club, hand[i].value }) +
				std::count(X, Card{ Color::Diamond, hand[i].value }) +
				std::count(X, Card{ Color::Heart, hand[i].value }) +
				std::count(X, Card{ Color::Spade, hand[i].value });

			if (n == 2) n_pair++;
			if (n == 3) n_brelan++;

			if (n_pair && n_brelan) return true;
		}

		return false;
	};

	auto test_flush = [&](const std::array<Card, 7> & hand) -> bool {
		return get_dominant_color(hand).second >= 5;
	};

	auto test_straight_flush = [&](const std::array<Card, 7>& hand) -> bool {
		return test_straight(hand) && test_flush(hand);
	};
#undef X
#undef E

	std::vector<Combo> combos;

	for (size_t i = 0; i < players.size(); ++i) if (!players[i].folded) {
		Combo combo;
		combo.player_idx = i;
		auto& p = players[i];

		if (p.folded) continue;

		defer{ combos.push_back(combo); };

		std::array<Card, 7> combined_hand = {
			p.hand[0],
			p.hand[1],
			board[0],
			board[1],
			board[2],
			board[3],
			board[4]
		};

		if (test_royal_flush(combined_hand)) {
			combo.kind = Combo::Kind::Royal_Flush;
		}
		else if (test_straight_flush(combined_hand)) {
			combo.kind = Combo::Kind::Straight_Flush;
		}
		else if (test_four_of_a_kind(combined_hand)) {
			combo.kind = Combo::Kind::Four_Of_A_Kind;
		}
		else if (test_four_of_a_kind(combined_hand)) {
			combo.kind = Combo::Kind::Four_Of_A_Kind;
		}
		else if (test_full(combined_hand)) {
			combo.kind = Combo::Kind::Full;
		}
		else if (test_flush(combined_hand)) {
			combo.kind = Combo::Kind::Flush;
		}
		else if (test_straight(combined_hand)) {
			combo.kind = Combo::Kind::Straight;
		}
		else if (test_three_of_a_kind(combined_hand)) {
			combo.kind = Combo::Kind::Three_Of_A_Kind;
		}
		else if (test_two_pair(combined_hand)) {
			combo.kind = Combo::Kind::Two_Pair;
		}
		else if (test_pair(combined_hand)) {
			combo.kind = Combo::Kind::Pair;
		}
		else {
			combo.kind = Combo::Kind::High;
		}

		std::sort(BEG_END(combined_hand), [](auto a, auto b) {
			if (a.value == Value::As && b.value != Value::As) return true;
			if (a.value != Value::As && b.value == Value::As) return false;
			return (size_t)a.value > (size_t)b.value;
		});

		for (size_t j = 0; j < 5; j++) combo.cards[j] = combined_hand[j];
	}

	std::sort(BEG_END(combos), [](auto& a, auto& b) { return a > b; });

	std::array<size_t, 3> places;
	size_t place = 0;
	if (combos.size() > 0) places[combos[0].player_idx] = place;
	for (size_t i = 1; i < combos.size(); ++i) {
		if (!(combos[i] == combos[i - 1])) place++;
		places[combos[i].player_idx] = place;
	}
	return places;
}

size_t Game::best() noexcept {
	size_t best = 0;
	if (players[1].stack > players[best].stack) best = 1;
	if (players[2].stack > players[best].stack) best = 2;
	return best;
}
