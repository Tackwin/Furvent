#pragma once
#include <array>
#include <vector>
#include <string>
#include <optional>

#include "math/random.hpp"

enum class Color {
	Spade = 0,
	Heart,
	Diamond,
	Club,
	Size
};

enum class Value {
	Two = 0,
	Three,
	Four,
	Five,
	Six,
	Seven,
	Eight,
	Nine,
	Ten,
	Jacket,
	Queen,
	King,
	As,
	Size
};

struct Card {
	Color color;
	Value value;

	std::string str() const noexcept {
		std::string ret;
		switch (value) {
		case Value::Two :
			ret = "2";
			break;
		case Value::Three :
			ret = "3";
			break;
		case Value::Four :
			ret = "4";
			break;
		case Value::Five :
			ret = "5";
			break;
		case Value::Six :
			ret = "6";
			break;
		case Value::Seven :
			ret = "7";
			break;
		case Value::Eight :
			ret = "8";
			break;
		case Value::Nine :
			ret = "9";
			break;
		case Value::Ten :
			ret = "T";
			break;
		case Value::Jacket :
			ret = "J";
			break;
		case Value::Queen :
			ret = "Q";
			break;
		case Value::King :
			ret = "K";
			break;
		case Value::As :
			ret = "A";
			break;
		default:
			break;
		}
		switch (color) {
		case Color::Club :
			ret += "C";
			break;
		case Color::Diamond :
			ret += "D";
			break;
		case Color::Heart :
			ret += "H";
			break;
		case Color::Spade :
			ret += "S";
			break;
		default:
			break;
		}

		return ret;
	}

	bool operator==(const Card& other) const noexcept {
		return color == other.color && value == other.value;
	}
	bool operator<(const Card& other) const noexcept {
		return value < other.value;
	}
	bool operator>(const Card& other) const noexcept {
		return value > other.value;
	}
};

struct Deck {
	std::vector<Card> deck;
	size_t top = 0;

	Deck() noexcept;

	void shuffle(RNG_State& rng_state) noexcept;
	void reset() noexcept;
	Card draw() noexcept;
};

struct Agent;
struct Player {
	std::array<Card, 2> hand{};
	size_t stack{ 0 };

	size_t bet{ 0 };
	size_t current_bet{ 0 };
	bool folded{ false };
};

struct Hand {
	RNG_State start_seed;
	Deck draw;

	std::array<Card, 3> flop;
	Card turn;
	Card river;

	size_t pot{ 0 };
	size_t running_bet = 0;
	size_t big_blind{ 0 };

	std::array<bool, 3> won;
};

struct Action {
	size_t value;
	enum {
		Follow = 0,
		Raise,
		Check,
		None,
		Fold,
		Size
	} kind;
};

struct Game;
struct Agent {
	static Agent Default_Agent;
	
	virtual Action act(const Player& player, const Game& game) noexcept;
	virtual const char* type_name() const noexcept { return "Default"; }
};


struct Game {
	Game() noexcept;

	Hand current_hand;

	std::array<Player, 3> players;
	std::array<Agent*, 3> agents;

	size_t big_blind{ 10 };
	size_t big_bling_idx{ 0 };

	size_t phase{ 0 };
	static constexpr size_t N_Phase{ 6 };

	size_t n_rounds = 0;

	bool raised_turn{ false };

	void play_new_hand(RNG_State& rng_state) noexcept;
	void step(RNG_State& rng_state) noexcept;
	void apply(Player& player, Action x) noexcept;
	bool is_over() noexcept;

	void render() noexcept;
};


