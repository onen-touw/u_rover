#pragma once

#include "appdata.h"
#include "apptypes.h"
#include "appconfig.h"

#include "u_sys/utils.h"
#include "u_sys/mutex.h"
#include "u_sys/cns.h"
#include "sens.h"


namespace app
{
    namespace console
    {
        void console_echo(ufo::cns::console_t::block_t &block)
        {
            using namespace ufo;
            vector_t<string_t> &arg_list = block->get_buf();

            if (!arg_list.empty())
            {
                if (arg_list.size() > 1)
                {
                    cns::opt_t opt(arg_list[1]);

                    uint16_t f = 50;
                    if (opt.arg_count() == 1)
                    {
                        f = opt.get_arg<uint16_t>(0);
                        if (!f || f > 1000)
                        {
                            f = 50;
                        }
                        block->fwrite("change freq to %ums\n", f);
                    }

                    if (opt == "remote")
                    {
                        while (!block->is_read_out_signal())
                        {
                            auto d = app::__global_app_data._control_sig.get_data();
                            using m_t = types::control_sig_rover_t::mode_t;

                            if (d._mode == m_t::angle)
                            {
                                block->fwrite("mode:ang >alpha:%.3f\n>power:%d\n\n", d._angle._angle, d._angle._throt);
                            }
                            else if (d._mode == m_t::throt)
                            {
                                block->fwrite("mode:thr >left:%.3f\n>right:%d\n\n", d._throt._l, d._throt._r);
                            }
                            else
                            {
                                block->fwrite("mode:none\n\n");
                            }
                            utl::sleep_for(f);
                        }
                        block->write("echo stop\n");
                        return;
                    }
                    else if (opt == "imu")
                    {
                        while (!block->is_read_out_signal())
                        {
                            auto d = app::__global_app_data._imu.get_data();
                            block->fwrite(">r:%.3f\n>p:%.3f\n>y:%.3f\n\n",
                                          d._ang._r,
                                          d._ang._p,
                                          d._ang._y);
                            utl::sleep_for(f);
                        }
                        block->write("echo stop\n");
                        return;
                    }
                    else if (opt == "barometer")
                    {
                        while (!block->is_read_out_signal())
                        {
                            auto d = app::__global_app_data._baro.get_data();
                            block->fwrite(">Pr:%.3f\n>Te:%.3f\n\n",
                                          d._p,
                                          d._t);
                            utl::sleep_for(f);
                        }
                        block->write("echo stop\n");
                        return;
                    }
                    else if (opt == "motors")
                    {
                        while (!block->is_read_out_signal())
                        {
                            auto d = app::__global_app_data._rover.get_data();
                            block->fwrite(">L:%.3f\t>Lp:%d\n>R:%.3f\t>Lr:%d\n\n",
                                          d._sig._ll,
                                          d._pwm._lpwm,
                                          d._sig._rr,
                                          d._pwm._rpwm);
                            utl::sleep_for(f);
                        }
                        block->write("echo stop\n");
                        return;
                    }
                    else if (opt == 'h' || opt == "h" || opt == "help")
                    {
                        block->write("echo --<module>={freq}  freq-optional frequency value\n");
                        block->write("available moduls:\n\tbarometer\n\timu\n\tremote\n\tmotors\n");
                        block->write("'-h / --h / --help' for see this\n\n");
                        return;
                    }
                }
                block->log_incorrect_arg();
            }
        }

