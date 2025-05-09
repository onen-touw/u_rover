#pragma once

#include "u_drivers/pwm/UFO_ESC.h"
#include "u_sys/thread.h"

// #define UFO_MOTOR_RES_VAL_MAX       ((int16_t)(std::pow(2, 16)/10))           //2^16/10
#define UFO_MOTOR_RES_VAL_MAX       (/* (float) */(6653.f))      //2^16/10
#define UFO_MOTOR_RES_VAL_MIN       (/* (float) */(3326.f))                           //2^16/2/10

class esc_quad_t
{
public:
    using motor_t = ufo::drv::UFO_ESC_driver;

public:
    static constexpr gpio_num_t gpio_top_l = gpio_num_t::GPIO_NUM_26;
    static constexpr gpio_num_t gpio_top_r = gpio_num_t::GPIO_NUM_27;
    static constexpr gpio_num_t gpio_bot_l = gpio_num_t::GPIO_NUM_14;
    static constexpr gpio_num_t gpio_bot_r = gpio_num_t::GPIO_NUM_13;
    
    static constexpr float kpit = 0.05f;
    static constexpr float krol = 0.05f;
    static constexpr float kyaw = 0.05f;

    static constexpr float mot_min = -1.f;
    static constexpr float mot_max = 1.f;
private:
    motor_t _r1;
    motor_t _r2;
    motor_t _r3;
    motor_t _r4;

    float _arm_per = 0.05f;
    bool _arm = false;

public:
    esc_quad_t() {
        _r1.Setup(0, gpio_top_r);
        _r2.Setup(1, gpio_bot_r);
        _r3.Setup(2, gpio_bot_l);
        _r4.Setup(3, gpio_top_l);

        set_min();
    }
    ~esc_quad_t() {}

    void set_min(){
        _r1.Write(motor_t::esc_min_out);
        _r2.Write(motor_t::esc_min_out);
        _r3.Write(motor_t::esc_min_out);
        _r4.Write(motor_t::esc_min_out);
    }

    void arm(){
        if (!_arm)
        {
            printf("arm\n");
            _arm = true;
            uint16_t val = _arm_per * (motor_t::esc_max_out - motor_t::esc_min_out)+ motor_t::esc_min_out;
            _r1.Write(val);
            _r2.Write(val);
            _r3.Write(val);
            _r4.Write(val);
        }
    }

    bool get_arm() const { 
        return _arm; 
    }

    void disarm(){
        _arm = false;
        _r1.Write(motor_t::esc_min_out);
        _r2.Write(motor_t::esc_min_out);
        _r3.Write(motor_t::esc_min_out);
        _r4.Write(motor_t::esc_min_out);
        printf("disarm\n");
    }

    void target_write(float m1, float m2, float m3, float m4){
        if (!_arm)
        {
            return;
        }
        _r1.Write(ufo::utl::map(m1, mot_min, mot_max, static_cast<float>(motor_t::esc_min_out), static_cast<float>(motor_t::esc_max_out)));
        _r2.Write(ufo::utl::map(m2, mot_min, mot_max, static_cast<float>(motor_t::esc_min_out), static_cast<float>(motor_t::esc_max_out)));
        _r3.Write(ufo::utl::map(m3, mot_min, mot_max, static_cast<float>(motor_t::esc_min_out), static_cast<float>(motor_t::esc_max_out)));
        _r4.Write(ufo::utl::map(m4, mot_min, mot_max, static_cast<float>(motor_t::esc_min_out), static_cast<float>(motor_t::esc_max_out)));
        printf(">r1:%u, r2:%u, r3:%u, r4: %u\n", _r1.GetDuty(), _r2.GetDuty(), _r3.GetDuty(), _r4.GetDuty());
    }

