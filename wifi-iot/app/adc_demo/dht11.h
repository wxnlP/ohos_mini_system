#ifndef DHT11_H
#define DHT11_H

#include <stdio.h>
#include "ohos_init.h"
#include <stdint.h>
#include "iot_gpio.h"
#include "hi_io.h"
#include "hi_time.h"

uint8_t Dht11ReadData(unsigned int dht_data_pin, uint8_t* humidity_int, uint8_t* humidity_dec, 
    uint8_t* temperature_int, uint8_t* temperature_dec);

#endif  // DHT11_H