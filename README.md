Thermistor

Simple NTC / PTC thermistor reader for Arduino, ESP32 and ESP8266, using the Beta equation. Works with PlatformIO and the Arduino framework.

Temperature in Kelvin, Celsius and Fahrenheit
NTC and PTC (PTC = negative beta)
Beta from the datasheet, or calculated from two resistance/temperature points
ADC averaging and calibration offset
Errors returned as NAN with a lastError() reason
Hardware-free helper functions, easy to unit test
Installation

PlatformIO (platformio.ini):

ini
lib_deps =
    https://github.com/Reenko09/Thermistor.git

Wiring
Default wiring: series resistor on the supply side, thermistor to GND.

Vref ---[ series resistor ]---+--- ADC pin
                              |
                        [ thermistor ]
                              |
                             GND

If your thermistor is on the supply side instead, call ntc.setWiring(Thermistor::Wiring::ThermistorToSupply);.

The calculation is ratiometric (adc / adcMax), so the divider must be supplied by the same voltage the ADC uses as full scale.

Quick start |||||||||||||||||||||||||
cpp

#include <Thermistor.h>

Thermistor ntc;

void setup() {
  Serial.begin(115200);

  // pin, series resistor [ohm], R at 25 °C [ohm], beta [K]
  ntc.begin(A0, 10000, 100000, 3950.0f);
  ntc.setSamples(16);
}

void loop() {
  float c = ntc.readCelsius();
  if (isnan(c)) Serial.println((int)ntc.lastError());
  else          Serial.println(c);
  delay(1000);
}

|||||||||||||||||||||||||||||||||||||

See examples/BasicUsage for a complete sketch.

Finding the parameters
Parameter	Where to find it
r25	Datasheet: nominal resistance at 25 °C (e.g. 100 kΩ for a "100k" NTC)
beta	Datasheet "B25/50", "B25/85" or similar (e.g. 3950). Without it, use Thermistor::betaFromPoints(r1, tempC1, r2, tempC2) with two points from the resistance table
seriesResistor	The fixed resistor of your divider, ideally close to r25

cpp
// Example: 325550 ohm at 0 °C, 6730 ohm at 100 °C
float beta = Thermistor::betaFromPoints(325550, 0, 6730, 100);

API Setup

Method	Description
bool begin(pin, seriesResistor, r25, beta, adcMax)	Configures the sensor. adcMax is the ADC full-scale value (default: 4095 on ESP32, 1023 elsewhere). Returns false on invalid parameters
void setWiring(Wiring)	ThermistorToGround (default) or ThermistorToSupply
void setSamples(uint8_t n)	Averages n ADC readings per measurement (default 1)
void setOffsetCelsius(float)	Calibration offset added to the result, if needed

Reading Method Returns
float readCelsius()	Temperature in °C
float readKelvin()	Temperature in K
float readFahrenheit()	Temperature in °F
float readResistance()	Thermistor resistance in ohms
Error lastError()	Reason for the last NAN
void printDebug(Print&)	Prints configuration and a fresh reading, e.g. ntc.printDebug(Serial)

All read*() methods return NAN on error.

Error codes
lastError()	Value	Meaning
Error::None	0	OK
Error::NotInitialized	-1	begin() was not called or failed
Error::InvalidParam	-2	Invalid value passed to begin()
Error::AdcOutOfRange	-3	ADC at 0 or full scale (open circuit or short)
Error::InvalidResistance	-4	Resistance could not be converted to a temperature


Static helpers (no hardware access)

static float betaFromPoints(float r1, float tempC1, float r2, float tempC2);
static float resistanceToKelvin(float resistance, float r25, float beta);
static float adcToResistance(float adc, uint16_t adcMax, uint32_t seriesResistor,
                             Wiring wiring = Wiring::ThermistorToGround);

How it works

The resistance is computed from the divider:

R = Rseries * ratio / (1 - ratio),   ratio = adc / adcMax

and the temperature from the Beta equation:

1/T = 1/T25 + ln(R / R25) / beta        (T25 = 298.15 K)

The Beta model is an approximation, accurate near the temperatures used to derive beta. For a wide range, prefer a beta calculated from points close to your operating range.

Platform notes
ESP32: the ADC is non-linear, especially near 0 V and 3.3 V. Use ADC1 pins (ADC2 does not work while Wi-Fi is on) and consider setSamples().
ESP8266: the bare chip ADC reads 0–1 V only. Boards like the NodeMCU add an internal divider (~0–3.2 V), but this library assumes the divider supply equals the ADC full scale, so results may need an offset or an adapted circuit.
Other boards: if you changed the ADC resolution (analogReadResolution()), pass the matching adcMax to begin().
Contributing

Issues and pull requests are welcome.

License

MIT