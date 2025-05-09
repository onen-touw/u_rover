#pragma once

#include "u_sys/config.h"
#include "u_drivers/i2c/UFO_I2C_Interface.h"
#include "u_math/u_math.h"
#include "u_sys/trace.h"
#include "u_sys/error.h"
#include "u_sys/utils.h"

#define UFO_COMPASS_ENABLE_TEMRETURE_MODULE     UFO_DISABLE 
#define UFO_COMPASS_CASTOM_POSITION             UFO_DISABLE
#define UFO_COMPASS_ADVANCED_CALIBRATION        UFO_DISABLE /*UFO_ENABLE */ 

#if UFO_COMPASS_CASTOM_POSITION // see /docks/imu_directions.png
#include "UFO_SensorPosition.h"
#endif

#define UFO_HMC5883L_ADDRESS ((uint8_t)0xD)

#define QMC5883L_X_LSB		0x00
#define QMC5883L_X_MSB		0x01
#define QMC5883L_Y_LSB		0x02
#define QMC5883L_Y_MSB		0x03
#define QMC5883L_Z_LSB		0x04
#define QMC5883L_Z_MSB		0x05
#define QMC5883L_STATUS     0x06
#define QMC5883L_T_LSB		0x07
#define QMC5883L_T_MSB      0x08
#define QMC5883L_CTRL1      0x09
#define QMC5883L_CTRL2		0x0A
#define QMC5883L_PERIOD		0x0B
#define QMC5883L_WHOAMI		0x0D

#define QMC5883L_WHOAMI_VALUE	0xFF

enum class UFO_CompassMode : uint8_t
{
    COMPASS_MODE_STANDBY = 0b00,
    COMPASS_MODE_CONTINUOUS = 0b01,
    COMPASS_MODE_DEFAULT = COMPASS_MODE_CONTINUOUS
};

// ODR
enum class UFO_CompassOutputDataRate : uint8_t
{
    COMPASS_ODR_10HZ = 0b00,
    COMPASS_ODR_50HZ = 0b01,
    COMPASS_ODR_100HZ = 0b10,
    COMPASS_ODR_200HZ = 0b11,
    COMPASS_ODR_DEFAULT = COMPASS_ODR_200HZ,
};
// RNG
enum class UFO_CompassFullScaleRange : uint8_t
{
    COMPASS_RNG_2G = 0b00,
    COMPASS_RNG_8G = 0b01,
    COMPASS_RNG_DEFAULT = COMPASS_RNG_8G,
};
// OSR
enum class UFO_CompassOverSamplingRatio : uint8_t
{
    COMPASS_OSR_512 = 0b00,
    COMPASS_OSR_DEFAULT = COMPASS_OSR_512,
    COMPASS_OSR_256 = 0b01,
    COMPASS_OSR_128 = 0b10,
    COMPASS_OSR_64 = 0b11,
};

struct UFO_CompassCalibration
{
    Vector3<float> _offsets = {0.f, 0.f, 0.f};
    Vector3<float> _scale = {0.f, 0.f, 0.f};
};


// currently only HMC5883L
class UFO_Compass :  private ufo::UFO_I2C_Interface
{
private:
    using _int = int16_t;
    using _byte = int8_t;
    Vector3<float> _data = {0.f, 0.f, 0.f};
    UFO_CompassCalibration _calibration;
    float _magRange = 10.f * 819.2f / 32768.f;

#if UFO_COMPASS_ENABLE_TEMRETURE_MODULE
    float _tempreture = 0.f;
#endif
#if UFO_COMPASS_CASTOM_POSITION
    UFO_SensorPosition _position = IMU_DIR_BASIC;
#endif

public:
    UFO_Compass(ufo::drv::UFO_I2C_Driver *driver) {
        esp_err_t err = this->Init(UFO_HMC5883L_ADDRESS, driver);
        if (err!= ESP_OK)
        {
            //SetCritical;      //think about it
            ufo::Trace_t::log("driver initializing priblem\n");
        }
    }
    
    ~UFO_Compass() {}