    void update(float t, float r, float p, float y)
    {
        if (!_arm)
        {
            return;
        }

        float r1 = t - p * kpit + r * krol - y * kyaw;
        float r2 = t - p * kpit - r * krol + y * kyaw;
        float r3 = t + p * kpit - r * krol + y * kyaw;
        float r4 = t + p * kpit - r * krol - y * kyaw;

        r1 = ufo::utl::map(r1, 0.f, 1.f, static_cast<float>(motor_t::esc_min_out), static_cast<float>(motor_t::esc_max_out));
        r2 = ufo::utl::map(r2, 0.f, 1.f, static_cast<float>(motor_t::esc_min_out), static_cast<float>(motor_t::esc_max_out));
        r3 = ufo::utl::map(r3, 0.f, 1.f, static_cast<float>(motor_t::esc_min_out), static_cast<float>(motor_t::esc_max_out));
        r4 = ufo::utl::map(r4, 0.f, 1.f, static_cast<float>(motor_t::esc_min_out), static_cast<float>(motor_t::esc_max_out));
        
        int16_t rr1 = ufo::utl::constrain(r1, static_cast<float>(motor_t::esc_min_out), static_cast<float>(motor_t::esc_max_out));
        int16_t rr2 = ufo::utl::constrain(r1, static_cast<float>(motor_t::esc_min_out), static_cast<float>(motor_t::esc_max_out));
        int16_t rr3 = ufo::utl::constrain(r1, static_cast<float>(motor_t::esc_min_out), static_cast<float>(motor_t::esc_max_out));
        int16_t rr4 = ufo::utl::constrain(r1, static_cast<float>(motor_t::esc_min_out), static_cast<float>(motor_t::esc_max_out));

        _r1.Write(rr1);
        _r2.Write(rr2);
        _r3.Write(rr3);
        _r4.Write(rr4);
        
        printf(">r1:%.3f, r2:%.3f, r3:%.3f, r4: %.3f\n", r1, r2, r3, r4);
        printf(">r1:%d, r2:%d, r3:%d, r4: %d\n", rr1, rr2, rr3, rr4);

        // p = 0.5*ufo::utl::map(p, mot_min, mot_max, UFO_MOTOR_RES_VAL_MIN, UFO_MOTOR_RES_VAL_MAX);

        // int16_t rr1 = ufo::utl::constrain(t - 0.1f * UFO_MOTOR_RES_VAL_MAX * p, UFO_MOTOR_RES_VAL_MIN, UFO_MOTOR_RES_VAL_MAX);
        // int16_t rr2 = ufo::utl::constrain(t - 0.1f * UFO_MOTOR_RES_VAL_MAX * p, UFO_MOTOR_RES_VAL_MIN, UFO_MOTOR_RES_VAL_MAX);
        // int16_t rr3 = ufo::utl::constrain(t + 0.1f * UFO_MOTOR_RES_VAL_MAX * p, UFO_MOTOR_RES_VAL_MIN, UFO_MOTOR_RES_VAL_MAX);
        // int16_t rr4 = ufo::utl::constrain(t + 0.1f * UFO_MOTOR_RES_VAL_MAX * p, UFO_MOTOR_RES_VAL_MIN, UFO_MOTOR_RES_VAL_MAX);

        // int16_t rr1= ufo::utl::map((t-p)*0.5f, 0.f, mot_max, UFO_MOTOR_RES_VAL_MIN, UFO_MOTOR_RES_VAL_MAX);
        // int16_t rr2= ufo::utl::map((t-p)*0.5f, 0.f, mot_max, UFO_MOTOR_RES_VAL_MIN, UFO_MOTOR_RES_VAL_MAX);
        // int16_t rr3= ufo::utl::map((t+p)*0.5f, 0.f, mot_max, UFO_MOTOR_RES_VAL_MIN, UFO_MOTOR_RES_VAL_MAX);
        // int16_t rr4= ufo::utl::map((t+p)*0.5f, 0.f, mot_max, UFO_MOTOR_RES_VAL_MIN, UFO_MOTOR_RES_VAL_MAX);
        // float ft = ufo::utl::map((float)t, (float)mot_min, (float)mot_max, 0.f, 1.f);
        // float fr = ufo::utl::map((float)r, (float)mot_min, (float)mot_max, 0.f, 1.f);
        // float fp = ufo::utl::map((float)p, (float)mot_min, (float)mot_max, 0.f, 1.f);
        // float fy = ufo::utl::map((float)y, (float)mot_min, (float)mot_max, 0.f, 1.f);
        // printf(">r1:%.3f, r2:%.3f, r3:%.3f, r4: %.3f\n", rr1, rr2, rr3, rr4);


        // printf("r1: %.3f, r2: %.3f, r3: %.3f, r4: %.3f\n",
        //     ft - fr + fp - fy,
        //     ft - fr - fp + fy,
        //     ft + fr - fp - fy,
        //     ft + fr + fp + fy);
        // _r1.Write(rr1);
        // _r2.Write(rr2);
        // _r3.Write(rr3);
        // _r4.Write(rr4);
        // _r2.Write(t - r - p + y);
        // _r3.Write(t + r - p - y);
        // _r4.Write(t + r + p + y);
    }


    // void task(ufo::token_t token){
    //     while (token)
    //     {
    //         update(0.f, 0.f, 0.f, 0.f);
    //     }
    //     ufo::utl::sleep_for(50);
    // }
};
