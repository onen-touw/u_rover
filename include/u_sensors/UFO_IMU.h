#pragma once

#include "u_sys/config.h"
#include "u_drivers/i2c/UFO_I2C_Interface.h"
#include "u_math/u_math.h"
#include "u_sys/trace.h"
#include "u_sys/error.h"
#include "u_sys/utils.h"

#define UFO_IMU_ENABLE_TEMRETURE_MODULE     UFO_DISABLE 
#define UFO_IMU_CASTOM_POSITION             UFO_ENABLE
// #define UFO_IMU_CASTOM_POSITION             UFO_DISABLE
#define UFO_IMU_ADVANCED_CALIBRATION        UFO_DISABLE /*UFO_ENABLE */ 

#if UFO_IMU_CASTOM_POSITION // see /docks/imu_directions.png
#include "UFO_SensorPosition.h"
#endif

#define MPU6050_SELF_TEST_X_ACCEL 0x0D
#define MPU6050_SELF_TEST_Y_ACCEL 0x0E
#define MPU6050_SELF_TEST_Z_ACCEL 0x0F
#define MPU6050_SELF_TEST_A 0x10

#define MPU6050_SMPLRT_DIV 0x19
#define MPU6050_MPU_CONFIG 0x1A
#define MPU6050_GYRO_CONFIG 0x1B
#define MPU6050_ACCEL_CONFIG 0x1C

#define MPU6050_FIFO_EN 0x23
#define MPU6050_I2C_MST_CTRL 0x24
#define MPU6050_I2C_SLV0_ADDR 0x25
#define MPU6050_I2C_SLV0_REG 0x26
#define MPU6050_I2C_SLV0_CTRL 0x27
#define MPU6050_I2C_SLV1_ADDR 0x28
#define MPU6050_I2C_SLV1_REG 0x29
#define MPU6050_I2C_SLV1_CTRL 0x2A
#define MPU6050_I2C_SLV2_ADDR 0x2B
#define MPU6050_I2C_SLV2_REG 0x2C
#define MPU6050_I2C_SLV2_CTRL 0x2D
#define MPU6050_I2C_SLV3_ADDR 0x2E
#define MPU6050_I2C_SLV3_REG 0x2F
#define MPU6050_I2C_SLV3_CTRL 0x30
#define MPU6050_I2C_SLV4_ADDR 0x31
#define MPU6050_I2C_SLV4_REG 0x32
#define MPU6050_I2C_SLV4_DO 0x33
#define MPU6050_I2C_SLV4_CTRL 0x34
#define MPU6050_I2C_SLV4_DI 0x35
#define MPU6050_I2C_MST_STATUS 0x36
#define MPU6050_INT_PIN_CFG 0x37
#define MPU6050_INT_ENABLE 0x38
#define MPU6050_INT_STATUS 0x3A
#define MPU6050_ACCEL_XOUT_H 0x3B
#define MPU6050_ACCEL_XOUT_L 0x3C
#define MPU6050_ACCEL_YOUT_H 0x3D
#define MPU6050_ACCEL_YOUT_L 0x3E
#define MPU6050_ACCEL_ZOUT_H 0x3F
#define MPU6050_ACCEL_ZOUT_L 0x40
#define MPU6050_TEMP_OUT_H 0x41
#define MPU6050_TEMP_OUT_L 0x42
#define MPU6050_GYRO_XOUT_H 0x43
#define MPU6050_GYRO_XOUT_L 0x44
#define MPU6050_GYRO_YOUT_H 0x45
#define MPU6050_GYRO_YOUT_L 0x46
#define MPU6050_GYRO_ZOUT_H 0x47
#define MPU6050_GYRO_ZOUT_L 0x48
#define MPU6050_I2C_MST_DELAY_CTRL 0x67
#define MPU6050_SIGNAL_PATH_RESET 0x68
#define MPU6050_USER_CTRL 0x6A
#define MPU6050_PWR_MGMT_1 0x6B // Device defaults to the SLEEP mode
#define MPU6050_PWR_MGMT_2 0x6C
#define MPU6050_FIFO_COUNTH 0x72
#define MPU6050_FIFO_COUNTL 0x73
#define MPU6050_FIFO_R_W 0x74
#define MPU6050_WHO_AM_I_MPU6050 0x75 // Should return 0x68
#define MPU6050_WHOAMI_DEFAULT_VALUE 0x68

