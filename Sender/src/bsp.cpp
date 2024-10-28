#include "bsp.h"

/**
 * @brief Funcion que inicializa la pantalla
 *
 */
void PantallaTxInit(void) {

    pinMode(OLED_RESET, OUTPUT);
    digitalWrite(OLED_RESET, LOW);
    delay(20);
    digitalWrite(OLED_RESET, HIGH);

    // initialize OLED
    Wire.begin(OLED_SDA, OLED_SCL);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { // Address 0x3C for 128x32
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ;
    }
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("LORA SENDER ");
    display.display();
}

void LoraInit(void) {

    SPI.begin(SCK, MISO, MOSI, SS);
    LoRa.setPins(SS, RST, DI0);
    if (!LoRa.begin(BAND)) {
        Serial.println("Starting LoRa failed!");
        while (1)
            ;
    }
    Serial.println("LoRa init succeeded.");
    // // LoRa.onReceive(cbk);
    // //  LoRa.receive();
    // Serial.println("LoRa Initializing OK!");
    // display.setCursor(0, 10);
    // display.print("LoRa Initializing OK!");
    // display.display();
    // delay(2000);
}