# ESP32-S3 Analog to Digital Converter (OneShot) 🎛️
## Prograssion
## Schematic 📖
## Configuration 👩‍🔧
On the ESP32-S3 board, I defined what channels and pins I would use for the Analog to Digital Converter. 
I used channel 1 of ADC unit 1 and defined the bitwidth and attenuation.
```C
// Define ADC Channel and Pins
#define ADC_PIN ADC_CHANNEL_7           // Channel 7
#define ADC_UNIT ADC_UNIT_1             // ADC1
#define ADC_BITWIDTH ADC_BITWIDTH_12    // 12-bit res
#define ADC_ATTEN ADC_ATTEN_DB_12       // attenuation
```

Next came the initialization and configure of the adc value and handler for the new adc oneshot unit
```C
// Initializing adc value and handle
    int adc_value;
    adc_oneshot_unit_handle_t adc_handle;
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT,
        .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));
```

Lastly, I configured the bitwidth and attenuation
```C
// Configure bitwdth and attenuation
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH,
        .atten = ADC_ATTEN_DB_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, ADC_PIN, &config));
```
## Output
In my superloop, I use the ESP_ERROR_CHECK function to pass my adc function to read the analog value from my potentiometer and store it in my adc_value variable. From here, the analog to digital conversion happens and is printed using the ESP_LOGI function. I delay the task by 1 second to allow the readings to come in a timely manner. 
```C
while (1)
    {
        // Read adc value with oneshot
        ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, ADC_PIN, &adc_value));
        // Print the value
        ESP_LOGI("ADC Value:", "%d", adc_value);
        // Delay 1 second
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
```
<img width="1920" height="1080" alt="Screenshot 2025-12-19 075025" src="https://github.com/user-attachments/assets/1863fcd7-679d-4d1e-ab8b-4d0ad44d9e07" />