#define UFO_MPU6050_ADDRESS ((uint8_t)0x68)

enum class UFO_IMU_AccelRange : uint8_t
{
    IMU_ACCEL_RANGE_2G = 0,
    IMU_ACCEL_RANGE_4G,
    IMU_ACCEL_RANGE_8G,
    IMU_ACCEL_RANGE_16G,
    IMU_ACCEL_DEFAULT_G =IMU_ACCEL_RANGE_16G 
};

enum class UFO_IMU_GyroRange : uint8_t
{
    IMU_GYRO_DEFAULT_250_DPS= 0,
    IMU_GYRO_DEFAULT_500_DPS,
    IMU_GYRO_DEFAULT_1000_DPS,
    IMU_GYRO_DEFAULT_2000_DPS,
    IMU_GYRO_DEFAULT_DPS = IMU_GYRO_DEFAULT_2000_DPS
};

struct IMU_Calibration_t
{
    Vector3<float> _accelOffset = {0.f,0.f,0.f};
    Vector3<float> _gyroOffset = {0.f,0.f,0.f};
};

struct IMU_Data_t {
    Vector3<float> _accel = {0.f,0.f,0.f};
    Vector3<float> _gyro = {0.f,0.f,0.f};
    #if UFO_IMU_ENABLE_TEMRETURE_MODULE
    float _tempreture = 0;    
    #endif
};


class UFO_IMU : private ufo::UFO_I2C_Interface
{
private:
    float
        _accelRange = 16.0f / 32768.0f,    // ares value for full range (16g) readings
        _gyroRange = 2000.0f / 32768.0f;		
    IMU_Calibration_t _calibration;
    IMU_Data_t _data;
    #if UFO_IMU_CASTOM_POSITION
    UFO_SensorPosition _position = UFO_SensorPosition::IMU_DIR_BASIC;
    #endif

public:
    UFO_IMU(ufo::drv::UFO_I2C_Driver *driver)
    {
        esp_err_t err = this->Init(UFO_MPU6050_ADDRESS, driver);
        if (err!= ESP_OK)
        {
            //SetCritical;      //think about it
            ufo::Trace_t::log("driver initializing priblem\n");
        }
    }

    ~UFO_IMU()
    {
    }

