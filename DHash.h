#pragma once

#include <cstdint>
#include <immintrin.h>
#include "Image.h"

inline uint8_t Pack8U8(uint8_t* a)
{
	return _pext_u64(*((uint64_t*)a), 0x0101010101010101ULL);
}

uint64_t DHash(const Image& image)
{
	alignas(32) uint8_t resized_9x8[72];

	stbir_resize_uint8(image.data, image.width, image.height, 0, resized_9x8, 8, 9, 0, 1);

	// Load 0 1 2 3 each row is 8x8
	__m256i rows_0_0 = _mm256_load_si256((__m256i*)&resized_9x8[0]);
	// Load 1 2 3 4 each row is 8x8
	__m256i rows_0_1 = _mm256_load_si256((__m256i*)&resized_9x8[8]);
	// Load 5 6 7 8 each row is 8x8
	__m256i rows_1_0 = _mm256_load_si256((__m256i*)&resized_9x8[16]);
	// Load 6 7 8 9 each row is 8x8
	__m256i rows_1_1 = _mm256_load_si256((__m256i*)&resized_9x8[24]);

	__m256i res_0_0 = _mm256_cmpgt_epi8(rows_0_0, rows_0_1);
	__m256i res_1_0 = _mm256_cmpgt_epi8(rows_1_0, rows_1_1);

	alignas(32) uint8_t res_0_1[32];
	alignas(32) uint8_t res_1_1[32];

	_mm256_store_si256((__m256i*)&res_0_1[0], res_0_0);
	_mm256_store_si256((__m256i*)&res_1_1[0], res_1_0);

	union {
		struct {
			uint8_t l0;
			uint8_t l1;
			uint8_t l2;
			uint8_t l3;
			uint8_t l4;
			uint8_t l5;
			uint8_t l6;
			uint8_t l7;
		};
		uint64_t u64;
	} dhash;

	dhash.l0 = Pack8U8(&res_0_1[0]);
	dhash.l1 = Pack8U8(&res_0_1[8]);
	dhash.l2 = Pack8U8(&res_0_1[16]);
	dhash.l3 = Pack8U8(&res_0_1[24]);
	dhash.l4 = Pack8U8(&res_1_1[0]);
	dhash.l5 = Pack8U8(&res_1_1[8]);
	dhash.l6 = Pack8U8(&res_1_1[16]);
	dhash.l7 = Pack8U8(&res_1_1[24]);

	return dhash.u64;
}

uint32_t HammingDistance(uint64_t a, uint64_t b)
{
	return __popcnt64(a ^ b);
}
