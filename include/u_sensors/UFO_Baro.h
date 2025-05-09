#pragma once

#include "u_sys/config.h"
#include "u_drivers/i2c/UFO_I2C_Interface.h"
#include "u_sys/trace.h"
#include "u_sys/error.h"
#include "u_sys/utils.h"
#include "u_math/u_math.h"

#define UFO_BME280_ADDRESS ((uint8_t)0x76) // Primary I2C Address
#define UFO_BARO_ENABLE_HUMIDITY    UFO_DISABLE
// now only bme280

struct UFO_BaroData_t
{
    float Tempreture = 0.f;
    float Presure = 0.f;
    float Altitude = 0.f;
};

class UFO_Baro : private ufo::UFO_I2C_Interface
{
private:
    enum : uint8_t
    {
        BME280_REGISTER_DIG_T1 = 0x88,
        BME280_REGISTER_DIG_T2 = 0x8A,
        BME280_REGISTER_DIG_T3 = 0x8C,

        BME280_REGISTER_DIG_P1 = 0x8E,
        BME280_REGISTER_DIG_P2 = 0x90,
        BME280_REGISTER_DIG_P3 = 0x92,
        BME280_REGISTER_DIG_P4 = 0x94,
        BME280_REGISTER_DIG_P5 = 0x96,
        BME280_REGISTER_DIG_P6 = 0x98,
        BME280_REGISTER_DIG_P7 = 0x9A,
        BME280_REGISTER_DIG_P8 = 0x9C,
        BME280_REGISTER_DIG_P9 = 0x9E,

#if UFO_BARO_ENABLE_HUMIDITY
        BME280_REGISTER_DIG_H1 = 0xA1,
        BME280_REGISTER_DIG_H2 = 0xE1,
        BME280_REGISTER_DIG_H3 = 0xE3,
        BME280_REGISTER_DIG_H4 = 0xE4,
        BME280_REGISTER_DIG_H5 = 0xE5,
        BME280_REGISTER_DIG_H6 = 0xE7,
#endif

        BME280_REGISTER_CHIPID = 0xD0,
        BME280_REGISTER_VERSION = 0xD1,
        BME280_REGISTER_SOFTRESET = 0xE0,

        BME280_REGISTER_CAL26 = 0xE1, // R calibration stored in 0xE1-0xF0
#if UFO_BARO_ENABLE_HUMIDITY
        BME280_REGISTER_CONFIG_HUMID = 0xF2,
#endif
        BME280_REGISTER_STATUS = 0XF3,
        BME280_REGISTER_CONFIG_MEASURE = 0xF4,
        BME280_REGISTER_CONFIG = 0xF5,
        BME280_REGISTER_PRESSUREDATA = 0xF7,
        BME280_REGISTER_TEMPDATA = 0xFA,
#if UFO_BARO_ENABLE_HUMIDITY
        BME280_REGISTER_HUMIDDATA = 0xFD
#endif

    };
    struct
    {
        uint16_t dig_T1; ///< temperature compensation value
        int16_t dig_T2;  ///< temperature compensation value
        int16_t dig_T3;  ///< temperature compensation value

        uint16_t dig_P1; ///< pressure compensation value
        int16_t dig_P2;  ///< pressure compensation value
        int16_t dig_P3;  ///< pressure compensation value
        int16_t dig_P4;  ///< pressure compensation value
        int16_t dig_P5;  ///< pressure compensation value
        int16_t dig_P6;  ///< pressure compensation value
        int16_t dig_P7;  ///< pressure compensation value
        int16_t dig_P8;  ///< pressure compensation value
        int16_t dig_P9;  ///< pressure compensation value

#if UFO_BARO_ENABLE_HUMIDITY
        uint8_t dig_H1; ///< humidity compensation value
        int16_t dig_H2; ///< humidity compensation value
        uint8_t dig_H3; ///< humidity compensation value
        int16_t dig_H4; ///< humidity compensation value
        int16_t dig_H5; ///< humidity compensation value
        int8_t dig_H6;  ///< humidity compensation value
#endif
    } _calib;

