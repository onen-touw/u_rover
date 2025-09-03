#pragma once

#include "u_sys/fsk.h"
#include "u_sys/list.h"
#include "u_sys/thread.h"

#include <tuple>
#include <bits/shared_ptr.h>

#include "u_dev/lora/UFO_Lora.h"

enum class net_descriptors_t {
    sock_main,
    sock_1,
    lora,
};

class nettt_t
{
public:
    using sock_t = ufo::net::fsk;    
    using lora_t = dev::lora_t;
    using desc_t = uint16_t;
    using msg_block_t = std::shared_ptr<ufo::net::uSocketControlBlock_t>;
    
private:
    class ts_t;

    using base_t = ufo::net::fsk_base;
    using tlst_t = ufo::list_t<ts_t>;
    using titer_t = tlst_t::simple_iterator_t;

private:
    // lst_t _net_interfaces;
    tlst_t _threads;
    static desc_t _d;
public:
    enum class behav_t {
        wait,
        kill,
    };

private:
    class ts_t
    {
    private:
        ufo::thread _t;
        desc_t _d = 0;
        behav_t _b = behav_t::kill;
    public:
        
        template <typename... Args>
        ts_t(desc_t d,  ufo::thread_cfg cfg, Args... args) : _t(cfg, std::forward<Args>(args)...), _d(d) {}

        void set_b(behav_t b) { _b = b; }
        desc_t get_d() const {return  _d;}

        ~ts_t() {
            printf("ts::d-tor\n");

            if (_t.joinable())
            {
                if (_b == behav_t::kill)
                {
                    _t.terminate();
                    return;
                }
                _t.join();
            }
        }
    };
    
public:
    explicit nettt_t(){} 
    ~nettt_t(){
        printf("~nett");
    }

    // descriptor
    msg_block_t mk(desc_t& d, std::unique_ptr<base_t> sock) {
        if (!sock)
        {
            d = 0;
            return msg_block_t();
        }
        
        msg_block_t blk = sock->get_block();

        ufo::thread_cfg cfg;
        cfg._name = "net";
        cfg._core = 0;
        cfg._prio = 5;
        cfg._stackSize = 4096;

        _threads.emplace_back(++_d, cfg, task, std::move(sock));
        d = _d;
        return blk;
    }

    void rm(desc_t d, behav_t b = behav_t::wait) {
        if (d > _d)
        {
            return;
        }

        _threads.pop_if(
            d, 
            [](const ts_t &obj, const desc_t desct)
            {
                if (obj.get_d() == desct)
                {
                    return true;
                }
                return false; 
            });
    }

private:
    static void task(std::unique_ptr<base_t> ptr, ufo::token_t token)
    {
        printf("task-in\n");
        while (token)
        {
            ptr->snd();
            ptr->rcv();
            ptr->ch_snd();
            ptr->ch_rcv();
        }
        printf("task-out\n");
        
    }
};
 
nettt_t::desc_t nettt_t::_d = 0;