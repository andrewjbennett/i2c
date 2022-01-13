// Andrew Bennett, 2019-10-08
// Manually parse data from the scd30 sensor.
// (for debugging what's going on)

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "scd30.h"
#include "i2c.h"

void parse_measurement(SCD30 s, unsigned char p[20]);
SCD30 scd30_new(I2C i2c);



// copying the struct here because #yolo #adts
struct SCD30 {
	void * i2c_info; // y o l o

	float co2;
	float humidity;
	float temperature;

	int read_co2;
	int read_humidity;
	int read_temperature;

};

/*

   Data pulled from:

sudo pigpiod			# start pigpio daemon
pigs i2co 1 0x61 0		# open connection to addr 0x61 (scd30)
pigs i2cwd 0 0x00 0x10		# set continuous measurement
pigs i2cwd 0 0x02 0x02		# get data ready status
pigs i2crd 0 3			# check data is ready
pigs i2cwd 0 0x03 0x00		# read measurement
pigs i2crd 0 18			# actually get measurement

*/

unsigned char crc(unsigned char *buf, int size) {
    unsigned char crc = 0xff;
    for (int i = 0; i < size; i++) {
        crc ^= buf[i];
        if ((crc & 0x80) != 0) {
            crc = ((crc << 1) ^ 0x31) & 0xff
        } else {
            crc <<= 1;
        }
    }
    return crc;
}

int main(int argc, char *argv[]) {

	if (argc > 1 && strcmp(argv[1], "config") == 0) {
		printf("multigraph temp\n");
		printf("graph_title bedroom: temp\n");
		printf("temp.label temp\n");
		printf("multigraph humidity\n");
		printf("graph_title bedroom: humidity\n");
		printf("humidity.label humidity\n");
		printf("multigraph co2\n");
		printf("graph_title bedroom: co2\n");
		printf("co2.label co2\n");
		return 0;
	}

	system("pigs i2cwd 0 0x00 0x10 > /dev/null");
	system("pigs i2cwd 0 0x02 0x02 > /dev/null");
	system("pigs i2crd 0 3 > /dev/null");
	system("pigs i2cwd 0 0x03 0x00 > /dev/null");
	system("pigs i2crd 0 18 > /tmp/output.tmp");

	FILE* fin = fopen("/tmp/output.tmp", "r");
	/*
/home/pi/parse_manual_argv $(pigs i2crd 0 18) | tr "," "\n" | tr -d " "  | sed "s/scd30_CO2:/\nmultigraph co2\nco2.value /;s/scd30_temp:/\nmultigraph temp\ntemp.value /;s/scd30_humid:/\nmultigraph humidity\nhumidity.value /"

exit 0
*/


	// data pulled from
	//
	unsigned char buf[20] = {0}; // = {67, 222, 62, 44, 121, 153, 65, 168, 185, 59, 216, 148, 66, 90, 116, 129, 128, 44};

	// throw this one away
	int tmp = 0;
	fscanf(fin, "%d", &tmp);

	for (int i = 1; i < 18; i++) {
		int tmp = 0;
		fscanf(fin, "%d", &tmp);
		buf[i-1] = tmp;
		//fscanf(fin, "%d", &buf[i]);
	}

	/*
	for (int i = 0; i < 20; i++) {
		printf("%d ", buf[i]);
	}

	printf("\n");
	*/
	

	// should be fine to create this with a null pointer, because
	// we don't ever use the i2c stuff.... I hope.

	SCD30 s = scd30_new(NULL);

	parse_measurement(s, buf);

	//printf("scd30_CO2: %.0f, scd30_temp: %.1f, scd30_humid: %.1f\n",
	//		s->co2, s->temperature, s->humidity);

	printf("multigraph temp\ntemp.value %.1f\n\n", s->temperature);
	printf("multigraph humidity\nhumidity.value %.1f\n\n", s->humidity);
	printf("multigraph co2\nco2.value %.1f\n\n", s->co2);
}
