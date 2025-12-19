#include <stdio.h>
#include <string.h>
#include <esp_adc/adc_continuous.h>
#include <sdkconfig.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <freertos/semphr.h>

#define EXAMPLE_ADC_UNIT                            ADC_UNIT_1
#define _EXAMPLE_ADC_UNIT_STR(unit)                 #unit
#define EXAMPLE_ADC_UNIT_STR(unit)                  _EXAMPLE_ADC_UNIT_STR(unit)
#define EXAMPLE_ADC_CONV_MODE                       ADC_CONV_SINGLE_UNIT_1
#define EXAMPLE_ADC_ATTEN                           ADC_ATTEN_DB_0
#define EXAMPLE_ADC_BIT_WIDTH                       SOC_ADC_DIGI_MAX_BITWIDTH

#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S3
#define EXAMPLE_ADC_OUTPUT_TYPE                     ADC_DIGI_OUTPUT_FORMAT_TYPE1
#define EXAMPLE_ADC_GET_CHANNEL(p_data)             ((p_data)->type1.channel)
#define EXAMPLE_ADC_GET_DATA(p_data)                ((p_data)->type1.data)
#else
#define EXAMPLE_ADC_OUTPUT_TYPE                     ADC_DIGI_OUTPUT_FORMAT_TYPE2
#define EXAMPLE_ADC_GET_CHANNEL(p_data)             ((p_data)->type2.channel)
#define EXAMPLE_ADC_GET_DATA(p_data)                ((p_data)->type2.data)
#endif

#define EXAMPLE_READ_LEN                            256

static adc_channel_t channel[2] = {ADC_CHANNEL_7, ADC_CHANNEL_4};
static TaskHandle_t s_task_handle;
static const char *TAG = "POTENTIOMETER";

static bool IRAM_ATTR s_conv_done_cb(adc_continuous_handle_t handle,
                                     const adc_continuous_evt_data_t *data,
                                     void *user_data){
    BaseType_t mustYield = pdFALSE;
    // Notify that the ADC continuous driver has done enough # of conversions
    vTaskNotifyGiveFromISR(s_task_handle, &mustYield);

    return true;
}

static void continuous_adc_init(adc_channel_t *channel, uint8_t channel_num, adc_continuous_handle_t *out_handle){
    adc_continuous_handle_t handle = NULL;

    adc_continuous_handle_cfg_t adc_config = {
        .max_store_buf_size = 1024,
        .conv_frame_size = EXAMPLE_READ_LEN,
    };
    ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &handle));

    adc_continuous_config_t dig_config = {
        .sample_freq_hz = 20 * 1000,
        .conv_mode = EXAMPLE_ADC_CONV_MODE,
        .format = EXAMPLE_ADC_OUTPUT_TYPE,
    };
    
    adc_digi_pattern_config_t adc_pattern[SOC_ADC_PATT_LEN_MAX] = {0};
    dig_config.pattern_num = channel_num;
    for(int i = 0; i < channel_num; i++){
        adc_pattern[i].atten = ADC_ATTEN_DB_12;
        adc_pattern[i].bit_width = ADC_BITWIDTH_12;
        adc_pattern[i].unit = EXAMPLE_ADC_UNIT;
        adc_pattern[i].channel = channel[i] & 0x7;

        ESP_LOGI(TAG, "adc_pattern[%d].atten is: %"PRIx8, i, adc_pattern[i].atten);
        ESP_LOGI(TAG, "adc_pattern[%d].channel is: %"PRIx8, i, adc_pattern[i].channel);
        ESP_LOGI(TAG, "adc_pattern[%d].unit is: %"PRIx8, i, adc_pattern[i].unit);
    }
    dig_config.adc_pattern = adc_pattern;
    ESP_ERROR_CHECK(adc_continuous_config(handle, &dig_config));

    *out_handle = handle;
}

void app_main(void)
{
    esp_err_t ret;
    uint32_t ret_num = 0;
    uint8_t result[EXAMPLE_READ_LEN] = {0};
    memset(result, 0xcc, EXAMPLE_READ_LEN);

    s_task_handle = xTaskGetCurrentTaskHandle();

    adc_continuous_handle_t handle = NULL;
    continuous_adc_init(channel, sizeof(channel) / sizeof(adc_channel_t), &handle);

    adc_continuous_evt_cbs_t cbs = {
        .on_conv_done = s_conv_done_cb,
    };
    ESP_ERROR_CHECK(adc_continuous_register_event_callbacks(handle, &cbs, NULL));
    ESP_ERROR_CHECK(adc_continuous_start(handle));
        
    while (1)
    {
        char unit[] = EXAMPLE_ADC_UNIT_STR(EXAMPLE_ADC_UNIT);

        while(1){
            ret = adc_continuous_read(handle, result, EXAMPLE_READ_LEN, &ret_num, 0);
            if (ret == ESP_OK){
                ESP_LOGI("TASK", "ret is %x, ret_num is %"PRIu32" bytes", ret, ret_num);
                for (int i = 0; i < ret_num; i += SOC_ADC_DIGI_RESULT_BYTES){
                    adc_digi_output_data_t *p =(adc_digi_output_data_t *)&result[i];
                    uint32_t chan_num = (EXAMPLE_ADC_GET_CHANNEL(p));
                    uint32_t data = (EXAMPLE_ADC_GET_DATA(p));
                    // check
                    if (chan_num < SOC_ADC_CHANNEL_NUM(EXAMPLE_ADC_UNIT)){
                        // esp_logi
                        ESP_LOGI(TAG, "Unit: %s, Channel: %"PRIx32", Value: %lu", unit, chan_num, data);
                    } else {
                        ESP_LOGI(TAG, "Invalid data [%s_%"PRIx32"_%"PRIx32"]", unit, chan_num, data);
                    }
                }

                vTaskDelay(1);
            } else if (ret == ESP_ERR_TIMEOUT){
                //
                break;
            }
        }
    }
    ESP_ERROR_CHECK(adc_continuous_stop(handle));
    ESP_ERROR_CHECK(adc_continuous_deinit(handle));
    

}