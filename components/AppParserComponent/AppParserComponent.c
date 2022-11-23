/**********************************************************************************************************************/
/**
 * @file        AppParserComponent.c
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
#include "cJSON.h"
#include "sdkconfig.h"
#include "AppParserComponent.h"
#include "AppDeviceComponent.h"
#include "AppOTAComponent.h"
#include "AppSciComponent.h"
#include "AppSetupComponent.h"
#include "AppDiagnosticsComponent.h"
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
static const char *TAG = "Parser Component";
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
void AppParser_parse_config(char *json_string)
{
    cJSON *root = cJSON_Parse(json_string);
    if (!cJSON_IsObject(root))
    {
        AppDiagnostics_error_message("Config topic: JSON is not an object");
        cJSON_Delete(root);
        return;
    }

    cJSON *message_type = cJSON_GetObjectItem(root, "message_type");
    if (!cJSON_IsString(message_type))
    {
        AppDiagnostics_error_message("Config topic: Message type is not a string");
        cJSON_Delete(root);
        return;
    }
    ESP_LOGI(TAG, "message type: %s", message_type->valuestring);

    // Initial setting message arrived
    if (strcmp(message_type->valuestring, "setting") == 0)
    {

        cJSON *device = cJSON_GetObjectItem(root, "device");
        if (!cJSON_IsObject(device))
        {
            AppDiagnostics_error_message("Config topic: Device is not an object - setting");
            cJSON_Delete(root);
            return;
        }

        cJSON *unique = cJSON_GetObjectItem(device, "unique");
        if (!cJSON_IsString(unique))
        {
            AppDiagnostics_error_message("Config topic: Unique is not a string - setting");
            cJSON_Delete(root);
            return;
        }

        char mac_device[12];
        AppDevice_get_unique(mac_device);
        if (strcmp(unique->valuestring, mac_device) != 0)
        {
            AppDiagnostics_error_message("Unique from conrol topic -setting does not match unique from device");
            cJSON_Delete(root);
            return;
        }

        cJSON *software_version = cJSON_GetObjectItem(device, "software_version");
        if (!cJSON_IsString(software_version))
        {
            AppDiagnostics_error_message("Config topic: Software version is not a string in control topic - setting");
            cJSON_Delete(root);
            return;
        }

        if (AppDevice_compare_software_version_device(software_version->valuestring))
        {
            AppDiagnostics_error_message("Config topic: Software version from control topic is not the same as the one on the device - setting");
            cJSON_Delete(root);
            return;
        }

        cJSON *hardware_version = cJSON_GetObjectItem(device, "hardware_version");
        if (!cJSON_IsString(hardware_version))
        {
            AppDiagnostics_error_message("Config topic: Hardware version is not a string - setting");
            cJSON_Delete(root);
            return;
        }

        if (AppDevice_compare_hardware_version_device(hardware_version->valuestring))
        {
            AppDiagnostics_error_message("Config topic: Hardware version from control topic is not the same as the one on the device - setting");
            cJSON_Delete(root);
            return;
        }

        cJSON *nbr_stairs = cJSON_GetObjectItem(root, "nbr_stairs");
        if (!cJSON_IsNumber(nbr_stairs))
        {
            AppDiagnostics_error_message("Config topic: Nbr stairs is not a number in control topic - setting");
            cJSON_Delete(root);
            return;
        }
        if (nbr_stairs->valueint <= 0 || nbr_stairs->valueint > CONFIG_MAX_NBR_STAIRS)
        {
            AppDiagnostics_error_message("Config topic: Nbr stairs is not in range in control topic - setting");
            cJSON_Delete(root);
            return;
        }

        cJSON *stairs = cJSON_GetObjectItem(root, "stairs");
        if (!cJSON_IsArray(stairs))
        {
            AppDiagnostics_error_message("Config topic: Stairs is not an array - setting");
            cJSON_Delete(root);
            return;
        }

        if (cJSON_GetArraySize(stairs) <= 0)
        {
            AppDiagnostics_error_message("Config topic: Stairs is empty - setting");
            cJSON_Delete(root);
            return;
        }

        // Iterate trough everty stair in the stairs array and get the configs
        cJSON *iterator = NULL;
        int i = 0;
        cJSON_ArrayForEach(iterator, stairs)
        {
            if (!cJSON_IsObject(iterator))
            {
                AppDiagnostics_error_message("Config topic: Iterator stairs is not an object - setting");
                cJSON_Delete(root);
                return;
            }

            cJSON *nbr = cJSON_GetObjectItem(iterator, "nbr");
            if (!cJSON_IsNumber(nbr))
            {
                AppDiagnostics_error_message("Config topic: nbr is not number - setting");
                cJSON_Delete(root);
                return;
            }

            cJSON *det = cJSON_GetObjectItem(iterator, "det");
            if (!cJSON_IsNumber(det))
            {
                AppDiagnostics_error_message("Config topic: det is not number - setting");
                cJSON_Delete(root);
                return;
            }
            cJSON *r, *g, *b, *w;
            r = cJSON_GetObjectItem(iterator, "redbasic");
            g = cJSON_GetObjectItem(iterator, "greenbasic");
            b = cJSON_GetObjectItem(iterator, "bluebasic");
            w = cJSON_GetObjectItem(iterator, "whitebasic");
            if (!cJSON_IsNumber(r) || !cJSON_IsNumber(g) || !cJSON_IsNumber(b) || !cJSON_IsNumber(w))
            {
                AppDiagnostics_error_message("Config topic: rgbw is not number - setting");
                cJSON_Delete(root);
                return;
            }

            // TODO: Put values in the stairs array

            i++;
        }
        cJSON_Delete(root);
        return;
    }
    cJSON_Delete(root);
    return;
}
/*--------------------------------------------------------------------------------------------------------------------*/
void AppParser_parse_setup(char *json_string)
{
    cJSON *root = cJSON_Parse(json_string);
    if (!cJSON_IsObject(root))
    {
        AppDiagnostics_error_message("Setup topic: JSON is not an object");
        cJSON_Delete(root);
        return;
    }

    cJSON *message_type = cJSON_GetObjectItem(root, "message_type");
    if (!cJSON_IsString(message_type))
    {
        AppDiagnostics_error_message("Setup topic: Message type is not a string");
        cJSON_Delete(root);
        return;
    }
    ESP_LOGI(TAG, "message type: %s", message_type->valuestring);

    // Initial setting message arrived
    if (strcmp(message_type->valuestring, "setupevent") == 0)
    {
        cJSON *device = cJSON_GetObjectItem(root, "device");
        if (!cJSON_IsObject(device))
        {
            AppDiagnostics_error_message("Setup topic: Device is not an object - setupevent");
            cJSON_Delete(root);
            return;
        }

        cJSON *unique = cJSON_GetObjectItem(device, "unique");
        if (!cJSON_IsString(unique))
        {
            AppDiagnostics_error_message("Setup topic: Unique is not a string in setup topic - setupevent");
            cJSON_Delete(root);
            return;
        }

        char mac_device[12];
        AppDevice_get_unique(mac_device);
        if (strcmp(unique->valuestring, mac_device) != 0)
        {
            AppDiagnostics_error_message("Setup topic: Unique from - setupevent does not match unique from device");
            cJSON_Delete(root);
            return;
        }

        cJSON *stair_number = cJSON_GetObjectItem(root, "stair_number");
        if (!cJSON_IsNumber(stair_number))
        {
            AppDiagnostics_error_message("Setup topic: stair number not found- setupevent");
            cJSON_Delete(root);
            return;
        }
        if (stair_number->valueint <= 0 || stair_number->valueint > CONFIG_MAX_NBR_STAIRS)
        {
            AppDiagnostics_error_message("Setup topic: stair number is not in range - setupevent");
            cJSON_Delete(root);
            return;
        }

        cJSON *stair_procedure = cJSON_GetObjectItem(root, "stair_procedure");
        if (!cJSON_IsNumber(stair_procedure))
        {
            AppDiagnostics_error_message("Setup topic: stair procedure not a number in- setupevent");
            cJSON_Delete(root);
            return;
        }

        switch (stair_procedure->valueint) // TODO: Implement stair procedures
        {

        case 1:
            ESP_LOGI(TAG, "Stair procedure: 1 - Stair procedure 1");
            break;
        case 2:
            ESP_LOGI(TAG, "Stair procedure: 2 - Stair procedure 2");
            break;
        case 4:
            ESP_LOGI(TAG, "Stair procedure: 4 - Stair procedure 4");
            break;
        default:
            AppDiagnostics_error_message("Setup topic: Unknown stair procedure number - setupevent");
            break;
        }
    }
    else if (strcmp(message_type->valuestring, "resetevent") == 0)
    {
        cJSON *device = cJSON_GetObjectItem(root, "device");
        if (!cJSON_IsObject(device))
        {
            AppDiagnostics_error_message("Setup topic: Device is not an object in setup topic - resetevent");
            cJSON_Delete(root);
            return;
        }

        cJSON *unique = cJSON_GetObjectItem(device, "unique");
        if (!cJSON_IsString(unique))
        {
            AppDiagnostics_error_message("Setup topic: Unique is not a string - resetevent");
            cJSON_Delete(root);
            return;
        }

        char mac_device[12];
        AppDevice_get_unique(mac_device);
        if (strcmp(unique->valuestring, mac_device) != 0)
        {
            AppDiagnostics_error_message("Setup topic: unique from - resetevent does not match unique from device");
            cJSON_Delete(root);
            return;
        }

        // TODO: reset implementation
        // AppSetup_reset_installation();
    }

    cJSON_Delete(root);
    return;
}

