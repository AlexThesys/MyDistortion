#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>

#include "constants.h"

#define M_PI_4 0.785398163397448309616f  // pi/4



typedef union fp32_to_u32 {
	float f;
	uint32_t u;
} fp32_to_u32;

inline float fast_atan(float x) {
	fp32_to_u32 f2u;
	f2u.f = x;
	f2u.u &= 0x7FFFFFFF;
	return M_PI_4 * x - x * (f2u.f - 1.0f) * (0.2447f + 0.0663f * f2u.f);

	//const int abs_x_bits = *(uint32_t*)&x & 0x7FFFFFFF;
	//const float abs_x = *(float*)&abs_x_bits;
	//return M_PI_4 * x - x * (abs_x - 1.0f) * (0.2447f + 0.0663f * abs_x);
}

#define fast_atan_simd(x) \
abs_x = _mm_and_ps(x, *(__m128*)&not_sign_bit);\
temp = _mm_sub_ps(abs_x, one);\
temp = _mm_mul_ps(temp, x);\
x = _mm_mul_ps(x, pi_4);\
abs_x = _mm_mul_ps(abs_x, b);\
abs_x = _mm_add_ps(abs_x, a);\
temp = _mm_mul_ps(temp, abs_x);\
x = _mm_sub_ps(x, temp);\

void waveshaper(float* in, float* out, int buf_len, const params p) {
	for (int i = 0; i < buf_len; i++) {
		float sample = in[i];
		for (int j = 0; j < p.num_stages; j++) {
			const int32_t mask = *(int32_t*)&sample >> 0x1f;
			fp32_to_u32 coeff;
			coeff.u = (~mask & *(uint32_t*)&p.coef_pos) | (mask & *(uint32_t*)&p.coef_neg);
			sample = (1.0f / fast_atan(coeff.f)) * fast_atan(coeff.f * sample);
			const uint32_t inverted = *(uint32_t*)&sample ^ (0x80000000 & ~((p.invert_stages & j) - 0x01));
			sample = *(float*)&inverted;
		}
		sample *= p.gain;
		out[i] = sample;
	}
}

void waveshaper_simd(float* in, float* out, int buf_len, const params p) {
	const int buf_len_simd = buf_len & ~0x03;
	const __m128i not_sign_bit = _mm_set1_epi32(0x7FFFFFFF);
	const __m128 c_pos = _mm_set1_ps(p.coef_pos);
	const __m128 c_neg = _mm_set1_ps(p.coef_neg);
	const __m128i not_mask = _mm_set1_epi32(0xFFFFFFFF);
	const __m128 pi_4 = _mm_set1_ps(M_PI_4);
	const __m128 one = _mm_set1_ps(1.0f);
	const __m128 a = _mm_set1_ps(0.2447f);
	const __m128 b = _mm_set1_ps(0.0663f);
	const __m128 gain = _mm_set1_ps(p.gain);

	// process
	for (int i = 0; i < buf_len_simd; i += 4) {
		__m128 sample = _mm_load_ps(&in[i]);
		for (int j = 0; j < p.num_stages; j++) {
			__m128i mask = _mm_srai_epi32(*(__m128i*) & sample, 0x1f);
			__m128 coef = _mm_and_ps(*(__m128*) & mask, c_neg);
			mask = _mm_xor_si128(mask, not_mask);
			mask = _mm_and_si128(mask, *(__m128i*) & c_pos);
			coef = _mm_or_ps(coef, *(__m128*) & mask);
			sample = _mm_mul_ps(sample, coef);
			__m128 abs_x, temp;
			fast_atan_simd(coef);
			coef = _mm_div_ps(one, coef);
			fast_atan_simd(sample);
			sample = _mm_mul_ps(sample, coef);
			const uint32_t invert = 0x80000000 & ~((p.invert_stages & j) - 0x01);
			const __m128i inv = _mm_set1_epi32(invert);
			sample = _mm_xor_ps(sample, *(__m128*) & inv);
		}
		sample = _mm_mul_ps(sample, gain);
		_mm_store_ps(&out[i], sample);
	}

	// process the rest
	for (int i = buf_len_simd; i < buf_len; i++) {
		float sample = in[i];
		for (int j = 0; j < p.num_stages; j++) {
			const int32_t mask = *(int32_t*)&sample >> 0x1f;
			fp32_to_u32 coeff;
			coeff.u = (~mask & *(uint32_t*)&p.coef_pos) | (mask & *(uint32_t*)&p.coef_neg);
			sample = (1.0f / fast_atan(coeff.f)) * fast_atan(coeff.f * sample);
			const uint32_t inverted = *(uint32_t*)&sample ^ (0x80000000 & ~((p.invert_stages & j) - 0x01));
			sample = *(float*)&inverted;
		}
		sample *= p.gain;
		out[i] = sample;
	}
}