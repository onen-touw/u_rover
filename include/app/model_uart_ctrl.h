#pragma once

#include "u_sys/thread.h"
#include "u_sys/mutex.h"
#include "u_sys/fskdef.h"

#include "appdata.h"
#include "u_drivers/drv.h"

namespace model
{
    class model_uart_ctrl_t
    {
    public:
        using rcv_t = ufo::net::fsk_base::rcv_t;
        using callback_t = void (*)(rcv_t*);
        using uart_t = ufo::drv::UFO_Uart;

    private:
        struct snd_bf_t
        {
            char *_payload = nullptr;
            uint32_t _len = 0;
        };

        static QueueHandle_t _q;
        ufo::thread _thr;

    public:
        model_uart_ctrl_t(uart_t* uart, callback_t cb) {
            if (!uart)
            {
                // error
                return;
            }
            if (!cb)
            {
                // error
                return;
            }

            _q = xQueueCreate(5, sizeof(snd_bf_t));
            if (!_q)
            {
                // error
            }

            _thr = ufo::thread(uart_control_task, uart, cb);
        }

        ~model_uart_ctrl_t() {
            _thr.terminate();
            vQueueDelete(_q);
        }

        void write(char *msg, uint32_t sz)
        {
            snd_bf_t snd = {msg, sz};
            xQueueSend(_q, &snd, 20);
        }

    private:
        static void uart_control_task(ufo::drv::UFO_Uart *uart, callback_t callback, ufo::token_t token)
        {
            rcv_t buffer;
            snd_bf_t snd = {};

            while (token)
            {
                uint32_t av = uart->Available();
                if (av > 0)
                {
                    auto sz = uart->Read(buffer._payload, std::min(av, uint32_t(UFO_SOCKET_BUFFER_SIZE)));
                    buffer._len = sz;
                    buffer._ready = true;
                    buffer._lastCallTick = ufo::utl::get_time_millis();

                    callback(&buffer);
                }

                if (xQueueReceive(_q, &snd, 10))
                {
                    if (snd._len && snd._payload)
                    {
                        uart->SendMsg(snd._payload, snd._len);
                    }
                }
            }
        }
    };
    QueueHandle_t model_uart_ctrl_t::_q = nullptr;

} // namespace model
