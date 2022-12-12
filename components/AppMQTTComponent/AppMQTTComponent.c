/**********************************************************************************************************************/
/**
 * @file        AppMQTT.c
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
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include "esp_system.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "esp_log.h"

#include "mqtt_client.h"
#include "sdkconfig.h"

// DRIVER lib include section

// STANDARD lib include section
// APPLICATION lib include section
#include "AppMQTTComponent.h"
#include "AppDeviceComponent.h"
#include "AppParserComponent.h"
#include "AppOTAComponent.h"

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
esp_mqtt_client_handle_t client;
static const char *TAG = "MQTT Component";

static char mac_str[18];
static char config_topic[100];
static char control_topic[100];
static char ota_topic[100];
static char diagnostics_topic[100];
static char setup_topic[100];

//extern const uint8_t client_key_pem_start[] asm("_binary_client_key_pem_start");
//extern const uint8_t client_key_pem_end[] asm("_binary_client_key_pem_end");

extern const uint8_t server_pem_start[] asm("_binary_server_pem_start");
extern const uint8_t server_pem_end[] asm("_binary_server_pem_end");

/**********************************************************************************************************************/

/***********************************************************************************************************************
; E X P O R T E D   V A R I A B L E S
;---------------------------------------------------------------------------------------------------------------------*/

/**********************************************************************************************************************/

/***********************************************************************************************************************
; L O C A L   F U N C T I O N S
;---------------------------------------------------------------------------------------------------------------------*/
static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0)
    {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/
/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void AppMqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");

        // Once connected to broker subscribe to Config and Control topic
        ESP_LOGI(TAG, "Config topic: %s", config_topic);
        ESP_LOGI(TAG, "Control topic: %s", control_topic);
        ESP_LOGI(TAG, "OTA topic: %s", ota_topic);
        ESP_LOGI(TAG, "Setup topic SC: %s", setup_topic);

        esp_mqtt_client_subscribe(client, config_topic, 2);
        esp_mqtt_client_subscribe(client, control_topic, 2);
        esp_mqtt_client_subscribe(client, ota_topic, 2);
        esp_mqtt_client_subscribe(client, setup_topic, 2);

        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        event->data[event->data_len] = '\0';
        event->topic[event->topic_len] = '\0';

        // Select parser based on topic
        if (strcmp(event->topic, config_topic) == 0)
        {
            ESP_LOGI(TAG, "Parsing config message\r\n");
            AppParser_parse_config(event->data);
        }
        else if (strcmp(event->topic, control_topic) == 0)
        {
            ESP_LOGI(TAG, "Parsing control message\r\n");
            AppParser_parse_control(event->data);
        }
        else if (strcmp(event->topic, ota_topic) == 0)
        {
            ESP_LOGI(TAG, "Parsing OTA message\r\n");
            AppParser_parse_ota(event->data);
        }
        else if (strcmp(event->topic, setup_topic) == 0)
        {
            ESP_LOGI(TAG, "Parsing setup message\r\n");
            AppParser_parse_setup(event->data);
        }
        else
        {
            ESP_LOGE(TAG, "Unknown topic\r\n");
        }
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
        {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/

/**********************************************************************************************************************/

/***********************************************************************************************************************
; E X P O R T E D   F U N C T I O N S
;---------------------------------------------------------------------------------------------------------------------*/
void AppMQT_client_Init()
{

    AppDevice_get_unique(mac_str);
    sprintf(control_topic, "%s/%s", CONFIG_CONTROL_TOPIC, mac_str);
    sprintf(config_topic, "%s/%s", CONFIG_CONFIG_TOPIC, mac_str);
    sprintf(ota_topic, "%s/%s", CONFIG_OTA_TOPIC, mac_str);
    sprintf(diagnostics_topic, "%s/%s", CONFIG_DIAGNOSTICS_TOPIC, mac_str);
    sprintf(setup_topic, "%s/%s", CONFIG_SETUP_TOPIC, mac_str);

    esp_log_level_set(TAG, ESP_LOG_INFO);

    //esp_mqtt_client_config_t mqtt_cfg = {
    //   .uri = CONFIG_BROKER_URL,
    //};

    /*Test S-tronics*/
    // esp_mqtt_client_config_t mqtt_cfg = {
    //     //.host = "69ae9e26e67e44c6840f742a889493ef.s2.eu.hivemq.cloud",
    //     .uri = "mqtts://69ae9e26e67e44c6840f742a889493ef.s2.eu.hivemq.cloud",
    //     .port = 8883,
    //     .username = "ATCBragaTest",
    //     .password = "ATCBragaTestpass1",
    //     .client_id = "clientId-bq3K52DGsM",
    //     .cert_pem = (const char*)server_pem_start,
    // };

    /*Test Oddessey Solutions*/
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = "mqtts://ecec22b4c20f44b0b02f4011523b1c1c.s1.eu.hivemq.cloud",
        .port = 8883,
        .username = "driver",
        .password = "zihnaQ-3zymki-muqbox",
        .client_id = "clientId-bq3K52DGsM",
        .cert_pem = (const char*)server_pem_start,
    };

    //mqtts:///username:password@xxxxxxxx.s2.eu.hivemq.cloud:8883

    client = esp_mqtt_client_init(&mqtt_cfg);

    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, AppMqtt_event_handler, NULL);
    esp_mqtt_client_start(client);

    ESP_LOGI(TAG, "Init Mqtt Done");
}
/*--------------------------------------------------------------------------------------------------------------------*/
void AppMQTT_client_publish(char *topic, char *data, int qos, int retain)
{
    esp_mqtt_client_publish(client, topic, data, 0, qos, retain);
}

/*--------------------------------------------------------------------------------------------------------------------*/
void AppMqtt_client_subscribe(char *topic, int qos)
{
    esp_mqtt_client_subscribe(client, topic, qos);
}
/*--------------------------------------------------------------------------------------------------------------------*/
void AppMQTT_client_publish_diagnostics(char *data, int qos, int retain)
{
    esp_mqtt_client_publish(client, diagnostics_topic, data, 0, qos, retain);
}
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/

/**********************************************************************************************************************/
