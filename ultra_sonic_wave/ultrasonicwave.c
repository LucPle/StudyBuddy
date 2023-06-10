#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_MAX 3
#define DIRECTION_MAX 45

#define IN  0
#define OUT 1
#define PWM 0

#define LOW  0
#define HIGH 1

#define VALUE_MAX 256

#define POUT 22
#define PIN 23

#define POUT2 9
#define PIN2 10

#define POUT3 17
#define PIN3 18

#define POUT4 14
#define PIN4 15

#define FSRPIN 4

double frontDist = 0;
double backDist = 0;
double leftDist = 0;
double rightDist = 0;

int prev_input = 0;
int input = 0;

char msg[2];
int state=0;

int str_len = 0;



void error_handling(char *message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

static int GPIOExport(int pin) {
  #define BUFFER_MAX 3
  char buffer[BUFFER_MAX];
  ssize_t bytes_written;
  int fd;

  fd = open("/sys/class/gpio/export", O_WRONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open export for writing!\n");
    return (-1);
  }

  bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
  write(fd, buffer, bytes_written);
  close(fd);
  return (0);
}

static int GPIOUnexport(int pin) {
  char buffer[BUFFER_MAX];
  ssize_t bytes_written;
  int fd;

  fd = open("/sys/class/gpio/unexport", O_WRONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open unexport for writing!\n");
    return (-1);
  }

  bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
  write(fd, buffer, bytes_written);
  close(fd);
  return (0);
}

static int GPIODirection(int pin, int dir) {
  static const char s_directions_str[] = "in\0out";

  //#define DIRECTION_MAX 35
  char path[DIRECTION_MAX];
  int fd;

  snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
  fd = open(path, O_WRONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open gpio direction for writing!\n");
    return (-1);
  }

  if (-1 == write(fd, &s_directions_str[IN == dir ? 0 : 3], IN == dir ? 2 : 3)) {
    fprintf(stderr, "Failed to set direction!\n");
    return (-1);
  }

  close(fd);
  return (0);
}

static int GPIORead(int pin) {
  char path[VALUE_MAX];
  char value_str[3];
  int fd;

  snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
  fd = open(path, O_RDONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open gpio value for reading!\n");
    return (-1);
  }

  if (-1 == read(fd, value_str, 3)) {
    fprintf(stderr, "Failed to read value!\n");
    return (-1);
  }

  close(fd);

  return (atoi(value_str));
}

static int GPIOWrite(int pin, int value) {
  static const char s_values_str[] = "01";

  char path[VALUE_MAX];
  int fd;

  snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
  fd = open(path, O_WRONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open gpio value for writing!\n");
    return (-1);
  }

  if (1 != write(fd, &s_values_str[LOW == value ? 0 : 1], 1)) {
    fprintf(stderr, "Failed to write value!\n");
    return (-1);
  }

  close(fd);
  return (0);
}

void *frontwave_thd() {
    clock_t start_t, end_t;
    double time;

    if (-1 == GPIOExport(POUT) || -1 == GPIOExport(PIN)) {
        printf("gpio export err1\n");
        exit(0);
    }
    usleep(100000);

    if (-1 == GPIODirection(POUT, OUT) || -1 == GPIODirection(PIN, IN)) {
        printf("gpiodirection err1\n");
        exit(0);
    }

    GPIOWrite(POUT, 0);
    usleep(100000);

    while (1) {
        if (-1 == GPIOWrite(POUT, 1)) {
            printf("gpio write/trigger err\n");
            exit(0);
        }
        usleep(10);
        GPIOWrite(POUT, 0);

        while (GPIORead(PIN) == 0) {
            start_t = clock();
        }
        while (GPIORead(PIN) == 1) {
            end_t = clock();
        }
        time = (double)(end_t-start_t)/CLOCKS_PER_SEC;
        frontDist = time/2*34000;

        if (frontDist > 900) {
            frontDist = 900;
        }

        //printf("time : %.4lf\n", time);
        //printf("front : %.2lfcm\n", frontDist);

        usleep(500000);
    }
}

void *rightwave_thd() {
    clock_t start_t, end_t;
    double time;

    if (-1 == GPIOExport(POUT2) || -1 == GPIOExport(PIN2)) {
        printf("gpio export err\2n");
        exit(0);
    }
    usleep(100000);

    if (-1 == GPIODirection(POUT2, OUT) || -1 == GPIODirection(PIN2, IN)) {
        printf("gpiodirection err2\n");
        exit(0);
    }

    GPIOWrite(POUT2, 0);
    usleep(100000);

    while (1) {
        if (-1 == GPIOWrite(POUT2, 1)) {
            printf("gpio write/trigger err\n");
            exit(0);
        }
        usleep(10);
        GPIOWrite(POUT2, 0);

        while (GPIORead(PIN2) == 0) {
            start_t = clock();
        }
        while (GPIORead(PIN2) == 1) {
            end_t = clock();
        }
        time = (double)(end_t-start_t)/CLOCKS_PER_SEC;
        rightDist = time/2*34000;

        if (rightDist > 900) {
            rightDist = 900;
        }

        //printf("right : %.2lfcm\n", rightDist);
        

        usleep(500000);
    }
}

void *backwave_thd() {
    clock_t start_t, end_t;
    double time;

    if (-1 == GPIOExport(POUT3) || -1 == GPIOExport(PIN3)) {
        printf("gpio export err3\n");
        exit(0);
    }
    usleep(100000);

    if (-1 == GPIODirection(POUT3, OUT) || -1 == GPIODirection(PIN3, IN)) {
        printf("gpiodirection err2\n");
        exit(0);
    }

    GPIOWrite(POUT3, 0);
    usleep(100000);

    while (1) {
        if (-1 == GPIOWrite(POUT3, 1)) {
            printf("gpio write/trigger err\n");
            exit(0);
        }
        usleep(10);
        GPIOWrite(POUT3, 0);

        while (GPIORead(PIN3) == 0) {
            start_t = clock();
        }
        while (GPIORead(PIN3) == 1) {
            end_t = clock();
        }
        time = (double)(end_t-start_t)/CLOCKS_PER_SEC;
        backDist = time/2*34000;

        if (backDist > 900) {
            backDist = 900;
        }

        //printf("back : %.2lfcm\n", backDist);
        

        usleep(500000);
    }
}


void *leftwave_thd(void*arg) {
    clock_t start_t, end_t;
    double time;

    int sock=*((int*)arg);
    if (-1 == GPIOExport(POUT4) || -1 == GPIOExport(PIN4)) {
        printf("gpio export err4\n");
        exit(0);
    }
    usleep(100000);

    if (-1 == GPIODirection(POUT4, OUT) || -1 == GPIODirection(PIN4, IN)) {
        printf("gpiodirection err4\n");
        exit(0);
    }

    GPIOWrite(POUT4, 0);
    usleep(100000);

    while (1) {
        if (-1 == GPIOWrite(POUT4, 1)) {
            printf("gpio write/trigger err\n");
            exit(0);
        }
        usleep(10);
        GPIOWrite(POUT4, 0);

        while (GPIORead(PIN4) == 0) {
            start_t = clock();
        }
        while (GPIORead(PIN4) == 1) {
            end_t = clock();
        }
        time = (double)(end_t-start_t)/CLOCKS_PER_SEC;
        leftDist = time/2*34000;

        if (leftDist > 900) {
            leftDist = 900;
        }

        //printf("left : %.2lfcm\n", leftDist);
        if(state){
            if(frontDist > 15 || abs(leftDist - rightDist) > 5 || leftDist >20 || rightDist >20){
                snprintf(msg,2, "%d", 1);
                write(sock, msg, sizeof(msg));
            }
        }
        str_len =read(sock, msg, sizeof(msg));
        if(str_len == -1)
            error_handling("read() error");
        
        printf("%s\n", msg);
        if(msg[0] == '0')state=0;

        usleep(500000);
    }
}



void *fsr_thd(void*arg){


    int sock=*((int*)arg);
    if (GPIOExport(FSRPIN) == -1) {
        printf("GPIOExport err");
        exit(0);
    }
    usleep(100000);

    if (GPIODirection(FSRPIN, 0)) {
        printf("GPIODirection err");
        exit(0);
    }

while (1) {
        input = GPIORead(FSRPIN);

        if (!prev_input && input&&!state) {
            printf("FSR value %d\n", prev_input);
            printf("ok\n");
            snprintf(msg,2, "%d", 1);
            write(sock, msg, sizeof(msg));
            state=1;
        }
        prev_input = input;

        usleep(10000); // 10ms
    }
}


int main(int argc, char *argv[]) {
    pthread_t frontWave_tid, rightWave_tid, backWave_tid, leftWave_tid, fsr_tid;
    
    int sock;
    struct sockaddr_in serv_addr;
    //char msg[2];
    char on[2] ="1";
    int str_len;
    int light = 0;

    if(argc!=3){
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }



    sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error");
    


    // ultrawave_thread를 생성하여 실행합니다
    if (pthread_create(&frontWave_tid, NULL, frontwave_thd, NULL) != 0) {
        printf("Failed to create ultrawave_thread\n");
        return -1;
    }

    if (pthread_create(&rightWave_tid, NULL, rightwave_thd, NULL) != 0) {
        printf("Failed to create hipwave_thread\n");
        return -1;
    }
    if (pthread_create(&backWave_tid, NULL, backwave_thd, NULL) != 0) {
        printf("Failed to create backwave_thread\n");
        return -1;
    }
    if (pthread_create(&leftWave_tid, NULL, leftwave_thd, &sock) != 0) {
        printf("Failed to create leftwave_thread\n");
        return -1;
    }
    
    if (pthread_create(&fsr_tid, NULL, fsr_thd, &sock) != 0) {
        printf("Failed to create fsrwave_thread\n");
        return -1;
    }
    
    pthread_join(frontWave_tid, NULL);
    pthread_join(rightWave_tid, NULL);
    pthread_join(backWave_tid, NULL);
    pthread_join(leftWave_tid, NULL);
    pthread_join(fsr_tid, NULL);



// double frontDist = 0;
// double hipDist = 0;
// double backDist = 0;
// double leftDist = 0;
// double rightDist = 0;

    

    if (-1 == GPIOUnexport(POUT) || -1 == GPIOUnexport(PIN)) {
        return -1;
    }
    if (-1 == GPIOUnexport(POUT2) || -1 == GPIOUnexport(PIN2)) {
        return -1;
    }
    if (-1 == GPIOUnexport(POUT3) || -1 == GPIOUnexport(PIN3)) {
        return -1;
    }
    if (-1 == GPIOUnexport(POUT4) || -1 == GPIOUnexport(PIN4)) {
        return -1;
    }
    if (-1 == GPIOUnexport(FSRPIN)){
        return -1;
    }

    close(sock);

    


    return 0;
}
