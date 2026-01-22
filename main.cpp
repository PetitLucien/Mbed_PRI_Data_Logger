#include "mbed.h"
#include "LTR390.h"
#include "BME680.hpp"
#include "SCD41.hpp"
#include "Memory.hpp"

// Définir les broches pour les capteurs et la mémoire
#define I2C_SDA_PIN D14
#define I2C_SCL_PIN D15
#define SPI_MOSI_PIN D11
#define SPI_MISO_PIN D12
#define SPI_SCLK_PIN D13
#define SPI_CS_PIN D10

// Initialisation des objets pour les capteurs et la mémoire
LTR390 ltr390(I2C_SDA_PIN, I2C_SCL_PIN);
BME680 bme680(I2C_SDA_PIN, I2C_SCL_PIN);
SCD41 scd41(I2C_SDA_PIN, I2C_SCL_PIN);
Memory memory(SPI_MOSI_PIN, SPI_MISO_PIN, SPI_SCLK_PIN, SPI_CS_PIN);

// Fonction pour configurer l'heure actuelle
void initializeRTC() {
    set_time(1740384000); // Timestamp Unix pour 2025-04-19 23:30:00 UTC
}

// Fonction pour récupérer la date et l'heure actuelles
void getDateTime(char *date_buffer, size_t date_size, char *time_buffer, size_t time_size) {
    time_t seconds = time(NULL);
    strftime(date_buffer, date_size, "%Y-%m-%d", localtime(&seconds)); // Format : YYYY-MM-DD
    strftime(time_buffer, time_size, "%H:%M:%S", localtime(&seconds)); // Format : HH:MM:SS
}

// Fonction pour écrire une mesure dans la mémoire
void writeToMemory(const char *date, const char *time, const char *type, int32_t value, uint32_t &memory_address) {
    char data_to_store[64];
    int len = snprintf(data_to_store, sizeof(data_to_store), "%s,%s,%s:%ld\n", date, time, type, value);

    // Vérifier si la mémoire est pleine
    if (memory_address + len > 0x100000) {
        printf("Mémoire pleine, suppression des données les plus anciennes...\n");
        memory.erase_sector(0); // Effacer le premier secteur
        memory_address = 0;    // Réinitialiser l'adresse
    }

    // Écrire les données dans la mémoire
    memory.write_data(memory_address, (uint8_t *)data_to_store, len);
    memory_address += len;
}

// Fonction pour lire les données UV du LTR390 avec 3 tentatives
bool readLTR390(uint32_t &uv_data) {
    char raw_uv[4];
    for (int attempt = 0; attempt < 3; attempt++) {
        if (ltr390.readUVData(uv_data, raw_uv) == 0) {
            return true; // Lecture réussie
        }
        ThisThread::sleep_for(500ms); // Attendre avant de réessayer
    }
    uv_data = 0; // Valeur par défaut après 3 échecs
    return false;
}

// Fonction pour lire les données du BME680 avec 3 tentatives
bool readBME680(int32_t &temperature, int32_t &humidity, uint32_t &gas_resistance) {
    for (int attempt = 0; attempt < 3; attempt++) {
        if (bme680.readData(&temperature, &humidity, &gas_resistance)) {
            return true; // Lecture réussie
        }
        ThisThread::sleep_for(500ms); // Attendre avant de réessayer
    }
    temperature = 0;
    humidity = 0;
    gas_resistance = 0;
    return false;
}

// Fonction pour lire les données du SCD41 avec 3 tentatives
bool readSCD41(uint16_t &co2, uint64_t &timestamp) {
    for (int attempt = 0; attempt < 3; attempt++) {
        if (scd41.read_co2(&co2, &timestamp)) {
            return true; // Lecture réussie
        }
        ThisThread::sleep_for(5000ms); // Attendre avant de réessayer
    }
    co2 = 0; // Valeur par défaut après 3 échecs
    return false; 
}

