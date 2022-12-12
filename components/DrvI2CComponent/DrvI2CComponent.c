/**********************************************************************************************************************/
/**
 * @file        DrvI2c.c
 *
 * @author      Stijn Vermeersch
 * @date        14.06.2022
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
#include "driver/i2c.h"
#include "esp_system.h"
#include "esp_log.h"
#include "sdkconfig.h"
//Only for debug reasons

//DRIVER lib include section

//STANDARD lib include section
//APPLICATION lib include section
#include "DrvI2CComponent.h"
/**********************************************************************************************************************/



/***********************************************************************************************************************
; L O C A L   D E F I N I T I O N S   A N D   M A C R O S
;---------------------------------------------------------------------------------------------------------------------*/
#define I2C_MASTER_SCL              CONFIG_BRAGA_I2C_MASTER_SCL
#define I2C_MASTER_SDA              CONFIG_BRAGA_I2C_MASTER_SDA
#define I2C_MASTER_NUM              0
#define I2C_MASTER_FREQ_HZ          400000
#define I2C_MASTER_TX_BUF_DISABLE   0
#define I2C_MASTER_RX_BUF_DISABLE   0
#define I2C_MASTER_TIMEOUT_MS       1000
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
static const char *TAG = "DRV I2C";

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
void DrvI2cInit(void)
{
    int i2c_master_port = I2C_MASTER_NUM;

    esp_log_level_set(TAG, ESP_LOG_INFO);
    ESP_LOGI(TAG, "Init I2c Driver");

    i2c_config_t i2cbus =
    {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA,
        .scl_io_num = I2C_MASTER_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ;
    };
    i2c_param_config(i2c_master_port, &i2cbus);

    i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
    ESP_LOGI(TAG, "Init I2c Driver Done");
}
/*--------------------------------------------------------------------------------------------------------------------*/
void esp_err_t DrvI2cWriteByte(uint8_t slaveaddress, uint16_t reg, uint8_t data)
{
    uint8_t write_buf[3];
    write_buf[0] = (uint8_t)(reg >> 8);
    write_buf[1] = (uint8_t)(reg & 0x00FF);
    write_buf[2] = data;
    return i2c_master_write_to_device(I2C_MASTER_NUM, slaveaddress, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS/ portTICK_RATE_MS);
}
/*--------------------------------------------------------------------------------------------------------------------*/
void esp_err_t DrvI2cReadByte(uint8_t slaveaddress, uint16_t reg, uint8_t* data)
{
    uint8_t write_buf[2];
    write_buf[0] = (uint8_t)(reg >> 8);
    write_buf[1] = (uint8_t)(reg & 0x00FF);
    return i2c_master_write_read_device(I2C_MASTER_NUM, slaveaddres, &write_buf, 2, data, 1, I2C_MASTER_TIMEOUT_MS/ portTICK_RATE_MS);
}
/*--------------------------------------------------------------------------------------------------------------------*/
void esp_err_t DrvI2cWriteData(uint8_t slaveaddress, uint16_t start_reg, uint8_t* data, uint8_t length)
{
    uint8_t temp = 0;
    uint8_t write_buf[258];
    uint8_t i = 0;
    if(length < 256)
    {
        write_buf[0] = (uint8_t)(start_reg >> 8);
        write_buf[1] = (uint8_t)(start_reg & 0x00FF);
        for(i = 0; i < length; i++)
        {
            write_buf[i+1] = *data;
        }
    }
    i2c_master_write_to_device(I2C_MASTER_NUM, slaveaddress, write_buf, length, I2C_MASTER_TIMEOUT_MS/ portTICK_RATE_MS);
}
/*--------------------------------------------------------------------------------------------------------------------*/
void esp_err_t DrvI2cReadData(uint8_t slaveaddress, uint16_t start_reg, uint8_t* data, uint8_t length)
{
    uint8_t write_buf[2];
    write_buf[0] = (uint8_t)(start_reg >> 8);
    write_buf[1] = (uint8_t)(start_reg & 0x00FF);
    return i2c_master_write_read_device(I2C_MASTER_NUM, slaveaddress, write_buf, 2, data, length, I2C_MASTER_TIMEOUT_MS/ portTICK_RATE_MS);
}
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/

/**********************************************************************************************************************/
