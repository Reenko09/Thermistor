#include <Thermistor.h>

#ifdef ESP32
  #define THERMISTOR_PIN 34   // ADC1 pin
#else
  #define THERMISTOR_PIN A0
#endif

Thermistor ntc;

void setup()
{
  Serial.begin(115'200);

  // Beta from two datasheet points (R at 0 °C and at 100 °C)...
  float beta = Thermistor::betaFromPoints(325'550, 0, 6'730, 100);
  // ...or use the datasheet B value directly, e.g. 3950.0f

  // pin, series resistor [ohm], R at 25 °C [ohm], beta
  // * negative beta for PTC thermistors
  if (!ntc.begin(THERMISTOR_PIN, 100'000, 100'000, beta))
  {
    Serial.println("Invalid thermistor configuration");
    while (true) delay(1'000);
  }

  ntc.setSamples(16);  // average 16 ADC readings
}

void loop()
{
  float c = ntc.readCelsius();

  if (isnan(c)){
    Serial.print("Read error: ");
    Serial.println((int)ntc.lastError());
  }
  else
  {
    Serial.print(c);
    Serial.println(" C");
  }

  delay(1000);
}
