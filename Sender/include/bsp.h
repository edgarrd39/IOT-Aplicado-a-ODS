#include <Arduino.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <LoRa.h>
#include <SPI.h>

#define OLED_SDA      21
#define OLED_SCL      22
#define OLED_RESET    15
#define SCREEN_WIDTH  128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32  // OLED display height, in pixels

#define SCK  5  // GPIO5  -- SX1278's SCK
#define MISO 19 // GPIO19 -- SX1278's MISO
#define MOSI 27 // GPIO27 -- SX1278's MOSI
#define SS   18 // GPIO18 -- SX1278's CS
#define RST  14 // GPIO14 -- SX1278's RESET
#define DI0  26 // GPIO26 -- SX1278's IRQ(Interrupt Request)
#define BAND 915E6

extern Adafruit_SSD1306 display;

/**
 * @brief Funcion que inicializa la pantalla
 *
 */
void PantallaTxInit(void);

/**
 * @brief Funcion que inicializa las funciones de lora
 *
 */
void LoraInit(void);