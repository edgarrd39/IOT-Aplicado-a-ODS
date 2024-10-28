#include "esp_adc_cal.h"
#include <Arduino.h>
#include <MQUnifiedsensor.h>

#define Board              ("ESP-32")
#define Pin                (34)
#define Type               ("MQ-2") // MQ2
#define Voltage_Resolution (3.3)
#define ADC_Bit_Resolution (12)
#define RatioMQ2CleanAir   (9.83)
// Potentiometer is connected to GPIO 34 (Analog ADC1_CH6)
#define PIN_ADC 34

/**
 * @brief Funcion que calcula la tension en el pin 34 a partir del ADC
 *
 * @return float Valor de tension en V
 */
float CalcularTension(void);

void MqGasInit(MQUnifiedsensor & sensor, int regression_method, float a, float b, float rl);

void MqGasCalibrate(MQUnifiedsensor & sensor);