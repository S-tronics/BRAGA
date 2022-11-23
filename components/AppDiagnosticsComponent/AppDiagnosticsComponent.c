/**********************************************************************************************************************/
/**
 * @file        AppDiagnosticsComponent.c
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
#include "AppDiagnosticsComponent.h"
#include "AppDeviceComponent.h"
#include "AppMQTTComponent.h"
#include "cJSON.h"

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
static const char *TAG = "Diagnostics Component";
int stairstaken = 0;
/**********************************************************************************************************************/

/***********************************************************************************************************************
; E X P O R T E D   V A R I A B L E S
;---------------------------------------------------------------------------------------------------------------------*/
extern char config_topic[100];
/**********************************************************************************************************************/

/***********************************************************************************************************************
; L O C A L   F U N C T I O N S
;---------------------------------------------------------------------------------------------------------------------*/
static void AppDiagnostics_device_keep_alive_task(void *arg)
{
    while (1)
    {
        cJSON *root;
        char *out;
        root = cJSON_CreateObject();
        cJSON_AddItemToObject(root, "message_type", cJSON_CreateString("diagnostics"));
        char keep_alive_string[25];
        sprintf(keep_alive_string, "minutes-%d", CONFIG_KEEP_ALIVE_INTERVAL_MINUTES);
        cJSON_AddItemToObject(root, "keep-alive", cJSON_CreateString(keep_alive_string));

        char device_unique[25];
        AppDevice_get_unique(device_unique);
        cJSON *device;
        device = cJSON_CreateObject();
        cJSON_AddItemToObject(device, "unique", cJSON_CreateString(device_unique));
        cJSON_AddItemToObject(device, "software_version", cJSON_CreateString(CONFIG_SOFTWARE_VERSION));
        cJSON_AddItemToObject(device, "hardware_version", cJSON_CreateString(CONFIG_HARDWARE_VERSION));
        cJSON_AddItemToObject(root, "device", device);

        out = cJSON_Print(root);
        AppMQTT_client_publish_diagnostics(out, 2, 0);

        /* free all objects under root and root itself */
        free(out);
        cJSON_Delete(root);

        // 60000ms = 1 minute
        vTaskDelay((CONFIG_KEEP_ALIVE_INTERVAL_MINUTES * 60000) / portTICK_RATE_MS);
    }
}

static void AppDiagnostics_free_ram_task(void *arg)
{
    while (1)
    {
        /* code */
        vTaskDelay(10000 / portTICK_RATE_MS);
        // get the free ram of esp32
        ESP_LOGI(TAG, "Free RAM: %d", esp_get_free_heap_size());
    }
}
static void AppDiagnostics_device_logging_task(void *arg)
{
    while (1)
    {
        cJSON *root;
        char *out;
        root = cJSON_CreateObject();
        cJSON_AddItemToObject(root, "message_type", cJSON_CreateString("logging"));

        char device_unique[25];
        AppDevice_get_unique(device_unique);
        cJSON *device;
        device = cJSON_CreateObject();
        cJSON_AddItemToObject(device, "unique", cJSON_CreateString(device_unique));
        cJSON_AddItemToObject(device, "software_version", cJSON_CreateString(CONFIG_SOFTWARE_VERSION));
        cJSON_AddItemToObject(device, "hardware_version", cJSON_CreateString(CONFIG_HARDWARE_VERSION));
        cJSON_AddItemToObject(root, "device", device);

        cJSON *logs;
        logs = cJSON_CreateObject();
        cJSON_AddItemToObject(logs, "stairs_taken", cJSON_CreateNumber(stairstaken));
        // TODO: implement driver voltage and current diagnostics with on/off
        cJSON_AddItemToObject(logs, "driver_voltage", cJSON_CreateNumber(3.3));
        cJSON_AddItemToObject(logs, "driver_current", cJSON_CreateNumber(0.05));
        cJSON_AddItemToObject(logs, "driver_onoff", cJSON_CreateNumber(1));
        cJSON_AddItemToObject(root, "logs", logs);

        out = cJSON_Print(root);
        AppMQTT_client_publish_diagnostics(out, 2, 0);

        /* free all objects under root and root itself */
        free(out);
        cJSON_Delete(root);

        stairstaken = 0;

        // 60000ms = 1 minute
        vTaskDelay((CONFIG_LOGGING_INTERVAL_MINUTES * 60000) / portTICK_RATE_MS);
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/

/**********************************************************************************************************************/

/***********************************************************************************************************************
; E X P O R T E D   F U N C T I O N S
;---------------------------------------------------------------------------------------------------------------------*/
void AppDiagnostics_init()
{
    ESP_LOGI(TAG, "AppDiagnostics_init");

    xTaskCreate(AppDiagnostics_device_keep_alive_task, "AppDiagnostics_device_keep_alive_task", 4096, NULL, 10, NULL);
    xTaskCreate(AppDiagnostics_free_ram_task, "AppDiagnostics_device_keep_alive_task", 4096, NULL, 10, NULL);
    xTaskCreate(AppDiagnostics_device_logging_task, "AppDiagnostics_device_logging_task", 4096, NULL, 10, NULL);
}
/*--------------------------------------------------------------------------------------------------------------------*/
void AppDiagnostics_stair_detection_message(DIRECTION d)
{
    cJSON *root;
    char *out;
    root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "message_type", cJSON_CreateString("detection"));

    char device_unique[25];
    AppDevice_get_unique(device_unique);
    cJSON *device;
    device = cJSON_CreateObject();
    cJSON_AddItemToObject(device, "unique", cJSON_CreateString(device_unique));
    cJSON_AddItemToObject(device, "software_version", cJSON_CreateString(CONFIG_SOFTWARE_VERSION));
    cJSON_AddItemToObject(device, "hardware_version", cJSON_CreateString(CONFIG_HARDWARE_VERSION));
    cJSON_AddItemToObject(root, "device", device);

    cJSON *detection_direction;
    detection_direction = cJSON_CreateObject();
    cJSON_AddItemToObject(device, "detection_direction", cJSON_CreateNumber((d == UP) ? 1 : 0));

    out = cJSON_Print(root);
    AppMQTT_client_publish_diagnostics(out, 2, 0);

    /* free all objects under root and root itself */
    free(out);
    cJSON_Delete(root);
}
/*--------------------------------------------------------------------------------------------------------------------*/
void AppDiagnostics_stairstaken_add()
{
    stairstaken++;
}
/*--------------------------------------------------------------------------------------------------------------------*/
void AppDiagnostics_error_message(char *error_message)
{

#if CONFIG_PRINT_ERROR_MESSAGE
    ESP_LOGE(TAG, "%s", error_message);
#endif

    cJSON *root;
    char *out;
    root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "message_type", cJSON_CreateString("error"));

    char device_unique[25];
    AppDevice_get_unique(device_unique);
    cJSON *device;
    device = cJSON_CreateObject();
    cJSON_AddItemToObject(device, "unique", cJSON_CreateString(device_unique));
    cJSON_AddItemToObject(device, "software_version", cJSON_CreateString(CONFIG_SOFTWARE_VERSION));
    cJSON_AddItemToObject(device, "hardware_version", cJSON_CreateString(CONFIG_HARDWARE_VERSION));
    cJSON_AddItemToObject(root, "device", device);

    cJSON *error;
    error = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "error_message", cJSON_CreateString(error_message));

    out = cJSON_Print(root);
    AppMQTT_client_publish_diagnostics(out, 2, 0);

    /* free all objects under root and root itself */
    free(out);
    cJSON_Delete(root);
}
/**********************************************************************************************************************/