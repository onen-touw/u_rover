#pragma once

#include "u_drivers/pwm/UFO_ESC.h"
#include "u_sys/gpio.h"
#include "u_sys/utils.h"
#include "u_sys/error.h"
#include "u_sys/trace.h"


class mpwm_t
{
public:
    static constexpr gpio_num_t mot_left = gpio_num_t::GPIO_NUM_14;
    static constexpr gpio_num_t mot_right = gpio_num_t::GPIO_NUM_27;
    
    static constexpr gpio_num_t mot_right_ctrl_pin1 =gpio_num_t::GPIO_NUM_33; 
    static constexpr gpio_num_t mot_right_ctrl_pin2 =gpio_num_t::GPIO_NUM_32; 
    static constexpr gpio_num_t mot_left_ctrl_pin1 = gpio_num_t::GPIO_NUM_26;
    static constexpr gpio_num_t mot_left_ctrl_pin2 = gpio_num_t::GPIO_NUM_25;
    
    
    static constexpr float throt_threshhold = 0.3f;
    static constexpr float rot_threshhold = 0.3f;
    static constexpr float rot_force = 0.5f;
    static constexpr float rotride_force = 0.9f;

public:
    using motor_t = ufo::drv::UFO_ESC_driver;

private:
    motor_t _ml;
    motor_t _mr;

    float
        ll_out = 0.f,
        rr_out = 0.f;
    uint16_t
            l_out = 0,
            r_out = 0;

public:

    mpwm_t()
    {
        _ml.setup(0, mot_left);
        _mr.setup(1, mot_right);

        ufo::utl::gpio_config(mot_right_ctrl_pin1, gpio_mode_t::GPIO_MODE_OUTPUT);
        ufo::utl::gpio_config(mot_right_ctrl_pin2, gpio_mode_t::GPIO_MODE_OUTPUT);
        ufo::utl::gpio_config(mot_left_ctrl_pin1, gpio_mode_t::GPIO_MODE_OUTPUT);
        ufo::utl::gpio_config(mot_left_ctrl_pin2, gpio_mode_t::GPIO_MODE_OUTPUT);

        gpio_set_level(mot_right_ctrl_pin1, 0);
        gpio_set_level(mot_right_ctrl_pin2, 0);
        gpio_set_level(mot_left_ctrl_pin1, 0);
        gpio_set_level(mot_left_ctrl_pin2, 0);
    }
    ~mpwm_t() {}

    void target_write(float m1, float m2){
        left_front();
        right_front();
        _ml.write(ufo::utl::map(m1, 0.f, 1.f, 0.f, static_cast<float>(motor_t::pwm_max_out)));
        _mr.write(ufo::utl::map(m2, 0.f, 1.f, 0.f, static_cast<float>(motor_t::pwm_max_out)));
    }

    float get_mot_throt_l() const
    {
        return ll_out;
    }
    float get_mot_throt_r() const 
    {
        return rr_out;
    }

    uint16_t get_rpwm() const 
    {
        return r_out;
    }
    uint16_t get_lpwm() const
    {
        return l_out;
    }

    void update(float t, float r)
    {
        // t := [-1.f, 1.f];
        // r := [-1.f, 1.f]
        // printf("in:\n\t tt:%.3f, rr:%.3f\n", t, r);               


        ll_out = 0.f;
        rr_out = 0.f;
        
        l_out = 0;
        r_out = 0;

        if (abs(t) > throt_threshhold)  // t
        {
            ll_out = t;
            rr_out = t;
            if (abs(r) > rot_threshhold)    // +- r
            {   
                // printf("ride-rot\n");

                ll_out += r * rotride_force;
                rr_out -= r * rotride_force;
            }
            if (t > 0.f)
            {
                left_front();
                right_front();
            }
            else {
                left_back();
                right_back();
            }
        }
        else {

            if (abs(r) > rot_threshhold)    // t = r
            {   
                // printf("rotrot\n");
                ll_out = r * rot_force;
                rr_out = r * rot_force;

                if (r > 0)
                {
                    left_front();
                    right_back();
                }
                else {
                    left_back();
                    right_front();
                }
            }
            else {

                right_front();
                left_front();
                _ml.write(0);
                _mr.write(0); 
                // printf("0 mt:\n\t r:%.3f, l:%.3f\n", ll_out, rr_out);               
                return;
            }
        }
        
        ll_out = ufo::utl::constrain(abs(ll_out), throt_threshhold, 1.f);
        rr_out = ufo::utl::constrain(abs(rr_out), throt_threshhold, 1.f);
        // printf("mt:\n\t r:%.3f, l:%.3f\n", ll_out, rr_out);               
        
        l_out = ufo::utl::map(ll_out, throt_threshhold, 1.f, 0.f, static_cast<float>(motor_t::pwm_max_out));
        r_out = ufo::utl::map(rr_out, throt_threshhold, 1.f, 0.f, static_cast<float>(motor_t::pwm_max_out));
        
        // printf("mt:\n\t r:%.3f, l:%.3f\nrp: %d, lp: %d", ll_out, rr_out, l_out, r_out);               

        _ml.write(l_out);
        _mr.write(r_out);
    }

private:
    void left_front()
    {
        gpio_set_level(mot_right_ctrl_pin1, 1);
        gpio_set_level(mot_right_ctrl_pin2, 0);
    }

    void left_back()
    {
        gpio_set_level(mot_right_ctrl_pin1, 0);
        gpio_set_level(mot_right_ctrl_pin2, 1);
    }

    void right_front()
    {
        gpio_set_level(mot_left_ctrl_pin1, 1);
        gpio_set_level(mot_left_ctrl_pin2, 0);
    }

    void right_back()
    {
        gpio_set_level(mot_left_ctrl_pin1, 0);
        gpio_set_level(mot_left_ctrl_pin2, 1);
    }

    void right_off(){
        gpio_set_level(mot_left_ctrl_pin1, 0);
        gpio_set_level(mot_left_ctrl_pin2, 0);
    }

    void left_off(){
        gpio_set_level(mot_right_ctrl_pin1, 0);
        gpio_set_level(mot_right_ctrl_pin2, 0);
    }
};