    void InitSensor(){
        uint8_t sensorID = this->Read8(QMC5883L_WHOAMI);
        if (sensorID != QMC5883L_WHOAMI_VALUE)
        {
            ufo::Trace_t::log("UFO_IMU: Sensor's id is incorrect: should be ");
            ufo::Trace_t::log(QMC5883L_WHOAMI_VALUE);
            ufo::Trace_t::log(", your id is ");
            ufo::Trace_t::log(sensorID);
            ufo::Trace_t::log("\n");
        }
        _calibration._scale = {1.f, 1.f, 1.f};

	    this->Write8(QMC5883L_CTRL2, 0b10000000);   // SOFT_RESET
        ufo::utl::sleep_for(100);
	    this->Write8(QMC5883L_PERIOD, 0x01);       // It is recommended that the register -//- is written by 0x01.
        ufo::utl::sleep_for(100);
        
        SetConfig();
    }

    void SetConfig(UFO_CompassMode mode = UFO_CompassMode::COMPASS_MODE_DEFAULT,
                   UFO_CompassOutputDataRate ODR = UFO_CompassOutputDataRate::COMPASS_ODR_DEFAULT,
                   UFO_CompassFullScaleRange RNG = UFO_CompassFullScaleRange::COMPASS_RNG_DEFAULT,
                   UFO_CompassOverSamplingRatio OSR = UFO_CompassOverSamplingRatio::COMPASS_OSR_DEFAULT)
    {
        uint8_t md = (
            static_cast<uint8_t>(OSR) << 6 | 
            static_cast<uint8_t>(RNG) << 4 | 
            static_cast<uint8_t>(ODR) << 2 | 
            static_cast<uint8_t>(mode)
        );  // QMC5883L_CTRL1:|OSR|RNG|ODR|MODE|
        this->Write8(QMC5883L_CTRL1, md);
    }

    void SetCalibrationData(UFO_CompassCalibration& cal){
        _calibration = cal;
    }

    bool DataAvailable() { return (this->Read8(QMC5883L_STATUS) & 0x01); }
    
    void operator()(){
        Update();
    }

