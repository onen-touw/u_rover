#pragma once
#include <stdint.h>
#include "sdkconfig.h"
#include "soc/gpio_num.h"
// ===================== minimal requared ===================== 

#define UFO_WIFI
#define UFO_WIFI_USE_DEFAULT_CONFIG 1       // default parameters

#define UFO_WIFI_AUTO_ENABLE 1              // enable wifi on start system (work only with default_config)
#define UFO_WIFI_USE_STATIC_IP 1            // use by default static ip (0 for DHCP)



#if UFO_WIFI_AUTO_ENABLE && UFO_WIFI_USE_DEFAULT_CONFIG 
#   define UFO_WIFI_DEFAULT_START_AP
// #    define UFO_WIFI_DEFAULT_START_STA
#endif


#if UFO_WIFI_USE_DEFAULT_CONFIG
#   define UFO_WIFI_DEFAULT_AP_BSSD "e-rover00"
#   define UFO_WIFI_DEFAULT_AP_PASS "12345687"

#   define UFO_WIFI_DEFAULT_STA_BSSD "e-rover00"
#   define UFO_WIFI_DEFAULT_STA_PASS "12345687"

#   if UFO_WIFI_USE_STATIC_IP
#       define UFO_WIFI_DEFAULT_AP_IPSTATIC_IP "192.168.1.5"
#       define UFO_WIFI_DEFAULT_AP_IPSTATIC_GATE "192.168.0.1"
#       define UFO_WIFI_DEFAULT_AP_IPSTATIC_MASK "255.255.255.0"

#       define UFO_WIFI_DEFAULT_STA_IPSTATIC_IP "192.168.1.5"
#       define UFO_WIFI_DEFAULT_STA_IPSTATIC_GATE "192.168.0.1"
#       define UFO_WIFI_DEFAULT_STA_IPSTATIC_MASK "255.255.255.0"
#   endif
#endif


#if defined (UFO_WIFI_DEFAULT_START_AP) && defined (UFO_WIFI_DEFAULT_START_STA)
#error "cant enable both AP and STA"
#endif


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