    void InitSensor()
    {
        uint8_t sensorID = this->Read8(MPU6050_WHO_AM_I_MPU6050);
        // log id
        if (sensorID != MPU6050_WHOAMI_DEFAULT_VALUE)
        {
            // log warning
            ufo::Trace_t::log("UFO_IMU: Sensor's id is incorrect: should be ");
            ufo::Trace_t::log(MPU6050_WHOAMI_DEFAULT_VALUE);
            ufo::Trace_t::log(", your id is ");
            ufo::Trace_t::log(sensorID);
            ufo::Trace_t::log('\n');
        }

        // reset device
        this->Write8(MPU6050_PWR_MGMT_1, 0x80); // Write a one to bit 7 reset bit; toggle reset device
        ufo::utl::sleep_for(100);
        // wake up device
        this->Write8(MPU6050_PWR_MGMT_1, 0x00); // Clear sleep mode bit (6), enable all sensors
        ufo::utl::sleep_for(100);

        // get stable time source
        this->Write8(MPU6050_PWR_MGMT_1, 0x03); // Auto select clock source to be PLL gyroscope reference if ready else
        ufo::utl::sleep_for(200);

        // Configure Gyro and Thermometer
        // Disable FSYNC and set thermometer and gyro bandwidth to 44 and 42 Hz, respectively;
        // minimum delay time for this setting is 4.9 ms, which means sensor fusion update rates cannot
        // be higher than 1 / 0.0049 = ~200 Hz
        // DLPF_CFG = bits [2:0] = 0b011 = 0x03 =3; this limits the sample rate to 1000 Hz for both
        // With the MPU6050, it is possible to get gyro sample rates of 8 kHz, or 1 kHz
        this->Write8(MPU6050_MPU_CONFIG, 0x03);
        // Set sample rate = gyroscope output rate/(1 + SMPLRT_DIV)
        this->Write8(MPU6050_SMPLRT_DIV, 0x03); // Use a 250 Hz rate; a rate consistent with the filter update rate
        // determined inset in CONFIG above

        // Set gyroscope full scale range
        // Range selects FS_SEL and GFS_SEL are 0 - 3, so 2-bit values are left-shifted into positions 4:3
        uint8_t c = this->Read8(MPU6050_GYRO_CONFIG); // get current GYRO_CONFIG register value
        // c = c & ~0xE0; // Clear self-test bits [7:5]
        c = c & ~0x03;           // Clear Fchoice bits [1:0]
        c = c & ~0x18;           // Clear GFS bits [4:3]
        c = c | (uint8_t)3 << 3; // Set 2000dps full scale range for the gyro (11 on 4:3)
        // c =| 0x00; // Set Fchoice for the gyro to 11 by writing its inverse to bits 1:0 of GYRO_CONFIG
        this->Write8(MPU6050_GYRO_CONFIG, c); // Write new GYRO_CONFIG value to register

        // Set accelerometer full-scale range configuration
        c = this->Read8(MPU6050_ACCEL_CONFIG); // get current ACCEL_CONFIG register value
        // c = c & ~0xE0; // Clear self-test bits [7:5]
        c = c & ~0x18;
        c = c | (uint8_t) 3 << 3;               // Set 16g full scale range for the accelerometer (11 on 4:3)
        this->Write8(MPU6050_ACCEL_CONFIG, c); // Write new ACCEL_CONFIG register value

        // Set accelerometer sample rate configuration
        // It is possible to get a 4 kHz sample rate from the accelerometer by choosing 1 for
        this->Write8(MPU6050_MPU_CONFIG, 0x03); // Set accelerometer rate to 1 kHz and bandwidth to 44 Hz

        // The accelerometer, gyro, and thermometer are set to 1 kHz sample rates,
        // but all these rates are further reduced by a factor of 5 to 200 Hz because of the SMPLRT_DIV setting

        // Configure Interrupts and Bypass Enable
        // Set interrupt pin active high, push-pull, hold interrupt pin level HIGH until interrupt cleared,
        // clear on read of INT_STATUS, and enable I2C_BYPASS_EN so additional chips
        // can join the I2C bus and all can be controlled by the Arduino as master
        this->Write8(MPU6050_INT_PIN_CFG, 0x22);
        this->Write8(MPU6050_INT_ENABLE, 0x01); // Enable data ready (bit 0) interrupt //??????
        ufo::utl::sleep_for(100);

    }

    void SetOffsets(IMU_Calibration_t& calibration){
        _calibration = calibration;
    }

    void SetOffsets(Vector3<float>acs, Vector3<float>gyro){
        _calibration._accelOffset = acs;
        _calibration._gyroOffset = gyro;
    }

    void SetAccelRange(UFO_IMU_AccelRange range)
    {
        _accelRange = static_cast<float>(0b10 << static_cast<uint8_t>(range)) / 32768.f;
        this->Write8(MPU6050_ACCEL_CONFIG, static_cast<uint8_t>(range) << 3);
    }

    void SetGyroRange(UFO_IMU_GyroRange range)
    {
        _gyroRange = static_cast<float>(0b1 << static_cast<uint8_t>(range) * 25 * 10);
        this->Write8(MPU6050_GYRO_CONFIG, static_cast<uint8_t>(range) << 3); // Write new GYRO_CONFIG register value
    }

    bool DataAvailable() { return (this->Read8(MPU6050_INT_STATUS) & 0x01); }
    void Update()
    {
        if (!DataAvailable()){
            return;
        }
        uint8_t rawData[14]= {0}; // x/y/z accel register data stored here
        this->Read(MPU6050_ACCEL_XOUT_H, &rawData[0], 14);
        __RawToReal(rawData);
    }

    void operator()(){
        Update();
    }

