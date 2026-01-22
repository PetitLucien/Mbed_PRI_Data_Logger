#include "lib_function.hpp"
LTR390 ltr390(I2C_SDA_PIN, I2C_SCL_PIN);
SCD41 scd41(I2C_SDA_PIN, I2C_SCL_PIN);
BME680 bme680(I2C_SDA_PIN, I2C_SCL_PIN);
Memory memory(SPI_MOSI_PIN, SPI_MISO_PIN, SPI_SCLK_PIN, SPI_CS_PIN);
BufferedSerial pc(USBTX, USBRX, 115200);

void initialization(){
     // Initialisation du port série
    
    pc.set_format(8, BufferedSerial::None, 1);
    printf("Initialisation des capteurs et de la mémoire...\n");

    // Initialiser l'horloge RTC
    initializeRTC();
    printf("Horloge RTC initialisée.\n");
    
    // Initialiser les capteurs
    if (!ltr390.init()) {
        printf("Erreur : Initialisation du capteur LTR390.\n");
    }
    if (!bme680.init()) {
        printf("Erreur : Impossible de communiquer avec le capteur BME680.\n");
    }
    if (!scd41.init()) {
        printf("Erreur : Impossible de communiquer avec le capteur SCD41.\n");
    }



    // Effacer la mémoire
    printf("Effacement de la mémoire SPI...\n");
    for (uint32_t address = 0; address < 0x100000; address += 0x1000) { // Effacer tous les secteurs
        memory.erase_sector(address);
    }
    printf("Mémoire effacée.\n");

   

}
    

// Fonction pour récupérer la date et l'heure actuelles
void getDateTime(char *date_buffer, size_t date_size, char *time_buffer, size_t time_size) {
    time_t seconds = time(NULL);
    strftime(date_buffer, date_size, "%Y-%m-%d", localtime(&seconds)); // Format : YYYY-MM-DD
    strftime(time_buffer, time_size, "%H:%M:%S", localtime(&seconds)); // Format : HH:MM:SS
}


void initializeRTC()
{

    set_time(TIMESTAMP);
    printf("RTC initialisée avec succès.\n");
    // Récupérer la date et l'heure actuelles
    char date[16], time_str[16];
    getDateTime(date, sizeof(date), time_str, sizeof(time_str));

    // affichage de la date et de l'heure
    printf("Date et heure actuelles : %s %s\n", date, time_str);

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