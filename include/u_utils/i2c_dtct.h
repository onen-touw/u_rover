#pragma once 
#include "u_drivers/i2c/UFO_I2C_driver.h"

namespace ufo
{
    void i2c_detecter_console(drv::UFO_I2C_Driver *driver)
    {

        // uint8_t inaAddrArr[127] = {};
        Trace_t::log("i2c-dev list:\n");

        uint8_t sz = 0;
        for (uint8_t address = 1; address < 127; ++address)
        {
            if (!driver->ZeroWrite(address))
            {
                Trace_t::flog("\tFound on adress:%u [0x%x]\n", address, address);
                ++sz;
            }
        }
        if (sz == 0)
        {
            Trace_t::log("\tno devices\n");
        }
    }

} // namespace ufo

