#ifndef DATA_LOGGER_PINOUT_H
#define DATA_LOGGER_PINOUT_H

#define MIN_VALID_TIMESTAMP 1577836800  // 2020-01-01 00:00:00 UTC

// Définir les broches pour les capteurs et la mémoire
#define I2C_SDA_PIN D14
#define I2C_SCL_PIN D15
#define SPI_MOSI_PIN D11
#define SPI_MISO_PIN D12
#define SPI_SCLK_PIN D13
#define SPI_CS_PIN D10

// définir les broches pour les I/O supplémentaires
#define LED_RED_PIN A13
#define LED_GREEN_PIN A14
#define PGOOD_PIN C6
#define CHARGING_STATUS_PIN C8
#define ENABLE_SCD41_PIN A3

#endif // DATA_LOGGER_PINOUT_H