    void Update(){
        if (!DataAvailable()){
            return /*-INF*/;
        }
        uint8_t raw[6] = {};
	    this->Read(QMC5883L_X_LSB, &raw[0], 6);
        __RawToReal(&raw[0]);

        #if UFO_COMPASS_ENABLE_TEMRETURE_MODULE
        this->Read(QMC5883L_T_LSB, &raw[0], 2);
        _tempreture  = (static_cast<float>( static_cast<int16_t>(raw[1] << 8) | raw[0]) * 100.f / 32768.f) + 20.f;
        #endif
    }

    
    void Calibrate() 
{
    #if UFO_COMPASS_ADVANCED_CALIBRATION
        ufo::Trace_t::log("UFO_IMU: COMPASS_ADVANCED_CALIBRATION currently not implemented\n");
    #else 
	int32_t
		mag_bias[3] = {0, 0, 0},
		mag_scale[3] = {0, 0, 0};
	int16_t
		mag_max[3] = {-INT16_MAX, -INT16_MAX, -INT16_MAX},
		mag_min[3] = {INT16_MIN, INT16_MIN, INT16_MIN},
		mag_temp[3] = {0, 0, 0};

	// shoot for ~fifteen seconds of mag data
	uint16_t sample_count = 3000;  // at 200 Hz ODR, new mag data is available every 5 ms
    uint8_t rawData[6] = {};
    for (uint16_t ii = 0; ii < sample_count; ++ii)
    {
        // x/y/z gyro register data, ST2 register stored here, must read ST2 at end of data acquisition
        if ((this->Read8(QMC5883L_STATUS) & 0x01) && !(this->Read8(QMC5883L_STATUS) & 0x02))
        {                                                                       // wait for magnetometer data ready bit to be set and overflow not to be.
            this->Read(QMC5883L_X_LSB, &rawData[0], 6);                         // Read the six raw data and ST2 registers sequentially into data array
            mag_temp[0] = (static_cast<int16_t>(rawData[1]) << 8) | rawData[0]; // Turn the MSB and LSB into a signed 16-bit value
            mag_temp[1] = (static_cast<int16_t>(rawData[3]) << 8) | rawData[2]; // Data stored as little Endian
            mag_temp[2] = (static_cast<int16_t>(rawData[5]) << 8) | rawData[4];
            for (int jj = 0; jj < 3; jj++)
            {
                if (mag_temp[jj] > mag_max[jj])
                {
                    mag_max[jj] = mag_temp[jj];
                }
                if (mag_temp[jj] < mag_min[jj])
                {
                    mag_min[jj] = mag_temp[jj];
                }
            }
        }
        
        // at 200 Hz ODR, new mag data is available every 5 ms
        ufo::utl::sleep_for(6);

    }

    // Get hard iron correction
    mag_bias[0] = (mag_max[0] + mag_min[0]) / 2; // get average x mag bias in counts
	mag_bias[1] = (mag_max[1] + mag_min[1]) / 2; // get average y mag bias in counts
	mag_bias[2] = (mag_max[2] + mag_min[2]) / 2; // get average z mag bias in counts

	_calibration._offsets._x = static_cast<float>(mag_bias[0]) * _magRange; // save mag biases in G for main program
	_calibration._offsets._y = static_cast<float>(mag_bias[1]) * _magRange;
	_calibration._offsets._z = static_cast<float>(mag_bias[2]) * _magRange;

	// Get soft iron correction estimate
	mag_scale[0] = (mag_max[0] - mag_min[0]) / 2; // get average x axis max chord length in counts
	mag_scale[1] = (mag_max[1] - mag_min[1]) / 2; // get average y axis max chord length in counts
	mag_scale[2] = (mag_max[2] - mag_min[2]) / 2; // get average z axis max chord length in counts

	float avg_rad = mag_scale[0] + mag_scale[1] + mag_scale[2];
	avg_rad /= 3.0;

	_calibration._scale._x = avg_rad / static_cast<float>(mag_scale[0]);
	_calibration._scale._y = avg_rad / static_cast<float>(mag_scale[1]);
	_calibration._scale._z = avg_rad / static_cast<float>(mag_scale[2]);
    #endif
}
    const Vector3<float>& Get() const {
        return _data;
    }

private:
    void __RawToReal(uint8_t* raw){
        _int magic = 0;
#if UFO_COMPASS_CASTOM_POSITION
        float
            mx = 0.f,
            my = 0.f,
            mz = 0.f;
        // reg 00 - Low bit, 01 - high bit
        magic = (static_cast<_int>(raw[1]) << 8) | raw[0];
        mx = (static_cast<float>(magic) * _magRange - _calibration._offsets._x) * _calibration._scale._x;

        magic = (static_cast<_int>(raw[3]) << 8) | raw[2];
        my = (static_cast<float>(magic) * _magRange - _calibration._offsets._y) * _calibration._scale._y;

        magic = (static_cast<_int>(raw[5]) << 8) | raw[4];
        mz = (static_cast<float>(magic) * _magRange - _calibration._offsets._z) * _calibration._scale._z;

        switch (_position)
        {
        case IMU_DIR_BASIC:
            _data._x = mx;
            _data._y = my;
            _data._z = mz;
            break;
        case IMU_DIR_Z_ROT_90:
            _data._x = -my;
            _data._y = mx;
            _data._z = mz;
            break;
        case IMU_DIR_Z_ROT_180:
            _data._x = mx;
            _data._y = my;
            _data._z = mz;
            break;
        case IMU_DIR_Z_ROT_270:
            _data._x = my;
            _data._y = -mx;
            _data._z = mz;
            break;
        case IMU_DIR_Y_ROT90_Z_ROT180:
            _data._x = -mz;
            _data._y = -my;
            _data._z = -mx;
            break;
        case IMU_DIR_Y_ROT90_Z_ROT90:
            _data._x = -mz;
            _data._y = mx;
            _data._z = -my;
            break;
        case IMU_DIR_Y_ROT90:
            _data._x = -mz;
            _data._y = my;
            _data._z = mx;
            break;
        case IMU_DIR_Y_ROT90_Z_ROT270:
            _data._x = -mz;
            _data._y = -mx;
            _data._z = my;
            break;
        }
#else
        //reg 00 - Low bit, 01 - high bit
        magic = (static_cast<_int>(raw[1])<<8) | raw[0];
        _data._x = (static_cast<float>(magic) * _magRange - _calibration._offsets._x ) * _calibration._scale._x;

        magic = (static_cast<_int>(raw[3])<<8) | raw[2];
        _data._y = (static_cast<float>(magic) * _magRange - _calibration._offsets._y) * _calibration._scale._y;

        magic = (static_cast<_int>(raw[5])<<8) | raw[4];
        _data._z = (static_cast<float>(magic) * _magRange - _calibration._offsets._z) * _calibration._scale._z;
#endif
    }

};