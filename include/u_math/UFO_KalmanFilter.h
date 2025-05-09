#pragma once
#include "u_sys/config.h"


class UFO_KalmanFilter
{
private:
    float _measureErr = 0.0; // примерный шум измерений
    float
        _err_estimate = 0.f,
        _lasEstimate = 0.f,
        _q = 0.f;

public:
    UFO_KalmanFilter(/* args */) {}
    ~UFO_KalmanFilter() {}
    float operator()(float val)
    {
        float kalman_gain = _err_estimate / (_err_estimate + _measureErr);
        float current_estimate = _lasEstimate + kalman_gain * (val - _lasEstimate);
        _err_estimate = (1.0 - kalman_gain) * _err_estimate + fabs(_lasEstimate - current_estimate) * _q;
        _lasEstimate = current_estimate;
        return current_estimate;
    }

    // разброс измерения, разброс оценки, скорость изменения значений
    // q === [0.001-1]
    void Set(float mea_e, float est_e, float q)
    {
        _measureErr = mea_e;
        _err_estimate = est_e;
        _q = q;
    }
};