int main() {
    // Initialisation du port série
    BufferedSerial pc(USBTX, USBRX, 115200);
    pc.set_format(8, BufferedSerial::None, 1);
    pc.write("Initialisation des capteurs et de la mémoire...\n", 45);

    // Initialiser les capteurs
    if (ltr390.verifyCommunication() != 0) {
        pc.write("Erreur : Impossible de communiquer avec le capteur LTR390.\n", 56);
    }
    if (!bme680.init()) {
        pc.write("Erreur : Impossible de communiquer avec le capteur BME680.\n", 56);
    }
    if (!scd41.init()) {
        pc.write("Erreur : Impossible de communiquer avec le capteur SCD41.\n", 56);
    }

    // Configurer le LTR390
    if (ltr390.configureResolutionAndRate(0x03, 0x06) != 0) {
        pc.write("Erreur : Configuration de la résolution et du taux de mesure du LTR390.\n", 68);
    }
    if (ltr390.configureGain(0x0F) != 0) {
        pc.write("Erreur : Configuration du gain du LTR390.\n", 41);
    }
    if (ltr390.configureMode(0x0A) != 0) {
        pc.write("Erreur : Configuration du mode UV du LTR390.\n", 45);
    }

    // Effacer la mémoire
    pc.write("Effacement de la mémoire SPI...\n", 31);
    for (uint32_t address = 0; address < 0x100000; address += 0x1000) { // Effacer tous les secteurs
        memory.erase_sector(address);
    }
    pc.write("Mémoire effacée.\n", 18);

    // Initialiser l'horloge RTC
    initializeRTC();
    pc.write("Horloge RTC initialisée.\n", 27);

    uint32_t memory_address = 0; // Adresse actuelle pour écrire dans la mémoire

    while (true) {
        // Variables pour les données des capteurs
        uint32_t uv_data = 0;
        int32_t temperature_bme = 0, humidity_bme = 0;
        uint32_t gas_resistance_bme = 0;
        uint16_t co2 = 0;
        uint64_t timestamp_scd = 0;

        // Récupérer la date et l'heure actuelles
        char date[16], time_str[16];
        getDateTime(date, sizeof(date), time_str, sizeof(time_str));

        // Obtenir l'heure actuelle
        time_t now = time(NULL);
        struct tm *current_time = localtime(&now);
        
        //printf("Heure actuelle : %02d:%02d:%02d\n", current_time->tm_hour, current_time->tm_min, current_time->tm_sec);
        //printf("Date et heure actuelles : %s %s\n", date, time_str);

        // Heures pour la lecture des données 
        // Mesure de l'humidité + UV --- 8h 12h 17h 
        if ((current_time->tm_hour == 8 || current_time->tm_hour == 12 || current_time->tm_hour == 17) && current_time->tm_min == 0 && current_time->tm_sec == 0) {
            if (readLTR390(uv_data)) {
                writeToMemory(date, time_str, "UV", uv_data, memory_address);
                printf("%s,%s,UV:%lu\n", date, time_str, uv_data);
            } else {
                pc.write("Erreur : Impossible de lire les données UV après 3 tentatives.\n", 61);
            }
            if (readBME680(temperature_bme, humidity_bme, gas_resistance_bme)) {
                writeToMemory(date, time_str, "H", humidity_bme, memory_address);
                printf("%s,%s,H:%lu\n", date, time_str, humidity_bme);
            } else {
                pc.write("Erreur : Impossible de lire l'humidité du BME680 après 3 tentatives.\n", 65);
            }
        }

        // Mesure de la température toutes les heures de 8h-17h & toutes les 3h de 17h_8h
        if ((((current_time->tm_hour >= 8 && current_time->tm_hour <= 17) && current_time->tm_min == 0 && current_time->tm_sec == 0) || ((current_time->tm_hour == 20 || current_time->tm_hour == 23 || current_time->tm_hour == 2 || current_time->tm_hour == 5) && current_time->tm_min == 0 && current_time->tm_sec == 0))) {
            if (readBME680(temperature_bme, humidity_bme, gas_resistance_bme)) {
                writeToMemory(date, time_str, "T", temperature_bme, memory_address);
                printf("%s,%s,T:%lu\n", date, time_str, temperature_bme);
            } else {
                pc.write("Erreur : Impossible de lire la température du BME680 après 3 tentatives.\n", 65);
            }
        }

        // Mesure de du CO2 + COV toutes les minutes en 8h-17h
        if (current_time->tm_hour >= 8 && current_time->tm_hour <= 17 && current_time->tm_sec == 0) {
            if (readBME680(temperature_bme, humidity_bme, gas_resistance_bme)) {
                writeToMemory(date, time_str, "G", gas_resistance_bme, memory_address);
                printf("%s,%s,COV:%lu\n", date, time_str, gas_resistance_bme);
            } else {
                pc.write("Erreur : Impossible de lire les données du BME680 après 3 tentatives.\n", 65);
            }
            scd41.init(); // Nécessaire pour le bon fonctionnement du capteur NE pas enlever
            if (readSCD41(co2, timestamp_scd)) {
                writeToMemory(date, time_str, "CO2", co2, memory_address);
                printf("%s,%s,CO2:%u\n", date, time_str, co2);
            } else {
                pc.write("Erreur : Impossible de lire les données du CO2 après 3 tentatives.\n", 64);
            }
        }
        // Attendre une minute avant la prochaine lecture
        ThisThread::sleep_for(1000ms);
    }
}