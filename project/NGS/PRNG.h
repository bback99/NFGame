#ifndef __PRNG_H__
#define __PRNG_H__


#include <cstddef>

// Windows does not have C99 <stdint.h> types..
typedef __int8           int8_t;
typedef unsigned __int8  uint8_t;
typedef __int16          int16_t;
typedef unsigned __int16 uint16_t;
typedef __int32          int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64          int64_t;
typedef unsigned __int64 uint64_t;


//
// Pseudo RNG
// = 결정론적 슈도 랜덤 생성기
//
class PRNG
{
public:
	PRNG();
	PRNG(uint32_t seed);                                
	virtual ~PRNG();

	// 랜덤 넘버 생성: pCount != NULL이면 GetCount()를 기록
	virtual uint32_t Generate(uint32_t *pCount = NULL);

	// 지정된 seed로 reseeding
	void Reseed(uint32_t seed);

	// Seeding 이후 생성된 랜덤 넘버 개수를 리턴
	uint32_t GetCount() const;

	uint32_t GetValue() const;

protected:
	uint32_t count_;
	uint32_t number_;
};


#endif//__PRNG_H__
