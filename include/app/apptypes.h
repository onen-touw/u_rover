#pragma once

#include "u_sys/config.h"
#include "u_sys/mutex.h"
#include "u_sys/btflg.h"

namespace app
{
    // this file describes only data structures for
    // sensors/devices and other in simple types
        namespace types
        {
            enum class mot_cmd_t
            {
                mot_no,
                mot_set_arm,
                mot_set_disarm,
                mot_vals,
                mot_valscns, // t== 1, r == 2, p == 3, y == 4
            };

            struct mot_t
            {
                uint16_t _m1 = 0;
                uint16_t _m2 = 0;
                uint16_t _m3 = 0;
                uint16_t _m4 = 0;
            };

            struct rover_mot_t
            {
                float _rr = 0.f;
                float _ll = 0.f;
            };

            struct remote_data_t
            {
                float _throt = 0.f;
                float _roll = 0.f;
                float _pitch = 0.f;
                float _yaw = 0.f;

                mot_cmd_t _mcmd = mot_cmd_t::mot_no;

                ufo::mutex_t _lock;
            };

            struct imu_data_t
            {
                // add calibration
                float _r = 0.f;
                float _p = 0.f;
                float _y = 0.f;
                ufo::mutex_t _lock;
            };

            struct bar_data_t
            {
                // add calibration
                float _p = 0.f;
                float _t = 0.f;
                ufo::mutex_t _lock;
            };
        } // namespace types
} // namespace app
