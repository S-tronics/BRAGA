/**********************************************************************************************************************/
/**
 * @file        AppTemplate.c
 *
 * @author      Stijn Vermeersch
 * @date        17.05.2022
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
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"
#include "sdkconfig.h"
//Only for debug reasons

//DRIVER lib include section

//STANDARD lib include section
#include "StdEepromComponent.h"
//APPLICATION lib include section
#include "AppSciComponent.h"
#include "AppEepromComponent.h"
/**********************************************************************************************************************/



/***********************************************************************************************************************
; L O C A L   D E F I N I T I O N S   A N D   M A C R O S
;---------------------------------------------------------------------------------------------------------------------*/
#define MAX_NBR_STAIRS              CONFIG_MAX_NBR_STAIRS
#define SMART                       7
#define ADDR_SSID                   0x0000
#define ADDR_PASSW                  0x0020
#define ADDR_STAIRSTAKEN            0x0040
#define ADDR_OVERCURRENT            0x0100
#define ADDR_UNDERCURRENT           0x0104
#define ADDR_OVERVOLTAGE            0x0108
#define ADDR_UNDERVOLTAGE           0x010A
#define ADDR_CONFIG                 0x0200
#define ADDR_NBR_STAIRS             0x0201
#define ADDR_STAIRS_CONFIG_START    0x0202
/**********************************************************************************************************************/



/***********************************************************************************************************************
; L O C A L   T Y P E D E F S
;---------------------------------------------------------------------------------------------------------------------*/

/**********************************************************************************************************************/



/***********************************************************************************************************************
; L O C A L   F U N C T I O N   P R O T O T Y P E S
;---------------------------------------------------------------------------------------------------------------------*/

/**********************************************************************************************************************/



/***********************************************************************************************************************
; L O C A L   V A R I A B L E S
;---------------------------------------------------------------------------------------------------------------------*/
static const char *TAG = "BRAGA EEPROM";

static uint32_t stairstaken = 0;
static uint32_t overcurrent = 0;
static uint32_t undercurrent = 0;
static uint32_t overvoltage = 0;
static uint32_t undervoltage = 0;

/**********************************************************************************************************************/



/***********************************************************************************************************************
; E X P O R T E D   V A R I A B L E S
;---------------------------------------------------------------------------------------------------------------------*/

/**********************************************************************************************************************/



/***********************************************************************************************************************
; L O C A L   F U N C T I O N S
;---------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/

/**********************************************************************************************************************/



