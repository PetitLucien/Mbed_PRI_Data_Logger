#include "mbed.h"
#include "LTR390.h"
#include "BME680.hpp"
#include "SCD41.hpp"
#include "Memory.hpp"
#include "Data_Logger_pinout.h"
#include "lib_function.hpp"
#include "timestamp.h"
#include <stdint.h>
#include <ctime>
#include <cstdio>
#include <signal.h>

int main()
{
    initialization();
    uint32_t memory_address = 0; // Adresse actuelle pour écrire dans la mémoire

    while (true)
    {
        
    }
}