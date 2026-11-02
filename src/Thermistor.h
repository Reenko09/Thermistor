#ifndef THERMISTOR_H
#define THERMISTOR_H

#include <Arduino.h>
#include <math.h>

// Default ADC full-scale value (2^bits - 1). Override with a build flag
// (-DTHERMISTOR_DEFAULT_ADC_MAX=...) or pass `adcMax` to begin().
#ifndef THERMISTOR_DEFAULT_ADC_MAX
  #if defined(ESP32)
    #define THERMISTOR_DEFAULT_ADC_MAX 4095  // 12 bits
  #else
    #define THERMISTOR_DEFAULT_ADC_MAX 1023  // 10 bits (AVR, ESP8266, ...)
  #endif
#endif

/**
 * NTC/PTC thermistor reader based on the Beta equation:
 *
 *   1/T = 1/T25 + ln(R / R25) / beta
 *
 * Works for both NTC and PTC: for a PTC the beta is simply negative
 * (betaFromPoints() already returns the right sign).
 *
 * Voltage divider (default wiring):
 *
 *   Vref ---[ series resistor ]---+--- ADC pin
 *                                 |
 *                           [ thermistor ]
 *                                 |
 *                                GND
 *
 * Use setWiring(Wiring::ThermistorToSupply) if the thermistor is on the
 * Vref side instead.
 *
 * NOTE: the calculation is ratiometric (adc / adcMax), so the divider must be
 * supplied by the same voltage the ADC uses as full scale. This does NOT hold
 * for the bare ESP8266 ADC (0-1 V range), for example.
 */
class Thermistor
{
public:
  enum class Error : int8_t
  {
    None              =  0,
    NotInitialized    = -1,  // begin() was not called (or failed)
    InvalidParam      = -2,  // bad value passed to begin()
    AdcOutOfRange     = -3,  // ADC reading at 0 or at full scale (open/short circuit)
    InvalidResistance = -4   // resistance cannot be converted to a temperature
  };

  enum class Wiring : uint8_t
  {
    ThermistorToGround,  // series resistor on Vref, thermistor on GND (default)
    ThermistorToSupply   // thermistor on Vref, series resistor on GND
  };

  Thermistor() = default;

  /**
   * @param pin              analog input pin
   * @param seriesResistor   divider resistor value [ohm]
   * @param r25              thermistor resistance at 25 °C [ohm]
   * @param beta             Beta constant [K] (negative for PTC)
   * @param adcMax           ADC full-scale value (2^bits - 1)
   * @return true on success; on failure see lastError()
   */
  bool begin(uint8_t pin, uint32_t seriesResistor, uint32_t r25, float beta,
             uint16_t adcMax = THERMISTOR_DEFAULT_ADC_MAX);

  void setWiring(Wiring wiring)  { _wiring = wiring; }
  void setSamples(uint8_t n)     { _samples = (n == 0) ? 1 : n; }  // averaged readings
  void setOffsetCelsius(float c) { _offset = c; }                   // calibration offset

  // All read*() return NAN on error; check lastError() for the reason.
  float readResistance();  // [ohm]
  float readKelvin();
  float readCelsius();
  float readFahrenheit();

  Error lastError() const { return _error; }

  // Prints configuration and a fresh reading (works on any Arduino core).
  void printDebug(Print &out);

  // ---- Pure helpers (no hardware access, easy to unit test) ----

  /** Beta from two points (R1 at tempC1, R2 at tempC2). NAN if invalid. */
  static float betaFromPoints(float r1, float tempC1, float r2, float tempC2);

  /** Temperature [K] from resistance. NAN if invalid. */
  static float resistanceToKelvin(float resistance, float r25, float beta);

  /** Thermistor resistance [ohm] from an ADC value. NAN if adc is at 0 or full scale. */
  static float adcToResistance(float adc, uint16_t adcMax, uint32_t seriesResistor,
                               Wiring wiring = Wiring::ThermistorToGround);

private:
  bool     _initialized {false};
  uint8_t  _pin         {255};
  uint16_t _adcMax      {THERMISTOR_DEFAULT_ADC_MAX};
  uint8_t  _samples     {1};
  Wiring   _wiring      {Wiring::ThermistorToGround};
  Error    _error       {Error::NotInitialized};

  uint32_t _seriesR {0};  // divider resistor [ohm]
  uint32_t _r25     {0};  // thermistor resistance at 25 °C [ohm]
  float    _beta    {0.0f};
  float    _offset  {0.0f};
  float    _lastAdc {0.0f};
};

#endif  // THERMISTOR_H