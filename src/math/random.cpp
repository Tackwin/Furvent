#include "random.hpp"

#include <math.h>


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

void import_seed(xorshift128_state& state, uint8_t b64[22]) noexcept {
	constexpr uint8_t table[] ={ 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
								 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
								 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
								 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
								 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
								 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
								 'w', 'x', 'y', 'z', '0', '1', '2', '3',
								 '4', '5', '6', '7', '8', '9', '+', '/' };

	for (auto& x : state.x) x = 0;

	size_t b = 0;
	for (size_t i = 0; i < 22; ++i) {

		size_t n = 0;
		for (size_t j = 0; j < 64; ++j) if (b64[i] == table[j]) {
			n = j;
			break;
		}

		size_t idx = b / 32;
		size_t shift = b % 32;
		state.x[idx] |= n << shift;

		b += 4;
	}
}

void export_seed(xorshift128_state& state, uint8_t b64[22]) noexcept {
	constexpr uint8_t table[] ={ 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
								 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
								 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
								 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
								 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
								 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
								 'w', 'x', 'y', 'z', '0', '1', '2', '3',
								 '4', '5', '6', '7', '8', '9', '+', '/' };

	size_t b = 0;
	for (size_t i = 0; i < 22; ++i) {
		size_t idx = b / 32;
		size_t shift = b % 32;

		size_t n = (state.x[idx] >> shift) & 0b1111;
		b64[i] = table[n];
		b += 4;
	}
}

double uniform(xorshift128_state& state) noexcept {
	return xorshift128(state) / (double)UINT32_MAX;
}

double normal(xorshift128_state& state) noexcept {
	double x = 0;
	double y = 0;
	double s = 0;
	do {
		x = uniform(state) * 2 - 1;
		y = uniform(state) * 2 - 1;
		s = x + y;
	} while(s < 0 || s > 1);

	return x / sqrt(-2 * log(s) / s);
}