/***********************************************************************************************************************
; E X P O R T E D   F U N C T I O N S
;---------------------------------------------------------------------------------------------------------------------*/
void appeeprom_init(void)
{
    esp_log_level_set(TAG, ESP_LOG_INFO);
    ESP_LOGI(TAG, "Init Eeprom");
    ESP_LOGI(TAG, "Init Eeprom");
}
/*--------------------------------------------------------------------------------------------------------------------*/
void appeeprom_write_wifiSSID(char* ssid)
{
    uint8_t length = strlen(ssid);

    stdeeprom_write_data(ADDR_SSID, (uint8_t*)ssid, length);
    ESP_LOGI(TAG, "EEprom Store: SSID: %d", length);
}
/*--------------------------------------------------------------------------------------------------------------------*/
void appeeprom_write_wifipassword(char* passwrd)
{
    uint8_t length = strlen(passwrd);

    stdeeprom_write_data(ADDR_PASSW, (uint8_t*)passwrd, length);
    ESP_LOGI(TAG, "EEprom Store: PASSWRD: %d", length);
}
/*--------------------------------------------------------------------------------------------------------------------*/
void appeeprom_write_stairstaken(uint32_t stairs_taken)
{
    static uint8_t r_data[4];
    //StdEepromReadData(ADDR_STAIRSTAKEN, r_data, sizeof(r_data));
    //stairstaken = (r_data[0] << 24) | (r_data[1] << 16) | (r_data[2] << 8) | r_data[3];
    //stairstaken++;
    r_data[0] = (uint8_t)(stairstaken >> 24);
    r_data[1] = (uint8_t)(stairstaken >> 16);
    r_data[2] = (uint8_t)(stairstaken >> 8);
    r_data[3] = (uint8_t)(stairstaken & 0x000000FF);
    stdeeprom_write_data(ADDR_STAIRSTAKEN, r_data, sizeof(r_data));
}
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
void appeeprom_write_configdone(CONFIG_DONE config)
{
    static uint8_t r_data = 0;
    stdeeprom_read_byte(ADDR_CONFIG, &r_data);
    r_data |= (1 << (uint8_t)config);
    stdeeprom_write_byte(ADDR_CONFIG, &r_data);
}
/*--------------------------------------------------------------------------------------------------------------------*/
void appeeprom_write_nbrofstairs(uint8_t stairs)
{
    stdeeprom_write_byte(ADDR_NBR_STAIRS, &stairs);
}
/*--------------------------------------------------------------------------------------------------------------------*/
void appeeprom_increment_overcurrent(void)
{
    static uint8_t r_data[4];
    stdeeprom_read_data(ADDR_OVERCURRENT, r_data, sizeof(r_data));
    overcurrent = (r_data[0] << 24) | (r_data[1] << 16) | (r_data[2] << 8) | r_data[3];
    overcurrent++;
    r_data[0] = (uint8_t)(overcurrent >> 24);
    r_data[1] = (uint8_t)(overcurrent >> 16);
    r_data[2] = (uint8_t)(overcurrent >> 8);
    r_data[3] = (uint8_t)(overcurrent & 0x000000FF);
    stdeeprom_write_data(ADDR_OVERCURRENT, r_data, sizeof(r_data));
}
/*--------------------------------------------------------------------------------------------------------------------*/
void appeeprom_increment_undercurrent(void)
{
    static uint8_t r_data[4];
    stdeeprom_read_data(ADDR_UNDERCURRENT, r_data, sizeof(r_data));
    undercurrent = (r_data[0] << 24) | (r_data[1] << 16) | (r_data[2] << 8) | r_data[3];
    undercurrent++;
    r_data[0] = (uint8_t)(undercurrent >> 24);
    r_data[1] = (uint8_t)(undercurrent >> 16);
    r_data[2] = (uint8_t)(undercurrent >> 8);
    r_data[3] = (uint8_t)(undercurrent & 0x000000FF);
    stdeeprom_write_data(ADDR_UNDERCURRENT, r_data, sizeof(r_data));
}
/*--------------------------------------------------------------------------------------------------------------------*/
void appeeprom_increment_overvoltage(void)
{
    static uint8_t r_data[4];
    stdeeprom_read_data(ADDR_OVERVOLTAGE, r_data, sizeof(r_data));
    overvoltage = (r_data[0] << 24) | (r_data[1] << 16) | (r_data[2] << 8) | r_data[3];
    overvoltage++;
    r_data[0] = (uint8_t)(overvoltage >> 24);
    r_data[1] = (uint8_t)(overvoltage >> 16);
    r_data[2] = (uint8_t)(overvoltage >> 8);
    r_data[3] = (uint8_t)(overvoltage & 0x000000FF);
    stdeeprom_write_data(ADDR_OVERVOLTAGE, r_data, sizeof(r_data));
}
/*--------------------------------------------------------------------------------------------------------------------*/
void appeeprom_increment_undervoltage(void)
{
    static uint8_t r_data[4];
    stdeeprom_read_data(ADDR_UNDERVOLTAGE, r_data, sizeof(r_data));
    undervoltage = (r_data[0] << 24) | (r_data[1] << 16) | (r_data[2] << 8) | r_data[3];
    undervoltage++;
    r_data[0] = (uint8_t)(undervoltage >> 24);
    r_data[1] = (uint8_t)(undervoltage >> 16);
    r_data[2] = (uint8_t)(undervoltage >> 8);
    r_data[3] = (uint8_t)(undervoltage & 0x000000FF);
    stdeeprom_write_data(ADDR_UNDERVOLTAGE, r_data, sizeof(r_data));
}
/*--------------------------------------------------------------------------------------------------------------------*/
void appeeprom_write_stairsconfig(APPSCISTAIR* stairs)
{
    uint8_t i = 0;
    APPSCISTAIR* stair;
    static uint8_t data = 0x00;

    for(i = 0; i < MAX_NBR_STAIRS; i++)
    {
        stair = &stairs[i];
        if(stair->has_px)
        {
            stdeeprom_read_byte(ADDR_STAIRS_CONFIG_START + (i * 16), &data);
            data |= (1 << SMART);
            stdeeprom_write_byte(ADDR_STAIRS_CONFIG_START + (i * 16), &data);
        }
        //RED
        data = stair->colorset.data[1]; 
        stdeeprom_write_byte(ADDR_STAIRS_CONFIG_START + 1 + (i * 16), &data);
        //GRN
        data = stair->colorset.data[2];
        stdeeprom_write_byte(ADDR_STAIRS_CONFIG_START + 2 + (i * 16), &data);
        //BLUE
        data = stair->colorset.data[3];
        stdeeprom_write_byte(ADDR_STAIRS_CONFIG_START + 3 + (i * 16), &data);
        //WHITE
        data = stair->colorset.data[0];
        stdeeprom_write_byte(ADDR_STAIRS_CONFIG_START + 4 + (i * 16), &data);
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/
void appeeprom_read_stairsconfig(APPSCISTAIR* stairs)
{
    uint8_t i = 0;
    APPSCISTAIR* stair;
    static uint8_t data = 0x00;
    for(i = 0; i < MAX_NBR_STAIRS; i++)
    {
        stair = &stairs[i];
        stdeeprom_read_byte(ADDR_STAIRS_CONFIG_START + (i * 16), &data);
        if((data & 0x01) == 0x01)   stair->has_px = true;
        else                        stair->has_px = false;
        //RED
        stdeeprom_read_byte(ADDR_STAIRS_CONFIG_START + 1 + (i * 16), &data);
        stair->colorset.data[1] = data;
        //GRN
        stdeeprom_read_byte(ADDR_STAIRS_CONFIG_START + 2 + (i * 16), &data);
        stair->colorset.data[2] = data;
        //BLUE
        stdeeprom_read_byte(ADDR_STAIRS_CONFIG_START + 3 + (i * 16), &data);
        stair->colorset.data[3] = data;
        //WHITE
        stdeeprom_read_byte(ADDR_STAIRS_CONFIG_START + 4 + (i * 16), &data);
        stair->colorset.data[0] = data;
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
void appeeprom_read_params(void)
{

}
/*--------------------------------------------------------------------------------------------------------------------*/

/**********************************************************************************************************************/

