// Dear ImGui: standalone example application for GLFW + OpenGL2, using legacy fixed pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

// **DO NOT USE THIS CODE IF YOUR CODE/ENGINE IS USING MODERN OPENGL (SHADERS, VBO, VAO, etc.)**
// **Prefer using the code in the example_glfw_opengl2/ folder**
// See imgui_impl_glfw.cpp for details.

#include "imgui/imgui.h"
#include "imgui/imgui_poker.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl2.h"

#include <stdio.h>

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include "GLFW/glfw3.h"

#include "Game.hpp"
#include "Tournament.hpp"

static void glfw_error_callback(int error, const char* description) {
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

void repopulate(
	std::vector<Table_Agent>& population,
	const std::vector<Agent*>& elites,
	RNG_State& rng
) noexcept {
	thread_local std::vector<Table_Agent> new_population;
	new_population.clear();
	new_population.reserve(population.size());

	if (elites.empty()) return;

	size_t rest = population.size();
	size_t div = population.size() / elites.size();

	for (size_t i = 0; i < elites.size(); ++i) {
		auto& elite = *dynamic_cast<Table_Agent*>(elites[i]);
		new_population.push_back(elite);
		
		for (size_t j = 1; j < div; ++j) {
			Table_Agent new_agent;
			elite.offspring(new_agent, rng);
			new_population.push_back(new_agent);
		}

		rest -= div;
	}

	for (; rest > 0; rest--) {
		Table_Agent new_agent;
		elites.front()->offspring(new_agent, rng);
		new_population.push_back(new_agent);
	}

	population.swap(new_population);
}

int main(int, char**) {
	// Setup window
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit()) return 1;

	GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL2 example", NULL, NULL);
	if (window == NULL) return 1;

	glfwMakeContextCurrent(window);
	glfwSwapInterval(0); // Enable vsync

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL2_Init();

	// Our state
	bool show_demo_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	RNG_State rng_state;
	for (size_t i = 0; i < 4; ++i) rng_state.x[i] = i;

	std::array<Table_Agent, 3> table_agents_sample;
	for (auto& x : table_agents_sample) x.init_random(rng_state);
	for (auto& w : table_agents_sample[0].weight) w = -100;
	for (size_t i = 0; i < 13; ++i)
		for (size_t j = 0;j < 13; ++j)
			if (i == j || i > 10 || j > 10)
				table_agents_sample[0].weight[j * 13 + i] = 100;

	ImGui::Interaction_State ui_state;
	Game game;
	for (size_t i = 0; i < 3; ++i) game.agents[i] = &table_agents_sample[i];
	
	Tournament tourney;
	Round_Robin round_robin;

	constexpr size_t Population_N = 3*3*3*3*3*3*3;
	std::vector<Table_Agent> population;
	population.resize(Population_N);
	for (auto& x : population) x.init_random(rng_state);

	tourney.append(population);
	tourney.shuffle(rng_state);

	round_robin.append(population);

	Table_Agent* opened_agent = nullptr;

	bool autorun_gen_tourney = false;
	bool autorun_gen_round_robin = false;

	ui_state.n_pop = Population_N;

	// Main loop
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL2_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);

		// GAME
		auto query = ImGui::display(game, ui_state);
		if (query.step) game.step(rng_state);
		if (query.hand) game.play_new_hand(rng_state);
		if (query.toggle_run) ui_state.run ^= true;
		if (ui_state.run) game.step(rng_state);
		if (query.replay) {
			game = {};
			for (size_t i = 0; i < 3; ++i) game.agents[i] = &table_agents_sample[i];
		}

		// TOURNEY
		query = ImGui::display(tourney, ui_state);
		ui_state.n_best = query.n_best;
		if (query.round) tourney.round(rng_state);
		if (query.did_clicked_agent) {
			auto agent = tourney.rounds[query.clicked_round][query.clicked_agent];
			opened_agent = dynamic_cast<Table_Agent*>(agent);
		}
		if (opened_agent) ImGui::display(*opened_agent);
		if (query.run_season) {
			tourney.at_least_n_best(ui_state.n_best, rng_state);
		}
		if (query.toggle_next_gen) autorun_gen_tourney ^= true;
		if (autorun_gen_tourney) {
			tourney.at_least_n_best(ui_state.n_best, rng_state);
		}
		if (query.next_gen || autorun_gen_tourney) {
			repopulate(population, tourney.rounds.back(), rng_state);
			tourney = {};
			tourney.append(population);
			tourney.shuffle(rng_state);
		}

		// ROUND ROBIN
		query = ImGui::display(round_robin, ui_state);
		ui_state.n_pop = query.n_pop;
		ui_state.n_best = query.n_best;
		if (query.did_clicked_agent) {
			auto agent = round_robin.agents[query.clicked_agent];
			opened_agent = dynamic_cast<Table_Agent*>(agent);
		}
		if (query.toggle_next_gen) autorun_gen_round_robin ^= true;
		if (query.run_season || autorun_gen_round_robin) {
			round_robin.play_n(ui_state.n_pop, 6, rng_state);
		}
		if (query.next_gen || autorun_gen_round_robin) {
			round_robin.agents.resize(ui_state.n_best);
			repopulate(population, round_robin.agents, rng_state);
			round_robin = {};
			round_robin.append(population);
		}


		// Rendering
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);

		ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

		glfwMakeContextCurrent(window);
		glfwSwapBuffers(window);
	}

	// Cleanup
	ImGui_ImplOpenGL2_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
