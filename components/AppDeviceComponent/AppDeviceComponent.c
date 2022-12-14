/**********************************************************************************************************************/
/**
 * @file        AppDeviceComponent.c
 *
 * @author      Stijn Vermeersch
 * @date        24.05.2022
 *
 * @brief
 *
 *
 * \n<hr>
 * Copyright (c) 2022, S-tronics BV\n
 * All rights reserved.
 * \n<hr>\n
 */
/**********************************************************************************************************************/

/**********************************************************************************************************************/

/***********************************************************************************************************************
; V E R I F Y    C O N F I G U R A T I O N
;---------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/

/**********************************************************************************************************************/

/***********************************************************************************************************************
; I N C L U D E S
;---------------------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "AppDeviceComponent.h"


/**********************************************************************************************************************/

/***********************************************************************************************************************
; L O C A L   D E F I N I T I O N S   A N D   M A C R O S
;---------------------------------------------------------------------------------------------------------------------*/
#define GPIO_RED    CONFIG_BRAGA_GPIO_RED
#define GPIO_GRN    CONFIG_BRAGA_GPIO_GREEN
#define GPIO_BLU    CONFIG_BRAGA_GPIO_BLUE
#define GPIO_ON_OFF CONFIG_BRAGA_GPIO_ON_OFF

#define GPIO_OUTPUT_LED_PIN_SEL ((1ULL<<CONFIG_BRAGA_GPIO_RED)|(1ULL<<CONFIG_BRAGA_GPIO_GREEN)|(1ULL<<CONFIG_BRAGA_GPIO_BLUE))  //For RGB_LED
#define GPIO_OUTPUT_PER_PIN_SEL ((1ULL<<CONFIG_BRAGA_GPIO_ON_OFF))                                                              //For all peripheral gpio    

#define ADC_CUR                 ADC1_CHANNEL_3
#define ADC_VOL                 ADC1_CHANNEL_4
//ADC Attenuation
#define ADC_BRAGA_ATTENUATION   ADC_ATTEN_DB_11             //0mV ~ 3100mV

#if CONFIG_IDF_TARGET_ESP32S3
#define ADC_BRAGA_CALI_SCHEME         ESP_ADC_CAL_VAL_EFUSE_TP_FIT
#endif

#define AVG_CNTR            30          //# of samples to calculate average value of current en voltage
#define GAIN_CURRENT        20          //20V/V
#define RES_CURRENT         0.047       //Ohmic value of currentresistor
#define CURRENT_MAX         3.19        //Maximum current that can be measured

#define RATIO_VOLTAGE       0.25        //Ration calculated out of resistance network R13 & R20

/**********************************************************************************************************************/

/***********************************************************************************************************************
; L O C A L   T Y P E D E F S
;---------------------------------------------------------------------------------------------------------------------*/

/**********************************************************************************************************************/

/***********************************************************************************************************************
; L O C A L   F U N C T I O N   P R O T O T Y P E S
;---------------------------------------------------------------------------------------------------------------------*/
static int AppDevice_compare_versions(const char *v1, const char*v2);
void AppDevice_initled(void);
void AppDevice_intperipheral(void);
void AppDevice_ledtask(void *arg);
static bool AppDevice_adc_calibration_init(void);
/**********************************************************************************************************************/

/***********************************************************************************************************************
; L O C A L   V A R I A B L E S
;---------------------------------------------------------------------------------------------------------------------*/
static const char *TAG = "BRAGA DEVICE";

static esp_adc_cal_characteristics_t adc1_chars;
/**********************************************************************************************************************/

/***********************************************************************************************************************
; E X P O R T E D   V A R I A B L E S
;---------------------------------------------------------------------------------------------------------------------*/

/**********************************************************************************************************************/

