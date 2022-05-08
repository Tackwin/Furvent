#include "imgui_poker.h"

#include <cmath>
#include <stdio.h>

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include "Macros.hpp"
#include "Game.hpp"
#include "math/random.hpp"

ImVec2 operator/(const ImVec2& left, int right) noexcept {
	return {left.x / right, left.y / right};
}
ImVec2 operator*(const ImVec2& left, float right) noexcept {
	return {left.x * right, left.y * right};
}
ImVec2 operator+(const ImVec2& left, const ImVec2& right) noexcept {
	return {left.x + right.x, left.y + right.y};
}
ImVec2 operator-(const ImVec2& left, const ImVec2& right) noexcept {
	return {left.x - right.x, left.y - right.y};
}

#define COL(r, g, b, a) \
	IM_COL32((uint8_t)(255 * r), (uint8_t)(255 * g), (uint8_t)(255 * b), (uint8_t)(255 * a))

ImGui::Interaction_Query ImGui::display(Game& game, const ImGui::Interaction_State& state) noexcept
{
	ImGui::Interaction_Query ret;

	ImGui::PushID(&game);
	defer { ImGui::PopID(); };

	ImGui::Begin("Game");
	defer { ImGui::End(); };

	#define X(n) ImGui::Text(#n ": %d", (int)game.n);
	if (game.is_over()) ImGui::Text("Game over !");
	X(big_blind);
	X(current_hand.running_bet);

	ret.step = ImGui::Button("Step");
	ImGui::SameLine();
	ret.hand = ImGui::Button("Hand");
	ImGui::SameLine();
	if (state.run) ret.toggle_run = ImGui::Button("Stop");
	else           ret.toggle_run = ImGui::Button("Play");

	auto draw_list = ImGui::GetWindowDrawList();
	auto offset = ImGui::GetWindowPos();
	auto pos = ImGui::GetCursorPos();
	auto size = ImGui::GetContentRegionAvail();
	auto center = offset + size / 2;
	center.y += 20;
	center.y += pos.y;
	const ImVec2 CARD_SIZE = ImVec2(40, 60);
	const ImU32 CARD_COLOR[4] = {
		COL(0.1, 0.1, 0.1, 1.0),
		COL(0.9, 0.1, 0.1, 1.0),
		COL(0.1, 0.1, 0.9, 1.0),
		COL(0.1, 0.9, 0.1, 1.0)
	};

	uint8_t bufferSeed[22];
	export_seed(game.current_hand.start_seed, bufferSeed);

	draw_list->AddRectFilled(center - size / 3, center + size / 3, COL(0.4, 0.4, 0.4, 1.0));
	draw_list->AddRect(center - size / 3, center + size / 3, COL(0.1, 0.1, 0.1, 1.0), 0, 15, 2);
	draw_list->AddText(
		center + ImVec2{ size.x / 3 + 20, 0 },
		COL(1, 1, 1, 1),
		(const char*)bufferSeed,
		(const char*)(bufferSeed + 11)
	);
	draw_list->AddText(
		center + ImVec2{ size.x / 3 + 20, 15 },
		COL(1, 1, 1, 1),
		(const char*)(bufferSeed + 11),
		(const char*)(bufferSeed + 22)
	);

	auto draw_card = [&] (const Card* card, ImVec2 p) {
		if (!card) {
			draw_list->AddRectFilled(p - CARD_SIZE / 2, p + CARD_SIZE / 2, COL(0.9, 0.6, 0.6, 1.0));
			return;
		}
		draw_list->AddRectFilled(p - CARD_SIZE / 2, p + CARD_SIZE / 2, COL(0.7, 0.7, 0.7, 1.0));
		draw_list->AddText(
			p - CARD_SIZE / 2 + ImVec2(10, 10),
			CARD_COLOR[(int)card->color],
			card->str().c_str(),
			NULL
		);
	};

	for (size_t i = 0; i < game.players.size(); ++i) {
		const auto& player = game.players[i];
		char bufferName[] = "Player XXXXXXX";
		char bufferAgent[] = "Agent XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
		char bufferBet[]  = "Bet XXXXXXXX";
		char bufferStack[]  = "Stack XXXXXXXX";
		sprintf(bufferName, "Player %d", (int)i);
		sprintf(bufferAgent, "Agent %s", game.agents[i]->type_name());
		sprintf(bufferBet, "Bet %d", (int)player.current_bet);
		sprintf(bufferStack, "Stack %d", (int)player.stack);

		float t = 2 * 3.1415926 * (0.5 + i / 3.0);

		ImVec2 p = center + ImVec2{ std::cos(t), std::sin(t) / 1.5f } * size.x / 3;
		p.x += CARD_SIZE.x;

		if (game.current_hand.won[i]) {
			draw_list->AddRectFilled(
				p - ImVec2{CARD_SIZE.x + 10, CARD_SIZE.y / 2 + 5},
				p + ImVec2{CARD_SIZE.x + 10, CARD_SIZE.y / 2 + 5},
				COL(0, 0, 1, 1)
			);
		}

		bool hide_card = game.phase == 0 || player.folded;
		draw_card(hide_card ? nullptr : &player.hand[0], p - ImVec2{CARD_SIZE.x / 2 + 5, 0});
		draw_card(hide_card ? nullptr : &player.hand[1], p + ImVec2{CARD_SIZE.x / 2 + 5, 0});
		draw_list->AddText(
			p - ImVec2{CARD_SIZE.x, CARD_SIZE.y / 2 + 25}, COL(1, 1, 1, 1), bufferName
		);
		draw_list->AddText(
			p - ImVec2{CARD_SIZE.x, CARD_SIZE.y / 2 + 45}, COL(1, 1, 1, 1), bufferAgent
		);
		draw_list->AddText(
			p - ImVec2{CARD_SIZE.x, -CARD_SIZE.y / 2 - 5}, COL(1, 1, 1, 1), bufferBet
		);
		draw_list->AddText(
			p - ImVec2{CARD_SIZE.x, -CARD_SIZE.y / 2 - 20}, COL(1, 1, 1, 1), bufferStack
		);

		if (i == (game.big_bling_idx % game.players.size())) {
			draw_list->AddCircleFilled(p + ImVec2{CARD_SIZE.x + 20, 0}, 5, COL(0, 0, 1, 1));
		}
		if (i == ((game.big_bling_idx + 1) % game.players.size())) {
			draw_list->AddCircleFilled(p + ImVec2{CARD_SIZE.x + 20, 0}, 3, COL(0, 1, 0, 1));
		}
	}

	draw_list->AddRectFilled(
		center - ImVec2{ (CARD_SIZE.x + 10) * 2.5f + 2, CARD_SIZE.y / 2 + 5 },
		center + ImVec2{ (CARD_SIZE.x + 10) * 2.5f + 2, CARD_SIZE.y / 2 + 5 },
		COL(0.1, 0.7, 0.1, 1.0)
	);

	draw_list->AddLine(
		center + ImVec2{ (CARD_SIZE.x + 10) * 0.5f - 2, CARD_SIZE.y / 2 + 3},
		center + ImVec2{ (CARD_SIZE.x + 10) * 0.5f - 2, -CARD_SIZE.y / 2 - 3},
		COL(1, 0, 0, 1)
	);

	draw_list->AddLine(
		center + ImVec2{ (CARD_SIZE.x + 10) * 1.5f - 0, CARD_SIZE.y / 2 + 3},
		center + ImVec2{ (CARD_SIZE.x + 10) * 1.5f - 0, -CARD_SIZE.y / 2 - 3},
		COL(0, 0, 1, 1)
	);


	char buffer[] = "Pot: XXXXXXXXXX";
	sprintf(buffer, "Pot: %d", (int)game.current_hand.pot);
	draw_list->AddText(center + ImVec2{0, -CARD_SIZE.y}, COL(0, 0, 0, 1), buffer);

	if (game.phase > 2) {
		draw_card(
			&game.current_hand.flop[0],
			center - ImVec2{(CARD_SIZE.x + 10) * 2.f + 2, 0}
		);
		draw_card(
			&game.current_hand.flop[1],
			center - ImVec2{(CARD_SIZE.x + 10) * 1.f + 2, 0}
		);
		draw_card(
			&game.current_hand.flop[2],
			center - ImVec2{(CARD_SIZE.x + 10) * 0.f + 2, 0}
		);
	}

	if (game.phase > 3) {
		draw_card(
			&game.current_hand.turn,
			center + ImVec2{(CARD_SIZE.x + 10) * 1.f - 0, 0}
		);
	}

	if (game.phase > 4) {
		draw_card(
			&game.current_hand.river,
			center + ImVec2{(CARD_SIZE.x + 10) * 2.f + 2, 0}
		);
	}

	return ret;
}