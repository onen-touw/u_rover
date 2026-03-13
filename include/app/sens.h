#pragma once

#include "u_sys/thread.h"
#include "u_sys/list.h"

#include "u_sensors/UFO_Baro.h"
#include "u_sensors/UFO_Compass.h"
#include "u_sensors/UFO_IMU.h"
#include "u_sensors/UFO_INA219.h"
#include "u_sensors/UFO_SensorPosition.h"

#include "u_math/Madgwick.h"
#include "u_math/UFO_KalmanFilter.h"
#include "appdata.h"

#define to_angle 180.f / ufo_M_PI

// sensor control class
class sens_t
{
private:
    ufo::drv::UFO_I2C_Driver* _driver = nullptr;
public:

    sens_t(ufo::drv::UFO_I2C_Driver* drv) {
        if (drv->Initialized())
        {
            _driver = drv;
        }
    }

    ~sens_t() {
    
    }

    // void mk(){
    //     if (!_driver)
    //     {
    //         return;
    //     }
    //     _list.emplace_back(ufo::thread(&sens_t::task_imu, this));    
    // }

 

// private:
public:

    // void task_bar(ufo::token_t token) {
    //     UFO_Baro baro(_driver);
    //     baro.InitSensor();
    //     UFO_BaroData_t data = {};

    //     while (token)
    //     {
    //         baro.Update();
    //         data = baro.Get();
    //         // _msg->fMsg("vzik_bar@%d=%.3f;%.3f\n", ufo::utl::get_time_millis(), data.Presure, data.Tempreture);
    //         // ufo::Trace_t::flog("baro: %.3f,%.3f,\n", data.Presure, data.Tempreture);
    //         // ufo::Trace_t::flog(">b:%.3f, T:%.3f\n", data.Presure, data.Tempreture);
    //         {
    //             __global_app_data._baro._p = data.Presure;
    //             __global_app_data._baro._t = data.Tempreture;
    //         }
    //         ufo::utl::sleep_for(50);
    //     }
    // }
    
