#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <wiringPi.h> 

#include <fcntl.h>
#include <getopt.h>
#include <linux/spi/spidev.h>
#include <linux/types.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define IN 1
#define OUT 0
#define LOW 0
#define HIGH 1

//#define PIN_LIGHT 20
#define PIN_DHT 2
//#define POUT_LIGHT 21

#define VALUE_MAX 256
#define BUFFER_MAX 3
#define DIRECTION_MAX 45
#define TIMINGS_MAX	85

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

//DHT
int data[5] = { 0, 0, 0, 0, 0 };

double discomfort_index_function(double humi, double temp) {
    return ((temp * 9.0 / 5.0) + 32.0 - 0.55 * (1.0 - humi / 100) * ((9.0 / 5.0 * temp) - 26.0));
}

void readDHT() {
    uint8_t prev_status = HIGH;
    uint8_t counter = 0;
    uint8_t j = 0, i;
    char str_humi[6];
    char str_temp[6];
    char* tmp_data;
    double doub_humi, doub_temp;
    double discomfort_index;

    data[0] = data[1] = data[2] = data[3] = data[4] = 0;

    pinMode(PIN_DHT, OUTPUT);
    digitalWrite(PIN_DHT, LOW);
    delay(18);

    pinMode(PIN_DHT, INPUT);

    for (i = 0; i < TIMINGS_MAX; i++) {
        counter = 0;
        while (digitalRead(PIN_DHT) == prev_status) {
            counter++;
            delayMicroseconds(1);
            if (counter == 255) {
                break;
            }
        }
        prev_status = digitalRead(PIN_DHT);

        if (counter == 255) {
            break;
        }

        if ((i >= 4) && (i % 2 == 0)) {
            data[j / 8] <<= 1;
            if (counter > 50) {
                data[j / 8] |= 1;
            }
            j++;
        }
    }

    if ((j >= 40) && (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF))) {
        if ((data[0] + data[1] + data[2] + data[3]) != 0) {
            printf("Humidity : %d.%d %%, Temperature : %d.%d C\n", data[0], data[1], data[2], data[3]);
            sprintf(str_humi, "%d.%d", data[0], data[1]);
            printf("str_humi : %s, ", str_humi);
            //str_humi = strcat(tmp_data, "");
            //while(getchar()=='\n');
            sprintf(str_temp, "%d.%d", data[2], data[3]);
            printf("str_temp : %s\n", str_temp);
            //str_temp = strcat(tmp_data, "");
            //while(getchar()=='\n');


            doub_humi = atof(str_humi);
            doub_temp = atof(str_temp);
            discomfort_index = discomfort_index_function(doub_humi, doub_temp);
            printf("discomfort index : %.2lf\n", discomfort_index);
            if (discomfort_index > 75) {
                printf("it's discomfort.");
            }
        }

    }
    else {
        printf("skip\n");
    }
}

void* start_dht() {
    readDHT();
    delay(2000);
}

//LIGHT
static const char* DEVICE = "/dev/spidev0.0";
static uint8_t MODE = 0;
static uint8_t BITS = 8;
static uint32_t CLOCK = 1000000;
static uint16_t DELAY = 5;

static int prepare(int fd) {
    if (ioctl(fd, SPI_IOC_WR_MODE, &MODE) == -1) {
        perror("Can't set MODE");
        return -1;
    }
    if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &BITS) == -1) {
        perror("Can't set number of BITS");
        return -1;
    }
    if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &CLOCK) == -1) {
        perror("Can't set write CLOCK");
        return -1;
    }
    if (ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &CLOCK) == -1) {
        perror("Can't set read CLOCK");
        return -1;
    }
    return 0;
}

uint8_t control_bits_differential(uint8_t channel) {
    return (channel & 7) << 4;
}

uint8_t control_bits(uint8_t channel) {
    return 0x8 | control_bits_differential(channel);
}

int readadc(int fd, uint8_t channel) {
    uint8_t tx[] = { 1, control_bits(channel), 0 };
    uint8_t rx[3];
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = ARRAY_SIZE(tx) ,
        .delay_usecs = DELAY,
        .speed_hz = CLOCK,
        .bits_per_word = BITS,
    };
    if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) == 1) {
        perror("IO Error");
        abort();
    }
    return ((rx[1] << 8) & 0x300) | (rx[2] & 0xFF);
}

void* start_light(void* fd) {
    int light = 0;
    int main_fd = (int)fd;
    light = readadc(main_fd, 0);
    if (light < 600) {
        printf("Illuminance: %d -> Dark\n", light);
    }
    sleep(1);

}

int main() {
    pthread_t p_thread[2];
    int thr_id;
    int status;
    char p1[] = "thread_1";
    char p2[] = "thread_2";
    char pM[] = "thread_m";
    if (wiringPiSetupGpio() == -1) //for DHT
        return -1;

    int fd = open(DEVICE, O_RDWR); //for Light
    if (fd <= 0) {
        perror("Device open error");
        return -1;
    }
    if (prepare(fd) == -1) {
        perror("Device prepare error");
        return -1;
    }

    for (int i = 0; i < 10; i++) {
        thr_id = pthread_create(&p_thread[0], NULL, start_dht, NULL);
        if (thr_id < 0) {
            perror("thread create error : ");
            exit(0);
        }
        thr_id = pthread_create(&p_thread[1], NULL, start_light, (void*)fd);
        if (thr_id < 0) {
            perror("thread create error : ");
            exit(0);
        }

        //    t_function((void *)pM);
        pthread_join(p_thread[0], (void**)&status);
        pthread_join(p_thread[1], (void**)&status);
    }


    return 0;
}