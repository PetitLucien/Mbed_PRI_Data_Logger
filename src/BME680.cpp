#include "BME680.hpp"

// Adresse I2C du capteur
#define BME680_I2C_ADDR BME68X_I2C_ADDR_HIGH // Adresse 0x77

// Constructeur
BME680::BME680(PinName sda, PinName scl) : i2c(sda, scl) {
    dev.intf = BME68X_I2C_INTF;
    dev.read = [](uint8_t reg, uint8_t *data, uint32_t len, void *intf) -> int8_t {
        I2C *i2c = static_cast<I2C *>(intf);
        char reg_addr = reg;
        if (i2c->write(BME680_I2C_ADDR << 1, &reg_addr, 1, true) != 0) return BME68X_E_COM_FAIL;
        if (i2c->read(BME680_I2C_ADDR << 1, (char *)data, len) != 0) return BME68X_E_COM_FAIL;
        return BME68X_OK;
    };
    dev.write = [](uint8_t reg, const uint8_t *data, uint32_t len, void *intf) -> int8_t {
        I2C *i2c = static_cast<I2C *>(intf);
        char buffer[len + 1];
        buffer[0] = reg;
        memcpy(&buffer[1], data, len);
        return i2c->write(BME680_I2C_ADDR << 1, buffer, len + 1) == 0 ? BME68X_OK : BME68X_E_COM_FAIL;
    };
    dev.delay_us = [](uint32_t period, void *intf) {
        wait_us(period);
    };
    dev.intf_ptr = &i2c;
    dev.amb_temp = 25; // Température ambiante par défaut
}

// Initialisation du capteur
bool BME680::init() {
    int8_t result = bme68x_init(&dev);
    if (result != BME68X_OK) {
        printf("Erreur : Impossible d'initialiser le capteur BME680 (code : %d)\n", result);
        return false;
    }

    // Configuration du capteur
    conf.os_hum = BME68X_OS_2X; // Suréchantillonnage de l'humidité
    conf.os_temp = BME68X_OS_4X; // Suréchantillonnage de la température
    conf.os_pres = BME68X_OS_NONE; // Pas de pression
    conf.filter = BME68X_FILTER_OFF; // Pas de filtrage
    conf.odr = BME68X_ODR_NONE; // Pas de fréquence de mesure
    result = bme68x_set_conf(&conf, &dev);
    if (result != BME68X_OK) {
        printf("Erreur : Impossible de configurer le capteur BME680 (code : %d)\n", result);
        return false;
    }

    // Configuration du chauffage pour les gaz
    heatr_conf.enable = BME68X_ENABLE;
    heatr_conf.heatr_temp = 320; // Température de chauffage (°C)
    heatr_conf.heatr_dur = 150; // Durée de chauffage (ms)
    result = bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr_conf, &dev);
    if (result != BME68X_OK) {
        printf("Erreur : Impossible de configurer le chauffage du capteur BME680 (code : %d)\n", result);
        return false;
    }

    return true;
}

// Lire les données de température, d'humidité et de gaz
bool BME680::readData(int32_t *temperature, int32_t *humidity, uint32_t *gas_resistance) {
    // Définir le mode forcé
    int8_t result = bme68x_set_op_mode(BME68X_FORCED_MODE, &dev);
    if (result != BME68X_OK) {
        printf("Erreur : Impossible de définir le mode forcé (code : %d)\n", result);
        return false;
    }

    // Attendre la fin de la mesure
    dev.delay_us(bme68x_get_meas_dur(BME68X_FORCED_MODE, &conf, &dev) + heatr_conf.heatr_dur, dev.intf_ptr);

    // Lire les données
    struct bme68x_data data;
    uint8_t n_fields;
    result = bme68x_get_data(BME68X_FORCED_MODE, &data, &n_fields, &dev);
    if (result != BME68X_OK) {
        printf("Erreur : Impossible de lire les données du capteur (code : %d)\n", result);
        return false;
    }

    // Convertir les données en entiers
    *temperature = (int32_t)(data.temperature * 100); // Température en °C * 100
    *humidity = (int32_t)(data.humidity * 1000);      // Humidité en % * 1000
    *gas_resistance = data.gas_resistance;           // Résistance des gaz en Ohms

    return true;
}