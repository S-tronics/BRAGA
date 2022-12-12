/**********************************************************************************************************************/
/**
 * @file        AppDeviceComponent.h
 *
 * @author      Stijn Vermeersch
 * @date        05.07.2022
 *
 * @brief
 *
 *
 *
 * \n<hr>
 * Copyright (c) 2022, S-tronics BV\n
 * All rights reserved.
 * \n<hr>\n
 */
/**********************************************************************************************************************/
#ifndef AppDeviceComponent_H
#define AppDeviceComponent_H
/**********************************************************************************************************************/

/***********************************************************************************************************************
; I N C L U D E S
;---------------------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>

#include <stdbool.h>
/**********************************************************************************************************************/

/***********************************************************************************************************************
; E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
;---------------------------------------------------------------------------------------------------------------------*/
/**********************************************************************************************************************/

/***********************************************************************************************************************
; E X P O R T E D   T Y P E D E F S
;---------------------------------------------------------------------------------------------------------------------*/
typedef enum
{
    RED = 0,
    GREEN = 1,
    BLUE = 2
}
LED_COLOR;
/**********************************************************************************************************************/

/***********************************************************************************************************************
; E X P O R T E D   V A R I A B L E S
;---------------------------------------------------------------------------------------------------------------------*/
/**********************************************************************************************************************/

/***********************************************************************************************************************
; E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
;---------------------------------------------------------------------------------------------------------------------*/
/**
 * @brief Get the mac address of the device, ESP32
 *
 */
void AppDeviceInit(void);

/**
 * @brief Get the mac address of the device, ESP32
 *
 * @param unique string to store the mac address in
 */
void AppDevice_get_unique(char *unique);

/**
 * @brief Helper function to compare a version string with the current version of the software
 *        A version string is in the form of "major.minor.patch"
 *
 * @param version Version string that is to be compared with the current version.
 * @return int: -2 if something went wrong comparing
 *              -1 if version is lower than current version
 *              0 if version is equal to current version
 *              1 if version is higher than current version
 */
int AppDevice_compare_software_version_device(const char *version);

/**
 * @brief Helper function to compare a version string with the current version of the hardware
 *        A version string is in the form of "major.minor.patch"
 *
 * @param version Version string that is to be compared with the current version.
 * @return int: -2 if something went wrong comparing
 *              -1 if version is lower than current version
 *              0 if version is equal to current version
 *              1 if version is higher than current version
 */
int AppDevice_compare_hardware_version_device(const char *version);



/**
 * @brief Set the color of the diagnostic led of de leddriver board
 *
 * @param color   color defined by enum RED, GREEN, BLUE
 */
void AppDevice_SetLed(LED_COLOR color);

/**
 * @brief Control the voltage of the RS485-bus
 *
 * @param onoff   true: voltage enabled | false: voltage disabled
 */
void AppDevice_CtrlPeripheral(bool onoff);

/**
 * @brief Measured the current towards the RS485-bus
 *
 * @return  the value of the measured current towards the RS485-bus
 */
uint16_t AppDevice_GetCurrentValue(void);

/**
 * @brief Measured the voltage on the RS485-bus
 *
 * @return  the value of the measured voltage on the RS485-bus
 */
uint16_t AppDevice_GetVoltageValue(void);
/**********************************************************************************************************************/

/***********************************************************************************************************************
; E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
;---------------------------------------------------------------------------------------------------------------------*/
/**********************************************************************************************************************/

#endif /* AppDeviceComponent_H */