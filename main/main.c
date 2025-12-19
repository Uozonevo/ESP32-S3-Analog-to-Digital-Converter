#include <stdio.h>
#include <esp_adc/adc_oneshot.h>
#include <inttypes.h>
#include <sdkconfig.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_chip_info.h>
#include <esp_log.h>
#include <esp_flash.h>
#include <esp_system.h>

// Define ADC Channel and Pins
#define ADC_PIN ADC_CHANNEL_7           // Channel 7
#define ADC_UNIT ADC_UNIT_1             // ADC1
#define ADC_BITWIDTH ADC_BITWIDTH_12    // 12-bit res
#define ADC_ATTEN ADC_ATTEN_DB_12       // attenuation

void app_main(void)
{
    // Initializing adc value and handle
    int adc_value;
    adc_oneshot_unit_handle_t adc_handle;
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT,
        .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));

    // Configure bitwdth and attenuation
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH,
        .atten = ADC_ATTEN_DB_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, ADC_PIN, &config));

    while (1)
    {
        // Read adc value with oneshot
        ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, ADC_PIN, &adc_value));
        // Print the value
        ESP_LOGI("ADC Value:", "%d", adc_value);
        // Delay 1 second
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    

}