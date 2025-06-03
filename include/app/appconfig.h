#pragma once
#include <stdint.h>
#include "sdkconfig.h"
#include "soc/gpio_num.h"
// ===================== minimal requared ===================== 

#define UFO_WIFI
// #define UFO_WIFI_DEFAULT_START_AP
#define UFO_WIFI_DEFAULT_START_STA

// #define UFO_I2C_SOFT     // use i2c second driver
// #define UFO_SPI             // use spi
// #define UFO_SPI_CNT 1       // 2 max
#define UFO_UART_CNT 2      // 3 max

// ============================================================ 
namespace app
{
    class appconfig
    {
    public:
        static constexpr gpio_num_t gpio_buzz = gpio_num_t::GPIO_NUM_23; 
    };
}