#include "sensores.h"
#include <Arduino.h>
#include <MQUnifiedsensor.h>



int potValue = 0;
float Voltage = 0.0;

uint32_t readADC_Cal(int ADC_Raw) {
    esp_adc_cal_characteristics_t adc_chars;

    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
    return (esp_adc_cal_raw_to_voltage(ADC_Raw, &adc_chars));
}

float CalcularTension(void) {
    potValue = analogRead(PIN_ADC);
    Voltage = readADC_Cal(potValue);
    Serial.println(Voltage / 1000.0);
    return Voltage / 1000;
}

/**
 * @brief  Funcion que inicializa el sensor de gas con las configuraciones necesarias
 *
 * @param sensor
 * @param regression_method
 * @param a
 * @param b
 * @param rl
 */
void MqGasInit(MQUnifiedsensor & sensor, int regression_method, float a, float b, float rl) {
    sensor.setRegressionMethod(regression_method); //_PPM =  a*ratio^b
    sensor.setA(a);
    sensor.setB(b);
    sensor.init();
    sensor.setRL(rl);
}

void MqGasCalibrate(MQUnifiedsensor & sensor) {
    Serial.print("Calibrating please wait.");
    float calcR0 = 0;
    for (int i = 1; i <= 10; i++) {
        sensor.update(); // Update data, the arduino will read the voltage from the analog pin
        calcR0 += sensor.calibrate(RatioMQ2CleanAir);
        Serial.print(".");
    }
    sensor.setR0(calcR0 / 10);
    Serial.println("  done!.");

    if (isinf(calcR0)) {
        Serial.println("Warning: Conection issue, R0 is infinite (Open circuit detected) please "
                       "check your wiring and supply");
        while (1)
            ;
    }
    if (calcR0 == 0) {
        Serial.println("Warning: Conection issue found, R0 is zero (Analog pin shorts to ground) "
                       "please check your wiring and supply");
        while (1)
            ;
    }
}