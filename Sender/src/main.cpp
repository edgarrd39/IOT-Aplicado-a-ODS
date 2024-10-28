/* * Copyright 2023, Edgardo Rodrigo Díaz <rodrigo.09tuc@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/** \brief Brief description of the file
 **
 ** Full file description
 **
 ** \addtogroup name Module denomination
 ** \brief Brief description of the module
 ** @{ */

/* === Headers files inclusions ================================================================ */

#include "bsp.h"
#include "sensores.h"
#include <AHT10.h>
#include <Adafruit_Sensor.h>
#include <MQUnifiedsensor.h>
#include <Wire.h>

/* === Macros definitions ====================================================================== */

#define Board              ("ESP-32")
#define Pin                (34)
#define Type               ("MQ-2") // MQ2
#define Voltage_Resolution (3.3)
#define ADC_Bit_Resolution (12)
#define RatioMQ2CleanAir   (9.83)

#define PIN_PIR 2

/* === Declaration of public data types ======================================================== */

/* === Declaration and initialization of public global variables =============================== */
String outgoing; // outgoing message
String packet = "";
String values = "";

byte msgCount = 0;        // count of outgoing messages
byte localAddress = 0xFF; // address of this device
byte destination = 0xBB;  // destination to send to
long lastSendTime = 0;    // last send time
int interval = 2000;      // interval between sends

const int PIN_LED = 4;
float CO_ppm = 0;
float RS = 0;
float Ratio = 0;
float R0 = 0;

String LoRaData;
/* === Declaration and initialization of public global objects ================================= */

MQUnifiedsensor MQ2(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin, Type);

AHT10 myAHT10(0x38);

/* === Declarations (prototypes) of public functions =========================================== */

void sendMessage(String message);

void SendPacket(String packet);

void onReceive(int packetSize);

/* === Main function, the program entry point after power on or reset ========================== */

void setup() {
    Serial.begin(115200); // initialize serial
    while (!Serial)
        ;

    // Serial.println("LoRa Duplex");
    LoraInit();

    MqGasInit(MQ2, 1, 36974, -3.109, 1);

    pinMode(PIN_LED, OUTPUT);
    Wire.begin();
    if (!myAHT10.begin()) { // If communication with the sensor fails, an error message is printed
        Serial.println("Sensor error!");
        while (1)
            ;
    }
    pinMode(PIN_PIR, INPUT);
    MqGasCalibrate(MQ2);
}

/* === Loop function =========================================================================== */
void loop() {

    onReceive(LoRa.parsePacket());
}

/* === Implementations of public functions ===================================================== */

void sendMessage(String outgoing) {
    LoRa.beginPacket();            // start packet
    LoRa.write(destination);       // add destination address
    LoRa.write(localAddress);      // add sender address
    LoRa.write(msgCount);          // add message ID
    LoRa.write(outgoing.length()); // add payload length
    LoRa.print(outgoing);          // add payload
    LoRa.endPacket();              // finish packet and send it
    msgCount++;                    // increment message ID
}

void onReceive(int packetSize) {
    if (packetSize) {
        while (LoRa.available()) {
            LoRaData = LoRa.readString();
            Serial.println(LoRaData);
        }

        if (LoRaData == "ON") {
            int mediciones = 0;
            while (mediciones < 250) {
                int estadoPresencia = digitalRead(PIN_PIR);

                // Leemos la humedad relativa
                float h = myAHT10.readHumidity();
                // Leemos la temperatura en grados centígrados (por defecto)
                float t = myAHT10.readTemperature();

                MQ2.update();              // Update data, the arduino will read the voltage from the analog pin
                CO_ppm = MQ2.readSensor(); // Sensor will read PPM concentration using the model, a and b values
                                           // set previously or from the setup

                values = String(t) + "," + String(h) + "," + String(CO_ppm);
                packet = values;
                SendPacket(packet);
                delay(5000);
                mediciones++;
            }
        }
    }
}
/**
 * @brief Funcion que envia el paquete por lora
 *
 * @param packet Cadena de caracteres del formato "valor1,valor2,...,valorn"
 */
void SendPacket(String packet) {
    LoRa.beginPacket();

    LoRa.print(packet);

    LoRa.endPacket();
}