/**********************************************************************************************************************/
/**
 * @file        AppSci.c
 *
 * @author      Stijn Vermeersch
 * @date        16.06.2022
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
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "sdkconfig.h"
// DRIVER lib include section

// STANDARD lib include section
//#include "Standard/CRC/StdCRC.h"

// APPLICATION lib include section
#include "AppSciComponent.h"
#include "AppColorComponent.h"
//#include "AppConfig.h"
//#include "AppSci.h"
/**********************************************************************************************************************/

/***********************************************************************************************************************
; L O C A L   D E F I N I T I O N S   A N D   M A C R O S
;---------------------------------------------------------------------------------------------------------------------*/

#define BAUD_RATE (CONFIG_UART_BAUD_RATE)
#define UART_STACK_SIZE (CONFIG_UART_TASK_STACK_SIZE)
#define UART_PORT (CONFIG_UART_PORT_NUM)
#define UART_TXD (CONFIG_UART_TX_PIN)
#define UART_RXD (CONFIG_UART_RX_PIN)
#define UART_RTS (CONFIG_UART_RTS_PIN)
#define BUF_SIZE (127)

// Mux definition
#define MUX_PUBLISH 0x01
#define MUX_PUBLISH_CONFIRM 0x02
#define MUX_PX_GET 0x03
#define MUX_PX_SET 0x04
#define MUX_PX_CONFIG 0x05
#define MUX_COLOR_SET 0x06

#define MUX_PUBLISH_CONFIRM_OK 1 << 7
#define MUX_PUBLISH_CONFIRM_STAIR_UP 1 << 6
#define MUX_PUBLISH_CONFIRM_STAIR_DOWN 1 << 5
#define MUX_PUBLISH_CONFIRM_STAIR_NR(x) x << 0

#define QUEUE_SIZE (10)

/**********************************************************************************************************************/

/***********************************************************************************************************************
; L O C A L   T Y P E D E F S
;---------------------------------------------------------------------------------------------------------------------*/

typedef enum
{
    IDLE = 0,
    PX_GET = 1,
    PX_ANSWER = 2,
    COLOR_SET = 3
} APPSCISTATE;

typedef struct bus_message
{
    uint8_t body[20];
    int length;
} bus_message_t;

/**********************************************************************************************************************/

/***********************************************************************************************************************
; L O C A L   F U N C T I O N   P R O T O T Y P E S
;---------------------------------------------------------------------------------------------------------------------*/
static uint8_t AppSciCalcCrc(uint8_t *data, uint8_t length);
static void AppSciColorSetSend(uint8_t stairsnbr);
static void AppSciPxGetSend(uint8_t stairsnbr);
/**********************************************************************************************************************/

/***********************************************************************************************************************
; L O C A L   V A R I A B L E S
;---------------------------------------------------------------------------------------------------------------------*/
static const char *TAG = "Sci Component";
const int uart_num = UART_PORT;
// static APPSCISTAIR stairs[MAX_NBR_STAIRS];
static uint8_t nbrofstairs = 0;
static uint8_t idx_setupstair = 0;
static uint8_t data[BUF_SIZE];
static QueueHandle_t uart_queue;

int CAN_SEND_VLAG = 0;

xQueueHandle xQueue;


/**********************************************************************************************************************/

/***********************************************************************************************************************
; E X P O R T E D   V A R I A B L E S
;---------------------------------------------------------------------------------------------------------------------*/

/**********************************************************************************************************************/

