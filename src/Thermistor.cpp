#include "Thermistor.h"

namespace
{
  constexpr float KELVIN_OFFSET = 273.15f;
  constexpr float T25_KELVIN    = 298.15f;  // 25 °C
}

bool Thermistor::begin(uint8_t pin, uint32_t seriesResistor, uint32_t r25, float beta,
                       uint16_t adcMax)
{
  _initialized = false;

  if (seriesResistor == 0 || r25 == 0 || adcMax == 0 || isnan(beta) || beta == 0.0f)
  {
    _error = Error::InvalidParam;
    return false;
  }

  _pin     = pin;
  _seriesR = seriesResistor;
  _r25     = r25;
  _beta    = beta;
  _adcMax  = adcMax;

  _initialized = true;
  _error = Error::None;
  return true;
}

float Thermistor::readResistance()
{
  if (!_initialized)
  {
    _error = Error::NotInitialized;
    return NAN;
  }

  uint32_t sum = 0;
  for (uint8_t i = 0; i < _samples; i++)
    sum += analogRead(_pin);

  _lastAdc = (float)sum / _samples;

  float r = adcToResistance(_lastAdc, _adcMax, _seriesR, _wiring);
  if (isnan(r))
  {
    _error = Error::AdcOutOfRange;
    return NAN;
  }

  _error = Error::None;
  return r;
}

float Thermistor::readKelvin()
{
  float r = readResistance();
  if (isnan(r)) return NAN;

  float k = resistanceToKelvin(r, (float)_r25, _beta);
  if (isnan(k))
  {
    _error = Error::InvalidResistance;
    return NAN;
  }

  return k + _offset;
}

float Thermistor::readCelsius()
{
  float k = readKelvin();
  if (isnan(k)) return NAN;

  return k - KELVIN_OFFSET;
}

float Thermistor::readFahrenheit()
{
  float c = readCelsius();
  if (isnan(c)) return NAN;

  return c * 9.0f / 5.0f + 32.0f;
}

void Thermistor::printDebug(Print &out)
{
  out.println(F("--- THERMISTOR DEBUG ---"));

  if (!_initialized)
  {
    out.println(F("Not initialized (begin() failed or was not called)"));
    return;
  }

  float c = readCelsius();  // fresh reading, also updates _lastAdc and _error

  out.print(F("Pin: "));                 out.println(_pin);
  out.print(F("Series resistor [ohm]: ")); out.println(_seriesR);
  out.print(F("R25 [ohm]: "));           out.println(_r25);
  out.print(F("Beta: "));                out.println(_beta, 4);
  out.print(F("ADC max: "));             out.println(_adcMax);
  out.print(F("Samples: "));             out.println(_samples);
  out.print(F("Offset [C]: "));          out.println(_offset, 2);
  out.print(F("Wiring: "));
  out.println(_wiring == Wiring::ThermistorToGround ? F("thermistor to GND")
                                                    : F("thermistor to supply"));
  out.print(F("Raw ADC (avg): "));       out.println(_lastAdc, 1);
  out.print(F("Temperature [C]: "));     out.println(c, 2);
  out.print(F("Last error: "));          out.println((int)_error);
}

// ---------------------------------------------------------------------------
// Pure helpers
// ---------------------------------------------------------------------------

float Thermistor::betaFromPoints(float r1, float tempC1, float r2, float tempC2)
{
  float t1 = tempC1 + KELVIN_OFFSET;
  float t2 = tempC2 + KELVIN_OFFSET;

  if (r1 <= 0.0f || r2 <= 0.0f || t1 <= 0.0f || t2 <= 0.0f || t1 == t2)
    return NAN;

  float beta = logf(r1 / r2) / ((1.0f / t1) - (1.0f / t2));
  if (isnan(beta) || beta == 0.0f) return NAN;

  return beta;
}

float Thermistor::resistanceToKelvin(float resistance, float r25, float beta)
{
  if (resistance <= 0.0f || r25 <= 0.0f || isnan(beta) || beta == 0.0f)
    return NAN;

  float invT = (1.0f / T25_KELVIN) + logf(resistance / r25) / beta;
  if (isnan(invT) || invT <= 0.0f) return NAN;

  return 1.0f / invT;
}

float Thermistor::adcToResistance(float adc, uint16_t adcMax, uint32_t seriesResistor,
                                  Wiring wiring)
{
  if (adcMax == 0 || seriesResistor == 0) return NAN;

  float ratio = adc / (float)adcMax;
  if (ratio <= 0.0f || ratio >= 1.0f) return NAN;

  if (wiring == Wiring::ThermistorToGround)
    return (float)seriesResistor * ratio / (1.0f - ratio);

  return (float)seriesResistor * (1.0f - ratio) / ratio;
}