/**********************************************************************************************************************/
/**
 * @file        AppEepromComponent.h
 *
 * @author      Stijn Vermeersch
 * @date        12.12.2022
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
#ifndef APPEEPROMCOMPONENT_H
#define APPEEPROMCOMPONENT_H
/**********************************************************************************************************************/



/***********************************************************************************************************************
; I N C L U D E S
;---------------------------------------------------------------------------------------------------------------------*/
#include "AppSciComponent.h"
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
    SSID = 0,
    PASSW = 1,
    STAIRS = 2,
}
CONFIG_DONE;
/**********************************************************************************************************************/



/***********************************************************************************************************************
; E X P O R T E D   V A R I A B L E S
;---------------------------------------------------------------------------------------------------------------------*/
/**********************************************************************************************************************/



/***********************************************************************************************************************
; E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
;---------------------------------------------------------------------------------------------------------------------*/
/**
 * @brief   Initialise function for this module
 *
 *
 */
void appeeprom_init(void);

/**
 * @brief   Initialise function for this module
 *
 *
 */
void appeeprom_write_wifiSSID(char* ssid);

/**
 * @brief   Initialise function for this module
 *
 *
 */
void appeeprom_write_wifipassword(char* passwrd);

/**
 * @brief   Initialise function for this module
 *
 *
 */
void appeeprom_write_stairstaken(uint32_t stairs_taken);

/**
 * @brief   Initialise function for this module
 *
 *
 */
void appeeprom_write_configdone(CONFIG_DONE config);

/**
 * @brief   Initialise function for this module
 *
 *
 */
void appeeprom_write_nbrofstairs(uint8_t stairs);

/**
 * @brief   Initialise function for this module
 *
 *
 */
void appeeprom_increment_overcurrent(void);

/**
 * @brief   Initialise function for this module
 *
 *
 */
void appeeprom_increment_undercurrent(void);

/**
 * @brief   Initialise function for this module
 *
 *
 */
void appeeprom_increment_overvoltage(void);

/**
 * @brief   Initialise function for this module
 *
 *
 */
void appeeprom_increment_undervoltage(void);

/**
 * @brief   Initialise function for this module
 *
 *
 */
void appeeprom_write_stairsconfig(APPSCISTAIR* stairs);

/**
 * @brief   Initialise function for this module
 *
 *
 */
void appeeprom_read_stairsconfig(APPSCISTAIR* stairs);


/**********************************************************************************************************************/



/***********************************************************************************************************************
; E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
;---------------------------------------------------------------------------------------------------------------------*/
/**********************************************************************************************************************/



#endif /* APPEEPROMCOMPONENT_H */
