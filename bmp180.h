// Andrew Bennett, 2019-09-22

typedef struct BMP180 *BMP180;

// Set up stuff, idk
BMP180 bmp180_init(int bus);

// Temperature in degrees Celsius.
double bmp180_get_temperature(BMP180);

// Pressure in hPa.
double bmp180_get_pressure(BMP180);
