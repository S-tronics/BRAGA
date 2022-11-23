idf_component_register(SRCS "AppMQTTComponent.c"
                    REQUIRES "mqtt AppDeviceComponent AppParserComponent AppOTAComponent"
                    INCLUDE_DIRS "include"
                    EMBED_TXTFILES "server.pem")
