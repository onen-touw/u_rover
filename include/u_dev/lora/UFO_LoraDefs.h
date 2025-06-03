#pragma once

#include "u_sys/config.h"
#include "u_sys/utils.h"
#include "u_sys/fskdef.h"

// #define UFO_LORA_BUFFER_SIZE 180
#define UFO_LORA_M0_PIN (gpio_num_t::GPIO_NUM_2)
#define UFO_LORA_M1_PIN (gpio_num_t::GPIO_NUM_5)
#define UFO_LORA_AUX_PIN (gpio_num_t::GPIO_NUM_4)

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

enum UFO_LoraRegAddr : uint8_t
{
    LORA_REG_ADDRESS_CFG = 0x00,
    LORA_REG_ADDRESS_SPED = 0x02,
    LORA_REG_ADDRESS_TRANS_MODE = 0x03,
    LORA_REG_ADDRESS_CHANNEL = 0x04,
    LORA_REG_ADDRESS_OPTION = 0x05,
    LORA_REG_ADDRESS_CRYPT = 0x06,
    LORA_REG_ADDRESS_PID = 0x08
};

enum UFO_LoraCMD
{
    WRITE_CFG_PWR_DWN_SAVE = 0xC0,
    READ_CONFIGURATION = 0xC1,
    WRITE_CFG_PWR_DWN_LOSE = 0xC2,
    WRONG_FORMAT = 0xFF,
    // RETURNED_COMMAND 		= 0xC1,
    // SPECIAL_WIFI_CONF_COMMAND = 0xCF
};
enum UFO_LoraMode
{
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

namespace dev
{
    class lorall_t
    {
    protected:
        ufo::drv::UFO_Uart *_port = nullptr;
        UFO_LoraSettings _conf = {};

    public:
        lorall_t(ufo::drv::UFO_Uart *port)
        {
            if (!port)
            {
                ufo::Trace_t::log("!port error\n");
                return;
            }
            _port = port;
            ufo::Trace_t::log("lora constructor\n");
        }
        ~lorall_t() {}

        // 255 - broadcast
        void set_addr(uint8_t a)
        {
            _conf._targAddr._addl = a;
        }

        void set_chanel(uint8_t c)
        {
            _conf._targAddr._chan = ufo::utl::constrain(static_cast<int>(c), 0, 83);
        }

        void set_cfg(UFO_LoraSettings conf)
        {
            ufo::Trace_t::log("lora:: set-config\n");
            _conf = conf;
            // _conf._targAddr._chan = constrain(_conf._targAddr._chan, 0, 83);
            // _conf._selfAddr._chan = constrain(_conf._selfAddr._chan, 0, 83);
        }

        void setup()
        {
            ufo::Trace_t::log("lora:: setup\n");

            ufo::utl::gpio_config(UFO_LORA_AUX_PIN, gpio_mode_t::GPIO_MODE_INPUT);
            ufo::utl::gpio_config(UFO_LORA_M0_PIN, gpio_mode_t::GPIO_MODE_OUTPUT);
            ufo::utl::gpio_config(UFO_LORA_M1_PIN, gpio_mode_t::GPIO_MODE_OUTPUT);
            ufo::Trace_t::log("lora:: gpio\n");

            asm volatile("" : : "r,m"(_conf) : "memory");

            if (_conf._targAddr._chan > 84)
            {
                _conf._targAddr._chan = 20;
                ufo::Trace_t::log("Incorrect chanel; Setted to 20");
            }

            _conf.GenCfg();
            ufo::Trace_t::log("lora:: gen cfg\n");

            api_setmode(LORA_MODE_CMD);
            ufo::utl::sleep_for(10);
            ufo::Trace_t::log("lora:: set mode\n");

            api_writeCMD(_conf._cfg);
            ufo::Trace_t::log("lora:: write cmd\n");
            api_readCMD();
            ufo::Trace_t::log("lora:: read cmd\n");

            ufo::utl::sleep_for(40);

            char c[] = {READ_CONFIGURATION, 0, 8};
            api_writeMSG(c, 3);
            api_readCMD(); // todo (parse and compare)
            ufo::Trace_t::log("lora:: read cfg\n");

            api_setmode(LORA_MODE_NORMAL);
            ufo::utl::sleep_for(40);

            // todo: check if
            ufo::Trace_t::log("lora:: Setup end\n");
        }

