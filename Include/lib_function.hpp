#ifndef  LIB_FUNCTION_HPP
#define  LIB_FUNCTION_HPP

#include <mbed.h>
#include "LTR390.h"
#include "BME680.hpp"
#include "SCD41.hpp"
#include "Memory.hpp"
#include "Data_Logger_pinout.h"
#include "timestamp.h"
#include <stdint.h>
#include <ctime>
#include <cstdio>


extern LTR390 ltr390;
extern SCD41 scd41;
extern BME680 bme680;
extern Memory memory;
extern BufferedSerial pc;


void initialization();
void getDateTime(char *date_buffer, size_t date_size, char *time_buffer, size_t time_size);
void initializeRTC();
void writeToMemory(const char *date, const char *time, const char *type, int32_t value, uint32_t &memory_address);
bool readLTR390(uint32_t &uv_data) ;
bool readBME680(int32_t &temperature, int32_t &humidity, uint32_t &gas_resistance);
bool readSCD41(uint16_t &co2, uint64_t &timestamp);

#endif      // LIB_FUNCTION_HPP