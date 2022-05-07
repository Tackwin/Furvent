#include "random.hpp"


uint32_t xorshift128(xorshift128_state& state) noexcept {
	/* Algorithm "xor128" from p. 5 of Marsaglia, "Xorshift RNGs" */
	uint32_t t  = state.x[3];

	uint32_t s  = state.x[0];  /* Perform a contrived 32-bit shift. */
	state.x[3] = state.x[2];
	state.x[2] = state.x[1];
	state.x[1] = s;

	t ^= t << 11;
	t ^= t >> 8;
	return state.x[0] = t ^ s ^ (s >> 19);
}

double uniform(xorshift128_state& state) noexcept {
	return xorshift128(state) / UINT32_MAX;
}