    void Calibrate(){
#if UFO_IMU_ADVANCED_CALIBRATION
        ufo::Trace_t::log("UFO_IMU: IMU_ADVANCED_CALIBRATION currently not implemented\n");
        // currently not implemented
        return;
#else

    uint8_t data[12]={}; // data array to hold accelerometer and gyro x, y, z, data
	uint16_t ii, packet_count, fifo_count;
	int32_t gyro_bias[3] = { 0, 0, 0 }, accel_bias[3] = { 0, 0, 0 };

    
	// reset device
	this->Write8(MPU6050_PWR_MGMT_1, 0b10000000); // Write a one to bit 7 reset bit; toggle reset device
    ufo::utl::sleep_for(100);

    this->Write8(MPU6050_PWR_MGMT_1, 0b11);
	this->Write8(MPU6050_PWR_MGMT_2, 0x00);
    ufo::utl::sleep_for(200);
    
    	// Configure device for bias calculation
	this->Write8(MPU6050_INT_ENABLE, 0x00);   // Disable all interrupts
	this->Write8(MPU6050_FIFO_EN, 0x00);      // Disable FIFO
	this->Write8(MPU6050_PWR_MGMT_1, 0x00);   // Turn on internal clock source
	this->Write8(MPU6050_I2C_MST_CTRL, 0x00); // Disable I2C master
	this->Write8(MPU6050_USER_CTRL, 0x00);    // Disable FIFO and I2C master modes
	this->Write8(MPU6050_USER_CTRL, 0b100);    // Reset FIFO
    ufo::utl::sleep_for(15);

    // Configure MPU6050 gyro and accelerometer for bias calculation
	this->Write8(MPU6050_MPU_CONFIG, 0x01);      // Set low-pass filter to 188 Hz
	this->Write8(MPU6050_SMPLRT_DIV, 0x00);  // Set sample rate to 1 kHz
	this->Write8(MPU6050_GYRO_CONFIG, 0x00);  // Set gyro full-scale to 250 degrees per second, maximum sensitivity
	this->Write8(MPU6050_ACCEL_CONFIG, 0x00); // Set accelerometer full-scale to 2 g, maximum sensitivity
    
	float  
        gyrosensitivity = 131.f,   // = 131 LSB/degrees/sec
	    accelsensitivity = 16384.f;  // = 16384 LSB/g
    
    // Configure FIFO to capture accelerometer and gyro data for bias calculation
	this->Write8(MPU6050_USER_CTRL, 0x40);   // Enable FIFO
	this->Write8(MPU6050_FIFO_EN, 0x78);     // Enable gyro and accelerometer sensors for FIFO  (max size 512 bytes in MPU-9150)
	ufo::utl::sleep_for(40); // accumulate 40 samples in 40 milliseconds = 480 bytes
    

// At end of sample accumulation, turn off FIFO sensor read
	this->Write8(MPU6050_FIFO_EN, 0x00);        // Disable gyro and accelerometer sensors for FIFO
	this->Read(MPU6050_FIFO_COUNTH, &data[0], 2); // read FIFO sample count
	fifo_count = (static_cast<int16_t>( data[0]) << 8) | data[1];
	packet_count = fifo_count / 12;// How many sets of full gyro and accelerometer data for averaging

    int16_t
        accel_temp[3] = {0, 0, 0},
        gyro_temp[3] = {0, 0, 0};

    for (ii = 0; ii < packet_count; ++ii)
	{
		
		this->Read(MPU6050_FIFO_R_W, &data[0], 12); // read data for averaging
		accel_temp[0] = (static_cast<int16_t>(data[0] << 8) | data[1]);  // Form signed 16-bit integer for each sample in FIFO
		accel_temp[1] = (static_cast<int16_t>(data[2] << 8) | data[3]);
		accel_temp[2] = (static_cast<int16_t>(data[4] << 8) | data[5]);
		gyro_temp[0] = (static_cast<int16_t>(data[6] << 8) | data[7]);
		gyro_temp[1] = (static_cast<int16_t>(data[8] << 8) | data[9]);
		gyro_temp[2] = (static_cast<int16_t>(data[10] << 8) | data[11]);

		accel_bias[0] += static_cast<int32_t>(accel_temp[0]); // Sum individual signed 16-bit biases to get accumulated signed 32-bit biases
		accel_bias[1] +=static_cast<int32_t>(accel_temp[1]);
		accel_bias[2] +=static_cast<int32_t>(accel_temp[2]);
		gyro_bias[0] += static_cast<int32_t>(gyro_temp[0]);
		gyro_bias[1] += static_cast<int32_t>(gyro_temp[1]);
		gyro_bias[2] += static_cast<int32_t>(gyro_temp[2]);
	}
    int32_t div = static_cast<int32_t>(packet_count);
    accel_bias[0] /= div; // Normalize sums to get average count biases
    accel_bias[1] /= div;
    accel_bias[2] /= div;
    gyro_bias[0] /= div;
    gyro_bias[1] /= div;
    gyro_bias[2] /= div;



#if UFO_IMU_CASTOM_POSITION
    switch (_position) {
	case UFO_SensorPosition::IMU_DIR_BASIC:
	case UFO_SensorPosition::IMU_DIR_Z_ROT_90:
	case UFO_SensorPosition::IMU_DIR_Z_ROT_180:
	case UFO_SensorPosition::IMU_DIR_Z_ROT_270:
		if (accel_bias[2] > 0L) {
			accel_bias[2] -= (int32_t)accelsensitivity; // Remove gravity from the z-axis accelerometer bias calculation
		}
		else {
			accel_bias[2] += (int32_t)accelsensitivity;
		}
		break;
	case UFO_SensorPosition::IMU_DIR_Y_ROT90_Z_ROT180:
	case UFO_SensorPosition::IMU_DIR_Y_ROT90:
		if (accel_bias[0] > 0L) {
			accel_bias[0] -= (int32_t)accelsensitivity; // Remove gravity from the z-axis accelerometer bias calculation
		}
		else {
			accel_bias[0] += (int32_t)accelsensitivity;
		}
		break;
	case UFO_SensorPosition::IMU_DIR_Y_ROT90_Z_ROT90:
	case UFO_SensorPosition::IMU_DIR_Y_ROT90_Z_ROT270:
		if (accel_bias[1] > 0L) {
			accel_bias[1] -= (int32_t)accelsensitivity; // Remove gravity from the z-axis accelerometer bias calculation
		}
		else {
			accel_bias[1] += (int32_t)accelsensitivity;
		}
		break;
	}
#else 
        if (accel_bias[2] > 0L)
    {
        accel_bias[2] -= (int32_t)accelsensitivity; // Remove gravity from the z-axis accelerometer bias calculation
    }
    else
    {
        accel_bias[2] += (int32_t)accelsensitivity;
    }
#endif

    _calibration._accelOffset._x = static_cast<float>(accel_bias[0]) / accelsensitivity;
	_calibration._accelOffset._y = static_cast<float>(accel_bias[1]) / accelsensitivity;
	_calibration._accelOffset._z = static_cast<float>(accel_bias[2]) / accelsensitivity;
	_calibration._gyroOffset._x = static_cast<float> (gyro_bias[0]) / gyrosensitivity;
	_calibration._gyroOffset._y = static_cast<float> (gyro_bias[1]) / gyrosensitivity;
	_calibration._gyroOffset._z = static_cast<float> (gyro_bias[2]) / gyrosensitivity;
#endif
        ufo::utl::sleep_for(1000);
        InitSensor();
    }

