#include "Thermistor.h"

int Thermistor::begin(
            float Vin, uint32_t tension_resistor_value, uint8_t read_pin, 
            char coeficient,
            uint32_t thermistor_resistence, 
            uint32_t r1, uint32_t r2
        ) 
{
    if (r2 == 0 || r1 == 0) return -1; 
    
    if (toupper(coeficient) != 'N' && toupper(coeficient) != 'P') return -3;

    _beta = (log((float)r1 / r2)) / ((1.0 / T1) - (1.0 / T2));
    if ( isnan(_beta) || _beta == 0 ) return -2;

    _Vin =  Vin;
    _read_pin = read_pin;
    _coeficient = toupper(coeficient);;

    _RT = tension_resistor_value;
    _R0 = thermistor_resistence;
    
    _is_beginned = true;
    return 0;
}

float Thermistor::read_kelvin()
{
    if (!_is_beginned) return -1;

    int adc = analogRead(_read_pin);
    float vOut = 0.0;
    
     #if defined(ESP32)
        // ESP32: 12 bits (0-4095)
        if (adc <= 0 || adc >= 4095) return -2.0;
        vOut = adc * (_Vin / 4095.0);
        
    #elif defined(ESP8266)
        // ESP8266: 10 bits (0-1023)
        if (adc <= 0 || adc >= 1023) return -2.0;
        vOut = adc * (_Vin / 1023.0); 
        
    #else
        // Default Arduino 
        if (adc <= 0 || adc >= 1023) return -2.0;
        vOut = adc * (_Vin / 1023.0);
    #endif

    float r_thermistor = _RT / ((_Vin / vOut) - 1.0);
    if (r_thermistor <= 0) return -3;

    float steinhart;
    steinhart = r_thermistor / _R0;       
    steinhart = log(steinhart);         
    steinhart /= _beta;                  
    steinhart += 1.0 / T0;              
    steinhart = 1.0 / steinhart;
    
    return steinhart;
    
}

float Thermistor::read_celcius()
{
    float kelvin = read_kelvin();
    if (kelvin == 0.0) return -273.15; 

    return kelvin - 273.15;
}

float Thermistor::read_farenheit()
{
    float celcius = read_celcius();
    if (celcius == -273.15) return -459.67;
    
    return (celcius * 9.0 / 5.0) + 32.0;
}

void Thermistor::debug()
{
    Thermistor t_debug;
    int res {0};
    const char* message = "";

    Serial.println("\n--- THERMISTOR DEBUG ---\n\n");

    Serial.printf("Pin: %d | Raw ADC: %d\n", _read_pin, analogRead(_read_pin));
    Serial.printf("Fixed Resistor (RT): %u Ohms\n", _RT);
    Serial.printf("Thermistor R0: %u Ohms\n", _R0);
    Serial.printf("Calculated Beta: %.4f\n", _beta);
    Serial.printf("--------------------------------\n\n");


    res = t_debug.begin(5.0, 10000, A0, 'N', 100000, 35563, 549);
    message = (res == 0) ? "EXPECTED" : "UNEXPECTED";
    Serial.print("Test 1 - Correct params\n-> Expected return: 0\n");
    Serial.printf("-> Returned: %d | %s\n\n", res, message);
    
    res = t_debug.begin(5.0, 10000, A0, 'N', 100000, 35563, 0);
    message = (res == -2) ? "EXPECTED" : "UNEXPECTED";
    Serial.print("Test 2 (r2 = 0)\n-> Expected return: -1\n");
    Serial.printf("-> Returned: %d | %s\n\n", res, message);

    res = t_debug.begin(5.0, 10000, A0, 'P', 100000, 35563, 549);
    message = (res == 0) ? "EXPECTED" : "UNEXPECTED";
    Serial.print("Test 3 ('P' coeficient)\n-> Expected return: 0\n");
    Serial.printf("-> Returned: %d | %s\n\n", res, message);
}
