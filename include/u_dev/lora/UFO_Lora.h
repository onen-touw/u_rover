#pragma once

#include "UFO_LoraDefs.h"
#include "bits/unique_ptr.h"
#include "u_sys/gpio.h"
#include "u_sys/error.h"
#include "u_drivers/uart/UFO_Uart.h"
#include "u_sys/thread.h"

namespace dev
{

    class lora_t : public lorall_t, public ufo::net::fsk_base
    {
    private:
    public:
        lora_t(ufo::drv::UFO_Uart *port, callback_t cb)
            : lorall_t(port), fsk_base(cb)
        {
            printf("fsk::c-tor\n");
        }
        virtual ~lora_t() override {
            printf("fsk::d-tor\n");
        }

        virtual void snd() override
        {
            {
                ufo::lock_guard<ufo::mutex_t> _l(_snd->get_lock());

                if (_snd->GetDataBlock()._ready)
                {
                    api_write(_snd->GetDataBlock()._payload, _snd->GetDataBlock()._len);
                    _snd->GetDataBlock()._ready = false;
                    _snd->GetDataBlock()._len = 0;

                    // __AddTargetAddr(_snd->GetDataBlock()._payload);
                    // _port->SendMsg((char*)_snd->GetDataBlock()._payload, _snd->GetDataBlock()._len);
                    // _ctrlBlk._data._lastCallTick = ...
                }
            }
            ufo::utl::sleep_for(25); // ??
            wait_lora_done();        // for rcv and then wait
                                     // (otherwise: rcv-data-from-lora then
                                     // wait lora-done then copy-data-to-user)
        }

        virtual void rcv() override
        {
            uint16_t len = 0;
            api_read(_rcv->_payload, len);
            if (len)
            {
                _rcv->_len = len;
                _callback(_rcv.get());
            }
            wait_lora_done(); // for rcv and then wait
                              // (otherwise: rcv-data-from-lora then
                              // wait lora-done then copy-data-to-user)
        }
        virtual void ch_snd() override {}
        virtual void ch_rcv() override {}
    };

} // namespace dev

class lora 
{
public:
    using snd_t = ufo::net::uSocketControlBlock_t;
    using rcv_t = ufo::net::uSocketDataPacket_t;
    using msg_block_t = std::shared_ptr<snd_t>;
    using callback_t = void (*)(rcv_t *);

private:
    
    ufo::drv::UFO_Uart* _port = nullptr;
    UFO_LoraSettings _conf = {};

    std::shared_ptr<snd_t> _snd;
    std::unique_ptr<rcv_t> _rcv;
    callback_t _callback = nullptr;

public:
    lora(ufo::drv::UFO_Uart* port) {
        if (!port)
        {
            printf("!port error\n");
            return;
        }
        
        _snd = std::make_shared<snd_t>();
        _rcv = std::make_unique<rcv_t>();
        _port = port;
        printf("lora constructor\n");
    }
    ~lora() {}

    //255 - broadcast
    void SetAddr(uint8_t a){
        _conf._targAddr._addl =  a;
    }

    void SetChannel(uint8_t c){
        _conf._targAddr._chan = ufo::utl::constrain(static_cast<int> (c), 0, 83);
    }

    const std::shared_ptr<snd_t> get_block() const
    {
        return _snd;
    }

    void SetConfig(UFO_LoraSettings conf, callback_t cb){
        ufo::Trace_t::log("lora:: set-config\n");
        _conf = conf;
        // _conf._targAddr._chan = constrain(_conf._targAddr._chan, 0, 83);
        // _conf._selfAddr._chan = constrain(_conf._selfAddr._chan, 0, 83);

        _callback = cb;
        if (!_callback)
        {
            ufo::Trace_t::log("lora:: null cb\n");
        }
    }