    void task_imu(ufo::token_t token) {
        

        UFO_IMU imu(_driver);

        UFO_KalmanFilter 
            kalmanRoll,
            kalmanPitch;
        Madgwick madgwick;
        
        IMU_Data_t data;

        imu.InitSensor();
        
        {
            ufo::lock_guard<ufo::mutex_t> _lock(app::__global_app_data._imu._lock);
            imu.SetOffsets(app::__global_app_data._imu._calib._acs, app::__global_app_data._imu._calib._gyro);
        }
        
        madgwick.begin(0.2f);

        kalmanRoll.Set(0.5f, 0.5f, 0.8f);
        kalmanPitch.Set(0.5f, 0.5f, 0.8f);

        // ufo::Trace_t::log("Keep IMU level...");
        // {
        //     IMU_Calibration_t calib;
        //     ufo::utl::sleep_for(2000);
        //     imu.Calibrate();

        //     calib = imu.GetOffsets();
        //     ufo::Trace_t::log("Calibration done!\n");
        //     ufo::Trace_t::log("Accel biases X/Y/Z:\n");
        //     ufo::Trace_t::log(calib._accelOffset._x);
        //     ufo::Trace_t::log(", ");
        //     ufo::Trace_t::log(calib._accelOffset._y);
        //     ufo::Trace_t::log(", ");
        //     ufo::Trace_t::log(calib._accelOffset._z);
        //     ufo::Trace_t::log('\n');
        //     ufo::Trace_t::log("Gyro biases X/Y/Z:\n");
        //     ufo::Trace_t::log(calib._gyroOffset._x);
        //     ufo::Trace_t::log(", ");
        //     ufo::Trace_t::log(calib._accelOffset._y);
        //     ufo::Trace_t::log(", ");
        //     ufo::Trace_t::log(calib._accelOffset._z);
        //     ufo::Trace_t::log('\n');
        //     ufo::utl::sleep_for(100);
        // }

        
        while (token)
        {
            imu.Update();
            data = imu.Get();
            madgwick.updateIMU(
                data._gyro._x,
                data._gyro._y,
                data._gyro._z,
                data._accel._x,
                data._accel._y,
                data._accel._z);

            float
                q0 = madgwick.getQuatW(),
                q1 = madgwick.getQuatX(),
                q2 = madgwick.getQuatY(),
                q3 = madgwick.getQuatZ();

            float roll = atan2(0.5f - q1 * q1 - q2 * q2, q0 * q1 + q2 * q3);
            float pitch = asinf(-2.0f * (q1 * q3 - q0 * q2));
            float yaw = atan2f(q1 * q2 + q0 * q3, 0.5f - q2 * q2 - q3 * q3);

            // ufo::Trace_t::log(">rollFiltered:");
            // ufo::Trace_t::log(kalmanRoll(roll) * 180.f / ufo_M_PI+90); // <-------------- filter
            // ufo::Trace_t::log("\n>pitchFiltered:");
            // ufo::Trace_t::log(kalmanPitch(pitch) * 180.f / ufo_M_PI); // <-------------- filter
            
            // ufo::Trace_t::flog("> r:%.3f, p:%.3f, y:%.3f\n", roll * to_angle + 90.f, pitch * to_angle, yaw*to_angle);
         
            // ufo::Trace_t::log("\n>yaw:");
            // ufo::Trace_t::log(yaw * 180.f / ufo_M_PI);
            // ufo::Trace_t::log("\n>pitch:");
            // ufo::Trace_t::log(pitch * 180.f / ufo_M_PI);
            // ufo::Trace_t::log("\n>roll:");
            // ufo::Trace_t::log(roll * +90.f);
            // ufo::Trace_t::log('\n');
            roll = roll * 180.f / ufo_M_PI+90;
            pitch = pitch * 180.f / ufo_M_PI;
            yaw = yaw * 180.f / ufo_M_PI;
            {
                // __global_app_data._imu._r = roll;
                // __global_app_data._imu._p = pitch;
                // __global_app_data._imu._y = yaw;
            }
            // _msg->fMsg("vzik_imu@%d=%.3f;%.3f;%.3f\n", ufo::utl::get_time_millis(), roll, pitch, yaw);
            ufo::utl::sleep_for(1);
        }
        
    }
};

void task_imu_calibrate(ufo::drv::UFO_I2C_Driver *drv, uint16_t iterations, ufo::token_t token)
{
    if (!drv)
    {
        return;
    }
    
    if (!drv->Initialized())
    {
        return;
    }

    if (iterations > 10)
    {
        return;
    }
    UFO_IMU imu(drv);
    imu.InitSensor();
    
    IMU_Calibration_t result = {};

    for (size_t i = 0; i < iterations; i++)
    {
        imu.Calibrate();
        IMU_Calibration_t calib = imu.GetOffsets();

        result._accelOffset._x += calib._accelOffset._x;
        result._accelOffset._y += calib._accelOffset._y;
        result._accelOffset._z += calib._accelOffset._z;
        
        result._gyroOffset._x += calib._gyroOffset._x;
        result._gyroOffset._y += calib._gyroOffset._y;
        result._gyroOffset._z += calib._gyroOffset._z;
        ufo::utl::sleep_for(10);
    }
    
    result._accelOffset._x /= iterations;
    result._accelOffset._y /= iterations;
    result._accelOffset._z /= iterations;

    result._gyroOffset._x /= iterations;
    result._gyroOffset._y /= iterations;
    result._gyroOffset._z /= iterations;
    

    ufo::lock_guard<ufo::mutex_t> _lock (app::__global_app_data._imu._lock);
    app::__global_app_data._imu._calib._acs = result._accelOffset;
    app::__global_app_data._imu._calib._gyro = result._gyroOffset;
}