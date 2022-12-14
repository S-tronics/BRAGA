idf_component_register(SRCS "AppEepromComponent.c"
                    REQUIRES "DrvI2CComponent AppSciComponent StdEepromComponent"
                    INCLUDE_DIRS "include")
