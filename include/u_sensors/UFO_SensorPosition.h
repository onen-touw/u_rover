// #include "UFO_Config.h" 
#pragma once
#include "u_sys/config.h"

enum class UFO_SensorPosition : uint8_t{
    IMU_DIR_BASIC = 0,
    IMU_DIR_Z_ROT_90,               // axis: z ... direction: counterclockwise
    IMU_DIR_Z_ROT_180,              // axis: z ... direction: counterclockwise
    IMU_DIR_Z_ROT_270,              // axis: z ... direction: counterclockwise
    IMU_DIR_Y_ROT90_Z_ROT180 = 4,   // axis: y ... direction: clockwise; -> axis: z ... direction: counterclockwise  
    IMU_DIR_Y_ROT90_Z_ROT90 = 5,    // axis: y ... direction: clockwise; -> axis: z ... direction: counterclockwise  
    IMU_DIR_Y_ROT90 = 6,            // axis: y ... direction: clockwise; -> axis: z ... direction: counterclockwise  
    IMU_DIR_Y_ROT90_Z_ROT270 = 7,   // axis: y ... direction: clockwise; -> axis: z ... direction: counterclockwise  
};