/***********************************************************************************************************************
; L O C A L   F U N C T I O N S
;---------------------------------------------------------------------------------------------------------------------*/
void print_hex(char *pre, const uint8_t *s, int len)
{
    printf("%s", pre);
    for (int i = 0; i < len; i++)
    {
        printf("%02x ", (unsigned int)*s++);
    }
    printf("\n");
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void uart_write_bus_task(void *arg)
{
    bus_message_t bus_msg;

    while (1)
    {
        if (xQueue != 0)
        {

            // Receive a message on the created queue.  Block for 40 ticks if a
            // message is not immediately available.
            if (xQueueReceive(xQueue, &bus_msg, (TickType_t)40))
            {
                // pcRxedMessage now points to the struct AMessage variable posted
                // by vATask.
                // printf("Message: %s\nLenght: %d", (char *) rxmessage, strlen(rxmessage));
                uart_write_bytes(uart_num, bus_msg.body, bus_msg.length);
                // uart_write_bytes(ECHO_UART_PORT_NUM, (const char *)rxmessage, strlen(rxmessage));
            }
        }
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void uart_read_task(void *arg)
{
    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *)malloc(BUF_SIZE);
    static uint8_t stair_nbr = 1;

    while (1)
    {
        // uart_write_bytes(uart_num, "a", 1);
        //  Read data from the UART
        int len = uart_read_bytes(CONFIG_UART_PORT_NUM, data, BUF_SIZE, 110 / portTICK_RATE_MS);
        // Write data back to the UART
        if (len > 0)
        {
            data[len] = '\0';
            print_hex("Ontvangen: ", data, len);

            switch(data[0])
            {
                case MUX_PUBLISH:
                    stairs[idx_setupstair].config = true;
                    for(int i = 1; i <= 9; i++)
                    {
                        stairs[idx_setupstair].publishconfirm.mui[i-1] = data[i];
                    }
                    break;
                default:
                    break;
            }

            // Quick and dirty test
            if (data[0] == MUX_PUBLISH)
            {
                // set 100ms delay
                vTaskDelay(100 / portTICK_RATE_MS);

                uint8_t bytes[11];
                bytes[0] = MUX_PUBLISH_CONFIRM;
                for (int i = 1; i <= 9; i++)
                {
                    bytes[i] = data[i];
                }
                bytes[10] = MUX_PUBLISH_CONFIRM_STAIR_DOWN | MUX_PUBLISH_CONFIRM_STAIR_NR(stair_nbr) | MUX_PUBLISH_CONFIRM_OK;

                // Write data back to the UART
                uart_write_bytes(uart_num, bytes, 11);
                print_hex("Versturen: ", bytes, 11);
                stair_nbr++;

                CAN_SEND_VLAG = 1;
            }
        }
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void uart_write_rainbow_task(void *arg)
{
    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *)malloc(BUF_SIZE);
    static uint8_t red = 100, green = 0, blue = 0;
    static uint8_t bytes[11];
    while (1)
    {
        AppColor_rainbow_cycle(&red, &green, &blue);
        color_rgb_t color_rgb_rainbow = {.red = red, .green = green, .blue = blue};
        color_rgbw_t color_rgbw_rainbow = AppColor_rgb_to_rgbw(color_rgb_rainbow);

        bytes[0] = MUX_COLOR_SET;
        bytes[1] = 0x02; // stairsnbr
        bytes[2] = color_rgbw_rainbow.white;
        bytes[3] = color_rgbw_rainbow.red;
        bytes[4] = color_rgbw_rainbow.green;
        bytes[5] = color_rgbw_rainbow.blue;
        // Write data back to the UART
        uart_write_bytes(uart_num, bytes, 6);
        // print_hex("Versturen: ", bytes, 6);

        vTaskDelay(300 / portTICK_RATE_MS);
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void uart_write_white_task(void *arg)
{
    // 2500: (255, 161, 72)
    // 2600: (255, 165, 79)
    // 2700: (255, 169, 87)
    // 2800: (255, 173, 94)
    // 2900: (255, 177, 101)
    // 3000: (255, 180, 107)
    // 3100: (255, 184, 114)
    // 3200: (255, 187, 120)
    // 3300: (255, 190, 126)
    // 3400: (255, 193, 132)
    // 3500: (255, 196, 137)
    // 3600: (255, 199, 143)
    // 3700: (255, 201, 148)
    // 3800: (255, 204, 153)
    // 3900: (255, 206, 159)
    // 4000: (255, 209, 163)
    // 4100: (255, 211, 168)
    // 4200: (255, 213, 173)
    // 4300: (255, 215, 177)
    // 4400: (255, 217, 182)
    // 4500: (255, 219, 186)
    // 4600: (255, 221, 190)
    // 4700: (255, 223, 194)
    // 4800: (255, 225, 198)
    // 4900: (255, 227, 202)
    // 5000: (255, 228, 206)
    kelvin_rgb_t kelvin_rgb_list[] = {
        [0].kelvin = 2700,
        [0].color.red = 255,
        [0].color.green = 169,
        [0].color.blue = 87,
        [1].kelvin = 2800,
        [1].color.red = 255,
        [1].color.green = 173,
        [1].color.blue = 94,
        [2].kelvin = 2900,
        [2].color.red = 255,
        [2].color.green = 177,
        [2].color.blue = 101,
        [3].kelvin = 3000,
        [3].color.red = 255,
        [3].color.green = 180,
        [3].color.blue = 107,
        [4].kelvin = 3100,
        [4].color.red = 255,
        [4].color.green = 184,
        [4].color.blue = 114,
        [5].kelvin = 3200,
        [5].color.red = 255,
        [5].color.green = 187,
        [5].color.blue = 120,
        [6].kelvin = 3300,
        [6].color.red = 255,
        [6].color.green = 190,
        [6].color.blue = 126,
        [7].kelvin = 3400,
        [7].color.red = 255,
        [7].color.green = 193,
        [7].color.blue = 132,
        [8].kelvin = 3500,
        [8].color.red = 255,
        [8].color.green = 196,
        [8].color.blue = 137,
        [9].kelvin = 3600,
        [9].color.red = 255,
        [9].color.green = 199,
        [9].color.blue = 143,
        [10].kelvin = 3700,
        [10].color.red = 255,
        [10].color.green = 201,
        [10].color.blue = 148,
        [11].kelvin = 3800,
        [11].color.red = 255,
        [11].color.green = 204,
        [11].color.blue = 153,
        [12].kelvin = 3900,
        [12].color.red = 255,
        [12].color.green = 206,
        [12].color.blue = 159,
        [13].kelvin = 4000,
        [13].color.red = 255,
        [13].color.green = 209,
        [13].color.blue = 163,
        [14].kelvin = 4100,
        [14].color.red = 255,
        [14].color.green = 211,
        [14].color.blue = 168,
        [15].kelvin = 4200,
        [15].color.red = 255,
        [15].color.green = 213,
        [15].color.blue = 173,
        [16].kelvin = 4300,
        [16].color.red = 255,
        [16].color.green = 215,
        [16].color.blue = 177,
        [17].kelvin = 4400,
        [17].color.red = 255,
        [17].color.green = 217,
        [17].color.blue = 182,
        [18].kelvin = 4500,
        [18].color.red = 255,
        [18].color.green = 219,
        [18].color.blue = 186,
        [19].kelvin = 4600,
        [19].color.red = 255,
        [19].color.green = 221,
        [19].color.blue = 190,
        [20].kelvin = 4700,
        [20].color.red = 255,
        [20].color.green = 223,
        [20].color.blue = 194,
        [21].kelvin = 4800,
        [21].color.red = 255,
        [21].color.green = 225,
        [21].color.blue = 198,
        [22].kelvin = 4900,
        [22].color.red = 255,
        [22].color.green = 227,
        [22].color.blue = 202,
        [23].kelvin = 5000,
        [23].color.red = 255,
        [23].color.green = 228,
        [23].color.blue = 206};

    while ((1))
    {

        // for each item in kelvin_rgb_list print it to uart
        for (int i = 0; i < sizeof(kelvin_rgb_list) / sizeof(kelvin_rgb_list[0]); i++)
        {

            color_rgbw_t test = AppColor_rgb_to_rgbw(kelvin_rgb_list[i].color);

            uint8_t bytes[11];
            bytes[0] = MUX_COLOR_SET;
            bytes[1] = 0x01; // stairsnbr
            bytes[2] = test.white;
            bytes[3] = test.red;
            bytes[4] = test.green;
            bytes[5] = test.blue;
            // Write data back to the UART
            uart_write_bytes(uart_num, bytes, 6);
            char kelvin_string[25];
            sprintf(kelvin_string, "Versturen %dK: ", kelvin_rgb_list[i].kelvin);
            // print_hex(kelvin_string, bytes, 6);
            vTaskDelay(300 / portTICK_RATE_MS);
        }

        vTaskDelay(80 / portTICK_RATE_MS);
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void uart_write_px_get(void *arg)
{
    bus_message_t bus_message;
    
    while (1)
    {
        uint8_t bytes[3];
        bytes[0] = MUX_PX_GET;
        bytes[1] = 0x01; // stairsnbr
        bytes[2] = 0x55; // ctrl byte

        // Write data back to the UART
        // uart_write_bytes(uart_num, bytes, 3);

        bus_message.body[0] = bytes[0];
        bus_message.body[1] = bytes[1];
        bus_message.body[2] = bytes[2];
        bus_message.length = 3;

        xQueueSend(xQueue, (void *)&bus_message, (TickType_t)30);
        print_hex("MUX_PX_GET: ", bus_message.body, 3);
        vTaskDelay(2000 / portTICK_RATE_MS);
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/
static uint8_t AppSciCalcCrc(uint8_t *data, uint8_t length)
{
    uint8_t crc = 0x00;
    for (uint8_t i = 0; i < length; i++)
    {
        crc ^= *data;
        data++;
    }
    return crc;
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void AppSciColorSetSend(uint8_t stairsnbr)
{
    if (stairs[stairsnbr].config)
    {
        data[0] = stairs[stairsnbr].colorset.mux;
        data[1] = stairs[stairsnbr].colorset.address;
        data[2] = stairs[stairsnbr].colorset.data[0]; // White
        data[3] = stairs[stairsnbr].colorset.data[1]; // Red
        data[4] = stairs[stairsnbr].colorset.data[2]; // Green
        data[5] = stairs[stairsnbr].colorset.data[3]; // Blue
        stairs[stairsnbr].colorset.crc = AppSciCalcCrc(data, 6);
        data[6] = stairs[stairsnbr].colorset.crc;
        uart_write_bytes(uart_num, data, 7);
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/
static void AppSciPxGetSend(uint8_t stairsnbr)
{
    if (stairs[stairsnbr].config)
    {
        if (stairs[stairsnbr].has_px)
        {
            data[0] = stairs[stairsnbr].pxget.mux;
            data[1] = stairs[stairsnbr].pxget.address;
            data[2] = stairs[stairsnbr].pxget.data[0];
            stairs[stairsnbr].pxget.crc = AppSciCalcCrc(data, 3);
            data[3] = stairs[stairsnbr].pxget.crc;
            uart_write_bytes(uart_num, data, 4);
        }
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------*/
/**********************************************************************************************************************/

/***********************************************************************************************************************
; E X P O R T E D   F U N C T I O N S
;---------------------------------------------------------------------------------------------------------------------*/
void AppSciInit(void)
{
    uint8_t i = 0;

    for (i = 0; i < CONFIG_MAX_NBR_STAIRS; i++)
    {
        stairs[i].config = false;
        stairs[i].publish.mux = MUX_PUBLISH;
        stairs[i].publish.length = 1;
        stairs[i].publish.crc = 0;
        stairs[i].publishconfirm.mux = MUX_PUBLISH_CONFIRM;
        stairs[i].publishconfirm.length = 1;
        stairs[i].publishconfirm.crc = 0;
        stairs[i].pxconfig.mux = MUX_PX_CONFIG;
        stairs[i].pxconfig.length = 2;
        stairs[i].pxconfig.crc = 0;
        stairs[i].pxget.mux = MUX_PX_GET;
        stairs[i].pxget.data[0] = 0x55;
        stairs[i].pxget.length = 1;
        stairs[i].pxget.crc = 0;
        stairs[i].pxset.mux = MUX_PX_SET;
        stairs[i].pxset.length = 1;
        stairs[i].pxset.crc = 0;
        stairs[i].colorset.mux = MUX_COLOR_SET;
        stairs[i].colorset.length = 4; // 4 colors
        stairs[i].colorset.crc = 0;
    }

    uart_config_t uart_config = {
        .baud_rate = BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
        .source_clk = UART_SCLK_APB,
    };
    esp_log_level_set(TAG, ESP_LOG_INFO);
    ESP_LOGI(TAG, "Init Sci and configure UART for RS485!");
    uart_driver_install(uart_num, BUF_SIZE * 2, 0, 0, NULL, 0);
    // uart_driver_install(uart_num, BUF_SIZE * 2, BUF_SIZE * 2, BUF_SIZE, &uart_queue, 0);
    uart_param_config(uart_num, &uart_config);
    uart_set_pin(uart_num, UART_TXD, UART_RXD, UART_RTS, UART_PIN_NO_CHANGE);
    uart_set_mode(uart_num, UART_MODE_RS485_HALF_DUPLEX);

    uint8_t *dataSerial = (uint8_t *)malloc(BUF_SIZE / 4);
    xQueue = xQueueCreate(5, sizeof(bus_message_t));

    // Task configuration

    // xTaskCreate(AppSciHandler, "AppSciHandlerTask", UART_STACK_SIZE, NULL, 10, NULL);

    xTaskCreate(uart_read_task, "uart_read_task", UART_STACK_SIZE, NULL, 10, NULL);
    xTaskCreate(uart_write_bus_task, "uart_write_bus_task", UART_STACK_SIZE, NULL, 24, NULL);
    // xTaskCreate(uart_write_task, "uart_write_task", UART_STACK_SIZE, NULL, 10, NULL);
    // xTaskCreate(uart_write_rainbow_task, "uart_write_rainbow_task", UART_STACK_SIZE, NULL, 10, NULL);
    // xTaskCreate(uart_write_white_task, "uart_write_white_task", UART_STACK_SIZE, NULL, 10, NULL);
    //xTaskCreate(uart_write_px_get, "uart_write_px_get", UART_STACK_SIZE, NULL, 10, NULL);

    ESP_LOGI(TAG, "Init Sci Done");
}
/*--------------------------------------------------------------------------------------------------------------------*/
bool AppSciSetupStair(uint8_t stair_number)
{
    idx_setupstair = stair_number - 1;
    if(stairs[idx_setupstair].config == true)               //Publish received from 1st free strip
    {
        uint8_t *data = (uint8_t *)malloc(BUF_SIZE);
        uint8_t bytes[11];
        bytes[0] = stairs[idx_setupstair].publishconfirm.mux;
        for(int i=1; i<=9; i++)
        {
            bytes[i]=stairs[idx_setupstair].publishconfirm.mui[i-1];
        }
        bytes[10] = MUX_PUBLISH_CONFIRM_STAIR_NR(stair_number) | MUX_PUBLISH_CONFIRM_OK;

        // Write data back to the UART
        uart_write_bytes(uart_num, bytes, 11);
        print_hex("Versturen: ", bytes, 11);
        return true;
    }
    return false;
}
/*--------------------------------------------------------------------------------------------------------------------*/
void AppSciPxStartUp(void)
{
    uint8_t i = 0;

    for (i = 0; i < nbrofstairs; i++)
    {
        if (stairs[i].config)
        {
            if (stairs[i].has_px)
            {
                data[0] = stairs[i].pxconfig.mux;
                data[1] = stairs[i].pxconfig.address;
                data[2] = stairs[i].pxconfig.data[0]; // Max Treshold to detect a movement
                data[3] = stairs[i].pxconfig.data[1]; // Min Treshold to detect a movement
                stairs[i].pxconfig.crc = AppSciCalcCrc(data, 4);
                data[4] = stairs[i].pxconfig.crc;
                uart_write_bytes(uart_num, data, 5);
            }
        }
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/
void AppSciHandler(void)
{
    static APPSCISTATE state = IDLE;
    static uint8_t i = 0;

    while (1)
    {
        switch (state)
        {
        case IDLE:
            state = PX_GET;
            break;
        case PX_GET:
            if (stairs[i].config)
            {
                // AppSciPxGetSend(i);
                state = PX_ANSWER;
            }
            i++;
            break;
        case PX_ANSWER:
            state = PX_GET;
            break;
        case COLOR_SET:
            // AppSciColorSetAll();
            state = PX_GET;
            break;
        default:
            state = IDLE;
            break;
        }
        ESP_LOGI(TAG, "AppSciHandler Done");
        vTaskDelay(10000 / portTICK_RATE_MS);
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/
void AppSciColorSet(uint8_t stairsnbr, uint8_t white, uint8_t red, uint8_t grn, uint8_t blue)
{

    if (stairsnbr < nbrofstairs)
    {
        stairs[stairsnbr].colorset.data[0] = white; // White
        stairs[stairsnbr].colorset.data[1] = red;   // Red
        stairs[stairsnbr].colorset.data[2] = grn;   // Green
        stairs[stairsnbr].colorset.data[3] = blue;  // Blue

        AppSciColorSetSend(stairsnbr);
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/
void AppSciColorSetAll(void)
{
    uint8_t i = 0;
    for (i = 0; i < nbrofstairs; i++)
    {
        AppSciColorSetSend(i);
    }
}
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/

/**********************************************************************************************************************/
