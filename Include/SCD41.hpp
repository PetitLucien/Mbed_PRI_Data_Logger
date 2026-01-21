#ifndef SCD41_HPP
#define SCD41_HPP

#include "mbed.h"

class SCD41 {
public:
    // Constructeur
    SCD41(PinName sda, PinName scl);

    // Initialisation du capteur
    bool init();

    // Lire la concentration de CO2 avec horodatage
    bool read_co2(uint16_t *co2, uint64_t *timestamp);

    // Réinitialiser le capteur
    void reset_sensor();

    // Mode de mesure unique (single shot)
    bool single_shot_measurement(uint16_t *co2, uint64_t *timestamp);

private:
    I2C i2c; // Interface I2C
    Timer timer; // Timer pour l'horodatage

    // Méthode pour envoyer une commande au capteur
    void send_command(uint16_t command);

    // Vérifier si les données sont prêtes
    bool is_data_ready();

    // Calculer le CRC pour les commandes I2C
    uint8_t calculate_crc(uint8_t data[], uint8_t length);
};

#endif // SCD41_HPP