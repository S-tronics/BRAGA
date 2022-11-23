idf_component_register(SRCS "AppOTAComponent.c"
                    REQUIRES "esp_http_client app_update nvs_flash esp_https_ota"
                    INCLUDE_DIRS "include")