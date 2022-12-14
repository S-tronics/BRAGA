/**********************************************************************************************************************/
/**
 * @file        AppSetupComponent.c
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

#include "esp_log.h"
#include "sdkconfig.h"
#include "cJSON.h"
#include "AppSetupComponent.h"
#include "AppMQTTComponent.h"
#include "AppDeviceComponent.h"
#include "AppBluetooth.h"
#include "AppSciComponent.h"
#include "AppDiagnosticsComponent.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
/**********************************************************************************************************************/

/***********************************************************************************************************************
; L O C A L   D E F I N I T I O N S   A N D   M A C R O S
;---------------------------------------------------------------------------------------------------------------------*/

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
static const char *TAG = "Setup Component";
static uint8_t stair_number_latch = 0;          //0 is an unvalid value.

static TaskHandle_t xHandleSetup;
/**********************************************************************************************************************/

/***********************************************************************************************************************
; E X P O R T E D   V A R I A B L E S
;---------------------------------------------------------------------------------------------------------------------*/

/**********************************************************************************************************************/

/***********************************************************************************************************************
; L O C A L   F U N C T I O N S
;---------------------------------------------------------------------------------------------------------------------*/
static void setuptask(void *arg)
{

    if(AppSciSetupStair(stair_number_latch))
    {
        AppSetup_setup_message(stair_number_latch, MUI_RECEIVED, 0);
        vTaskDelete(xHandleSetup);
    }
    else if (AppDiagnostics_PeripheralsFast())
    {
        AppSetup_setup_message(stair_number_latch, ERROR, 1);
        vTaskDelete(xHandleSetup);
    }
    
}
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/

/**********************************************************************************************************************/

/***********************************************************************************************************************
; E X P O R T E D   F U N C T I O N S
;---------------------------------------------------------------------------------------------------------------------*/
void AppSetup_init(void)
{
    
}
/*--------------------------------------------------------------------------------------------------------------------*/
void AppSetup_setup_message(int stair_number, PROCEDURE p, uint8_t value)
{
    char *out;

    cJSON *root;
    cJSON *device;

    char device_unique[25];
    AppDevice_get_unique(device_unique);

    root = cJSON_CreateObject();
    device = cJSON_CreateObject();

    cJSON_AddItemToObject(root, "message_type", cJSON_CreateString("setupevent"));
    cJSON_AddItemToObject(device, "unique", cJSON_CreateString(device_unique));
    //cJSON_AddItemToObject(device, "software_version", cJSON_CreateString(CONFIG_SOFTWARE_VERSION));
    //cJSON_AddItemToObject(device, "hardware_version", cJSON_CreateString(CONFIG_HARDWARE_VERSION));
    cJSON_AddItemToObject(root, "device", device);
    cJSON_AddItemToObject(root, "stair_number", cJSON_CreateNumber(stair_number));
    cJSON_AddItemToObject(root, "stair_procedure", cJSON_CreateNumber(p));
    cJSON_AddItemToObject(root, "value", cJSON_CreateNumber(value));

    out = cJSON_Print(root);

    //AppMQTT_client_publish_diagnostics(out, 2, 0);
    AppBTLE_server_publish_setup(out);
     /* free all objects under root and root itself */
    free(out);
    cJSON_Delete(root);
}
/*--------------------------------------------------------------------------------------------------------------------*/
void AppSetup_execute(int stair_number, PROCEDURE p)            //Command from Client
{
    switch(p)
    {
        case START_INSTALLATION:
            AppSetup_setup_message(stair_number, LED_CONNECTED, 0);
            break;
        case LED_CONNECTED:
            AppDevice_CtrlPeripheral(true);
            ets_delay_us(500);                  //Wait until everything is startet up
            //xTaskCreate(setuptask, "setuptask", 4096, NULL, 10, &xHandleSetup);
            stair_number_latch = stair_number;
            break;
        case STOP_INSTALLATION:
            break;
        default:
            break;
    }

}
/*--------------------------------------------------------------------------------------------------------------------*/
void AppSetup_reset_installation()
{
    // TODO: reset installation
    ESP_LOGI(TAG, "Reset installation");
}
/**********************************************************************************************************************/