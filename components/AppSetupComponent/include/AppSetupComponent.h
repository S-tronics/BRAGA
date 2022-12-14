/**********************************************************************************************************************/
/**
 * @file        AppSetupComponent.h
 *
 * @author      Stijn Vermeersch
 * @date        06.07.2022
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
#ifndef AppSetupComponent_H
#define AppSetupComponent_H
/**********************************************************************************************************************/

/***********************************************************************************************************************
; I N C L U D E S
;---------------------------------------------------------------------------------------------------------------------*/

/**********************************************************************************************************************/

/***********************************************************************************************************************
; E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
;---------------------------------------------------------------------------------------------------------------------*/
/**********************************************************************************************************************/

/***********************************************************************************************************************
; E X P O R T E D   T Y P E D E F S
;---------------------------------------------------------------------------------------------------------------------*/
typedef enum PROCEDURES
{
    START_INSTALLATION = 1,
    LED_CONNECTED = 2,
    MUI_RECEIVED = 3,
    STOP_INSTALLATION = 4,
    ERROR = 5
} PROCEDURE;
/**********************************************************************************************************************/

/***********************************************************************************************************************
; E X P O R T E D   V A R I A B L E S
;---------------------------------------------------------------------------------------------------------------------*/
/**********************************************************************************************************************/

/***********************************************************************************************************************
; E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
;---------------------------------------------------------------------------------------------------------------------*/
void AppSetup_init(void);
/**
 * @brief   Function to reset isntallation procedure
 *
 * @param   None
 */
void AppSetup_reset_installation();

/**
 * @brief   Function to send a JSON setup message to the MQTT broker
 *          This could be a start, stop or led connected message
 *
 * @param   stair_number    The stair number that will be sent in JSON
 * @param   p               The procedure that will be sent in JSON, see PROCEDURE enum
 * @param   value           Some value to send with the message
 */
void AppSetup_setup_message(int stair_number, PROCEDURE p, uint8_t value);

/**
 * @brief   Not implemented
 *
 * @param   stair_number    The stair number that will be sent in JSON
 * @param   p               The procedure that will be sent in JSON, see PROCEDURE enum
 */
void AppSetup_execute(int stair_number, PROCEDURE p);            //Command from Client
/**********************************************************************************************************************/

/***********************************************************************************************************************
; E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
;---------------------------------------------------------------------------------------------------------------------*/
/**********************************************************************************************************************/

#endif /* AppDiagnosticsComponent_H */