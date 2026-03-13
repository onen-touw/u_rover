#pragma once

#include "u_sys/config.h"
#include "u_sys/mutex.h"
#include "u_sys/btflg.h"
#include "u_math/u_math.h"

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
                    _q = xQueueCreate(5, sizeof(cmd_t));
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
                // event_base_t<app_event_control_e> _control = {};
                event_base_t<app_event_alarm_e> _alarm = {};
            };

            struct eu_angle_t {
                float _r = 0.f;
                float _p = 0.f;
                float _y = 0.f; 
            };

            
            struct rover_mot_t
            {
                struct data_t
                {
                    struct pwm_t
                    {
                        uint16_t _rpwm = 0;
                        uint16_t _lpwm = 0;
                    };

                    struct sig_t
                    {
                        float _rr = 0.f;
                        float _ll = 0.f;
                    };
                    pwm_t _pwm = {};
                    sig_t _sig = {};
                };
                
                mutable ufo::mutex_t _lock;
                data_t _data = {};

                data_t get_data() const {
                    ufo::lock_guard<ufo::mutex_t>lock(_lock);
                    data_t d = _data;
                    return d;
                }
                
            };

            struct control_sig_drone_t
            {
                struct data_t
                {
                    int16_t _throt = 0.f;
                    eu_angle_t _ang_sig = {};
                };

                mutable ufo::mutex_t _lock;
                data_t _data = {};

                data_t get_data() const {
                    ufo::lock_guard<ufo::mutex_t>lock(_lock);
                    data_t d = _data;
                    return d;
                }
            };


            struct control_sig_rover_t
            {
                struct throt_ctrl_t {
                    float _l = 0.f;
                    float _r = 0.f;
                };
                struct angle_ctrl_t {
                    float _throt = 0.f;
                    float _angle = 0.f;
                };
                
                enum class mode_t {
                    none,
                    angle,
                    throt,
                };

                struct data_t
                {
                    mode_t _mode = mode_t::none;
                    throt_ctrl_t _throt = {};
                    angle_ctrl_t _angle = {};
                };

                mutable ufo::mutex_t _lock;
                data_t _data = {};

                data_t get_data() const {
                    ufo::lock_guard<ufo::mutex_t>lock(_lock);
                    data_t d = _data;
                    return d;
                }
            };

            struct imu_data_t
            {
                struct calibration_t
                {
                    Vector3<float> _gyro = {};
                    Vector3<float> _acs = {};
                };

                struct raw_t{
                    Vector3<float> _gyro = {};
                    Vector3<float> _acs = {};
                };

                struct data_t {
                    raw_t _raw = {};
                    eu_angle_t _ang = {};
                };
                
                mutable ufo::mutex_t _lock;
                data_t _data = {};
                calibration_t _calib = {};

                data_t get_data() const {
                    ufo::lock_guard<ufo::mutex_t>lock(_lock);
                    data_t d = _data;
                    return d;
                }

                calibration_t get_calibration() const {
                    ufo::lock_guard<ufo::mutex_t>lock(_lock);
                    calibration_t c = _calib;
                    return c;
                }
            };

            struct bar_data_t
            {

                struct calibration_t
                {
                    float _t = 0.f;
                    float _p = 0.f;
                };

                struct data_t 
                {
                    float _p = 0.f;
                    float _t = 0.f;
                };

                mutable ufo::mutex_t _lock;
                data_t _data = {};
                calibration_t _calib = {};


                data_t get_data() const {
                    ufo::lock_guard<ufo::mutex_t>lock(_lock);
                    data_t d = _data;
                    return d;
                }

                calibration_t get_calibration() const
                {
                    ufo::lock_guard<ufo::mutex_t> lock(_lock);
                    calibration_t c = _calib;
                    return c;
                }
            };
        } // namespace types
} // namespace app