        void console_motors(ufo::cns::console_t::block_t &block)
        {

            using namespace ufo;
            vector_t<string_t> &arg_list = block->get_buf();

            if (!arg_list.empty())
            {
                if (arg_list.size() > 1)
                {
                    cns::opt_t opt(arg_list[1]);
                    if (opt == 'p' || opt == "param")
                    {
                        using m_t = types::control_sig_rover_t::mode_t;
                        m_t mode = m_t::none;

                        if (arg_list.size() > 2)
                        {
                            cns::opt_t mode_opt(arg_list[2]);
                            if (mode_opt =="angle")
                            {
                                mode = m_t::angle;

                                //temp
                                block->write("angle currently not implemented\n");
                                return;
                            }
                            else if (mode_opt == "throt")
                            {
                                mode = m_t::throt;
                            }
                            else
                            {
                                block->write("incorrect mode use --<angle/throt>\n");
                                return;
                            }
                        }

                        if (arg_list.size() > 3)
                        {
                            cns::opt_t mot_opt(arg_list[3]);

                            char m = '\0'; // get char: q/w/s/a
                            float val = 0.f;
                            char force = '\0';

                            // check force-mode
                            if (arg_list.size() > 4)
                            {
                                cns::opt_t force_opt(arg_list[4]);
                                if (force_opt == 'y')
                                {
                                    force = 'y';
                                }
                                else if (force_opt == "force")
                                {
                                    force = 'f';
                                }
                                else
                                {
                                    block->fwrite("incorrect force-mode arg\n");
                                    return;
                                }
                            }

                            if (mode == m_t::throt)
                            {
                                // parse second -arg
                                {
                                    if (mot_opt == 'a' || mot_opt == "left") // --a / --left == left track
                                    {
                                        m = 'a';
                                    }
                                    else if (mot_opt == 'd' || mot_opt == "right") // -d / --right == left track
                                    {
                                        m = 'd';
                                    }
                                    else if (mot_opt == 'w' || mot_opt == "both") // -w / --both == both
                                    {
                                        m = 'w';
                                    }
                                    else if (mot_opt == 'r' || mot_opt == "rst") // -r / --reset == reset
                                    {
                                        m = 'r';
                                    }
                                    else
                                    {
                                        block->log_incorrect_arg();
                                        return;
                                    }
                                }

                                // check if param exists after -arg name
                                if (mot_opt.arg_count() != 1)
                                {
                                    // if reset continue
                                    if (m != 'r' && mot_opt.arg_count() != 0)
                                    {
                                        block->log_incorrect_arg();
                                        return;
                                    }
                                }

                                // parse value for motor power
                                val = mot_opt.get_arg<float>(0);
                                if (abs(val) > 1.f)
                                {
                                    block->write("val out of range [-1.0 .. 1.0]\n");
                                    return;
                                }
                                else if (abs(val) > 0.2f)
                                {
                                    if (force == '\0')
                                    {
                                        block->write("val will set to 20%. write <y / f / ' '> (0.2*val / val / out) for continue\n");
                                        while (true)
                                        {
                                            string_t sss = block->read();
                                            if (sss.size())
                                            {
                                                if (sss[0] == 'y')
                                                {
                                                    force = 'y';
                                                    break;
                                                }
                                                else if (sss == "f")
                                                {
                                                    force = 'f';
                                                    break;
                                                }
                                                else
                                                {
                                                    block->write("cancel\n");
                                                    return;
                                                }
                                                
                                            }
                                            utl::sleep_for(50);
                                        }
                                    }
                                }
                                else {
                                    force = 'p'; // set not \0
                                }

                                if (force == '\0')
                                {
                                    block->log_incorrect_arg();
                                    return;
                                }
                                else if (force == 'y')
                                {
                                    val = utl::constrain(val, -0.2f, 0.2f);
                                }
                                else
                                {
                                    val = utl::constrain(val, -1.f, 1.f);
                                }

                                // todo translate to new distance of pwm [0, x]

                                lock_guard<mutex_t> _l(__global_app_data._control_sig._lock);

                                switch (m)
                                {
                                case 'a':
                                    block->fwrite("set left track to %.3f\n", val);
                                    __global_app_data._control_sig._data._throt._l = val;
                                    __global_app_data._control_sig._data._throt._r = 0.f;
                                    break;
                                case 'd':
                                    block->fwrite("set right track to %.3f\n", val);
                                    __global_app_data._control_sig._data._throt._l = 0.f;
                                    __global_app_data._control_sig._data._throt._r = val;
                                    break;
                                case 'w':
                                    block->fwrite("set both track to %.3f\n", val);
                                    __global_app_data._control_sig._data._throt._l = val;
                                    __global_app_data._control_sig._data._throt._r = val;
                                    break;
                                default:
                                    block->write("reset\n");
                                    __global_app_data._control_sig._data._throt._l = 0.f;
                                    __global_app_data._control_sig._data._throt._r = 0.f;
                                    break;
                                }
                                return;
                            }
                            else
                            {
                                block->write("angle mode currently not implemented\n");
                                return;
                            }
                            
                        }
                        block->log_incorrect_arg();
                        return;
                    }
                    else if (opt == 'h' || opt == "help" || opt == "h")
                    {
                        block->write("motors '-p / --param' --<angle/throt> '-w / -a / -s / -r'={-1.0 .. 1.0} '--y / --force / '\n ");
                        block->write("\tcomand: 'motors --param --throt --left=0.1' will set 10'%' of throt to left track\n");

                        block->write("'-h / --h / --help' for see this\n\n");
                        return;
                    }
                }
            }
        }

