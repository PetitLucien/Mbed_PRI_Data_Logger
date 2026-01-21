#ifndef LTR390_H
#define LTR390_H

#include "mbed.h"

class LTR390 {
public:
    LTR390(PinName sda, PinName scl);

    int configureResolutionAndRate(uint8_t resolution, uint8_t rate);
    int configureGain(uint8_t gain);
    int configureMode(uint8_t mode);
    int readUVData(uint32_t &data, char *raw_data);
    int verifyCommunication();

private:
    I2C i2c;
    const int LTR390_I2C_ADDR = 0x1C << 1;
};

#endif
