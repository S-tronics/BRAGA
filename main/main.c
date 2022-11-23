#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>


#include "AppMQTTComponent.h"
#include "AppDeviceComponent.h"
#include "AppOTAComponent.h"
#include "AppDiagnosticsComponent.h"
#include "AppSciComponent.h"
#include "AppSetupComponent.h"
#include "AppColorComponent.h"
#include "AppBluetooth.h"
#include "protocol_examples_common.h"

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "sdkconfig.h"

#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"

static const char *TAG = "Main file";

void app_main(void)
{
    printf("Hello world!\n");

    // Initialize NVS.
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // 1.OTA app partition table has a smaller NVS partition size than the non-OTA
        // partition table. This size mismatch may cause NVS initialization to fail.
        // 2.NVS partition contains data in new format and cannot be recognized by this version of code.
        // If this happens, we erase NVS partition and initialize NVS again.
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */

    example_connect();

    /* Ensure to disable any WiFi power save mode, this allows best throughput
     * and hence timings for overall OTA operation.
     */
    //esp_wifi_set_ps(WIFI_PS_NONE);

    AppDeviceInit();
    AppColor_Init();
    AppMQT_client_Init();
    AppDiagnostics_init();
    AppSciInit();
    AppBluetooth_Init();
    
    // AppTimeComponent_Init();
    // AppDiagnostics_stair_detection_message(UP);
    // AppDiagnostics_stair_detection_message(DOWN);

    // AppSetup_setup_message(1, START_INSTALLATION);
    // AppSetup_setup_message(2, LED_CONNECTED);
    // AppSetup_setup_message(5, STOP_INSTALLATION);

    // AppDiagnostics_error_message("This is an error message test!");
}