        void console_imu(ufo::cns::console_t::block_t &block){
            using namespace ufo;
            vector_t<string_t> &arg_list = block->get_buf();

            if (!arg_list.empty())
            {
                if (arg_list.size() > 1)
                {
                    cns::opt_t opt(arg_list[1]);

                    if (opt == "info")
                    {
                        auto c = app::__global_app_data._imu.get_calibration();
                        auto d = app::__global_app_data._imu.get_data();
                        block->fwrite("calibration\n\tgyro:\nx:%.3f y:%.3f z:%.3f\n",
                            c._gyro._x,
                            c._gyro._y,
                            c._gyro._z
                        );
                        block->fwrite("\tacsel:\nx:%.3f y:%.3f z:%.3f\n\n",
                            c._acs._x,
                            c._acs._y,
                            c._acs._z
                        );

                        block->fwrite("last data:\n\tgyro:\nx:%.3f y:%.3f z:%.3f\n",
                            d._raw._gyro._x,
                            d._raw._gyro._y,
                            d._raw._gyro._z
                        );
                        block->fwrite("\tacsel:\nx:%.3f y:%.3f z:%.3f\n",
                            d._raw._acs._x,
                            d._raw._acs._y,
                            d._raw._acs._z
                        );         
                        block->fwrite("\tangles:\nr:%.3f p:%.3f y:%.3f\n\n",
                            d._ang._r,
                            d._ang._p,
                            d._ang._y
                        );         
                        return;
                    }
                    else if (opt == "calibrate")
                    {
                        uint16_t it = 1;
                        if (opt.arg_count() == 1)
                        {
                            uint16_t i = opt.get_arg<uint16_t>(0);
                            if (!i || i > 10)
                            {
                                block->fwrite("iterations count [1, 10]. using default: %u\n", it);
                            }
                            else {
                                it = i;
                                block->fwrite("iterations count set to %u\n", it);
                            }
                        }

                        block->fwrite("start calibrating it(&u)\n", it);
                        block->fwrite("keep board on floor and dont touch it\n");

                        ufo::thread_cfg cfg_imu;
                        cfg_imu._name = "imu";
                        cfg_imu._core = 0;
                        cfg_imu._prio = 5;
                        cfg_imu._stackSize = 4096;

                        ufo::thread t(cfg_imu, task_imu_calibrate, __global_system_data._drv._i2c.get(), it);
                        t.join();

                        auto c = app::__global_app_data._imu.get_calibration();
                        block->fwrite("calibration\n\tgyro:\nx:%.3f y:%.3f z:%.3f\n",
                            c._gyro._x,
                            c._gyro._y,
                            c._gyro._z
                        );

                        block->fwrite("\tacsel:\nx:%.3f y:%.3f z:%.3f\n\n",
                            c._acs._x,
                            c._acs._y,
                            c._acs._z
                        );
                        return;
                    }
                }
            }
        }

    } // namespace console
} // namespace app
