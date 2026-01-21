#ifndef BME680_HPP
#define BME680_HPP

#include "mbed.h"
#include "BME680/bme68x.h" // Inclure la bibliothèque BME680

class BME680 {
public:
    // Constructeur
    BME680(PinName sda, PinName scl);

    // Initialisation du capteur
    bool init();

    // Lire les données de température, d'humidité et de gaz
    bool readData(int32_t *temperature, int32_t *humidity, uint32_t *gas_resistance);

private:
    I2C i2c; // Interface I2C
    struct bme68x_dev dev; // Structure du capteur
    struct bme68x_conf conf; // Configuration du capteur
    struct bme68x_heatr_conf heatr_conf; // Configuration du chauffage
};

#endif // BME680_HPP