        void api_write(char *payload, uint16_t sz)
        {
            api_addMSGaddr(payload);
            _port->SendMsg(payload, sz);
        }

        uint16_t api_read(char *buf, uint16_t &len)
        {
            uint16_t t = _port->Available();
            if (t > 1)
            {
                _port->Read(buf, t);
                len = t;
                _port->Flush();
            }
            return t; //?? return after wait --it is bad behavior
        }

        void wait_lora_done()
        {
            api_waitAUX(500);
        }

    private:
        void api_setmode(UFO_LoraMode m)
        {
            _conf._mode = m;
            switch (_conf._mode)
            {
            case LORA_MODE_NORMAL:
                gpio_set_level(UFO_LORA_M0_PIN, 0);
                gpio_set_level(UFO_LORA_M1_PIN, 0);
                break;
            case LORA_MODE_CMD:
                gpio_set_level(UFO_LORA_M0_PIN, 1);
                gpio_set_level(UFO_LORA_M1_PIN, 1);
                break;
            case LORA_MODE_WORTX:
                // todo
                break;
            default:
                gpio_set_level(UFO_LORA_M0_PIN, 0);
                gpio_set_level(UFO_LORA_M1_PIN, 0);
                break;
            }
            ufo::utl::sleep_for(40);
            api_waitAUX(200);
        }

        void api_writeCMD(char *cmd)
        {
            if (_conf._mode != LORA_MODE_CMD)
            {
                ufo::Trace_t::log("LORA ERROR: writing cmd when mode is not CMD");
                return;
            }

            if (gpio_get_level(UFO_LORA_AUX_PIN) == 0)
            {
                ufo::Trace_t::log("LORA ERROR: writing cmd when aux is 0");
                return;
            }

            char ccmd[11] = {};
            ccmd[0] = UFO_LoraCMD::WRITE_CFG_PWR_DWN_SAVE;
            ccmd[1] = 0;
            ccmd[2] = 8;
            for (size_t i = 3; i < 11; i++)
            {
                ccmd[i] = cmd[i - 3ull];
            }
            api_writeMSG(ccmd, 11);
        }

        esp_err_t /*__attribute__((optimize("O0")))*/ api_waitAUX(uint32_t ttime)
        {
            uint32_t t = ufo::utl::get_time_millis();
            asm volatile("" : : "r,m"(t) : "memory");

            while (gpio_get_level(UFO_LORA_AUX_PIN) == 0)
            {
                if ((ufo::utl::get_time_millis() - t) > ttime)
                {
                    ufo::Trace_t::log("Timeout error!");
                    return ESP_FAIL;
                }
                ufo::utl::sleep_for(1);
            }
            return ESP_OK;
        }

        void api_writeMSG(char *msg, uint32_t size)
        {
            _port->SendMsg(msg, size);
            ufo::utl::sleep_for(20);
            api_waitAUX(500);
        }

        void api_readCMD()
        {
            char buf[11] = {}; // for a while
            if (_port->Available() > 10)
            {
                _port->Read(buf, 11);
            }
            // ufo::Trace_t::flog("recv: %s\n", buf);
            _port->Flush();
        }

        void api_addMSGaddr(char *s)
        {
            // WARNING
            //_ctrlBlk->Lock() - should be locked before call this func
            s[0] = _conf._targAddr._addh;
            s[1] = _conf._targAddr._addl;
            s[2] = _conf._targAddr._chan;
        }
    };
} // namespace dev
