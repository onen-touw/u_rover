#pragma once

#include "u_sys/config.h"
#include "u_sys/mutex.h"
#include "u_sys/btflg.h"

namespace app
{
   enum remote_cmd_e : uint8_t {
        null,
        
        ask,
        req,
        
        arm,
        servo,
        trpy,
        find_mode,

    };
    // =========================== events ===========================
    enum class app_event_e
    {
        null,

        idle,
        control,
        alarm,
    };

    enum class app_event_control_e
    {
        idle,      // idle
        trpy,      // throt/roll/pitch/yaw
        arm_state, // arm/disarm cmd
        tunm,      // tumblers
        pot,       // potenciometrs
    };

    enum class app_event_alarm_e
    {
        null,

        battery,
        battery_crit,
        disconn,
        find_mode,

        warning,
        critical,
    };
    // ==============================================================


    // this file describes only data structures for
    // sensors/devices and other in simple types
        namespace types
        {
            struct app_cmd_queue_t
            {
                enum class cmd_t
                {
                    null,
                    req,

                    arm,
                    disarm,
                    find_on,
                    find_off,
                };

                QueueHandle_t _q = nullptr;

                app_cmd_queue_t()
                {
                    _q = xQueueCreate(2, sizeof(cmd_t));
                    if (!_q)
                    {
                        //
                    }
                }
                ~app_cmd_queue_t()
                {
                    if (_q)
                    {
                        vQueueDelete(_q);
                    }
                }
            };

            template <typename Ty>
            struct event_base_t
            {
                Ty _event;
                Ty _prev;

                void set(Ty e)
                {
                    _prev = _event;
                    _event = e;
                }

                Ty get() const
                {
                    return _event;
                }

                Ty get_prev() const
                {
                    return _prev;
                }

                void back()
                {
                    std::swap(_event, _prev);
                }

                bool operator==(const Ty &other)
                {
                    return _event == other;
                }
            };

            struct event_t
            {
                // ufo::mutex_t _lock;
                event_base_t<app_event_e> _app = {};
                event_base_t<app_event_control_e> _control = {};
                event_base_t<app_event_alarm_e> _alarm = {};

                // event_subj_t<app_event_calibrate_e> _subj_calibrate = {};
            };

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

                uint16_t _rpwm =0;
                uint16_t _lpwm =0;
            };

            struct control_sig_t
            {
                int16_t _throt = 0.f;
                int16_t _roll = 0.f;
                int16_t _pitch = 0.f;
                int16_t _yaw = 0.f;
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
