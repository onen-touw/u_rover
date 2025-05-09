#pragma once
#include "u_sys/config.h"


class UFO_AdaptiveFilter
{
private:
    float _val = 0.f;
    float _k = 0.f;
    float _kBase = 0.1f;
    float _kPick = 0.9f;
    float _tresh = 1.5f;

public:
    UFO_AdaptiveFilter(/* args */) {}
    ~UFO_AdaptiveFilter() {}

    bool Set(float kBase, float kPick, float treshhold)
	{
		if (kBase < 0 || kBase > 1)
		{
			return false;
		}
        _kBase = kBase;
        if (kPick < 0 || kPick > 1)
		{
			return false;
		}
        _kPick = kPick;
        _tresh = treshhold;
		return true;
	}

    float operator()(float newVal){
        if (abs(newVal - _val) > _tresh){
            _k = _kPick;
        }
        else {
            _k = _kBase;
        }

        _val += (newVal - _val) * _k;
        return _val;
    }
};