    const IMU_Calibration_t& GetOffsets() const {
        return _calibration;
    }

    const IMU_Data_t& Get() const {
        return _data;
    }

private:
    void __RawToReal(uint8_t* raw){
        int16_t magic = 0;

#if UFO_IMU_CASTOM_POSITION
        float
            ax = 0.f,
            ay = 0.f,
            az = 0.f,
            gx = 0.f,
            gy = 0.f,
            gz = 0.f;
        magic = (static_cast<int16_t>(raw[0]) << 8) | raw[1];
        ax = static_cast<float>(magic) * _accelRange - _calibration._accelOffset._x;
        magic = (static_cast<int16_t>(raw[2]) << 8) | raw[3];
        ay = static_cast<float>(magic) * _accelRange - _calibration._accelOffset._y;
        magic = (static_cast<int16_t>(raw[4]) << 8) | raw[5];
        az = static_cast<float>(magic) * _accelRange - _calibration._accelOffset._z;

        magic = (static_cast<int16_t>(raw[8]) << 8) | raw[9];
        gx = static_cast<float>(magic) * _gyroRange - _calibration._gyroOffset._x;
        magic = (static_cast<int16_t>(raw[10]) << 8) | raw[11];
        gy = static_cast<float>(magic) * _gyroRange - _calibration._gyroOffset._y;
        magic = (static_cast<int16_t>(raw[12]) << 8) | raw[13];
        gz = static_cast<float>(magic) * _gyroRange - _calibration._gyroOffset._z;

        switch (_position)
        {
        case UFO_SensorPosition::IMU_DIR_BASIC:
            _data._accel._x = ax;
            _data._accel._y = ay;
            _data._accel._z = az;
            _data._gyro._x = gx;
            _data._gyro._y = gy;
            _data._gyro._z = gz;
            break;
        case UFO_SensorPosition::IMU_DIR_Z_ROT_90:
            _data._accel._x = -ay;
            _data._accel._y = ax;
            _data._accel._z = az;
            _data._gyro._x = -gy;
            _data._gyro._y = gx;
            _data._gyro._z = gz;
            break;
        case UFO_SensorPosition::IMU_DIR_Z_ROT_180:
            _data._accel._x = -ax;
            _data._accel._y = -ay;
            _data._accel._z = az;
            _data._gyro._x = -gx;
            _data._gyro._y = -gy;
            _data._gyro._z = gz;
            break;
        case UFO_SensorPosition::IMU_DIR_Z_ROT_270:
            _data._accel._x = ay;
            _data._accel._y = -ax;
            _data._accel._z = az;
            _data._gyro._x = gy;
            _data._gyro._y = -gx;
            _data._gyro._z = gz;
            break;
        case UFO_SensorPosition::IMU_DIR_Y_ROT90_Z_ROT180:
            _data._accel._x = -az;
            _data._accel._y = -ay;
            _data._accel._z = -ax;
            _data._gyro._x = -gz;
            _data._gyro._y = -gy;
            _data._gyro._z = -gx;
            break;
        case UFO_SensorPosition::IMU_DIR_Y_ROT90_Z_ROT90:
            _data._accel._x = -az;
            _data._accel._y = ax;
            _data._accel._z = -ay;
            _data._gyro._x = -gz;
            _data._gyro._y = gx;
            _data._gyro._z = -gy;
            break;
        case UFO_SensorPosition::IMU_DIR_Y_ROT90:
            _data._accel._x = -az;
            _data._accel._y = ay;
            _data._accel._z = ax;
            _data._gyro._x = -gz;
            _data._gyro._y = gy;
            _data._gyro._z = gx;
            break;
        case UFO_SensorPosition::IMU_DIR_Y_ROT90_Z_ROT270:
            _data._accel._x = -az;
            _data._accel._y = -ax;
            _data._accel._z = ay;
            _data._gyro._x = -gz;
            _data._gyro._y = -gx;
            _data._gyro._z = gy;
            break;
        default:
            break;
        }

#else

        magic = (static_cast<int16_t>(raw[0]) << 8) | raw[1];
        _data._accel._x = static_cast<float>(magic) * _accelRange - _calibration._accelOffset._x;

        magic = (static_cast<int16_t>(raw[2]) << 8) | raw[3];
        _data._accel._y = static_cast<float>(magic) * _accelRange - _calibration._accelOffset._y;

        magic = (static_cast<int16_t>(raw[4]) << 8) | raw[5];
        _data._accel._z = static_cast<float>(magic) * _accelRange - _calibration._accelOffset._z;

        magic = (static_cast<int16_t>(raw[8]) << 8) | raw[9];
        _data._gyro._x = static_cast<float>(magic) * _gyroRange - _calibration._gyroOffset._x;

        magic = (static_cast<int16_t>(raw[10]) << 8) | raw[11];
        _data._gyro._y = static_cast<float>(magic) * _gyroRange - _calibration._gyroOffset._y;

        magic = (static_cast<int16_t>(raw[12]) << 8) | raw[13];
        _data._gyro._z = static_cast<float>(magic) * _gyroRange - _calibration._gyroOffset._z;
#endif

#if UFO_IMU_ENABLE_TEMRETURE_MODULE
        magic = (static_cast<int16_t>(raw[6]) << 8) | raw[7];
        _data = (static_cast<float>(magic) / 340.f) + 36.53f;
#endif
    }
};