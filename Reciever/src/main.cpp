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

#include "Boton.h"
#include "bsp.h"
#include "wifiid.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <LoRa.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <WiFi.h>

/* === Macros definitions ====================================================================== */

#define BOTON           2
#define LED_TEMPERATURA 4
#define LED_PPM         5

#define MAX_TEMPERATURA 35
#define MAX_PPM         100

/* === Declaration of public data types ======================================================== */

/* === Declaration and initialization of public global variables =============================== */

byte msgCount = 0;        // count of outgoing messages
byte localAddress = 0xBB; // address of this device
byte destination = 0xFF;  // destination to send to
long lastSendTime = 0;    // last send time
int interval = 2000;      // interval between sends

int contador = 0;
int estado_boton = 0;
unsigned long tiemporebote = 100;

const int numSensores = 4;
float sensorValues[numSensores];

const char * ssid = SSID_WIFI;
const char * password = PASSWORD_WIFI;
const char * mqttServer = MQTT_SERVER;
const int mqttPort = PORT;
const char * mqttUser = MQTT_USER;
const char * mqttPassword = MQTT_PASS;

float temperatura = 0;
float humedad = 0;
float co_ppm = 0;

String outgoing; // outgoing message
/* === Declaration and initialization of public global objects ================================= */

BotonSimple boton(BOTON, tiemporebote);

WiFiClient espClient;
PubSubClient client(espClient);

/* === Declarations (prototypes) of public functions =========================================== */

void sendMessage(String message);

void onReceive(int packetSize);

void parseData(String data);

void setupWiFi();

void reconnectMQTT();

void publishSensorData(const char * baseTopic, const char * dataName, float dataValue);
/* === Main function, the program entry point after power on or reset ========================== */

void setup() {
    Serial.begin(115200); // initialize serial
    while (!Serial)
        ;
    LoraInit();

    setupWiFi();

    // pinMode(LED_TEMPERATURA, OUTPUT);
    // pinMode(LED_PPM, OUTPUT);

    client.setServer(mqttServer, mqttPort);
    reconnectMQTT();
}

/* === Loop function =========================================================================== */

void loop() {

    if (!client.connected()) {
        reconnectMQTT();
    }
    client.loop();

    boton.actualizar();
    estado_boton = boton.leer();
    if (estado_boton == APRETANDOLO) {
        LoRa.beginPacket();
        LoRa.print("ON");
        LoRa.endPacket();
    }
    onReceive(LoRa.parsePacket());

    // if (temperatura > MAX_TEMPERATURA)
    //     digitalWrite(LED_TEMPERATURA, HIGH);
    // else
    //     digitalWrite(LED_TEMPERATURA, LOW);
    // if (co_ppm > MAX_PPM)
    //     digitalWrite(LED_PPM, HIGH);
    // else
    //     digitalWrite(LED_PPM, LOW);
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
        String LoRaData = "";
        while (LoRa.available()) {
            LoRaData += LoRa.readString();
        }

        int rssi = LoRa.packetRssi();
        Serial.println(LoRaData);
        parseData(LoRaData);
        temperatura = sensorValues[0];
        humedad = sensorValues[1];
        co_ppm = sensorValues[2];
        publishSensorData("LaboratorioIA", "Temperatura [°C]", temperatura);
        publishSensorData("LaboratorioIA", "Humedad [%]", humedad);
        publishSensorData("LaboratorioIA", "CO_ppm", co_ppm);
    }
}

void parseData(String data) {
    int start = 0;
    int sensorIndex = 0;

    while (start < data.length() && sensorIndex < numSensores) {
        int commaIndex = data.indexOf(',', start);
        if (commaIndex == -1) {
            commaIndex = data.length();
        }
        sensorValues[sensorIndex] = data.substring(start, commaIndex).toFloat();
        start = commaIndex + 1;
        sensorIndex++;
    }

    Serial.println("Valores de los sensores parseados:");
    for (int i = 0; i < sensorIndex; i++) {
        Serial.print("Sensor ");
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.println(sensorValues[i]);
    }
}

void setupWiFi() {
    delay(10);
    Serial.println();
    Serial.print("Conectando a ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi conectado");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void reconnectMQTT() {
    // Loop hasta que esté conectado a MQTT
    while (!client.connected()) {
        Serial.print("Intentando conexión MQTT...");
        if (client.connect("LoRaClient", mqttUser, mqttPassword)) {
            Serial.println("conectado");
        } else {
            Serial.print("error, rc=");
            Serial.print(client.state());
            Serial.println(" intentando de nuevo en 5 segundos");
            delay(5000);
        }
    }
}

void publishSensorData(const char * baseTopic, const char * dataName, float dataValue) {
    // Crear el nombre completo del tópico combinando baseTopic y dataName
    String topic = String(baseTopic) + "/" + String(dataName);
    String payload = String(dataValue);

    Serial.println("Publicando en MQTT: " + topic + " -> " + payload);

    // Publicar el dato en el tópico MQTT especificado
    client.publish(topic.c_str(), payload.c_str());
}
/* === End of documentation ==================================================================== */

/** @} End of module definition for doxygen */