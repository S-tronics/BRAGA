/**********************************************************************************************************************/
/**
 * @file        AppParserComponent.h
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
#ifndef AppParserComponent_H
#define AppParserComponent_H
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

/**********************************************************************************************************************/

/***********************************************************************************************************************
; E X P O R T E D   V A R I A B L E S
;---------------------------------------------------------------------------------------------------------------------*/
/**********************************************************************************************************************/

/***********************************************************************************************************************
; E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
;---------------------------------------------------------------------------------------------------------------------*/

/**
 * @brief Parser that parses the JSON string received from MQTT CONFIG topic
 *
 * @param json_string JSON string to parse
 */
void AppParser_parse_config(char *json_string);

/**
 * @brief Parser that parses the JSON string received from MQTT SETUP topic
 *
 * @param json_string JSON string to parse
 */
void AppParser_parse_setup(char *json_string);

/**
 * @brief Parser that parses the JSON string received from MQTT CONTROL topic
 *
 * @param json_string JSON string to parse
 */
void AppParser_parse_control(char *json_string);

/**
 * @brief Parser that parses the JSON string received from MQTT OTA topic
 *
 * @param json_string JSON string to parse
 */
void AppParser_parse_ota(char *json_string);
/**********************************************************************************************************************/

/***********************************************************************************************************************
; E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
;---------------------------------------------------------------------------------------------------------------------*/
/**********************************************************************************************************************/

#endif /* AppParserComponent_H */