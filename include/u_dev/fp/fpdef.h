#pragma once


namespace ufo
{
    namespace dev
    {
        
        enum class fp_alert_req {
            warning = 0x04,
            alarm = 0x08,
        };


        struct fp_vals
        {
            float _vout = 0.f;  // V
            float _cur = 0.f;   // A
            uint8_t _vin = 0;   // V
            int8_t _t1 = 0;     // deg C
            int8_t _t2 = 0;     // deg C
        };

    } // namespace dev
    
} // namespace ufo


