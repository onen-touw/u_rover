#pragma once

#include "u_drivers/pwm/sndgen.h"
#include "app/appconfig.h"

class bazz_t
{
public:
    using gen_t = ufo::drv::sndgen_t;

private:
    gen_t _gen;
public:
    bazz_t() {
        _gen.setup(app::appconfig::gpio_buzz);
    }

    void update(uint16_t freq) {
        _gen.write(freq);
    }

    ~bazz_t() {}
};