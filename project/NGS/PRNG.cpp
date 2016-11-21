#include <stdafx.h>
#include "PRNG.h"

PRNG::PRNG()
{
	count_ = 0;
	number_ = 0;
}

PRNG::PRNG(uint32_t seed)
{
	Reseed(seed);
}

PRNG::~PRNG()
{
}

uint32_t PRNG::Generate(uint32_t *pCount /*= NULL*/)
{
	// 선형 합동 생성기: 가장 간단하고 빠르지만 품질은 별로임.
	// * http://en.wikipedia.org/wiki/Linear_congruential_generator
	// * (m, a, c) = (2^48, 25214903917, 11), 하위 16 bit 버림 => Random class in Java API

	number_ = static_cast<uint32_t>((static_cast<uint64_t>(number_) * 25214903917 + 11) >> 16);

	++count_;
	if (pCount)
		*pCount = count_;

	return number_;
}

void PRNG::Reseed(uint32_t seed)
{
	count_ = 0;
	number_ = seed;
}

uint32_t PRNG::GetCount() const
{
	return count_;
}

uint32_t PRNG::GetValue() const
{
	return number_;
}

