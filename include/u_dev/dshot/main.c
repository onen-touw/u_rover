/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

 #include "freertos/FreeRTOS.h"
 #include "freertos/task.h"
 #include "esp_log.h"
 #include "driver/rmt_tx.h"
 #include "dshot_esc_encoder.h"
 
 #if CONFIG_IDF_TARGET_ESP32H2
 #define DSHOT_ESC_RESOLUTION_HZ 32000000 // 32MHz resolution, DSHot protocol needs a relative high resolution
 #else
 #define DSHOT_ESC_RESOLUTION_HZ 40000000 // 40MHz resolution, DSHot protocol needs a relative high resolution
 #endif

 #define DSHOT_BAUD_RATE 300000, // DSHOT300 protocol
 //пины к которым подключенны ESC
 #define DSHOT_ESC1_GPIO_NUM 19
 #define DSHOT_ESC2_GPIO_NUM 18
 #define DSHOT_ESC3_GPIO_NUM 17
 #define DSHOT_ESC4_GPIO_NUM 16

 static const char *TAG = "example";
 
 void app_main(void)
 {
     ESP_LOGI(TAG, "Create RMT TX channels");
     //создать переменные
     rmt_channel_handle_t esc1_chan = NULL;
     rmt_channel_handle_t esc2_chan = NULL;
     rmt_channel_handle_t esc3_chan = NULL;
     rmt_channel_handle_t esc4_chan = NULL;

     //инициализирвать каналы RMT
     rmt_tx_channel_config_t tx1_chan_config = {
         .clk_src = RMT_CLK_SRC_DEFAULT, // select a clock that can provide needed resolution
         .gpio_num = DSHOT_ESC1_GPIO_NUM,
         .mem_block_symbols = 64,
         .resolution_hz = DSHOT_ESC_RESOLUTION_HZ,
         .trans_queue_depth = 10, // set the number of transactions that can be pending in the background
     };
     rmt_tx_channel_config_t tx2_chan_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT, // select a clock that can provide needed resolution
        .gpio_num = DSHOT_ESC2_GPIO_NUM,
        .mem_block_symbols = 64,
        .resolution_hz = DSHOT_ESC_RESOLUTION_HZ,
        .trans_queue_depth = 10, // set the number of transactions that can be pending in the background
    };
    rmt_tx_channel_config_t tx3_chan_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT, // select a clock that can provide needed resolution
        .gpio_num = DSHOT_ESC3_GPIO_NUM,
        .mem_block_symbols = 64,
        .resolution_hz = DSHOT_ESC_RESOLUTION_HZ,
        .trans_queue_depth = 10, // set the number of transactions that can be pending in the background
    };
    rmt_tx_channel_config_t tx4_chan_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT, // select a clock that can provide needed resolution
        .gpio_num = DSHOT_ESC4_GPIO_NUM,
        .mem_block_symbols = 64,
        .resolution_hz = DSHOT_ESC_RESOLUTION_HZ,
        .trans_queue_depth = 10, // set the number of transactions that can be pending in the background
    };

    //проверить на ошибки
     ESP_ERROR_CHECK(rmt_new_tx_channel(&tx1_chan_config, &esc1_chan));
     ESP_ERROR_CHECK(rmt_new_tx_channel(&tx2_chan_config, &esc2_chan));
     ESP_ERROR_CHECK(rmt_new_tx_channel(&tx3_chan_config, &esc3_chan));
     ESP_ERROR_CHECK(rmt_new_tx_channel(&tx4_chan_config, &esc4_chan));
 
     ESP_LOGI(TAG, "Install Dshot ESC encoder");

     //создать энкодер
     rmt_encoder_handle_t dshot_encoder1 = NULL;
     dshot_esc_encoder_config_t encoder_config1 = {
         .resolution = DSHOT_ESC_RESOLUTION_HZ,
         .baud_rate = DSHOT_BAUD_RATE
         .post_delay_us = 50, // extra delay between each frame
     };
     rmt_encoder_handle_t dshot_encoder2 = NULL;
     dshot_esc_encoder_config_t encoder_config2 = {
         .resolution = DSHOT_ESC_RESOLUTION_HZ,
         .baud_rate = DSHOT_BAUD_RATE
         .post_delay_us = 50, // extra delay between each frame
     };
     rmt_encoder_handle_t dshot_encoder3 = NULL;
     dshot_esc_encoder_config_t encoder_config3 = {
         .resolution = DSHOT_ESC_RESOLUTION_HZ,
         .baud_rate = DSHOT_BAUD_RATE
         .post_delay_us = 50, // extra delay between each frame
     };
     rmt_encoder_handle_t dshot_encoder4 = NULL;
     dshot_esc_encoder_config_t encoder_config4 = {
         .resolution = DSHOT_ESC_RESOLUTION_HZ,
         .baud_rate = DSHOT_BAUD_RATE
         .post_delay_us = 50, // extra delay between each frame
     };

     //проверить на ошибки
     ESP_ERROR_CHECK(rmt_new_dshot_esc_encoder(&encoder_config1, &dshot_encoder1));
     ESP_ERROR_CHECK(rmt_new_dshot_esc_encoder(&encoder_config2, &dshot_encoder2));
     ESP_ERROR_CHECK(rmt_new_dshot_esc_encoder(&encoder_config3, &dshot_encoder3));
     ESP_ERROR_CHECK(rmt_new_dshot_esc_encoder(&encoder_config4, &dshot_encoder4));
 
     ESP_LOGI(TAG, "Enable RMT TX channel");

     //проверить на ошибки
     ESP_ERROR_CHECK(rmt_enable(esc1_chan));
     ESP_ERROR_CHECK(rmt_enable(esc2_chan));
     ESP_ERROR_CHECK(rmt_enable(esc3_chan));
     ESP_ERROR_CHECK(rmt_enable(esc4_chan));

     //настройки rtm transmit
     rmt_transmit_config_t tx1_config = {
         .loop_count = -1, // infinite loop
     };
     rmt_transmit_config_t tx2_config = {
        .loop_count = -1, // infinite loop
    };
    rmt_transmit_config_t tx3_config = {
        .loop_count = -1, // infinite loop
    };
    rmt_transmit_config_t tx4_config = {
        .loop_count = -1, // infinite loop
    };
    
    //изначальные настройки каждого esc
    dshot_esc_throttle_t throttle1 = {
        .throttle = 0,
        .telemetry_req = false, // telemetry is not supported in this example
    };
    dshot_esc_throttle_t throttle2 = {
        .throttle = 0,
        .telemetry_req = false, // telemetry is not supported in this example
    };
    
    dshot_esc_throttle_t throttle3 = {
        .throttle = 0,
        .telemetry_req = false, // telemetry is not supported in this example
    };
     dshot_esc_throttle_t throttle4 = {
         .throttle = 0,
         .telemetry_req = false, // telemetry is not supported in this example
     };
 
     ESP_LOGI(TAG, "Start ESC by sending zero throttle for a while...");

     ESP_ERROR_CHECK(rmt_transmit(esc1_chan, dshot_encoder1, &throttle1, sizeof(throttle1), &tx1_config));
     vTaskDelay(pdMS_TO_TICKS(5000));
     ESP_ERROR_CHECK(rmt_transmit(esc2_chan, dshot_encoder2, &throttle2, sizeof(throttle2), &tx2_config));
     vTaskDelay(pdMS_TO_TICKS(5000));
     ESP_ERROR_CHECK(rmt_transmit(esc3_chan, dshot_encoder3, &throttle3, sizeof(throttle3), &tx3_config));
     vTaskDelay(pdMS_TO_TICKS(5000));
     ESP_ERROR_CHECK(rmt_transmit(esc4_chan, dshot_encoder4, &throttle4, sizeof(throttle4), &tx4_config));
     vTaskDelay(pdMS_TO_TICKS(5000));
     
 
     ESP_LOGI(TAG, "Increase throttle, no telemetry");
     for (uint16_t thro = 100; thro < 1000; thro += 10)
     {
         throttle1.throttle = thro;
         throttle2.throttle = thro;
         throttle3.throttle = thro;
         throttle4.throttle = thro;
         
         ESP_ERROR_CHECK(rmt_transmit(esc1_chan, dshot_encoder1, &throttle1, sizeof(throttle1), &tx1_config));
         ESP_ERROR_CHECK(rmt_transmit(esc2_chan, dshot_encoder2, &throttle2, sizeof(throttle2), &tx2_config));
         ESP_ERROR_CHECK(rmt_transmit(esc3_chan, dshot_encoder3, &throttle3, sizeof(throttle3), &tx3_config));
         ESP_ERROR_CHECK(rmt_transmit(esc4_chan, dshot_encoder4, &throttle4, sizeof(throttle4), &tx4_config));
         
         // the previous loop transfer is till undergoing, we need to stop it and restart,
         // so that the new throttle can be updated on the output
         ESP_ERROR_CHECK(rmt_disable(esc1_chan));
         ESP_ERROR_CHECK(rmt_disable(esc2_chan));
         ESP_ERROR_CHECK(rmt_disable(esc3_chan));
         ESP_ERROR_CHECK(rmt_disable(esc4_chan));

         ESP_ERROR_CHECK(rmt_enable(esc1_chan));
         ESP_ERROR_CHECK(rmt_enable(esc2_chan));
         ESP_ERROR_CHECK(rmt_enable(esc3_chan));
         ESP_ERROR_CHECK(rmt_enable(esc4_chan));

         vTaskDelay(pdMS_TO_TICKS(1000));
     }
     throttle1.throttle = 0;
     throttle2.throttle = 0;
     throttle3.throttle = 0;
     throttle4.throttle = 0;

     ESP_ERROR_CHECK(rmt_transmit(esc1_chan, dshot_encoder1, &throttle1, sizeof(throttle1), &tx1_config));
     ESP_ERROR_CHECK(rmt_transmit(esc2_chan, dshot_encoder2, &throttle2, sizeof(throttle2), &tx2_config));
     ESP_ERROR_CHECK(rmt_transmit(esc3_chan, dshot_encoder3, &throttle3, sizeof(throttle3), &tx3_config));
     ESP_ERROR_CHECK(rmt_transmit(esc4_chan, dshot_encoder4, &throttle4, sizeof(throttle4), &tx4_config));
     
     // the previous loop transfer is till undergoing, we need to stop it and restart,
     // so that the new throttle can be updated on the output
     ESP_ERROR_CHECK(rmt_disable(esc1_chan));
     ESP_ERROR_CHECK(rmt_disable(esc2_chan));
     ESP_ERROR_CHECK(rmt_disable(esc3_chan));
     ESP_ERROR_CHECK(rmt_disable(esc4_chan));

     ESP_ERROR_CHECK(rmt_enable(esc1_chan));
     ESP_ERROR_CHECK(rmt_enable(esc2_chan));
     ESP_ERROR_CHECK(rmt_enable(esc3_chan));
     ESP_ERROR_CHECK(rmt_enable(esc4_chan));
     vTaskDelay(pdMS_TO_TICKS(1000));
 }