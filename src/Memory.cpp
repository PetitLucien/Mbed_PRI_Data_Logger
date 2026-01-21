#include "Memory.hpp"

// Commandes spécifiques à la mémoire W25Q64JV
#define WRITE_ENABLE  0x06
#define WRITE_DISABLE 0x04
#define READ_DATA     0x03
#define PAGE_PROGRAM  0x02
#define SECTOR_ERASE  0x20
#define READ_STATUS   0x05

// Constructeur
Memory::Memory(PinName mosi, PinName miso, PinName sclk, PinName cs_pin)
    : spi(mosi, miso, sclk), cs(cs_pin) {
    spi.format(8, 0); // 8 bits par transfert, mode SPI 0
    spi.frequency(1000000); // Fréquence SPI : 1 MHz
    cs = 1; // Désactiver CS par défaut
}

// Méthode pour envoyer une commande SPI
void Memory::send_command(uint8_t command) {
    cs = 0; // Activer CS
    spi.write(command);
    cs = 1; // Désactiver CS
}

// Méthode pour lire le statut de la mémoire
uint8_t Memory::read_status() {
    cs = 0; // Activer CS
    spi.write(READ_STATUS);
    uint8_t status = spi.write(0x00); // Lire le statut
    cs = 1; // Désactiver CS
    return status;
}

// Méthode pour attendre que la mémoire soit prête
void Memory::wait_until_ready() {
    while (read_status() & 0x01) { // Vérifier le bit "busy"
        ThisThread::sleep_for(1ms);
    }
}

// Méthode pour effacer un secteur
void Memory::erase_sector(uint32_t address) {
    send_command(WRITE_ENABLE); // Activer l'écriture
    cs = 0; // Activer CS
    spi.write(SECTOR_ERASE); // Commande pour effacer un secteur
    spi.write((address >> 16) & 0xFF); // Adresse (MSB)
    spi.write((address >> 8) & 0xFF);  // Adresse (Middle Byte)
    spi.write(address & 0xFF);         // Adresse (LSB)
    cs = 1; // Désactiver CS
    wait_until_ready(); // Attendre que l'effacement soit terminé
}

// Méthode pour écrire des données
void Memory::write_data(uint32_t address, const uint8_t *data, size_t length) {
    send_command(WRITE_ENABLE); // Activer l'écriture
    cs = 0; // Activer CS
    spi.write(PAGE_PROGRAM); // Commande pour écrire une page
    spi.write((address >> 16) & 0xFF); // Adresse (MSB)
    spi.write((address >> 8) & 0xFF);  // Adresse (Middle Byte)
    spi.write(address & 0xFF);         // Adresse (LSB)
    for (size_t i = 0; i < length; i++) {
        spi.write(data[i]); // Écrire les données
    }
    cs = 1; // Désactiver CS
    wait_until_ready(); // Attendre que l'écriture soit terminée
}

// Méthode pour lire des données
void Memory::read_data(uint32_t address, uint8_t *data, size_t length) {
    cs = 0; // Activer CS
    spi.write(READ_DATA); // Commande pour lire des données
    spi.write((address >> 16) & 0xFF); // Adresse (MSB)
    spi.write((address >> 8) & 0xFF);  // Adresse (Middle Byte)
    spi.write(address & 0xFF);         // Adresse (LSB)
    for (size_t i = 0; i < length; i++) {
        data[i] = spi.write(0x00); // Lire les données
    }
    cs = 1; // Désactiver CS
}