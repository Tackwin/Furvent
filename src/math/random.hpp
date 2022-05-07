#pragma once

#include <stdint.h>

struct xorshift128_state {
	uint32_t x[4];
};

/* The state array must be initialized to not be all zero */
extern uint32_t xorshift128(xorshift128_state& state) noexcept;
extern double uniform(xorshift128_state& state) noexcept;

struct Uniform_uint32_t {
	using result_type = uint32_t;
	static constexpr result_type min() noexcept { return 0; };
	static constexpr result_type max() noexcept { return UINT32_MAX; };

	xorshift128_state& state;
	
	result_type operator()() { return xorshift128(state); }
};

using RNG_State = xorshift128_state;