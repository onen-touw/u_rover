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

			_conf._selfAddr._addh = 0;
			_conf._selfAddr._addl = 23;
			_conf._selfAddr._chan = 10;
			
			_conf._targAddr._addh = UFO_LORA_BROADCAST;
			_conf._targAddr._addl = UFO_LORA_BROADCAST;
			_conf._targAddr._chan = 10;
			_conf.adrt = LORA_AIR_DATA_RATE_110_384;

            setup();
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
            // ufo::utl::sleep_for(5); // ??

            wait_lora_done();        // for rcv and then wait
            //                          // (otherwise: rcv-data-from-lora then
            //                          // wait lora-done then copy-data-to-user)
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