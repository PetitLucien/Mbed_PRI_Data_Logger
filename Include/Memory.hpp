#ifndef MEMORY_HPP
#define MEMORY_HPP

#include "mbed.h"

class Memory {
public:
    // Constructeur
    Memory(PinName mosi, PinName miso, PinName sclk, PinName cs);

    // Méthodes publiques
    void erase_sector(uint32_t address); // Effacer un secteur
    void write_data(uint32_t address, const uint8_t *data, size_t length); // Écrire des données
    void read_data(uint32_t address, uint8_t *data, size_t length); // Lire des données

private:
    // Membres privés
    SPI spi;         // Interface SPI
    DigitalOut cs;   // Chip Select (CS)

    // Méthodes privées
    void send_command(uint8_t command); // Envoyer une commande SPI
    uint8_t read_status();              // Lire le statut de la mémoire
    void wait_until_ready();            // Attendre que la mémoire soit prête
};

#endif // MEMORY_HPP