    enum Sampling :uint8_t
    {
        SAMPLING_NONE = 0b000,
        SAMPLING_X1 = 0b001,
        SAMPLING_X2 = 0b010,
        SAMPLING_X4 = 0b011,
        SAMPLING_X8 = 0b100,
        SAMPLING_X16 = 0b101
    };
    enum Mode:uint8_t
    {
        MODE_SLEEP = 0b00,
        MODE_FORCED = 0b01,
        MODE_NORMAL = 0b11
    };
    enum Filter:uint8_t
    {
        FILTER_OFF = 0b000,
        FILTER_X2 = 0b001,
        FILTER_X4 = 0b010,
        FILTER_X8 = 0b011,
        FILTER_X16 = 0b100
    };
	enum Duration:uint8_t
	{
		STANDBY_MS_0_5 = 0b000,
		STANDBY_MS_10 = 0b110,
		STANDBY_MS_20 = 0b111,
		STANDBY_MS_62_5 = 0b001,
		STANDBY_MS_125 = 0b010,
		STANDBY_MS_250 = 0b011,
		STANDBY_MS_500 = 0b100,
		STANDBY_MS_1000 = 0b101
	};

private:
// todo
    using _int = int32_t;
    using _uint = uint32_t;
    using _shrt = int16_t;
    using _ushrt = uint16_t;
    using _byte = uint8_t;

	_int _tFine = 0;	   // fine tempreture (for this class like global var // see DS 4.2.3)
	int32_t _tAdjast = 0; // add to compensate temp readings
	float _seaLevelPre = 101325.f;  // todo correct this value
    
    UFO_BaroData_t _data;
    _int 
        _rawT = 0,
        _rawP = 0;
public:
    UFO_Baro(ufo::drv::UFO_I2C_Driver* driver) {
        esp_err_t err = this->Init(UFO_BME280_ADDRESS, driver);
        if (err!= ESP_OK)
        {
            //SetCritical;      //think about it
            ufo::Trace_t::log("driver initializing priblem\n");
        }
        
    }
    ~UFO_Baro() {}

    //todo:: errors
    void InitSensor()
    {
        // check if sensor, i.e. the chip ID is correct
        // int32_t _sensorID = this->Read8(BME280_REGISTER_CHIPID);
        // Serial.println(_sensorID);
        // if (_sensorID != 0x60){
        //     return false;
        // }

        // reset the device using soft-reset
        // this makes sure the IIR is off, etc.
        this->Write8(BME280_REGISTER_SOFTRESET, 0xB6);
        // wait for chip to wake up.
        ufo::utl::sleep_for(10);

        // if chip is still reading calibration, delay
        while (__IsReadingCalibration())
        {
            ufo::utl::sleep_for(10);

            ufo::Trace_t::log('.');
        }
        // Serial.println();
        ufo::Trace_t::log('\n');

        __ReadCoefficients(); // read trimming parameters, see DS 4.2.2

        SetConfig(); // use defaults

        ufo::utl::sleep_for(100);
    }

