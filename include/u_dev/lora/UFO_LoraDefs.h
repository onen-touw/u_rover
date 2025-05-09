#pragma once

#include "u_sys/config.h"
#include "u_sys/utils.h"
#include "u_sys/fskdef.h"

// #define UFO_LORA_BUFFER_SIZE 180
#define UFO_LORA_M0_PIN     (gpio_num_t::GPIO_NUM_2)
#define UFO_LORA_M1_PIN     (gpio_num_t::GPIO_NUM_5)
#define UFO_LORA_AUX_PIN    (gpio_num_t::GPIO_NUM_4)

#define UFO_LORA_OFFSET_FOR_ADDR ((uint8_t)3)

#define UFO_LORA_BROADCAST 0xff

enum UFO_LoraResponseStatus : uint8_t
{
    LORA_SUCCESS = 1,
    LORA_ERR_UNKNOWN, /* something shouldn't happened */
    LORA_ERR_NOT_SUPPORT,
    LORA_ERR_NOT_IMPLEMENT,
    LORA_ERR_NOT_INITIAL,
    LORA_ERR_INVALID_PARAM,
    LORA_ERR_DATA_SIZE_NOT_MATCH,
    LORA_ERR_BUF_TOO_SMALL,
    LORA_ERR_TIMEOUT,
    LORA_ERR_HARDWARE,
    LORA_ERR_HEAD_NOT_RECOGNIZED,
    LORA_ERR_NO_RESPONSE_FROM_DEVICE,
    LORA_ERR_WRONG_UART_CONFIG,
    LORA_ERR_WRONG_FORMAT,
    LORA_ERR_PACKET_TOO_BIG
};

enum UFO_LoraUartBPS : uint8_t
{
    LORA_UART_BPS_1200 = 0b000,
    LORA_UART_BPS_2400 = 0b001,
    LORA_UART_BPS_4800 = 0b010,
    LORA_UART_BPS_9600 = 0b011,
    LORA_UART_BPS_19200 = 0b100,
    LORA_UART_BPS_38400 = 0b101,
    LORA_UART_BPS_57600 = 0b110,
    LORA_UART_BPS_115200 = 0b111
};

enum UFO_LoraAirDataRate : uint8_t
{
    LORA_AIR_DATA_RATE_000_24 = 0b000,
    LORA_AIR_DATA_RATE_001_24 = 0b001,
    LORA_AIR_DATA_RATE_010_24 = 0b010,
    LORA_AIR_DATA_RATE_011_48 = 0b011,
    LORA_AIR_DATA_RATE_100_96 = 0b100,
    LORA_AIR_DATA_RATE_101_192 = 0b101,
    LORA_AIR_DATA_RATE_110_384 = 0b110,
    LORA_AIR_DATA_RATE_111_625 = 0b111
};

enum UFO_LoraParity : uint8_t
{
    LORA_MODE_00_8N1 = 0b00,
    LORA_MODE_01_8O1 = 0b01,
    LORA_MODE_10_8E1 = 0b10,
    LORA_MODE_11_8N1 = 0b11
};

enum UFO_LoraSubpacketSetting : uint8_t
{
    LORA_SPS_200_00 = 0b00,
    LORA_SPS_128_01 = 0b01,
    LORA_SPS_064_10 = 0b10,
    LORA_SPS_032_11 = 0b11
};

// todo
enum UFO_LoraRSSIorAmbientNoise : uint8_t
{
    LORA_RSSI_AMBIENT_NOISE_DISABLED = 0b0,
    LORA_RSSI_DISABLED = 0b0,
    LORA_RSSI_AMBIENT_NOISE_ENABLED = 0b1,
    LORA_RSSI_ENABLED = 0b1,
};

enum UFO_LoraWOR_Period : uint8_t
{
    LORA_WOR_500_000 = 0b000,
    LORA_WOR_1000_001 = 0b001,
    LORA_WOR_1500_010 = 0b010,
    LORA_WOR_2000_011 = 0b011,
    LORA_WOR_2500_100 = 0b100,
    LORA_WOR_3000_101 = 0b101,
    LORA_WOR_3500_110 = 0b110,
    LORA_WOR_4000_111 = 0b111
};

