#pragma once
#include "u_sys/config.h"


// k === [0;1]
// k>>0 - smooth, k>>1 - hard
class UFO_FastExpoFilter
{
private:
	float _val = 0.f;
	float _k = 0.0f;

public:
	UFO_FastExpoFilter(/* args */) {}
	~UFO_FastExpoFilter() {}

	bool Set(int8_t k)
	{
		if (k < 0) 
		{
			return false;
		}
		_k = k;
		return true;
	}

	float operator()(float newVal)
	{
		_val += (newVal - _val) * _k;
		return _val;
	}
};