    void SetConfig(
        Mode mode = MODE_NORMAL,
        Sampling tempSampling = SAMPLING_X1,
        Sampling pressSampling = SAMPLING_X16,
        Filter filter = FILTER_X16,
        Duration duration = STANDBY_MS_0_5)
    {
        // making sure sensor is in sleep mode before setting configuration
        // as it otherwise may be ignored
        this->Write8(BME280_REGISTER_CONFIG_MEASURE, MODE_SLEEP);

        // you must make sure to also set REGISTER_CONTROL after setting the
        // CONTROLHUMID register, otherwise the values won't be applied (see
        // DS 5.4.3)
        uint8_t conf = 0;
        #if UFO_BARO_ENABLE_HUMIDITY
        conf = SAMPLING_X1;
        this->Write8(BME280_REGISTER_CONFIG_HUMID, conf);
        #endif

        conf = (duration << 5) | (filter << 2) | 0;
        this->Write8(BME280_REGISTER_CONFIG, conf); //equal 0b000_100_00 by default

        conf = (tempSampling << 5) | (pressSampling << 2) | mode;
        this->Write8(BME280_REGISTER_CONFIG_MEASURE, conf); // equal 0b001_010_11 by default

        // this->Write8(BME280_REGISTER_CONFIG_MEASURE, conf); // equal 0b001_011_11
}

//new: 
//todo check and compare
void Update(){
    _rawT = this->Read24(BME280_REGISTER_TEMPDATA);
    _rawP = this->Read24(BME280_REGISTER_PRESSUREDATA);
    __RawToReal();
}

const UFO_BaroData_t& Get() const {
    return _data;
}

void SetTemperatureCompensation(float adjustment)
{
	// convert the value in C into and adjustment to t_fine
	_tAdjast = ((static_cast<int32_t>(adjustment * 100) << 8)) / 5;
};

void operator ()() {
    Update();
}

private:
void __ReadCoefficients()
{   
	_calib.dig_T1 = this->Read16_LittleEndian(BME280_REGISTER_DIG_T1);   
	_calib.dig_T2 = this->Read16_Signed_LittleEndian(BME280_REGISTER_DIG_T2);   
	_calib.dig_T3 = this->Read16_Signed_LittleEndian(BME280_REGISTER_DIG_T3);   

	_calib.dig_P1 = this->Read16_LittleEndian(BME280_REGISTER_DIG_P1);
	_calib.dig_P2 = this->Read16_Signed_LittleEndian(BME280_REGISTER_DIG_P2);
	_calib.dig_P3 = this->Read16_Signed_LittleEndian(BME280_REGISTER_DIG_P3);
	_calib.dig_P4 = this->Read16_Signed_LittleEndian(BME280_REGISTER_DIG_P4);
	_calib.dig_P5 = this->Read16_Signed_LittleEndian(BME280_REGISTER_DIG_P5);
	_calib.dig_P6 = this->Read16_Signed_LittleEndian(BME280_REGISTER_DIG_P6);
	_calib.dig_P7 = this->Read16_Signed_LittleEndian(BME280_REGISTER_DIG_P7);
	_calib.dig_P8 = this->Read16_Signed_LittleEndian(BME280_REGISTER_DIG_P8);
	_calib.dig_P9 = this->Read16_Signed_LittleEndian(BME280_REGISTER_DIG_P9);

#if UFO_BARO_ENABLE_HUMIDITY
	_calib.dig_H1 = this->Read8(BME280_REGISTER_DIG_H1);
	_calib.dig_H2 = this->Read16_Signed_LittleEndian(BME280_REGISTER_DIG_H2);
	_calib.dig_H3 = this->Read8(BME280_REGISTER_DIG_H3);
	_calib.dig_H4 = (static_cast<int8_t>(this->Read8(BME280_REGISTER_DIG_H4)) << 4) |
						   (this->Read8(BME280_REGISTER_DIG_H4 + 1) & 0xF);
	_calib.dig_H5 = (static_cast<int8_t>(this->Read8(BME280_REGISTER_DIG_H5 + 1)) << 4) |
						   (this->Read8(BME280_REGISTER_DIG_H5) >> 4);
	_calib.dig_H6 = static_cast<int8_t>(this->Read8(BME280_REGISTER_DIG_H6));
#endif
}
// if ret zero then bme ready to work
bool __IsReadingCalibration()
{
	uint8_t const rStatus = this->Read8(BME280_REGISTER_STATUS);
	return (rStatus & (1 << 0)) != 0;
}


void __RawToReal(){

    //tempreture like in DS 4.2.3
    _int
        magic1 = 0,
        magic2 = 0;
    _rawT >>= 4;
    magic1 = (((_rawT >> 3) - static_cast<_int>(_calib.dig_T1 << 1)) * (static_cast<_int>(_calib.dig_T2))) >> 11;
    magic2 = (((((_rawT >> 4) - static_cast<_int>(_calib.dig_T1)) * ((_rawT >> 4) - static_cast<_int>(_calib.dig_T1))) >> 12) * (static_cast<_int>(_calib.dig_T3))) >> 14;
    _tFine = magic1 + magic2 + _tAdjast; // where t_fine_adjust is not degC

    _data.Tempreture = static_cast<float>(((_tFine * 5 + 128) >> 8)) / 100.f;   // degC
    // Serial.print(">T:");
    // Serial.println (_data.Tempreture);

    // pressure like in DS 4.2.3
    int64_t
        magic11 = 0,
        magic22 = 0,
        magicP = 0;
    magic11 = static_cast<int64_t>(_tFine) - 128000;
    magic22 =  magic11*magic11*static_cast<int64_t>(_calib.dig_P6);
    magic22 = magic22 + ((magic11 * static_cast<int64_t>(_calib.dig_P5)) << 17); //+
    magic22 = magic22 + (static_cast<int64_t>(_calib.dig_P4) << 35);
    magic11 = ((magic11 * magic11 * static_cast<int64_t>(_calib.dig_P3)) >> 8) + ((magic11 * static_cast<int64_t>(_calib.dig_P2)) << 12);
    magic11 = ((1ll << 47) + magic11) * (static_cast<int64_t>(_calib.dig_P1)) >> 33; // +
    if (magic11 == 0)
    {
        return;
    }
    _rawP >>= 4;
    magicP = 1048576 - _rawP;
    magicP = (((magicP << 31) - magic22) * 3125) / magic11; // +
    magic11 = (static_cast<int64_t>(_calib.dig_P9) * (magicP >>13 ) * (magicP >>13)) >> 25; // +
    magic22 = (static_cast<int64_t> (_calib.dig_P8) * magicP) >> 19;
    magicP = ((magicP + magic11 + magic22) >> 8) + (static_cast<int64_t>(_calib.dig_P7) << 4);

    // pascal
    _data.Presure = static_cast<float>(magicP)/256.f;
    // calculating altitude by pressure
    _data.Altitude = 44330.0f * (1.0f - std::pow((_data.Presure) / _seaLevelPre, 0.1903f));
}

};


