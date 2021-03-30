set(COMPONENT_SRCS "main.c" 
                   "sources/sensor.c" 
                   "sources/wifi.c"
                   "sources/server.c" 
                   "sources/humidifier.c" 
                   "sources/nvs.c"
                   "libs/dht/src/DHT_c.c")
set(COMPONENT_ADD_INCLUDEDIRS "." "includes/" "libs/dht/include/")

register_component()

spiffs_create_partition_image(spiffs ../html FLASH_IN_PROJECT)