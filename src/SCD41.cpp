#include "SCD41.hpp"

// Adresse I2C du capteur SCD41
#define SCD41_I2C_ADDR 0x62 << 1 // Adresse 7 bits décalée pour Mbed

// Constructeur
SCD41::SCD41(PinName sda, PinName scl) : i2c(sda, scl) {}

// Initialisation du capteur
bool SCD41::init() {
    // Démarrer le timer pour l'horodatage
    timer.start();

    // Envoyer la commande pour démarrer les mesures continues
    send_command(0x21B1);

    // Attendre un peu pour s'assurer que le capteur est prêt
    ThisThread::sleep_for(30ms);

    return true;
}

// Méthode pour envoyer une commande au capteur
void SCD41::send_command(uint16_t command) {
    char cmd[2];
    cmd[0] = (command >> 8) & 0xFF; // MSB
    cmd[1] = command & 0xFF;       // LSB
    i2c.write(SCD41_I2C_ADDR, cmd, 2);
}

// Lire la concentration de CO2 avec horodatage
bool SCD41::read_co2(uint16_t *co2, uint64_t *timestamp) {
    // Vérifier si les données sont prêtes
    if (!is_data_ready()) {
        return false; // Données non prêtes
    }

    char cmd[2] = {0xEC, 0x05}; // Commande pour lire les données
    char data[9];               // Buffer pour les données lues

    // Envoyer la commande pour lire les données
    if (i2c.write(SCD41_I2C_ADDR, cmd, 2) != 0) {
        return false; // Erreur d'écriture
    }

    // Attendre que les données soient prêtes
    ThisThread::sleep_for(500ms);

    // Lire les données
    if (i2c.read(SCD41_I2C_ADDR, data, 9) != 0) {
        return false; // Erreur de lecture
    }

    // Vérifier le CRC des données
    if (calculate_crc((uint8_t *)data, 2) != data[2]) {
        return false; // CRC invalide
    }

    // Convertir les données brutes en concentration de CO2
    *co2 = (data[0] << 8) | data[1];

    // Obtenir l'horodatage en millisecondes depuis le démarrage
    *timestamp = chrono::duration_cast<chrono::milliseconds>(timer.elapsed_time()).count();

    return true;
}

// Réinitialiser le capteur
void SCD41::reset_sensor() {
    send_command(0x3646); // Commande "reinit"
    ThisThread::sleep_for(30ms); // Attendre la réinitialisation
}

// Vérifier si les données sont prêtes
bool SCD41::is_data_ready() {
    char cmd[2] = {0xE4, 0xB8}; // Commande "data ready"
    char status[3];
    i2c.write(SCD41_I2C_ADDR, cmd, 2);
    i2c.read(SCD41_I2C_ADDR, status, 3);
    return (status[0] & 0x07FF) != 0; // Les 11 bits de poids faible indiquent l'état
}

// Calculer le CRC pour les commandes I2C
uint8_t SCD41::calculate_crc(uint8_t data[], uint8_t length) {
    uint8_t crc = 0xFF;
    for (uint8_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x31;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

// Mode de mesure unique (single shot)
bool SCD41::single_shot_measurement(uint16_t *co2, uint64_t *timestamp) {
    char cmd[2] = {0x21, 0x9D}; // Commande "single shot"
    if (i2c.write(SCD41_I2C_ADDR, cmd, 2) != 0) {
        return false; // Erreur d'écriture
    }

    // Attendre 5 secondes pour la mesure
    ThisThread::sleep_for(5000ms);

    // Lire les données
    return read_co2(co2, timestamp);
}