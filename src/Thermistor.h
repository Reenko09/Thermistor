    #ifndef THERMISTOR_H
    #define THERMISTOR_H

    #include <Arduino.h>
    #include <math.h>

    #define T0 298.15 // [K]  25°C default datasheet value
    #define T1 273.15 // [K]   0°C default datasheet value
    #define T2 373.15 // [K] 100°C default datasheet value

    class Thermistor
    {
    private:
        boolean _is_beginned {false};

        uint8_t _read_pin {255};

        char _coeficient {'N'}; // 'N' or 'P'
        float _Vin {5.0};     // V input
        uint32_t _RT;     // Tension divisor resistor
        uint32_t _R0;     // Thermistor initial resistence at T0 (25°C)

        float _beta{0};

    public:
        Thermistor() = default;

        int begin(
            float Vin, uint32_t tension_resistor_value, uint8_t read_pin, 
            char coeficient = 'N',
            uint32_t thermistor_resistence = 100000, 
            uint32_t r1 = 325550, uint32_t r2 = 6730
        );

        double read_celcius();

        double read_kelvin();

        double read_farenheit();

        void debug();
    };

    #endif