enum UFO_LoraTransmitionPower : uint8_t
{
    LORA_POWER_22dbm = 0b00,
    LORA_POWER_17dbm = 0b01,
    LORA_POWER_13dbm = 0b10,
    LORA_POWER_10dbm = 0b11
};

enum UFO_LoraRegAddr : uint8_t {
    LORA_REG_ADDRESS_CFG = 0x00,
    LORA_REG_ADDRESS_SPED = 0x02,
    LORA_REG_ADDRESS_TRANS_MODE = 0x03,
    LORA_REG_ADDRESS_CHANNEL = 0x04,
    LORA_REG_ADDRESS_OPTION = 0x05,
    LORA_REG_ADDRESS_CRYPT = 0x06,
    LORA_REG_ADDRESS_PID = 0x08
};

enum UFO_LoraCMD {
    WRITE_CFG_PWR_DWN_SAVE = 0xC0,
    READ_CONFIGURATION = 0xC1,
    WRITE_CFG_PWR_DWN_LOSE = 0xC2,
    WRONG_FORMAT = 0xFF,
    // RETURNED_COMMAND 		= 0xC1,
    // SPECIAL_WIFI_CONF_COMMAND = 0xCF
};
enum UFO_LoraMode {
    LORA_MODE_NORMAL,
    LORA_MODE_WORTX,
    LORA_MODE_SLEEP = 2,
    LORA_MODE_CMD = 2,
};

struct UFO_LoraAddr
{
    uint8_t _addh = UFO_LORA_BROADCAST;
    uint8_t _addl = UFO_LORA_BROADCAST;
    uint8_t _chan = 255;
};


struct UFO_LoraSettings
{
    UFO_LoraAddr _selfAddr = {};
    UFO_LoraAddr _targAddr = {};
    UFO_LoraMode _mode = LORA_MODE_SLEEP;
    UFO_LoraUartBPS br = LORA_UART_BPS_9600;
    UFO_LoraParity prty = LORA_MODE_00_8N1;
    UFO_LoraAirDataRate adrt = LORA_AIR_DATA_RATE_010_24;
    UFO_LoraSubpacketSetting sbset = LORA_SPS_200_00;
    UFO_LoraRSSIorAmbientNoise ambns = LORA_RSSI_AMBIENT_NOISE_DISABLED;
    UFO_LoraTransmitionPower trsmpwr = LORA_POWER_22dbm;
    UFO_LoraRSSIorAmbientNoise rssi = LORA_RSSI_DISABLED;
    UFO_LoraWOR_Period worperi = LORA_WOR_500_000;
    uint8_t crypthi = 0;
    uint8_t cryptlo = 0;
    char _cfg[8] = {};

    void GenCfg()
    {
        // printf("gen start\n");
        _cfg[0] = _selfAddr._addh;
        _cfg[1] = _selfAddr._addl;

        uint8_t
            tmp = 0b00000000;

        tmp |= (static_cast<uint8_t>(br) << 5);
        tmp |= (static_cast<uint8_t>(prty) << 3);
        tmp |= (static_cast<uint8_t>(adrt));
        _cfg[2] = tmp;

        tmp = 0b00000000;
        tmp |= (static_cast<uint8_t>(sbset) << 6);
        tmp |= (static_cast<uint8_t>(ambns) << 5);
        // other bits are reserved (4,3,2)
        tmp |= static_cast<uint8_t>(trsmpwr);
        _cfg[3] = tmp;
        tmp = 0b00000000;

        if (_selfAddr._chan > 84)
        {
           _selfAddr._chan = 83;
        }

        _cfg[4] = (tmp | _selfAddr._chan);

        tmp = 0b00000000;
        tmp |= (rssi << 7);
        // fixed transmition only
        tmp |= (0b1 << 6);
        // 5th bit is reserved
        tmp |= (0b0 << 4);
        // 3th bit is reserved
        tmp |= (static_cast<uint8_t>(worperi));
        _cfg[5] = tmp;

        _cfg[6] = crypthi; // crypt hi
        _cfg[7] = cryptlo; // crypt lo

        // printf("gen end\n");
    }


    
};
