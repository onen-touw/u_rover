#pragma once

#include "u_sys/fsk.h"
#include "u_sys/list.h"
#include "u_sys/thread.h"

#include <tuple>
#include <bits/shared_ptr.h>

enum class net_descriptors_t {
    sock_main,
    sock_,
    lora_,
};

class net_t
{
    using sock_t = ufo::net::fast_sock;    
    using lora_t = nullptr_t;   // not implemented
    
    using base_t = ufo::net::fast_sock;
    using item_t = std::unique_ptr<base_t>;

    using s_pair_t = std::pair<net_descriptors_t, item_t>;
    using lst_t = ufo::list_t<s_pair_t>;
    using iter_t = lst_t::simple_iterator_t;
    using block_t = ufo::net::uSocketControlBlock_t;
public:
    using msg_block_t = std::shared_ptr<block_t>;
private:
    lst_t _net_interfaces;
    
public:
    net_t() {}
    
    ~net_t() {}

    bool mk_sock(net_descriptors_t d, const char* addr, sock_t::sockt_t type, sock_t::callback_t cb, const char* src = nullptr){
        if (!_net_interfaces.empty())
        {
            for (iter_t it = _net_interfaces.begin(); it; ++it)
            {
                if (it->first == d)
                {
                    return false;
                }
            }
        }
        item_t item = std::make_unique<sock_t>(addr, type, cb);

        if (src)
        {
            item->set_source(src);
        }
        _net_interfaces.emplace_back(s_pair_t(d, std::move(item)));
        
        return true;
    }

    void rm_sock(net_descriptors_t d){
        _net_interfaces.pop_if(d, [](const s_pair_t& obj, const net_descriptors_t& dd){
            if (obj.first == dd)
            {
                return true;
            }
            return false;
        });
    }

    // if not found return empty shared_ptr
    std::shared_ptr<block_t> get_block(net_descriptors_t d) {

        if (!_net_interfaces.empty())
        {
            for (iter_t it = _net_interfaces.begin(); it; ++it)
            {
                if (it->first == d)
                {
                    return it->second->get_block();
                }
            }
        }
        return std::shared_ptr<block_t>();
    }


    void task(ufo::token_t token){

        while (token)
        {
            if (!_net_interfaces.empty())
            {
                for (iter_t it = _net_interfaces.begin(); it; ++it)
                {
				    // printf("net::task-%d\n", __LINE__);
                    it->second->snd();
                    it->second->rcv();
                }
            }
            ufo::utl::sleep_for(5);
        }
    }

};