/***********************************************************************************************************************
; L O C A L   F U N C T I O N S
;---------------------------------------------------------------------------------------------------------------------*/
static int AppDevice_compare_versions(const char *v1, const char *v2)
{
    unsigned major_v1 = 0, minor_v1 = 0, patch_v1 = 0;
    unsigned major_v2 = 0, minor_v2 = 0, patch_v2 = 0;
    sscanf(v1, "%u.%u.%u", &major_v1, &minor_v1, &patch_v1);
    sscanf(v2, "%u.%u.%u", &major_v2, &minor_v2, &patch_v2);

    if (major_v1 < major_v2)
        return -1;
    if (major_v1 > major_v2)
        return 1;
    if (minor_v1 < minor_v2)
        return -1;
    if (minor_v1 > minor_v2)
        return 1;
    if (patch_v1 < patch_v2)
        return -1;
    if (patch_v1 > patch_v2)
        return 1;

    // check if every corresponding variable is equal to eachother
    if (major_v1 == major_v2 && minor_v1 == minor_v2 && patch_v1 == patch_v2)
        return 0;
    else
        return -2;
}
/*--------------------------------------------------------------------------------------------------------------------*/
void AppDevice_initled(void)
{
    ESP_LOGI(TAG, "Init Led");
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_LED_PIN_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;

    gpio_config(&io_conf);

    xTaskCreate(AppDevice_ledtask, "AppDevice_ledtask", 1024, NULL, 10, NULL);

    ESP_LOGI(TAG, "Init Led Done");
}
/*--------------------------------------------------------------------------------------------------------------------*/
void AppDevice_initperipheral(void)
{
    ESP_LOGI(TAG, "Init peripheral");
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_PER_PIN_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
    gpio_set_level(GPIO_ON_OFF, 0);
    ESP_LOGI(TAG, "Init peripheral Done");
}
/*--------------------------------------------------------------------------------------------------------------------*/
void AppDevice_ledtask(void *arg)
{
    static LED_COLOR color = RED;
    while(1)
    {
        if(color == RED)
        {
            AppDevice_SetLed(color);
            color = GREEN;
        }
        else if(color == GREEN)
        {
            AppDevice_SetLed(color);
            color = BLUE;
        }
        else if(color == BLUE)
        {
            AppDevice_SetLed(color);
            color = RED;
        }
         // 500ms = 0.5 sec
        vTaskDelay((500) / portTICK_RATE_MS);   
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/
static bool AppDevice_adc_calibration_init(void)
{
    esp_err_t ret;
    bool cali_enable = false;
    
    ret = esp_adc_cal_check_efuse(ADC_BRAGA_CALI_SCHEME);
    if(ret == ESP_ERR_NOT_SUPPORTED)
    {
        ESP_LOGW(TAG, "Calibration scheme not supported, skip software calibration");
    }
    else if (ret == ESP_ERR_INVALID_VERSION)
    {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    }
    else if (ret == ESP_OK)
    {
        cali_enable = true;
        esp_adc_cal_characterize(ADC_UNIT_1, ADC_BRAGA_ATTENUATION, ADC_WIDTH_BIT_DEFAULT, 0, &adc1_chars);
    }
    else
    {
        ESP_LOGE(TAG, "Invalid arg");
    }
    return cali_enable;
}
/*--------------------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------*/
/**********************************************************************************************************************/

/***********************************************************************************************************************
; E X P O R T E D   F U N C T I O N S
;---------------------------------------------------------------------------------------------------------------------*/
void AppDeviceInit(void)
{
    bool cali_enable = false;
    AppDevice_initled();
    AppDevice_initperipheral();

    cali_enable = AppDevice_adc_calibration_init();
    //ADC1 Config (Channel configuration)
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_DEFAULT));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC_CUR, ADC_BRAGA_ATTENUATION));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC_VOL, ADC_BRAGA_ATTENUATION));
}
/*--------------------------------------------------------------------------------------------------------------------*/
int AppDevice_compare_software_version_device(const char *version)
{
    return AppDevice_compare_versions(version, CONFIG_SOFTWARE_VERSION);
}
/*--------------------------------------------------------------------------------------------------------------------*/
int AppDevice_compare_hardware_version_device(const char *version)
{
    return AppDevice_compare_versions(version, CONFIG_HARDWARE_VERSION);
}
/*--------------------------------------------------------------------------------------------------------------------*/
void AppDevice_get_unique(char *unique)
{
    uint8_t ChipID[6];
    esp_efuse_mac_get_default(ChipID);
    sprintf(unique, "%02X%02X%02X%02X%02X%02X", ChipID[0], ChipID[1], ChipID[2], ChipID[3], ChipID[4], ChipID[5]);
    
}
/*--------------------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------*/
void AppDevice_SetLed(LED_COLOR color)
{
    switch(color)
    {
        case RED:
            gpio_set_level(GPIO_RED, 0);
            gpio_set_level(GPIO_GRN, 1);
            gpio_set_level(GPIO_BLU, 1);
            break;
        case GREEN:
            gpio_set_level(GPIO_RED, 1);
            gpio_set_level(GPIO_GRN, 0);
            gpio_set_level(GPIO_BLU, 1);
            break;
        case BLUE:
            gpio_set_level(GPIO_RED, 1);
            gpio_set_level(GPIO_GRN, 1);
            gpio_set_level(GPIO_BLU, 0);
            break;
        default:
            break;
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/
void AppDevice_CtrlPeripheral(bool onoff)
{
    if(onoff)
    {
        gpio_set_level(GPIO_ON_OFF, 1);
    }
    else
    {
        gpio_set_level(GPIO_ON_OFF, 0);
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/
uint16_t AppDevice_GetCurrentValue(void)
{
    int adc_raw;
    
    float current = 0;
    //CURRENT
    adc_raw = adc1_get_raw(ADC_CUR);
    current = (uint32_t)esp_adc_cal_raw_to_voltage(adc_raw, &adc1_chars);
    current /= (float)GAIN_CURRENT;
    current /= (float)RES_CURRENT;

    ESP_LOGI(TAG, "Current Measured = %d", (uint32_t)current);
    return current;
}
/*--------------------------------------------------------------------------------------------------------------------*/
uint16_t AppDevice_GetVoltageValue(void)
{
    int adc_raw;
    float voltage;
    //VOLTAGE
    adc_raw = adc1_get_raw(ADC_VOL);
    voltage = esp_adc_cal_raw_to_voltage(adc_raw, &adc1_chars);
    voltage /= RATIO_VOLTAGE;

    ESP_LOGI(TAG, "Voltage Measured = %d", (uint32_t)voltage);
    return voltage;
}
/*--------------------------------------------------------------------------------------------------------------------*/
/**********************************************************************************************************************/