    void Setup() {
        ufo::Trace_t::log("lora:: setup\n");

        ufo::utl::gpio_config(UFO_LORA_AUX_PIN, gpio_mode_t::GPIO_MODE_INPUT);
        ufo::utl::gpio_config(UFO_LORA_M0_PIN, gpio_mode_t::GPIO_MODE_OUTPUT);
        ufo::utl::gpio_config(UFO_LORA_M1_PIN, gpio_mode_t::GPIO_MODE_OUTPUT);
        ufo::Trace_t::log("lora:: gpio\n");

        asm volatile ("" : : "r,m"(_conf) : "memory");

        if (_conf._targAddr._chan > 84)
        {
            _conf._targAddr._chan = 20;
            ufo::Trace_t::log("Incorrect chanel; Setted to 20");
        }
        // if (_port->GetBaudRate() != 9600)
        // {
        //     ufo::Trace_t::log("br isnt 9600");
        // }
        
        _conf.GenCfg();
        ufo::Trace_t::log("lora:: gen cfg\n");

        SetMode(LORA_MODE_CMD);
        ufo::utl::sleep_for(10);
        ufo::Trace_t::log("lora:: set mode\n");
        
        WriteCMD(_conf._cfg);
        ufo::Trace_t::log("lora:: write cmd\n");
        ReadCMD();
        ufo::Trace_t::log("lora:: read cmd\n");

        ufo::utl::sleep_for(40);

        char c[] = { READ_CONFIGURATION, 0, 8};
        WriteMSG(c, 3);
        ReadCMD();            //todo (parse and compare)
        ufo::Trace_t::log("lora:: read cfg\n");

        SetMode(LORA_MODE_NORMAL);
        ufo::utl::sleep_for(40);

        // todo: check if 
        ufo::Trace_t::log("lora:: Setup end\n");
    }
    
    void Iteration () {
        ReadMSG();
        WriteBlock();
        // char c[10] = {};
        // c[0] = 0xff;
        // c[1] = 0xff;
        // c[2] = 2;
        // c[3] = 'f';
        // c[4] = 'a';
        // c[5] = 'a';
        // c[6] = 'f';
        // WriteMSG(c, 7);
    }
    

#pragma region
private:
    esp_err_t /*__attribute__((optimize("O0")))*/ WaitAUX_True(int64_t ttime)
    {
        int64_t t = ufo::utl::get_time_millis();
        asm volatile ("" : : "r,m"(t) : "memory");

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

    void WriteCMD(char *cmd)
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
        WriteMSG(ccmd, 11);
    }


    void __AddTargetAddr(char* s) {
        //WARNING 
        //_ctrlBlk->Lock() - should be locked before call this func
        s[0] = _conf._targAddr._addh;
        s[1] = _conf._targAddr._addl;
        s[2] = _conf._targAddr._chan;
    }

    void WriteBlock()
    {
        {
            ufo::lock_guard<ufo::mutex_t> _l(_snd->get_lock());

            if (_snd->GetDataBlock()._ready)
            {
                __AddTargetAddr(_snd->GetDataBlock()._payload);

                _port->SendMsg((char*)_snd->GetDataBlock()._payload, _snd->GetDataBlock()._len);

                _snd->GetDataBlock()._ready = false;
                _snd->GetDataBlock()._len = 0;
                // _ctrlBlk._data._lastCallTick = ...
            }
        }
        ufo::utl::sleep_for(25);
        WaitAUX_True(500);
    }

    void WriteMSG(char *msg, uint32_t size)
    {
        {
            ufo::lock_guard<ufo::mutex_t> _l(_snd->get_lock());
            _port->SendMsg(msg, size);
        }
        // printf("==");
        ufo::utl::sleep_for(20);
        WaitAUX_True(500);
    }

    void ReadMSG(){
        // int  i = Serial2.available();
        // if (i > 1)
        // {
        //     Serial.print("catch: ");
        //     // __Func();
        //     Serial.println(Serial2.readString());
        //     WaitAUX_True(1000);
        // }
        // __FlushPort();

        memset(_rcv->_payload, 0,_rcv->_len);
        if (_port->Available() > 1)
        {
            _rcv->_len = _port->Read(_rcv->_payload);
            _callback(_rcv.get());
        }
        WaitAUX_True(500);
        _port->Flush();
    }

    void ReadCMD(){
        char buf[11] = {}; // for a while
        if (_port->Available() > 1)
        {
            _port->Read(buf, 11);
        }
        // ufo::Trace_t::flog("recv: %s\n", buf);
        __FlushPort();
    }

    void __FlushPort(){
        char buf[100];
        while (_port->Available() > 1)
        {
            _port->Read(buf, 100);
        }
    }

    void SetMode(UFO_LoraMode m)
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
        WaitAUX_True(200);
    }
#pragma endregion 

};
