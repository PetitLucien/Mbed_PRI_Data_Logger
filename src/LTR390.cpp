#include "LTR390.h"

// Constructeur de la classe LTR390
LTR390::LTR390(PinName sda, PinName scl) : i2c(sda, scl) {
    // Configuration initiale si necessaire

boool LTR390::init() {
    // Verifier la communication avec le capteur
    if(this->verifyCommunication() != 0) {
        return false; // Echec de la communication
    }
    else
    {
        if (this->configureResolutionAndRate(0x03, 0x06) != 0) {
            printf("Erreur : Configuration de la résolution et du taux de mesure du LTR390.\n");
            return false;
        }
        if (this->configureGain(0x0F) != 0) {
            printf("Erreur : Configuration du gain du LTR390.\n");
            return false;
        }
        if (this->configureMode(0x0A) != 0) {
            printf("Erreur : Configuration du mode UV du LTR390.\n");
            return false;
        }

    }
    return true; // Succes de l'initialisation
    
}


// Configurer la resolution et le taux de mesure
int LTR390::configureResolutionAndRate(uint8_t resolution, uint8_t rate) {
    uint8_t config = (resolution << 3) | (rate & 0x07);
    char command[3] = {0x12, config, 0x00};  // Ajout du troisieme octet comme dans Test.cpp
    int status = i2c.write(LTR390_I2C_ADDR, command, 3);
    printf("configureResolutionAndRate() appele avec resolution: %X, rate: %X\n", resolution, rate);
    if (status != 0) {
        return 1; // Erreur d'ecriture en bus I2C
    }
    return 0; // Succes
}

// Configurer le gain
int LTR390::configureGain(uint8_t gain) {
    uint8_t config = gain & 0x07;
    char command[3] = {0x0B, config, 0x00};  // Ajout du troisieme octet comme dans Test.cpp
    int status = i2c.write(LTR390_I2C_ADDR, command, 3);
    printf("configureGain() appele avec gain: %X\n", gain);
    if (status != 0) {
        return 1; // Erreur d'ecriture en bus I2C
    }
    return 0; // Succes
}

// Configurer le mode (UV ou intensite lumineuse)
int LTR390::configureMode(uint8_t mode) {
    char command[3] = {0x13, mode, 0x00};  // Ajout du troisieme octet comme dans Test.cpp
    int status = i2c.write(LTR390_I2C_ADDR, command, 3);
    printf("configureMode() appele avec mode: %X\n", mode);
    if (status != 0) {
        return 1; // Erreur d'ecriture en bus I2C
    }
    return 0; // Succes
}

// Lire les donnees UV du capteur
int LTR390::readUVData(uint32_t &data, char *raw_data) {
    char buffer[4] = {0};  // 4 octets de donnees
    char reg[1] = {0x09};  // Adresse du registre pour lire les donnees UV (0x09)

    int status = i2c.write(LTR390_I2C_ADDR, reg, 1);
    if (status != 0) {
        return 1; // Erreur d'ecriture en bus I2C
    }

    ThisThread::sleep_for(10ms);  // Delai avant de lire les donnees

    status = i2c.read(LTR390_I2C_ADDR, buffer, 4);  // Lire 4 octets de donnees
    if (status != 0) {
        return 2; // Erreur de lecture du bus I2C
    }

    for (int i = 0; i < 4; ++i) {
        raw_data[i] = buffer[i];
    }

    data = ((uint32_t)buffer[3] << 24) | ((uint32_t)buffer[2] << 16) | ((uint32_t)buffer[1] << 8) | buffer[0];
    return 0; // Succes
}

// Verifier la communication avec le capteur en lisant le registre PART_ID
int LTR390::verifyCommunication() {
    char cmd[1] = {0x06};  // Commande pour acceder au registre PART_ID
    char part_id[1];       // Pour stocker la valeur lue depuis le registre PART_ID

    // Ecrire la commande au capteur
    int status = i2c.write(LTR390_I2C_ADDR, cmd, 1);
    if (status != 0) {
        return 1; // Erreur d'ecriture en bus I2C
    }

    // Lire la valeur du registre PART_ID du capteur
    status = i2c.read(LTR390_I2C_ADDR, part_id, 1);
    if (status != 0) {
        return 2; // Erreur de lecture du bus I2C
    }

    // Afficher la valeur lue du registre PART_ID
    printf("Part ID lu depuis le capteur: 0x%X\n", part_id[0]);

    // Verifier si la communication a reussi
    if (part_id[0] == 0xB2) {
        printf("Communication avec le capteur LTR-390UV reussie.\n");
        return 0; // Succes
    } else {
        printf("Echec de la communication avec le capteur LTR-390UV.\n");
        return 3; // Erreur de verification PART_ID
    }
}
