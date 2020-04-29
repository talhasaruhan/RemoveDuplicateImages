#pragma once

#include <cstdint>
#include <memory.h>
#include <cassert>
#include <string>

#define POOL_SIZE 8*1024*1024*1024

struct MemoryPool
{
	uint8_t* data = nullptr;
	size_t cur = 0;
	size_t pool_size = POOL_SIZE;
};

void Init(MemoryPool& memory_pool)
{
	memory_pool.pool_size = POOL_SIZE;
	memory_pool.data = (uint8_t*)malloc(memory_pool.pool_size);
}

void Uninit(MemoryPool& memory_pool)
{
	free(memory_pool.data);
}

void* Alloc(MemoryPool& memory_pool, size_t size)
{
	uint8_t* ptr = memory_pool.data;
	memory_pool.cur += size;
	memory_pool.data += size;
	assert(memory_pool.cur <= memory_pool.pool_size);
	return ptr;
}

void Free(MemoryPool& memory_pool, void* ptr)
{
	memory_pool.data -= memory_pool.cur;
	memory_pool.cur = 0;
}

