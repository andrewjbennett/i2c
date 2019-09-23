// Andrew Bennett, 2019-09-22

// ADT, holds onto state required for the sensor.
typedef struct SCD30 *SCD30;

// Set up stuff, idk
SCD30 init_scd30(unsigned int interval, unsigned int pressure_offset);

// Temperature in degrees Celsius.
double get_temperature(SCD30);

// Humidity in (unit??)
double get_humidity(SCD30);

// CO2 in (unit)
double get_co2(SCD30);
