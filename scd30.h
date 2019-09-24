// Andrew Bennett, 2019-09-22

// ADT, holds onto state required for the sensor.
typedef struct SCD30 *SCD30;

// Set up stuff, idk
SCD30 scd30_init(int bus, unsigned int interval, unsigned int pressure_offset);

// Temperature in degrees Celsius.
double scd30_get_temperature(SCD30);

// Humidity in %RH.
double scd30_get_humidity(SCD30);

// CO2 in ppm.
double scd30_get_co2(SCD30);
