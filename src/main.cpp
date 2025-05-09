
#include "u_sys/sys.h"
#include "esp_log.h"

extern "C" 
void app_main() {

// test case with full ufo-system
// check for unjoined thread in system-task
// check for dead-links with creating like this
    ufo::thread_cfg cfg;
    cfg._name= "sys0"; 
    cfg._core = 0; 
    cfg._prio = 5; 
    cfg._stackSize = 4096*2; 
    
    esp_log_level_set("wifi", ESP_LOG_WARN);
    esp_log_level_set("wifi_init", ESP_LOG_WARN);
    esp_log_level_set("phy_init", ESP_LOG_WARN);
    esp_log_level_set("esp_netif_lwip", ESP_LOG_WARN);
    
    ufo::thread systym_start(cfg, &ufo::sys::wrapped_task, ufo::sys());
    systym_start.detach();
}