void AppParser_parse_control(char *json_string)
{
    cJSON *root = cJSON_Parse(json_string);
    cJSON *r = NULL;
    cJSON *g = NULL;
    cJSON *b = NULL;
    cJSON *w = NULL;

    cJSON *start_time = NULL;
    cJSON *end_time = NULL;

    cJSON *animation_payload = NULL;

    if (!cJSON_IsObject(root))
    {
        AppDiagnostics_error_message("Control topic: JSON is not an object");
        cJSON_Delete(root);
        return;
    }
    cJSON *message_type = cJSON_GetObjectItem(root, "message_type");
    if (!cJSON_IsString(message_type))
    {
        AppDiagnostics_error_message("Setup topic: Message type is not a string");
        cJSON_Delete(root);
        return;
    }
    ESP_LOGI(TAG, "message type: %s", message_type->valuestring);

    // Initial setting message arrived
    if (strcmp(message_type->valuestring, "controlevent") == 0)
    {
        cJSON *device = cJSON_GetObjectItem(root, "device");
        if (!cJSON_IsObject(device))
        {
            AppDiagnostics_error_message("Control topic: Device is not an object");
            cJSON_Delete(root);
            return;
        }

        cJSON *unique = cJSON_GetObjectItem(device, "unique");
        if (!cJSON_IsString(unique))
        {
            ESP_LOGE(TAG, "Control topic Unique is not a string");
            cJSON_Delete(root);
            return;
        }

        char mac_device[12];
        AppDevice_get_unique(mac_device);
        if (strcmp(unique->valuestring, mac_device) != 0)
        {
            ESP_LOGE(TAG, "Control topic: unique does not match unique from device");
            cJSON_Delete(root);
            return;
        }

        cJSON *software_version = cJSON_GetObjectItem(device, "software_version");
        if (!cJSON_IsString(software_version))
        {
            AppDiagnostics_error_message("Control topic: Software version is not a string");
            cJSON_Delete(root);
            return;
        }

        if (AppDevice_compare_software_version_device(software_version->valuestring))
        {
            AppDiagnostics_error_message("Control topic: Software version is not the same as the one on the device");
            cJSON_Delete(root);
            return;
        }

        cJSON *hardware_version = cJSON_GetObjectItem(device, "hardware_version");
        if (!cJSON_IsString(hardware_version))
        {
            AppDiagnostics_error_message("Control topic: Hardware version is not a string");
            cJSON_Delete(root);
            return;
        }

        if (AppDevice_compare_hardware_version_device(hardware_version->valuestring))
        {
            AppDiagnostics_error_message("Control topic : Hardware version is not the same as the one on the device");
            cJSON_Delete(root);
            return;
        }

        // TODO: utilize nbr_animations?
        cJSON *nbr_animations = cJSON_GetObjectItem(root, "nbr_animations");
        if (!cJSON_IsNumber(nbr_animations))
        {
            AppDiagnostics_error_message("Control topic: Nbr animations is not a number");
            cJSON_Delete(root);
            return;
        }
        if (nbr_animations->valueint <= 0)
        {
            AppDiagnostics_error_message("Control topic: Nbr animations is not in range");
            cJSON_Delete(root);
            return;
        }

        cJSON *animations = cJSON_GetObjectItem(root, "animations");
        if (!cJSON_IsArray(animations))
        {
            AppDiagnostics_error_message("Control topic: Animations is not an array");
            cJSON_Delete(root);
            return;
        }
        int j = cJSON_GetArraySize(animations);

        if (cJSON_GetArraySize(animations) <= 0)
        {
            AppDiagnostics_error_message("Control topic: Animations is empty");
            cJSON_Delete(root);
            return;
        }

        // Iterate trough everty animation in the animation array
        cJSON *iterator = NULL;
        int i = 0;
        cJSON_ArrayForEach(iterator, animations)
        {
            if (!cJSON_IsObject(iterator))
            {
                AppDiagnostics_error_message("Control topic: Iterator animations is not an object");
                cJSON_Delete(root);
                return;
            }

            cJSON *animation_type = cJSON_GetObjectItem(iterator, "animation_type");
            if (!cJSON_IsString(animation_type))
            {
                AppDiagnostics_error_message("Control topic: Animation type is not a string");
                cJSON_Delete(root);
                return;
            }
            printf(animation_type->valuestring);
            // Create parser based on animation types. <=========================================================
            if (strcmp(animation_type->valuestring, "static") == 0)
            {
                animation_payload = cJSON_GetObjectItem(iterator, "animation_payload");
                if (!cJSON_IsObject(animation_payload))
                {
                    AppDiagnostics_error_message("Control topic: Animation payload is not an object - motion");
                    cJSON_Delete(root);
                    return;
                }

                start_time = cJSON_GetObjectItem(animation_payload, "time_start");
                if (!cJSON_IsString(start_time))
                {
                    AppDiagnostics_error_message("Control topic: Start time is not a string - motion");
                    cJSON_Delete(root);
                    return;
                }

                end_time = cJSON_GetObjectItem(animation_payload, "time_end");
                if (!cJSON_IsString(end_time))
                {
                    AppDiagnostics_error_message("Control topic: End time is not a string - motion");
                    cJSON_Delete(root);
                    return;
                }
                cJSON *motion = cJSON_GetObjectItem(animation_payload, "motion");
                if (!cJSON_IsNumber(motion))
                {
                    AppDiagnostics_error_message("Control topic: Motion is not a number - motion");
                    cJSON_Delete(root);
                    return;
                }
                r = cJSON_GetObjectItem(animation_payload, "redbasic");
                g = cJSON_GetObjectItem(animation_payload, "greenbasic");
                b = cJSON_GetObjectItem(animation_payload, "bluebasic");
                w = cJSON_GetObjectItem(animation_payload, "whitebasic");

                if (!cJSON_IsNumber(r) || !cJSON_IsNumber(g) || !cJSON_IsNumber(b) || !cJSON_IsNumber(w))
                {
                    AppDiagnostics_error_message("Control topic: rgbw is not number - static");
                    cJSON_Delete(root);
                    return;
                }

                if (r->valueint < 0 || r->valueint > 255 || g->valueint < 0 || g->valueint > 255 || b->valueint < 0 || b->valueint > 255 || w->valueint < 0 || w->valueint > 255)
                {
                    AppDiagnostics_error_message("Control topic: rgbw is not in range - static");
                    cJSON_Delete(root);
                    return;
                }

                ESP_LOGI(TAG, "Motion animation: Red: %d, Green: %d, Blue: %d, White: %d\nFrom %s to %s", r->valueint, g->valueint, b->valueint, w->valueint, start_time->valuestring, end_time->valuestring);

                // TODO: Implement motion animation

                // cJSON_Delete(root);
                // return;

            }
            else if (strcmp(animation_type->valuestring, "motions") == 0)
            {
                animation_payload = cJSON_GetObjectItem(iterator, "animation_payload");
                if (!cJSON_IsObject(animation_payload))
                {
                    AppDiagnostics_error_message("Control topic: Animation payload is not an object - motion");
                    cJSON_Delete(root);
                    return;
                }

                start_time = cJSON_GetObjectItem(animation_payload, "time_start");
                if (!cJSON_IsString(start_time))
                {
                    AppDiagnostics_error_message("Control topic: Start time is not a string - motion");
                    cJSON_Delete(root);
                    return;
                }

                end_time = cJSON_GetObjectItem(animation_payload, "time_end");
                if (!cJSON_IsString(end_time))
                {
                    AppDiagnostics_error_message("Control topic: End time is not a string - motion");
                    cJSON_Delete(root);
                    return;
                }
                cJSON *motion = cJSON_GetObjectItem(animation_payload, "motion");
                if (!cJSON_IsNumber(motion))
                {
                    AppDiagnostics_error_message("Control topic: Motion is not a number - motion");
                    cJSON_Delete(root);
                    return;
                }
                r = cJSON_GetObjectItem(animation_payload, "redbasic");
                g = cJSON_GetObjectItem(animation_payload, "greenbasic");
                b = cJSON_GetObjectItem(animation_payload, "bluebasic");
                w = cJSON_GetObjectItem(animation_payload, "whitebasic");

                if (!cJSON_IsNumber(r) || !cJSON_IsNumber(g) || !cJSON_IsNumber(b) || !cJSON_IsNumber(w))
                {
                    AppDiagnostics_error_message("Control topic: rgbw is not number - static");
                    cJSON_Delete(root);
                    return;
                }

                if (r->valueint < 0 || r->valueint > 255 || g->valueint < 0 || g->valueint > 255 || b->valueint < 0 || b->valueint > 255 || w->valueint < 0 || w->valueint > 255)
                {
                    AppDiagnostics_error_message("Control topic: rgbw is not in range - static");
                    cJSON_Delete(root);
                    return;
                }

                ESP_LOGI(TAG, "Motion animation: Red: %d, Green: %d, Blue: %d, White: %d\nFrom %s to %s", r->valueint, g->valueint, b->valueint, w->valueint, start_time->valuestring, end_time->valuestring);

                // TODO: Implement motion animation

                // cJSON_Delete(root);
                // return;

            }
            else if (strcmp(animation_type->valuestring, "fade") == 0)
            {
                // TODO: implement fade animation;
            }
            else
            {
                AppDiagnostics_error_message("Control topic: Unknown animation type");
                cJSON_Delete(root);
                return;
            }
            i++;
            
        }
        cJSON_Delete(root);
        return;
    }
}
/*
{
    "version": "0.0.2",
    "URL": "https://192.168.0.159/3443/hello_world.bin",
    "device": {
    "unique": "7CDFA1E2AC64"
    }
}
*/
void AppParser_parse_ota(char *json_string)
{
    cJSON *root = cJSON_Parse(json_string);
    if (!cJSON_IsObject(root))
    {
        AppDiagnostics_error_message("OTA topic: JSON is not an valid");
        cJSON_Delete(root);
        return;
    }
    cJSON *device = cJSON_GetObjectItem(root, "device");
    if (!cJSON_IsObject(device))
    {
        AppDiagnostics_error_message("OTA topic: Device object not in OTA JSON");
        cJSON_Delete(root);
        return;
    }

    cJSON *unique = cJSON_GetObjectItem(device, "unique");
    if (!cJSON_IsString(unique))
    {
        AppDiagnostics_error_message("OTA topic: Unique property not found in device");
        cJSON_Delete(root);
        return;
    }
    char mac_device[12];
    AppDevice_get_unique(mac_device);
    if (strcmp(unique->valuestring, mac_device) != 0)
    {
        AppDiagnostics_error_message("OTA topic: Unique from OTA does not match unique from device");
        cJSON_Delete(root);
        return;
    }

    cJSON *version = cJSON_GetObjectItem(root, "software_version");
    if (!cJSON_IsString(version))
    {
        AppDiagnostics_error_message("OTA topic: Software version not found");
        cJSON_Delete(root);
        return;
    }

    cJSON *url = cJSON_GetObjectItem(root, "url");
    if (!cJSON_IsString(url))
    {
        AppDiagnostics_error_message("OTA topic: url not found");
        cJSON_Delete(root);
        return;
    }

    switch (AppDevice_compare_software_version_device(version->valuestring))
    {
    case -2:
        AppDiagnostics_error_message("OTA topic: Something went wrong comparing software versions");
        break;
    case -1:
        AppDiagnostics_error_message("OTA topic: Version from OTA is lower than version from device");
        break;
    case 0:
        AppDiagnostics_error_message("OTA topic: Version from OTA is equal to version from device");
        break;
    case 1:
        AppDiagnostics_error_message("OTA topic: Version from OTA is higher than version from device");

        // Do firmware update :)
        AppOTA_firmware_update(url->valuestring);
        break;

    default:
        ESP_LOGE(TAG, "You should not get here");
        break;
    }

    cJSON_Delete(root);
    return;
}
/**********************************************************************************************************************/